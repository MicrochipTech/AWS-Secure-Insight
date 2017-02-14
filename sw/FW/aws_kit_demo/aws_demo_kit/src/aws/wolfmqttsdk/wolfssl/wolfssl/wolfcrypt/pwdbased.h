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



#ifndef WOLF_CRYPT_PWDBASED_H
#define WOLF_CRYPT_PWDBASED_H

#include <wolfssl/wolfcrypt/types.h>

#ifndef NO_PWDBASED

#ifndef NO_MD5
    #include <wolfssl/wolfcrypt/md5.h>       /* for hash type */
#endif

#include <wolfssl/wolfcrypt/sha.h>

#ifdef __cplusplus
    extern "C" {
#endif

/*
 * hashType renamed to typeH to avoid shadowing global declaration here:
 * wolfssl/wolfcrypt/asn.h line 173 in enum Oid_Types
 */
WOLFSSL_API int wc_PBKDF1(byte* output, const byte* passwd, int pLen,
                      const byte* salt, int sLen, int iterations, int kLen,
                      int typeH);
WOLFSSL_API int wc_PBKDF2(byte* output, const byte* passwd, int pLen,
                      const byte* salt, int sLen, int iterations, int kLen,
                      int typeH);
WOLFSSL_API int wc_PKCS12_PBKDF(byte* output, const byte* passwd, int pLen,
                            const byte* salt, int sLen, int iterations,
                            int kLen, int typeH, int purpose);

/* helper functions */
WOLFSSL_LOCAL int GetDigestSize(int typeH);
WOLFSSL_LOCAL int GetPKCS12HashSizes(int typeH, word32* v, word32* u);
WOLFSSL_LOCAL int DoPKCS12Hash(int typeH, byte* buffer, word32 totalLen,
                               byte* Ai, word32 u, int iterations);


#ifdef __cplusplus
    } /* extern "C" */
#endif

#endif /* NO_PWDBASED */
#endif /* WOLF_CRYPT_PWDBASED_H */
