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



#ifndef WOLF_CRYPT_DH_H
#define WOLF_CRYPT_DH_H

#include <wolfssl/wolfcrypt/types.h>

#ifndef NO_DH

#include <wolfssl/wolfcrypt/integer.h>
#include <wolfssl/wolfcrypt/random.h>

#ifdef __cplusplus
    extern "C" {
#endif


/* Diffie-Hellman Key */
typedef struct DhKey {
    mp_int p, g;                            /* group parameters  */
} DhKey;


WOLFSSL_API void wc_InitDhKey(DhKey* key);
WOLFSSL_API void wc_FreeDhKey(DhKey* key);

WOLFSSL_API int wc_DhGenerateKeyPair(DhKey* key, WC_RNG* rng, byte* priv,
                                 word32* privSz, byte* pub, word32* pubSz);
WOLFSSL_API int wc_DhAgree(DhKey* key, byte* agree, word32* agreeSz,
                       const byte* priv, word32 privSz, const byte* otherPub,
                       word32 pubSz);

WOLFSSL_API int wc_DhKeyDecode(const byte* input, word32* inOutIdx, DhKey* key,
                           word32);
WOLFSSL_API int wc_DhSetKey(DhKey* key, const byte* p, word32 pSz, const byte* g,
                        word32 gSz);
WOLFSSL_API int wc_DhParamsLoad(const byte* input, word32 inSz, byte* p,
                            word32* pInOutSz, byte* g, word32* gInOutSz);


#ifdef __cplusplus
    } /* extern "C" */
#endif

#endif /* NO_DH */
#endif /* WOLF_CRYPT_DH_H */

