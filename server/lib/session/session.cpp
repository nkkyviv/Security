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