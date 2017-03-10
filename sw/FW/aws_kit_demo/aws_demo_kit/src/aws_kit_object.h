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

#ifndef AWS_KIT_OBJECT_H_
#define AWS_KIT_OBJECT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asf.h>
#include <wolfssl/ssl.h>
#include <wolfssl/internal.h>

#include "aws_kit_debug.h"
#include "MQTTClient.h"



/**
 * \defgroup AWS Kit Definition
 *
 * \brief All data definition & structure are to comply with constraints of AWS IoT, WolfSSL & Paho MQTT 
 * specification and certificate architecture of CryptoAuthLib.
 *
 * @{
 */

/** \name Max buffer size definition for certificates, MQTT message, WIFI credential, and so on.
   @{ */
#define AWS_ROOT_CERT_MAX					(2048)
#define AWS_CERT_LENGH_MAX					(1024)
#define AWS_WIFI_SSID_MAX					(32)
#define AWS_WIFI_PSK_MAX					(32)
#define AWS_HOST_ADDR_MAX					(64)
#define AWS_THING_NAME_MAX					(32)
#define AWS_CLIENT_NAME_MAX					(12)
#define AWS_ROOT_PUBKEY_MAX					(64)
#define AWS_MQTT_BUF_SIZE_MAX				(1024)
#define AWS_MQTT_TOPIC_MAX					(128)
/** @} */

/** \name Offset address definition for user data
   @{ */
#define AWS_USER_DATA_OFFSET_SSID_LEN		(0)
#define AWS_USER_DATA_OFFSET_SSID			(4)
#define AWS_USER_DATA_OFFSET_PSK_LEN		(36)
#define AWS_USER_DATA_OFFSET_PSK			(40)
#define AWS_USER_DATA_OFFSET_HOST_LEN		(72)
#define AWS_USER_DATA_OFFSET_HOST			(76)
#define AWS_USER_DATA_OFFSET_THING_LEN		(140)
#define AWS_USER_DATA_OFFSET_THING			(144)
#define AWS_USER_DATA_BUTTON_STATE			(176)
#define AWS_USER_DATA_OFFSET_MAX			(180)
/** @} */

/**
 * Types of buttons.
 */
enum { 
	AWS_KIT_BUTTON_1,
	AWS_KIT_BUTTON_2,
	AWS_KIT_BUTTON_3,
	AWS_KIT_BUTTON_MAX,
};

/**
 * Types of LEDs.
 */
enum { 
	AWS_KIT_LED_1,
	AWS_KIT_LED_2,
	AWS_KIT_LED_3,
	AWS_KIT_LED_MAX,
};

/**
 * Button state.
 */
enum { 
	AWS_BUTTON_RELEASED,
	AWS_BUTTON_PRESSED,
	AWS_BUTTON_MAX,
};

/**
 * Types of exception.
 */
typedef enum { 
	AWS_EX_NONE,
	AWS_EX_UNPROVISIONED_CRYPTO,
	AWS_EX_UNAVAILABLE_WIFI,
	AWS_EX_TLS_FAILURE,
	AWS_EX_MQTT_FAILURE,
	AWS_EX_MAX,
} KIT_ERROR_STATE;

/**
 * Types of notification.
 */
typedef enum { 
	NOTI_INVALID,
	NOTI_RUN_MQTT_CLIENT,
	NOTI_QUIT_MQTT_CLIENT,
	NOTI_RESET_USER_DATA,
	NOTI_MAX
} KIT_NOTI_STATE;

/**
 * Types of current state for the Client task.
 */
typedef enum { 
	CLIENT_STATE_INVALID,
	CLIENT_STATE_INIT_MQTT_CLIENT,
	CLIENT_STATE_MQTT_SUBSCRIBE,
	CLIENT_STATE_MQTT_PUBLISH,
	CLIENT_STATE_MQTT_WAIT_MESSAGE,
	CLIENT_STATE_MAX
} KIT_CLIENT_STATE;

/**
 * Types of current state for the Main task.
 */
typedef enum { 
	MAIN_STATE_INVALID,
	MAIN_STATE_INIT_KIT,
	MAIN_STATE_CHECK_KIT,
	MAIN_STATE_PROVISIONING,
	MAIN_STATE_RUN_KIT,
	MAIN_STATE_MAX
} KIT_MAIN_STATE;

/**
 * Defines PEM certificates structure.
 */
typedef struct AWS_CERT {
	uint32_t signerCertLen;
	uint8_t signerCert[AWS_CERT_LENGH_MAX];
	uint32_t devCertLen;
	uint8_t devCert[AWS_CERT_LENGH_MAX];
} t_awsCert;

/**
 * Defines all data to be stored in slot 8 of ATECC508A.
 */
typedef struct AWS_USER_DATA {
	uint32_t ssidLen;
	uint8_t ssid[AWS_WIFI_SSID_MAX];
	uint32_t pskLen;
	uint8_t psk[AWS_WIFI_PSK_MAX];
	uint32_t hostLen;
	uint8_t host[AWS_HOST_ADDR_MAX];
	uint32_t thingLen;
	uint8_t thing[AWS_THING_NAME_MAX];
	uint32_t port;
	uint32_t clientIDLen;
	uint8_t clientID[AWS_CLIENT_NAME_MAX];
	uint32_t rootPubKeyLen;
	uint8_t rootPubKey[AWS_ROOT_PUBKEY_MAX];
} t_awsUserData;

/**
 * Defines MQTT buffers to be used by Paho MQTT.
 */
typedef struct AWS_MQTT_BUFFER {
	uint8_t mqttTxBuf[AWS_MQTT_BUF_SIZE_MAX];
	uint8_t mqttRxBuf[AWS_MQTT_BUF_SIZE_MAX];
} t_awsMqttBuffer;

/**
 * Defines the Delta & Update buffers to store MQTT topic.
 */
typedef struct AWS_TOPIC_BUFFER {
	uint8_t updateDeltaTopic[AWS_MQTT_TOPIC_MAX];
	uint8_t updateTopic[AWS_MQTT_TOPIC_MAX];
} t_awsMqttTopic;

/**
 * Defines callback functions to be called by application.
 */
typedef int (*LED_ON)(uint8_t id);
typedef int (*LED_OFF)(uint8_t id);

/**
 * Defines LED state.
 */
typedef struct AWS_LED_STATE {
	bool isDesired[AWS_KIT_LED_MAX];
	bool state[AWS_KIT_LED_MAX];
	LED_ON turn_on;
	LED_OFF turn_off;
} t_awsLedState;

/**
 * Defines button state.
 */
typedef struct AWS_BUTTON_STATE {
	bool isPressed[AWS_KIT_BUTTON_MAX];
	bool state[AWS_KIT_BUTTON_MAX];
} t_awsButtonState;

/**
 * Defines the WolfSSL context
 */
typedef struct AWS_TLS {
	WOLFSSL_CTX *context;
	WOLFSSL *ssl;	
} MQTTTls;

/**
 * Defines all the pieces which indicate Thing based on ATSAMG55 .
 */
typedef struct AWS_KIT {
	bool quitMQTT;					//!< determines MQTT disconnection with AWS IoT.
	bool blocking;					//!< determines blocking mode, when receiving packets.
	bool nonBlocking;				//!< determines non-blocking mode, when sending packets.
	bool pushButtonState;			//!< Indicates state of SW0 button.
	xQueueHandle notiQueue;			//!< Notification queue for communication between Provisioning and Main task.
	xQueueHandle buttonQueue;		//!< Queue to deal with button event from ISR.
	KIT_NOTI_STATE noti;			//!< Value of notiQueue above.
	KIT_CLIENT_STATE clientState;	//!< State of MQTT client task.
	KIT_MAIN_STATE mainState;		//!< State of Main task.
	KIT_ERROR_STATE	errState;		//!< State of exception.
	t_awsCert cert;					//!< Storage of both Signer and device certificates.
	t_awsUserData user;				//!< Storage of user data containing WIFI credential and so on.
	t_awsMqttBuffer buffer;			//!< Storage for Paho MQTT to use this buffer.
	t_awsMqttTopic topic;			//!< Storage for MQTT topics.
	t_awsLedState led;				//!< Indicates state of LEDS in OLED1 board.
	t_awsButtonState button;		//!< Indicates state of buttons in OLED1 board.
	Timer keepAlive;				//!< Timer to send PING packet.
	Timer buttonISR;				//!< Timer to solve debounce issue.
	Timer resetISR;					//!< Reset timer to measure 3 seconds.
	Timer exceptionTimer;			//!< Timer to notify exception.
	MQTTClient client;              //!< Indicates client of MQTT
	MQTTTls tls;                    //!< Indicates the WolfSSL TLS context
	SOCKET *socket;                 //!< Indicates the network socket
} t_aws_kit;

t_aws_kit* aws_kit_get_instance(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AWS_KIT_OBJECT_H_ */
