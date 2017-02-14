/*
 * Copyright (C) 2006-2016 wolfSSL Inc.  All rights reserved.
 *
 * This license is only for commercial evaluation purposes.  
 * 
 * Redistribution of the source and binary forms, with or without
 * modification, is not permitted.
 * 
 * To obtain a commercial license from wolfSSL for redistribution please contact licensing @wolfssl.com
 *
 * THIS SOFTWARE IS PROVIDED BY WOLFSSL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL WOLFSSL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Contact licensing@wolfssl.com with any questions or comments.
 *
 * http://www.wolfssl.com
 */



#ifndef WOLF_CRYPT_HASH_H
#define WOLF_CRYPT_HASH_H

#include <wolfssl/wolfcrypt/types.h>

#ifdef __cplusplus
    extern "C" {
#endif

/* Hash types */
enum wc_HashType {
    WC_HASH_TYPE_NONE = 0,
    WC_HASH_TYPE_MD2 = 1,
    WC_HASH_TYPE_MD4 = 2,
    WC_HASH_TYPE_MD5 = 3,
    WC_HASH_TYPE_SHA = 4, /* SHA-1 (not old SHA-0) */
    WC_HASH_TYPE_SHA256 = 5,
    WC_HASH_TYPE_SHA384 = 6,
    WC_HASH_TYPE_SHA512 = 7,
};

/* Find largest possible digest size
   Note if this gets up to the size of 80 or over check smallstack build */
#if defined(WOLFSSL_SHA512)
    #define WC_MAX_DIGEST_SIZE SHA512_DIGEST_SIZE
#elif defined(WOLFSSL_SHA384)
    #define WC_MAX_DIGEST_SIZE SHA384_DIGEST_SIZE
#elif !defined(NO_SHA256)
    #define WC_MAX_DIGEST_SIZE SHA256_DIGEST_SIZE
#elif !defined(NO_SHA)
    #define WC_MAX_DIGEST_SIZE SHA_DIGEST_SIZE
#elif !defined(NO_MD5)
    #define WC_MAX_DIGEST_SIZE MD5_DIGEST_SIZE
#else
    #define WC_MAX_DIGEST_SIZE 64 /* default to max size of 64 */
#endif

#ifndef NO_ASN
WOLFSSL_API int wc_HashGetOID(enum wc_HashType hash_type);
#endif

WOLFSSL_API int wc_HashGetDigestSize(enum wc_HashType hash_type);
WOLFSSL_API int wc_Hash(enum wc_HashType hash_type,
    const byte* data, word32 data_len,
    byte* hash, word32 hash_len);


#ifndef NO_MD5
#include <wolfssl/wolfcrypt/md5.h>
WOLFSSL_API void wc_Md5GetHash(Md5*, byte*);
WOLFSSL_API void wc_Md5RestorePos(Md5*, Md5*);
#if defined(WOLFSSL_TI_HASH)
    WOLFSSL_API void wc_Md5Free(Md5*);
#else
    #define wc_Md5Free(d)
#endif
#endif

#ifndef NO_SHA
#include <wolfssl/wolfcrypt/sha.h>
WOLFSSL_API int wc_ShaGetHash(Sha*, byte*);
WOLFSSL_API void wc_ShaRestorePos(Sha*, Sha*);
WOLFSSL_API int wc_ShaHash(const byte*, word32, byte*);
#if defined(WOLFSSL_TI_HASH)
     WOLFSSL_API void wc_ShaFree(Sha*);
#else
    #define wc_ShaFree(d)
#endif
#endif

#ifndef NO_SHA256
#include <wolfssl/wolfcrypt/sha256.h>
WOLFSSL_API int wc_Sha256GetHash(Sha256*, byte*);
WOLFSSL_API void wc_Sha256RestorePos(Sha256*, Sha256*);
WOLFSSL_API int wc_Sha256Hash(const byte*, word32, byte*);
#if defined(WOLFSSL_TI_HASH)
    WOLFSSL_API void wc_Sha256Free(Sha256*);
#else
    #define wc_Sha256Free(d)
#endif
#endif

#ifdef WOLFSSL_SHA512
#include <wolfssl/wolfcrypt/sha512.h>
WOLFSSL_API int wc_Sha512Hash(const byte*, word32, byte*);
#if defined(WOLFSSL_TI_HASH)
    WOLFSSL_API void wc_Sha512Free(Sha512*);
#else
    #define wc_Sha512Free(d)
#endif
    #if defined(WOLFSSL_SHA384)
        WOLFSSL_API int wc_Sha384Hash(const byte*, word32, byte*);
        #if defined(WOLFSSL_TI_HASH)
            WOLFSSL_API void wc_Sha384Free(Sha384*);
        #else
            #define wc_Sha384Free(d)
        #endif
    #endif /* defined(WOLFSSL_SHA384) */
#endif /* WOLFSSL_SHA512 */


#ifdef __cplusplus
    } /* extern "C" */
#endif

#endif /* WOLF_CRYPT_HASH_H */
