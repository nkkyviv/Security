#include "session.h"
#include "protocol.h"
#include <Arduino.h>
#include <mbedtls/md.h>
#include <mbedtls/rsa.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/aes.h>
#include <mbedtls/hmac_drbg.h>
#include <mbedtls/sha256.h>
#include <mbedtls/rsa.h>
#include <mbedtls/pk.h>

#define SESSION_CLOSE 0xFF

enum
{
    STATUS_OKAY,
    STATUS_ERROR,
};

constexpr int AES_SIZE{32};
constexpr int DER_SIZE{294};
constexpr int RSA_SIZE{256};
constexpr int HMAC_SIZE{32};
constexpr int MSG_SIZE{550}; // we need 256 and 294
constexpr int EXPONENT{65537};
constexpr int AES_BLOCK_SIZE{16};

static mbedtls_aes_context aes_ctx;
static mbedtls_md_context_t hmac_ctx;
static mbedtls_pk_context client_ctx;
static mbedtls_pk_context server_ctx;
static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;

static uint64_t session_id{0};
static uint8_t aes_key[AES_SIZE]{0};
static uint8_t enc_iv[AES_BLOCK_SIZE]{0};
static uint8_t dec_iv[AES_BLOCK_SIZE]{0};
static const uint8_t hmac_key[HMAC_SIZE] = {0x29, 0x49, 0xde, 0xc2, 0x3e, 0x1e, 0x34, 0xb5, 0x2d, 0x22, 0xb5,
                                            0xba, 0x4c, 0x34, 0x23, 0x3a, 0x9d, 0x3f, 0xe2, 0x97, 0x14, 0xbe,
                                            0x24, 0x62, 0x81, 0x0c, 0x86, 0xb1, 0xf6, 0x92, 0x54, 0xd6};

bool session_init(void)
{
    bool status = true;
    uint8_t initial[AES_SIZE]{0};

    protocol_init();

    // HMAC-SHA256
    mbedtls_md_init(&hmac_ctx);
    if (0 != mbedtls_md_setup(&hmac_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1))
    {
        status = false;
        goto end_init;
    }

    // AES-256
    mbedtls_aes_init(&aes_ctx);

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    for (size_t i = 0; i < sizeof(initial); i++)
    {
        initial[i] = random(0x100);
    }

    if (0 != mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func,
                                   &entropy, initial, sizeof(initial)))
    {
        status = false;
        goto end_init;
    }

    mbedtls_pk_init(&server_ctx);

    if (0 != mbedtls_pk_setup(&server_ctx, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA)))
    {
        status = false;
        goto end_init;
    }

    if (0 != mbedtls_rsa_gen_key(mbedtls_pk_rsa(server_ctx), mbedtls_ctr_drbg_random,
                                 &ctr_drbg, RSA_SIZE * CHAR_BIT, EXPONENT))
    {
        status = false;
    }

end_init:
    return status;
}

bool write_client(uint8_t *buffer, size_t size)
{
    bool status = false;

    uint8_t rec_hmac[HMAC_SIZE]{0};

    mbedtls_md_hmac_starts(&hmac_ctx, hmac_key, HMAC_SIZE);
    mbedtls_md_hmac_update(&hmac_ctx, buffer, size);
    mbedtls_md_hmac_finish(&hmac_ctx, buffer + size);

    if ((size + HMAC_SIZE) == protocol_send(buffer, (size + HMAC_SIZE)))
    {
        status = true;
    }
    return status;
}

size_t read_client(uint8_t *buffer, size_t size)
{
    size_t len = protocol_receive(buffer, size);
    if (len > HMAC_SIZE)
    {
        len -= HMAC_SIZE;
        uint8_t hmac[HMAC_SIZE]{0};

        mbedtls_md_hmac_starts(&hmac_ctx, hmac_key, HMAC_SIZE);
        mbedtls_md_hmac_update(&hmac_ctx, buffer, len);
        mbedtls_md_hmac_finish(&hmac_ctx, hmac);

        if (memcpy(hmac, buffer + len, HMAC_SIZE) != 0)
        {
            len = 0;
        }
    }
    return len;
}

static bool exchange_public_keys(uint8_t *buffer)
{
    session_id = 0;
    bool status = true;
    size_t plen, length;

    mbedtls_pk_init(&client_ctx);
    uint8_t cipher[3 * RSA_SIZE + HMAC_SIZE] = {0};

    if (0 != mbedtls_pk_parse_public_key(&client_ctx, buffer, DER_SIZE))
    {
        status = false;
        goto end_exchpubkey;
    }
    if (MBEDTLS_PK_RSA != mbedtls_pk_get_type(&client_ctx))
    {
        status = false;
        goto end_exchpubkey;
    }
    if (DER_SIZE != mbedtls_pk_write_pubkey_der(&server_ctx, buffer, DER_SIZE))
    {
        status = false;
        goto end_exchpubkey;
    }
    if (0 != mbedtls_pk_encrypt(&client_ctx, buffer, DER_SIZE / 2, cipher, &plen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        status = false;
        goto end_exchpubkey;
    }
    if (0 != mbedtls_pk_encrypt(&client_ctx, buffer + DER_SIZE / 2, DER_SIZE / 2, cipher + RSA_SIZE, &plen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        status = false;
        goto end_exchpubkey;
    }
    length = 2 * RSA_SIZE;
    if (!write_client(cipher, length))
    {
        status = false;
        goto end_exchpubkey;
    }
    length = read_client(cipher, sizeof(cipher));
    if (length != 3 * RSA_SIZE)
    {
        status = false;
        goto end_exchpubkey;
    }
    if (0 != mbedtls_pk_decrypt(&server_ctx, cipher, RSA_SIZE, buffer, &plen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        status = false;
        goto end_exchpubkey;
    }
    length = plen;
    if (0 != mbedtls_pk_decrypt(&server_ctx, cipher + RSA_SIZE, RSA_SIZE, buffer + length, &plen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        status = false;
        goto end_exchpubkey;
    }
    length += plen;
    if (0 != mbedtls_pk_decrypt(&server_ctx, cipher + 2 * RSA_SIZE, RSA_SIZE, buffer + length, &plen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        status = false;
        goto end_exchpubkey;
    }
    length += plen;
    if (length != (DER_SIZE + RSA_SIZE))
    {
        status = false;
        goto end_exchpubkey;
    }
    mbedtls_pk_init(&client_ctx);
    if (0 != mbedtls_pk_parse_public_key(&client_ctx, buffer, DER_SIZE))
    {
        status = false;
        goto end_exchpubkey;
    }
    if (MBEDTLS_PK_RSA != mbedtls_pk_get_type(&client_ctx))
    {
        status = false;
        goto end_exchpubkey;
    }
    if (0 != mbedtls_pk_verify(&client_ctx, MBEDTLS_MD_SHA256, hmac_key, HMAC_SIZE, buffer + DER_SIZE, RSA_SIZE))
    {
        status = false;
        goto end_exchpubkey;
    }
    strcpy((char *)buffer, "OKAY");
    if (0 != mbedtls_pk_encrypt(&client_ctx, buffer, strlen((const char *)buffer), cipher, &plen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        status = false;
        goto end_exchpubkey;
    }
    length = RSA_SIZE;
    if (!write_client(cipher, length))
    {
        status = false;
        goto end_exchpubkey;
    }
end_exchpubkey:
    return status;
}

static bool establish_session(uint8_t *buffer)
{
    session_id = 0;
    bool status = true;
    size_t plen, length;
    uint8_t cipher[RSA_SIZE]{0};
    uint8_t *ptr{(uint8_t *)session_id};

    if (0 != mbedtls_pk_decrypt(&server_ctx, buffer, RSA_SIZE, cipher, &plen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        status = false;
        goto end_estsess;
    }
    length = plen;
    if (0 != mbedtls_pk_decrypt(&server_ctx, buffer + RSA_SIZE, RSA_SIZE, cipher + length, &plen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        status = false;
        goto end_estsess;
    }
    length += plen;
    if(length != RSA_SIZE)
    {
        status = false;
        goto end_estsess;
    }
    if (0 != mbedtls_pk_verify(&client_ctx, MBEDTLS_MD_SHA256, hmac_key, HMAC_SIZE, cipher, RSA_SIZE))
    {
        status = false;
        goto end_estsess;
    }
    for (size_t i = 0; i < sizeof(session_id); i++)
    {
        ptr[i] = random(1, 0x100);
    }
    for (size_t i = 0; i < sizeof(enc_iv); i++)
    {
        enc_iv[i] = random(1, 0x100);
    }
    memcpy(dec_iv, enc_iv, sizeof(dec_iv));

    for (size_t i = 0; i < sizeof(aes_key); i++)
    {
        aes_key[i] = random(1, 0x100);
    }
end_estsess:
    return status;
}

bool session_response(const uint8_t *resp, size_t size)
{
    bool status = false;
    uint8_t response[AES_BLOCK_SIZE] = {0};
    uint8_t buffer[AES_BLOCK_SIZE + HMAC_SIZE] = {0};

    switch (resp[0])
    {
    case SESSION_OKAY:
        response[0] = STATUS_OKAY;
        break;
    case SESSION_ERROR:
        response[0] = STATUS_ERROR;
        break;
    default:
        response[0] = resp[0];
        break;
    }
    if (size > 1)
    {
        memcpy(response + 1, resp + 1, size - 1);
    }
    if (0 == mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, sizeof(response), enc_iv, response, buffer))
    {
        status = write_client(buffer, AES_BLOCK_SIZE);
    }
}
int session_request(void)
{
    int type = SESSION_OKAY;

    uint8_t buffer[DER_SIZE + RSA_SIZE] = {0};

    size_t length = read_client(buffer, sizeof(buffer));

    if (length == DER_SIZE)
    {
        exchange_public_keys(buffer);
    }
    else if (length == 2 * RSA_SIZE)
    {
        establish_session(buffer);
    }
    else if (length == AES_BLOCK_SIZE)
    {
    }

    return type;
}