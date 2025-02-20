/*
 * Copyright 2019 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

typedef struct prov_ccm_hw_st PROV_CCM_HW;

#if defined(OPENSSL_CPUID_OBJ) && defined(__s390__)
/*-
 * KMAC-AES parameter block - begin
 * (see z/Architecture Principles of Operation >= SA22-7832-08)
 */
typedef struct S390X_kmac_params_st {
    union {
        unsigned long long g[2];
        unsigned char b[16];
    } icv;
    unsigned char k[32];
} S390X_KMAC_PARAMS;
/* KMAC-AES parameter block - end */
#endif

/* Base structure that is shared by AES & ARIA for CCM MODE */
typedef struct prov_ccm_st {
    int enc;
    int key_set;                /* Set if key initialised */
    int iv_set;                 /* Set if an iv is set */
    int tag_set;                /* Set if tag is valid */
    int len_set;                /* Set if message length set */
    size_t l, m;                /* L and M parameters from RFC3610 */
    size_t keylen;
    int tls_aad_len;            /* TLS AAD length */
    int tls_aad_pad_sz;
    unsigned char iv[AES_BLOCK_SIZE];
    unsigned char buf[AES_BLOCK_SIZE];
    CCM128_CONTEXT ccm_ctx;
    ccm128_f str;
    const PROV_CCM_HW *hw;      /* hardware specific methods  */
} PROV_CCM_CTX;

typedef struct prov_aes_ccm_ctx_st {
    PROV_CCM_CTX base;          /* Must be first */
    union {
        OSSL_UNION_ALIGN;
        /*-
         * Padding is chosen so that s390x.kmac.k overlaps with ks.ks and
         * fc with ks.ks.rounds. Remember that on s390x, an AES_KEY's
         * rounds field is used to store the function code and that the key
         * schedule is not stored (if aes hardware support is detected).
         */
        struct {
            unsigned char pad[16];
            AES_KEY ks;
        } ks;
#if defined(OPENSSL_CPUID_OBJ) && defined(__s390__)
        struct {
            S390X_KMAC_PARAMS kmac;
            unsigned long long blocks;
            union {
                unsigned long long g[2];
                unsigned char b[AES_BLOCK_SIZE];
            } nonce;
            union {
                unsigned long long g[2];
                unsigned char b[AES_BLOCK_SIZE];
            } buf;
            unsigned char dummy_pad[168];
            unsigned int fc;    /* fc has same offset as ks.ks.rounds */
        } s390x;
#endif /* defined(OPENSSL_CPUID_OBJ) && defined(__s390__) */
    } ccm;
} PROV_AES_CCM_CTX;

PROV_CIPHER_FUNC(int, CCM_cipher, (PROV_CCM_CTX *ctx, unsigned char *out,      \
                                   size_t *padlen, const unsigned char *in,    \
                                   size_t len));
PROV_CIPHER_FUNC(int, CCM_setkey, (PROV_CCM_CTX *ctx,                          \
                                   const unsigned char *key, size_t keylen));
PROV_CIPHER_FUNC(int, CCM_setiv, (PROV_CCM_CTX *dat,                           \
                                  const unsigned char *iv, size_t ivlen,       \
                                  size_t mlen));
PROV_CIPHER_FUNC(int, CCM_setaad, (PROV_CCM_CTX *ctx,                          \
                                   const unsigned char *aad, size_t aadlen));
PROV_CIPHER_FUNC(int, CCM_auth_encrypt, (PROV_CCM_CTX *ctx,                    \
                                         const unsigned char *in,              \
                                         unsigned char *out, size_t len,       \
                                         unsigned char *tag, size_t taglen));
PROV_CIPHER_FUNC(int, CCM_auth_decrypt, (PROV_CCM_CTX *ctx,                    \
                                         const unsigned char *in,              \
                                         unsigned char *out, size_t len,       \
                                         unsigned char *tag, size_t taglen));
PROV_CIPHER_FUNC(int, CCM_gettag, (PROV_CCM_CTX *ctx,                          \
                                   unsigned char *tag,  size_t taglen));

/*
 * CCM Mode internal method table used to handle hardware specific differences,
 * (and different algorithms).
 */
struct prov_ccm_hw_st {
    OSSL_CCM_setkey_fn setkey;
    OSSL_CCM_setiv_fn setiv;
    OSSL_CCM_setaad_fn setaad;
    OSSL_CCM_auth_encrypt_fn auth_encrypt;
    OSSL_CCM_auth_decrypt_fn auth_decrypt;
    OSSL_CCM_gettag_fn gettag;
};

const PROV_CCM_HW *PROV_AES_HW_ccm(size_t keylen);

#if !defined(OPENSSL_NO_ARIA) && !defined(FIPS_MODE)
# include "internal/aria.h"
typedef struct prov_aria_ccm_ctx_st {
    PROV_CCM_CTX base; /* Must be first */
    union {
        OSSL_UNION_ALIGN;
        ARIA_KEY ks;
    } ks;                       /* ARIA key schedule to use */
} PROV_ARIA_CCM_CTX;

const PROV_CCM_HW *PROV_ARIA_HW_ccm(size_t keylen);
#endif /* !defined(OPENSSL_NO_ARIA) && !defined(FIPS_MODE) */
