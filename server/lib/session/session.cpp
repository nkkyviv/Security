#include "session.h"
#include "protocol.h"
#include <Arduino.h>
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <mbedtls/aes.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

#define LED_ERROR 32
#define DER_SIZE 294
#define RSA_SIZE 256
#define HASH_SIZE 32
#define EXPONENT 65537
#define AES_KEY_SIZE 32
#define AES_CIPHER_SIZE 16
#define BUFFER_SIZE (RSA_SIZE + DER_SIZE)
#define SESSION_CLOSE 0xFF

typedef struct
{
    size_t length;
    uint8_t buffer[BUFFER_SIZE];
} message_t;

static uint32_t prev_millis;
static mbedtls_aes_context aes_ctx;
static mbedtls_md_context_t hmac_ctx;
static mbedtls_pk_context client_ctx;
static mbedtls_pk_context server_ctx;
static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;

static uint64_t session_id{0};
static uint8_t aes_key[AES_KEY_SIZE]{0};
static uint8_t enc_iv[AES_CIPHER_SIZE]{0};
static uint8_t dec_iv[AES_CIPHER_SIZE]{0};
static const uint8_t hmac_hash[HASH_SIZE] = {0x29, 0x49, 0xde, 0xc2, 0x3e, 0x1e, 0x34, 0xb5, 0x2d, 0x22, 0xb5,
                                             0xba, 0x4c, 0x34, 0x23, 0x3a, 0x9d, 0x3f, 0xe2, 0x97, 0x14, 0xbe,
                                             0x24, 0x62, 0x81, 0x0c, 0x86, 0xb1, 0xf6, 0x92, 0x54, 0xd6};

static void led_error_check(int num)
{
    for (int i = 0; i < num; ++i)
    {
        digitalWrite(LED_ERROR, HIGH);
        delay(250);
        digitalWrite(LED_ERROR, LOW);
        delay(250);
    }
}

static size_t receive(uint8_t *buffer, size_t blen)
{

    size_t length = protocol_receive(buffer, blen);

    if (length > HASH_SIZE)
    {
        uint8_t hash[HASH_SIZE]{0};
        mbedtls_md_hmac_starts(&hmac_ctx, hmac_hash, HASH_SIZE);
        mbedtls_md_hmac_update(&hmac_ctx, buffer, length - HASH_SIZE);
        mbedtls_md_hmac_finish(&hmac_ctx, hash);

        if (0 == memcmp(hash, buffer + (length - HASH_SIZE), HASH_SIZE))
        {
            length -= HASH_SIZE;
        }
        else
        {
            length = 0;
        }
    }
    else
    {
        length = 0;
    }

    return length;
}

static bool send(uint8_t *buffer, size_t length)
{
    mbedtls_md_hmac_starts(&hmac_ctx, hmac_hash, HASH_SIZE);
    mbedtls_md_hmac_update(&hmac_ctx, buffer, length);
    mbedtls_md_hmac_finish(&hmac_ctx, buffer + length);

    length += HASH_SIZE;

    return protocol_send(buffer, length);
}

void session_init()
{
    pinMode(LED_ERROR, OUTPUT);

    protocol_init();

    // HMAC-SHA256
    mbedtls_md_init(&hmac_ctx);
    if(0 != mbedtls_md_setup(&hmac_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1))
    {
        led_error_check(1);
    }

    // AES-256
    mbedtls_aes_init(&aes_ctx);

    uint8_t initial[AES_KEY_SIZE]{0};
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    for (size_t i = 0; i < sizeof(initial); i++)
    {
        initial[i] = random(0x100);
    }
    if(0 != mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, initial, sizeof(initial)))
    {
        led_error_check(1);
    }

    // RSA-2048
    mbedtls_pk_init(&server_ctx);
    if(0 != mbedtls_pk_setup(&server_ctx, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA)))
    {
        led_error_check(1);
    }
    if(0 == mbedtls_rsa_gen_key(mbedtls_pk_rsa(server_ctx), mbedtls_ctr_drbg_random,
                                    &ctr_drbg, RSA_SIZE * CHAR_BIT, EXPONENT))
    {
        led_error_check(1);
    }
}

static void exchange_public_keys(uint8_t *buffer)
{
    session_id = 0;

    size_t olen, length;

    mbedtls_pk_init(&client_ctx);

    uint8_t cipher[3 * RSA_SIZE + HASH_SIZE] = {0};

    if (0 != mbedtls_pk_parse_public_key(&client_ctx, buffer, DER_SIZE))
    {
        led_error_check(2);
    }

    if (MBEDTLS_PK_RSA != mbedtls_pk_get_type(&client_ctx))
    {
        led_error_check(2);
    }

    if (DER_SIZE != mbedtls_pk_write_pubkey_der(&server_ctx, buffer, DER_SIZE))
    {
        led_error_check(2);
    }

    if (0 != mbedtls_pk_encrypt(&client_ctx, buffer, DER_SIZE / 2, cipher,
                                &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        led_error_check(2);
    }

    if (0 != mbedtls_pk_encrypt(&client_ctx, buffer + DER_SIZE / 2, DER_SIZE / 2,
                                cipher + RSA_SIZE, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        led_error_check(2);
    }

    length = 2 * RSA_SIZE;

    if (!send(cipher, length))
    {
        led_error_check(2);
    }

    length = receive(cipher, sizeof(cipher));

    if (!length == 3 * RSA_SIZE)
    {
        led_error_check(2);
    }

    if (0 != mbedtls_pk_decrypt(&server_ctx, cipher, RSA_SIZE, buffer, &olen, RSA_SIZE,
                                mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        led_error_check(2);
    }

    length = olen;

    if (0 != mbedtls_pk_decrypt(&server_ctx, cipher + RSA_SIZE, RSA_SIZE, buffer + length,
                                &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        led_error_check(2);
    }

    length += olen;

    if (0 != mbedtls_pk_decrypt(&server_ctx, cipher + 2 * RSA_SIZE, RSA_SIZE, buffer + length,
                                &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        led_error_check(2);
    }

    length += olen;

    if (length != (DER_SIZE + RSA_SIZE))
    {
        led_error_check(2);
    }

    mbedtls_pk_init(&client_ctx);

    if (0 != mbedtls_pk_parse_public_key(&client_ctx, buffer, DER_SIZE))
    {
        led_error_check(2);
    }

    if (MBEDTLS_PK_RSA != mbedtls_pk_get_type(&client_ctx))
    {
        led_error_check(2);
    }

    if (0 != mbedtls_pk_verify(&client_ctx, MBEDTLS_MD_SHA256, hmac_hash, HASH_SIZE, buffer + DER_SIZE, RSA_SIZE))
    {
        led_error_check(2);
    }

    strcpy((char *)buffer, "OKAY");

    if (0 != mbedtls_pk_encrypt(&client_ctx, buffer, strlen((const char *)buffer),
                                cipher, &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        led_error_check(2);
    }

    length = RSA_SIZE;

    if (!send(cipher, length))
    {
        led_error_check(2);
    }
}

static void establish_session(uint8_t *buffer)
{
    session_id = 0;

    size_t olen, length;

    uint8_t cipher[RSA_SIZE]{0};

    if (0 != mbedtls_pk_decrypt(&server_ctx, buffer, RSA_SIZE, cipher, &olen,
                                RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        led_error_check(3);
    }

    length = olen;

    if (0 != mbedtls_pk_decrypt(&server_ctx, buffer + RSA_SIZE, RSA_SIZE, cipher + length,
                                &olen, RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        led_error_check(3);
    }

    length += olen;

    if (length != RSA_SIZE)
    {
        led_error_check(3);
    }

    if (0 != mbedtls_pk_verify(&client_ctx, MBEDTLS_MD_SHA256, hmac_hash, HASH_SIZE, cipher, RSA_SIZE))
    {
        led_error_check(3);
    }

    uint8_t *ptr{(uint8_t *)&session_id};

    if (ptr != nullptr)
    {
        for (size_t i = 0; i < sizeof(session_id); i++)
        {
            ptr[i] = random(1, 0x100);
        }

        for (size_t i = 0; i < sizeof(enc_iv); i++)
        {
            enc_iv[i] = random(0x100);
        }
    }
    else
    {
        led_error_check(3);
    }

    memcpy(dec_iv, enc_iv, sizeof(dec_iv));

    for (size_t i = 0; i < sizeof(aes_key); i++)
    {
        aes_key[i] = random(0x100);
    }

    if (0 != mbedtls_aes_setkey_enc(&aes_ctx, aes_key, sizeof(aes_key) * CHAR_BIT))
    {
        led_error_check(3);
    }

    memcpy(buffer, &session_id, sizeof(session_id));

    length = sizeof(session_id);

    memcpy(buffer + length, enc_iv, sizeof(enc_iv));

    length += sizeof(enc_iv);

    memcpy(buffer + length, aes_key, sizeof(aes_key));

    length += sizeof(aes_key);

    if (0 != mbedtls_pk_encrypt(&client_ctx, buffer, length, cipher, &olen,
                                RSA_SIZE, mbedtls_ctr_drbg_random, &ctr_drbg))
    {
        led_error_check(3);
    }

    length = RSA_SIZE;

    if (send(cipher, length) == 0)
    {
        led_error_check(3);
    }
}

int session_request(void)
{
    message_t msg{0};
    int status{SESSION_OKAY};

    msg.length = receive(msg.buffer, BUFFER_SIZE);

    if (msg.length == DER_SIZE)
    {
        exchange_public_keys(msg.buffer);
    }
    else if (msg.length == 2 * RSA_SIZE)
    {
        establish_session(msg.buffer);
        prev_millis = millis();
    }
    else if (msg.length == AES_CIPHER_SIZE)
    {
        if (session_id != 0)
        {
            uint32_t now = millis();

            if (now - prev_millis <= 60000) // timer
            {
                uint8_t cipher[AES_CIPHER_SIZE]{0};

                if (0 != mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, msg.length, dec_iv, msg.buffer, cipher))
                {
                    led_error_check(4);
                }
                if (cipher[AES_CIPHER_SIZE - 1] == 9)
                {
                    status = cipher[0];
                }

                prev_millis = now;
            }
            else
            {
                response_t res = {0};
                res.data[0] = SESSION_CLOSE;
                session_response(&res);
                session_id = 0;
            }
        }
    }
    else
    {
        ;
    }

    return status;
}

int session_response(response_t *resp)
{
    int status{SESSION_OKAY};

    uint8_t cipher[AES_CIPHER_SIZE + HASH_SIZE]{0};

    resp->len = AES_CIPHER_SIZE;

    if (0 == mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, resp->len, enc_iv, resp->data, cipher))
    {
        if (!send(cipher, resp->len))
        {
            status = SESSION_ERROR;
            led_error_check(4);
        }
    }

    return status;
}