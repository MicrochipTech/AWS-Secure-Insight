/**
 *
 * \file
 *
 * \brief AWS IoT Demo kit.
 *
 * Copyright (c) 2014-2016 Atmel Corporation. All rights reserved.
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

#ifndef AWS_KIT_DEBUG_H_
#define AWS_KIT_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asf.h>

/**
 * \defgroup Various of debugging level definition
 *
 * @{
 */
 
#define AWS_KIT_DEBUG
#define AWS_KIT_INFO
#define AWS_KIT_WARN
#define AWS_KIT_ERROR

#define GFX_MONO_DISPLAY_X_POSITION		(4)
#define GFX_MONO_DISPLAY_Y_POSITION		(4)

enum {
	AWS_E_SUCCESS = 0,
	AWS_E_BAD_PARAM,
	AWS_E_FAILURE,
	AWS_E_NET_SOCKET_INVALID,
	AWS_E_NET_SOCKET_TIMEOUT,
	AWS_E_NET_DNS_TIMEOUT,
	AWS_E_NET_CONN_TIMEOUT,
	AWS_E_NET_CONN_FAILURE,
	AWS_E_NET_TLS_FAILURE,	
	AWS_E_NET_JITR_RETRY,
	AWS_E_WIFI_CONN_TIMEOUT,
	AWS_E_WIFI_INVALID,
	AWS_E_WIFI_CONN_FAILURE,
	AWS_E_CRYPTO_FAILURE,
	AWS_E_CRYPTO_CERT_FAILURE,
	AWS_E_CRYPTO_DATA_INVALID,
	AWS_E_USER_DATA_INVALID,
	AWS_E_CLI_PUB_FAILURE,
	AWS_E_CLI_SUB_FAILURE,
	AWS_E_MQTT_REINITIALIZE,
} AWS_KIT_RET;

typedef enum {
	AWS_KIT_MODE_INVALID = 0,
	AWS_KIT_MODE_PROVISIONING,
	AWS_KIT_MODE_INITIALIZATION,
	AWS_KIT_MODE_MQTT_CLIENT,
	AWS_KIT_SETUP_USER_DATA,
	AWS_KIT_MODE_MQTT_MAX,
} AWS_KIT_LCD_INFO;

#ifdef AWS_KIT_DEBUG
#define AWS_DEBUG(...)    \
    {\
    printf("DEBUG:   %s L#%d ", __PRETTY_FUNCTION__, __LINE__);  \
    printf(__VA_ARGS__); \
    printf("\r\n"); \
    }
#else
#define AWS_DEBUG(...)
#endif


#ifdef AWS_KIT_INFO
#define AWS_INFO(...)    \
    {\
    printf(__VA_ARGS__); \
    printf("\r\n"); \
    }
#else
#define AWS_INFO(...)
#endif

#ifdef AWS_KIT_WARN
#define AWS_WARN(...)   \
    { \
    printf("WARN:  %s L#%d ", __PRETTY_FUNCTION__, __LINE__);  \
    printf(__VA_ARGS__); \
    printf("\r\n"); \
    }
#else
#define AWS_WARN(...)
#endif

#ifdef AWS_KIT_ERROR
#define AWS_ERROR(...)  \
    { \
    printf("ERROR: %s L#%d ", __PRETTY_FUNCTION__, __LINE__); \
    printf(__VA_ARGS__); \
    printf("\r\n"); \
    }
#else
#define AWS_ERROR(...)
#endif

const char* aws_kit_get_string(AWS_KIT_LCD_INFO info);
void aws_kit_lcd_print(AWS_KIT_LCD_INFO info);
void aws_kit_software_reset(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AWS_KIT_DEBUG_H_ */
