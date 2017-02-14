/**
 *
 * \file
 *
 * \brief Interface between CryptoAuthLib and WolfSSL.
 *
 * Copyright (c) 2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#ifndef ATECC508CB_H
#define ATECC508CB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <asf.h>
#include <wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/memory.h>
#include "cryptoauthlib.h"

/**
 * \defgroup Interface between WolfSSL library and CryptoAuthLib.
 *
 * @{
 */

/** \name Initial certificate length definition.
   @{ */
#define DER_CERT_INIT_SIZE						(1024)
#define PEM_CERT_INIT_SIZE						(1024)
#define PEM_CERT_CHAIN_INIT_SIZE				(2048)
#define ATCERT_PUBKEY_SIZE						(64)
/** @} */

/** \name Certificate structure definition.
   @{ */
typedef struct {
	uint32_t	device_der_size;
	uint8_t*	device_der;
	uint32_t	device_pem_size;
	uint8_t*	device_pem;
	uint8_t*	device_pubkey;
	uint32_t	signer_der_size;
	uint8_t*	signer_der;
	uint32_t	signer_pem_size;
	uint8_t*	signer_pem;
	uint8_t*	signer_pubkey;
} t_atcert;
/** @} */

/** WolfSSL callback functions to communicate to ATECC508A. */
ATCA_STATUS atca_tls_set_enc_key(uint8_t* outKey, uint16_t keysize);
int atca_tls_init_enc_key(void);
int atca_tls_create_pms_cb(WOLFSSL* ssl, unsigned char* pubKey, unsigned int* size, unsigned char inOut);
int atca_tls_get_random_number(uint32_t count, uint8_t* rand_out);
int atca_tls_get_signer_public_key(uint8_t *pubKey);
int atca_tls_build_signer_cert(t_atcert* cert);
int atca_tls_build_device_cert(t_atcert* cert);
int atca_tls_sign_certificate_cb(WOLFSSL* ssl, const byte* in, word32 inSz, byte* out, word32* outSz, const byte* key, word32 keySz, void* ctx);
int atca_tls_verify_signature_cb(WOLFSSL* ssl, const byte* sig, word32 sigSz, const byte* hash, word32 hashSz, const byte* key, word32 keySz, int* result, void* ctx);

static const uint8_t ATCA_TLS_PARENT_ENC_KEY[ATCA_KEY_SIZE] = {
	0x44, 0x00, 0x44, 0x01, 0x44, 0x02, 0x44, 0x03, 0x44, 0x04, 0x44, 0x05, 0x44, 0x06, 0x44, 0x07,
	0x44, 0x08, 0x44, 0x09, 0x44, 0x0A, 0x44, 0x0B, 0x44, 0x0C, 0x44, 0x0D, 0x44, 0x0E, 0x44, 0x0F
};

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ATECC508CB_H */
