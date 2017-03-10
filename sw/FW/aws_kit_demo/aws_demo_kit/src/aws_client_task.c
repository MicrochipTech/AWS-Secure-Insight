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

#include <stdlib.h>
#include "atecc508cb.h"
#include "tls/atcatls_cfg.h"
#include "aws_iot_config.h"
#include "aws_net_interface.h"
#include "aws_main_task.h"
#include "aws_user_task.h"
#include "aws_client_task.h"
#include "aws/jsonlib/parson.h"
#include "MQTTClient.h"


Network mqtt_network;


/**
 * \brief Returns packet ID.
 *
 * \return mPacketIdLast
 */
int aws_client_mqtt_packet_id(void)
{
	static uint32_t mPacketIdLast = 0;

	mPacketIdLast = (mPacketIdLast >= 65536) ? 1 : mPacketIdLast + 1;
	return mPacketIdLast;
}

/**
 * \brief Check if a button is pressed or not.
 *
 * \param kit[in]             Pointer to an instance of AWS Kit
 * \return TRUE               On pressed state
 */
bool aws_client_scan_button(t_aws_kit* kit)
{
	return (kit->button.isPressed[AWS_KIT_BUTTON_1] || kit->button.isPressed[AWS_KIT_BUTTON_2]
			|| kit->button.isPressed[AWS_KIT_BUTTON_3]);
}

/**
 * \brief This function initializes MQTT client, and tries to connect to destination address depending on a user AWS account.
 * The host address should be installed in ATECC508A through Insight GUI.
 *
 * \param kit[inout]          Pointer to an instance of AWS Kit
 * \return AWS_E_SUCCESS      On success
 */
int aws_client_init_mqtt_client(t_aws_kit* kit)
{
	int ret = AWS_E_FAILURE;	
	MQTTPacket_connectData options = MQTTPacket_connectData_initializer;

	do {
		/* Set the network MQTT functions. */
		mqtt_network.mqttread  = mqtt_packet_read;
		mqtt_network.mqttwrite = mqtt_packet_write;

		if (kit->socket == NULL)
			kit->socket = malloc(sizeof(SOCKET));
		if (kit->socket == NULL) {
			AWS_ERROR("Failed to allocate heap!");
			break;
		}
			
		/* Initialize MQTT client object. */
		memset(kit->socket, 0, sizeof(SOCKET));
		MQTTClientInit(&kit->client, &mqtt_network, AWS_MQTT_CMD_TIMEOUT_MS, kit->buffer.mqttTxBuf, 
					   sizeof(kit->buffer.mqttTxBuf), kit->buffer.mqttRxBuf, sizeof(kit->buffer.mqttRxBuf));
					   
		/* Connect to AWS IoT over TLS handshaking with ECDHE-ECDSA-AES128-GCM-SHA256 cipher suite. 
		   If TLS connection fails by AWS IoT during JITR, then return normal failure for the retry. */
		ret = aws_client_mqtt_connect(kit, (const char *)kit->user.host, AWS_IOT_MQTT_PORT,
									  AWS_NET_CONN_TIMEOUT_MS, aws_client_net_tls_cb);
		if (ret != SUCCESS) {
			ret = AWS_E_NET_CONN_FAILURE;
			AWS_ERROR("Error(%d) : Failed to connect to Host!", ret);
			break;
		}

		/* If both JITR and TLS session establishment have been successfully made, Send MQTT CONN packet. */
		options.keepAliveInterval = AWS_IOT_KEEP_ALIVE_SEC;
		options.cleansession = 1;
		/* Client ID represents a serial number of ATECC508A. */
		options.clientID.cstring = (const char*)kit->user.clientID;
		ret = MQTTConnect(&kit->client, &options);
		if (ret != SUCCESS) {
			ret = AWS_E_NET_TLS_FAILURE;
			AWS_ERROR("Error(%d) : Failed to receive CONNACK!", ret);
			break;
		}
		
		/* According to AWS message broker requirements, by default, MQTT client connection is disconnected 
		   after 30 minutes of inactivity. When the client sends a PUBLISH, SUBSCRIBE, PING, or PUBACK message, 
		   the inactivity timer is reset. The keep-alive timer begins immediately after the server returns 
		   the client's sending of a CONNECT message and start of keep-alive behavior. 
		   For this AWS Kit Things, the keep-alive timer is initialized to 20 minutes, and then count down to zero seconds. */
		TimerInit(&kit->keepAlive);
		TimerCountdown(&kit->keepAlive, AWS_IOT_KEEP_ALIVE_SEC);
	} while(0);

	return ret;
}

int aws_client_mqtt_connect(t_aws_kit* kit, const char *host, uint16_t port,
							int timeout_ms, MqttTlsCb cb)
{
	int ret = AWS_E_SUCCESS;
	Timer conTimer;
	struct sockaddr_in dest_addr;
	
	/* Connect to the host */
	registerSocketCallback(aws_net_socket_cb, aws_net_dns_resolve_cb);
	delay_ms(50);
	gethostbyname((uint8*)host);
	TimerInit(&conTimer);
	TimerCountdownMS(&conTimer, timeout_ms);
	
	while (!aws_net_get_host_addr()) {
		if(TimerIsExpired(&conTimer)) {
			ret = AWS_E_NET_DNS_TIMEOUT;
			AWS_ERROR("Expired DNS timer!(%d)", ret);
			return ret;
		}
		m2m_wifi_handle_events(NULL);
	}
	
	/* Connect to the network socket */
	ret = network_socket_connect(kit->socket, aws_net_get_host_addr(), port, timeout_ms);
	if (ret != SOCK_ERR_NO_ERROR) {
		AWS_ERROR("Failed to connect to host!(%d)(%x)", ret, aws_net_get_host_addr());
		return AWS_E_NET_CONN_FAILURE;
	}

	aws_net_set_host_addr(0);
	
	/* Setup the WolfSSL library */
	wolfSSL_Init();
	
	ret = SSL_SUCCESS;
	if (cb)
		ret = cb(kit);

	if (ret == SSL_SUCCESS) {
		if (kit->tls.context == NULL) {
			kit->tls.context = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
			
			if (kit->tls.context)
				 wolfSSL_CTX_set_verify(kit->tls.context, SSL_VERIFY_NONE, 0);
		}

		if (kit->tls.context) {
			wolfSSL_SetIORecv(kit->tls.context, aws_client_tls_receive);
			wolfSSL_SetIOSend(kit->tls.context, aws_client_tls_send);
	
			kit->tls.ssl = wolfSSL_new(kit->tls.context);
			if (kit->tls.ssl) {
				wolfSSL_SetIOReadCtx(kit->tls.ssl, (void*)&kit->client);
				wolfSSL_SetIOWriteCtx(kit->tls.ssl, (void*)&kit->client);
			
				ret = wolfSSL_connect(kit->tls.ssl);
				if (ret != SSL_SUCCESS) {
					ret = AWS_E_NET_TLS_FAILURE;
					AWS_ERROR("Error(%d) : Failed to TLS connect!", ret);
				}
			} else {
				ret = AWS_E_NET_TLS_FAILURE;
				AWS_ERROR("Error(%d) : Failed to TLS init!", ret);
			}
		} else {
			ret = AWS_E_NET_TLS_FAILURE;
			AWS_ERROR("Error(%d) : Failed to TLS context init!", ret);
		}
	}
			
	/* Cleanup the WolfSSL library */
	if (ret == SSL_SUCCESS) {
		ret = SUCCESS;
	} else {
		if (kit->tls.ssl)
			wolfSSL_free(kit->tls.ssl);
		
		if (kit->tls.context)
			wolfSSL_CTX_free(kit->tls.context);
		
		wolfSSL_Cleanup();
		
		network_socket_disconnect(kit->socket);
	}
	
	return ret;
}

/**
 * \brief Send disconnection MQTT packet.
 *
 * \param kit[inout]          Pointer to an instance of AWS Kit
 * \return AWS_E_SUCCESS      On success
 */
int aws_client_mqtt_disconnect(t_aws_kit* kit)
{
	int ret = AWS_E_FAILURE;
	
	do {
		/* Unsubscribe the Update topic anymore. */
		snprintf((char*)kit->topic.updateDeltaTopic, sizeof(kit->topic.updateDeltaTopic), AWS_IOT_UPDATE_DELTA_TOPIC, kit->user.thing);
		kit->blocking = true;
		kit->nonBlocking = true;

		ret = MQTTUnsubscribe(&kit->client, (const char*)kit->topic.updateDeltaTopic);
		if (ret != SUCCESS) {
			AWS_ERROR("Error(%d) : Failed to unsubscribe delta topic!", ret);
			kit->blocking = false;
			kit->nonBlocking = false;
			break;
		}

		kit->blocking = false;
		kit->nonBlocking = false;
		
		/* Send the disconnect MQTT packet. */
		ret = MQTTDisconnect(&kit->client);
		if (ret != SUCCESS) {
			AWS_ERROR("Error(%d) : Failed to send disconnection packet!", ret);
			break;
		}
		
		/* Disconnect from AWS MQTT broker. */
		if (kit->tls.ssl)
			wolfSSL_free(kit->tls.ssl);
			
		if (kit->tls.context)
			wolfSSL_CTX_free(kit->tls.context);
		
		wolfSSL_Cleanup();	
		
		network_socket_disconnect(kit->socket);

		if (kit->socket) {
			free(kit->socket);
			kit->socket = NULL;
		}
	} while(0);

	return ret;
}

int aws_client_tls_receive(WOLFSSL* ssl, char *buf, int sz, void *ptr)
{
	int ret;
	MQTTClient* client = (MQTTClient*)ptr;
	t_aws_kit* kit = aws_kit_get_instance();

	ret = network_socket_read(kit->socket, (unsigned char*)buf, sz, kit->tls.ssl->rflags, client->command_timeout_ms);	
	if (ret == SUCCESS)
		ret = WOLFSSL_CBIO_ERR_WANT_READ;
	else if (ret < 0)
		ret = WOLFSSL_CBIO_ERR_GENERAL;
		
	return ret;
}

int aws_client_tls_send(WOLFSSL* ssl, char *buf, int sz, void *ptr)
{
	int ret;
	MQTTClient* client = (MQTTClient*)ptr;
	t_aws_kit* kit = aws_kit_get_instance();
	
	ret = network_socket_write(kit->socket, (unsigned char*)buf, sz, kit->tls.ssl->wflags, client->command_timeout_ms);
	if (ret == 0)
        ret = WOLFSSL_CBIO_ERR_WANT_WRITE;
	else if (ret < 0)
		ret = WOLFSSL_CBIO_ERR_GENERAL;
	
	return ret;
}

/**
 * \brief AWS IoT authenticates Things using X.509 certificates.
 * In addition to it, all traffic to and from AWS IoT MUST be encrypted over Transport Layer Security(TLS).
 * So the WolfSSL library is responsible for the TLS layer.
 *
 * \param kit[in]             Pointer to an instance of AWS Kit
 * \return AWS_E_SUCCESS      On success
 */
int aws_client_net_tls_cb(t_aws_kit* kit)
{
	int ret = AWS_E_SUCCESS;
	uint8_t* cert_chain = NULL;

	do {

#ifdef AWS_KIT_DEBUG
		wolfSSL_Debugging_ON();
#else
		wolfSSL_Debugging_OFF();
		delay_ms(500);
#endif
		/* Initialize SSL context. */
		kit->tls.context = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
		if (kit->tls.context == NULL) {
			AWS_ERROR("Failed to init context!");
			break;
		}

		/* ATECC508A supports ECC(ECDH, ECDSA) hardware acceleration. 
		   ECDHE-ECDSA-AES128-GCM-SHA256 cipher suite recommends by AWS IoT is set for authentication and data encryption. */
		ret = wolfSSL_CTX_set_cipher_list(kit->tls.context, AWS_IOT_CIPHER_SPEC);
		if (ret != SSL_SUCCESS) {
			AWS_ERROR("Failed to set cipher!");
			break;
		}

		/* Since AWS IoT server was signed by the VeriSign root CA, this root CA certificate should be loaded to WolfSSL to verify AWS ioT. */
		ret = wolfSSL_CTX_load_verify_buffer(kit->tls.context, AWS_IOT_ROOT_CERT, sizeof(AWS_IOT_ROOT_CERT), SSL_FILETYPE_PEM);
		if (ret != SSL_SUCCESS) {
			AWS_ERROR("Failed to set root cert!");
			break;
		}

		/* As the AT88CKECCSIGNER already signed ATECC508A of Thing, There are the Signer and Device certificates in the ATECC508A. 
		Both certificates should be set to WolfSSL for the JITR achievement. */
		cert_chain = (uint8_t*)malloc(kit->cert.signerCertLen + kit->cert.devCertLen);
		memcpy(&cert_chain[0], kit->cert.devCert, kit->cert.devCertLen);
		memcpy(&cert_chain[kit->cert.devCertLen], kit->cert.signerCert, kit->cert.signerCertLen);
		ret = wolfSSL_CTX_use_certificate_chain_buffer(kit->tls.context, cert_chain, kit->cert.signerCertLen + kit->cert.devCertLen);
		if (ret != SSL_SUCCESS) {
			AWS_ERROR("Failed to set cert chain!");
			break;
		}

		/* This Device private key is not actual private key of ATECC508A, and WolfSSL never use it as own Device key. 
		   This temporary key has been set to make sure Device owns a private key. */
		ret = wolfSSL_CTX_use_PrivateKey_buffer(kit->tls.context, AWS_TEMP_DEV_KEY, sizeof(AWS_TEMP_DEV_KEY), SSL_FILETYPE_PEM);
		if (ret != SSL_SUCCESS) {
			AWS_ERROR("Failed to set fake key!");
			break;
		}

		/* Turn on a certificate request from the server to the client. */
		wolfSSL_CTX_set_verify(kit->tls.context, SSL_VERIFY_PEER, NULL);
		/* Set the Public key Callback for ECC Signing. */
		wolfSSL_CTX_SetEccSignCb(kit->tls.context, atca_tls_sign_certificate_cb);
		/* Set the Public key Callback for ECC Verification. */
		wolfSSL_CTX_SetEccVerifyCb(kit->tls.context, atca_tls_verify_signature_cb);
		/* Set the Public key Callback for Pre-Master Secret creation. */
		wolfSSL_CTX_SetEccPmsCb(kit->tls.context, atca_tls_create_pms_cb);

	} while(0);

	if (cert_chain)
		free(cert_chain);

	return ret;
}

/**
 * \brief Subscribe the Delta topic if a difference is detected between the Reported and Desired sections of a Thing shadow.
 * aws_client_mqtt_msg_cb will be called, if there is a difference. 
 *
 * \param kit[in]             Pointer to an instance of AWS Kit
 * \return AWS_E_SUCCESS      On success
 */
int aws_client_mqtt_subscribe(t_aws_kit* kit)
{
	int ret = AWS_E_FAILURE;
	
	/* Subscribe the Delta topic. */
	snprintf((char*)kit->topic.updateDeltaTopic, sizeof(kit->topic.updateDeltaTopic), AWS_IOT_UPDATE_DELTA_TOPIC, kit->user.thing);

	ret = MQTTSubscribe(&kit->client, (const char*)kit->topic.updateDeltaTopic, QOS0, aws_client_mqtt_message_cb);
	if (ret != SUCCESS) {
		AWS_ERROR("Failed to subscribe delta topic!(%d)", ret);
	}

	return ret;
}

/**
 * \brief Send Thing's current state to the Thing Shadow service by sending an MQTT message to the Update topic.
 * Insight GUI also will subscribe the Delta topic to synchronize Thing Shadow with Thing.
 *
 * \param kit[in]             Pointer to an instance of AWS Kit
 * \return AWS_E_SUCCESS      On success
 */
int aws_client_mqtt_publish(t_aws_kit* kit)
{
	int ret = AWS_E_FAILURE;
	char pubMsg[AWS_MQTT_PAYLOAD_MAX * 2];
	MQTTMessage message;
	
	snprintf((char*)kit->topic.updateTopic, sizeof(kit->topic.updateTopic), AWS_IOT_UPDATE_TOPIC, kit->user.thing);
	message.qos = QOS0;
	message.retained = 0;
	message.dup = 0;
	message.id = aws_client_mqtt_packet_id();
	
	/* Publish LEDs state. */
	sprintf(pubMsg, AWS_IOT_LED_PUB_MESSAGE, kit->led.state[AWS_KIT_LED_1] ? "on" : "off",
			kit->led.state[AWS_KIT_LED_2] ? "on" : "off", kit->led.state[AWS_KIT_LED_3] ? "on" : "off");
	message.payload = (void*)pubMsg;
	message.payloadlen = strlen((char*)message.payload);

	ret= MQTTPublish(&kit->client, (const char*)kit->topic.updateTopic, &message);
	if (ret != SUCCESS) {
		AWS_ERROR("Failed to publish the update topic(%d)", ret);
		return AWS_E_CLI_PUB_FAILURE;
	}
	
#ifdef AWS_KIT_DEBUG
	AWS_INFO("Published LED Message : %s", pubMsg);
#else
	AWS_INFO("Published LED Message");
#endif
	
	/* Publish buttons state. */
	sprintf(pubMsg, AWS_IOT_BUT_PUB_MESSAGE, kit->button.state[AWS_KIT_BUTTON_1] ? "down" : "up",
			kit->button.state[AWS_KIT_BUTTON_2] ? "down" : "up", kit->button.state[AWS_KIT_BUTTON_3] ? "down" : "up");
	message.payload = (void*)pubMsg;
	message.payloadlen = strlen((char*)message.payload);

	ret= MQTTPublish(&kit->client, (const char*)kit->topic.updateTopic, &message);
	if (ret != SUCCESS) {
		AWS_ERROR("Failed to publish the update topic(%d)", ret);
		return AWS_E_CLI_PUB_FAILURE;
	}
	
#ifdef AWS_KIT_DEBUG
	AWS_INFO("Published BUTTON Message : %s", pubMsg);
#else
	AWS_INFO("Published BUTTON Message");
#endif

	return ret;
}

/**
 * \brief Subscribe the Delta topic if a difference is detected between the Reported and Desired sections of a Thing shadow.
 * aws_client_mqtt_msg_cb will be called, if there is a difference. 
 *
 * \param data[in]            Pointer to an message data
 * \return AWS_E_SUCCESS      On success
 */
void aws_client_mqtt_message_cb(MessageData* data)
{
	int ret = AWS_E_SUCCESS;
	t_aws_kit* kit = aws_kit_get_instance();

#ifdef AWS_KIT_DEBUG
	AWS_INFO("Subscribed Topic = %s\r\nMessage = %s", data->topicName->lenstring.data, data->message->payload);
#else
	AWS_INFO("Subscribed delta topic");
#endif
	/* Check if a topic name is same with "$aws/things/%s/shadow/update/delta".
	   Turn a specified LED on/off. */
	if(strncmp((const char*)kit->topic.updateDeltaTopic, data->topicName->lenstring.data, 
				strlen((const char*)kit->topic.updateDeltaTopic)) == 0) {

		char reportedMsg[AWS_MQTT_PAYLOAD_MAX];
		char intBuf[0], desiredBuf[16], reportedBuf[32];
		char* serializedStr = NULL;
		JSON_Value* jPubVal = NULL;
		JSON_Value* jSubVal = json_parse_string((const char*)data->message->payload);
		JSON_Object* jObject = json_value_get_object(jSubVal);

		for (uint8_t i = AWS_KIT_LED_1; i < AWS_KIT_LED_MAX; i++) {
			strcpy((char*)desiredBuf, (const char*)"state.led");
			strcat((char*)desiredBuf, (const char*)itoa(i + 1, (char*)intBuf, 10));
			if(json_object_dotget_string(jObject, (const char*)desiredBuf) != NULL) {
				kit->led.isDesired[i] = true;
				if(strcmp(json_object_dotget_string(jObject, (const char*)desiredBuf), "on") == 0) {
					kit->led.state[i] = true;
					kit->led.turn_on((uint8_t)i + 1);
				} else if (strcmp(json_object_dotget_string(jObject, (const char*)desiredBuf), "off") == 0) {
					kit->led.state[i] = false;
					kit->led.turn_off((uint8_t)i + 1);
				}
			}
		}

		jPubVal = json_value_init_object();
		jObject = json_value_get_object(jPubVal);

		for (uint8_t i = AWS_KIT_LED_1; i < AWS_KIT_LED_MAX; i++) {
			strcpy(reportedBuf, (const char*)"state.reported.led");
			strcat(reportedBuf, (const char*)itoa(i + 1, (char*)intBuf, 10));
			if(kit->led.isDesired[i]) {
				json_object_dotset_string(jObject, (const char*)reportedBuf, kit->led.state[i] ? "on" : "off");
			}
			kit->led.isDesired[i] = false;
		}

		serializedStr = json_serialize_to_string((const JSON_Value *)jPubVal);
		strcpy(reportedMsg, (const char*)serializedStr);
		json_free_serialized_string(serializedStr);

		MQTTMessage message;
		message.retained = 0;
		message.qos = QOS0;
		message.dup = 0;
		message.id = aws_client_mqtt_packet_id();
		message.payload = (void*)reportedMsg;
		message.payloadlen = strlen((char*)message.payload);
				
		ret = MQTTPublish(&kit->client, (const char*)kit->topic.updateTopic, &message);
		if (ret != SUCCESS) {
			AWS_ERROR("Failed to publish update topic!(%d)", ret);
			ret = AWS_E_CLI_PUB_FAILURE;
		}

		json_value_free(jSubVal);
		json_value_free(jPubVal);
    }  

	return ret;	
}

/**
 * \brief If a user makes change LEDs state using Insight GUI, it will send the Update topic to AWS broker.
 * Then AWS IoT also will send the Delta topic for Thing to read it.
 * 
 * \param kit[inout]          Pointer to an instance of AWS Kit
 * \return AWS_E_SUCCESS      On success
 */
int aws_client_mqtt_wait_msg(t_aws_kit* kit)
{
	int ret = AWS_E_SUCCESS;

	/* Wait for Publish packets to arrive. */
	ret = MQTTYield(&kit->client, AWS_MQTT_CMD_TIMEOUT_MS);
	if (ret == SUCCESS) {
		/* Check for a button state of OLED1 board. */
		if (aws_client_scan_button(kit)) {
			
			char* serializedStr = NULL;
			char reportedMsg[AWS_MQTT_PAYLOAD_MAX], intBuf[0], reportedBuf[32] = "state.reported.button";
			JSON_Value* jPubVal = json_value_init_object();
			JSON_Object* jObject = json_value_get_object(jPubVal);

			for (uint8_t i = AWS_KIT_BUTTON_1; i < AWS_KIT_BUTTON_MAX; i++) {
				if (kit->button.isPressed[i]) {
					kit->button.isPressed[i] = false;
					strcat((char*)reportedBuf, (const char*)itoa(i + 1, (char*)intBuf, 10));
					json_object_dotset_string(jObject, (const char*)reportedBuf, (kit->button.state[i] ? "up" : "down"));
					/* Save button state to toggle. */
					kit->button.state[i] = kit->button.state[i] ? false : true;
					ret = atcab_write_bytes_zone(ATCA_ZONE_DATA, TLS_SLOT8_ENC_STORE, AWS_USER_DATA_BUTTON_STATE, kit->button.state, 4);
					if (ret != ATCA_SUCCESS) {
						AWS_ERROR("Failed to write button state!(%d)", ret);
						ret = AWS_E_CRYPTO_FAILURE;
					}
					break;
				}
			}

			serializedStr = json_serialize_to_string((const JSON_Value *)jPubVal);
			strcpy(reportedMsg, (const char*)serializedStr);
			json_free_serialized_string(serializedStr);

			MQTTMessage message;
			message.retained = 0;
			message.qos = QOS0;
			message.dup = 0;
			message.id = aws_client_mqtt_packet_id();
			message.payload = (void*)reportedMsg;
			message.payloadlen = strlen((char*)message.payload);
			kit->nonBlocking = true;
		
			ret = MQTTPublish(&kit->client, (const char*)kit->topic.updateTopic, &message);
			if (ret != SUCCESS) {
				AWS_ERROR("Failed to publish update topic!(%d)", ret);
				ret = AWS_E_CLI_PUB_FAILURE;
			}

			kit->nonBlocking = false;

			json_value_free(jPubVal);		
		}		
	}

	return ret;
}

/**
 * \brief State machine for AWS MQTT client.
 * 
 * \param kit[inout]          Pointer to an instance of AWS Kit
 * \return AWS_E_SUCCESS      On success
 */
void aws_client_state_machine(t_aws_kit* kit)
{
	int ret = AWS_E_SUCCESS;
	static bool errorNoti = false;
	static uint8_t retryDelay = 0;
	static uint8_t currState = CLIENT_STATE_INIT_MQTT_CLIENT;
	uint8_t nextState = CLIENT_STATE_INVALID;

	switch (currState)
	{
		case CLIENT_STATE_INIT_MQTT_CLIENT:
		{
			/* After initializing Paho MQTT, try to connect to host address. */
			ret = aws_client_init_mqtt_client(kit);
			if (ret == AWS_E_SUCCESS) {
				retryDelay = 0;
				kit->errState = AWS_EX_NONE;
				errorNoti = false;
				for (uint8_t i = 0; i < AWS_KIT_LED_MAX; i++) {
					kit->led.turn_off((uint8_t)i + 1);
				}
				
				/* Since Thing is ready to communicate securely with AWS IoT, go to next state. */ 
				nextState = CLIENT_STATE_MQTT_SUBSCRIBE;
			/* For the Just In Time Registration, hold on seconds, and retry */
			} else if (ret == AWS_E_NET_JITR_RETRY || AWS_E_NET_TLS_FAILURE) {
				retryDelay += 2;
				if (retryDelay > 120) {
					AWS_ERROR("Failed to exceed the limited 2 minutes to wait for finishing lambda");
					aws_kit_software_reset();
				}
				delay_s((unsigned long)retryDelay);
				nextState = CLIENT_STATE_INIT_MQTT_CLIENT;
			} else {
				if (!errorNoti) {
					errorNoti = true;
					kit->errState = AWS_EX_MQTT_FAILURE;
					aws_user_exception_init_timer(kit);
				}
				nextState = CLIENT_STATE_INIT_MQTT_CLIENT;
			}
		}
		break;

		case CLIENT_STATE_MQTT_SUBSCRIBE:
		{
			/* Subscribe the Delta topic. */
			ret = aws_client_mqtt_subscribe(kit);
			if (ret != AWS_E_SUCCESS) {
				if (!errorNoti) {
					errorNoti = true;
					kit->errState = AWS_EX_MQTT_FAILURE;
					aws_user_exception_init_timer(kit);
				}
				nextState = CLIENT_STATE_MQTT_SUBSCRIBE;
			} else {
				kit->errState = AWS_EX_NONE;
				errorNoti = false;
				nextState = CLIENT_STATE_MQTT_PUBLISH;
			}
		}
		break;

		case CLIENT_STATE_MQTT_PUBLISH:
		{
			/* Publish the Update topic. */
			ret = aws_client_mqtt_publish(kit);
			if (ret != AWS_E_SUCCESS) {
				if (!errorNoti) {
					errorNoti = true;
					kit->errState = AWS_EX_MQTT_FAILURE;
					aws_user_exception_init_timer(kit);
				}
				nextState = CLIENT_STATE_MQTT_PUBLISH;
			} else {
				kit->errState = AWS_EX_NONE;
				errorNoti = false;
				nextState = CLIENT_STATE_MQTT_WAIT_MESSAGE;
			}
		}
		break;

		case CLIENT_STATE_MQTT_WAIT_MESSAGE:
		{
			if (kit->quitMQTT) {
				ret = aws_client_mqtt_disconnect(kit);
				if (ret != AWS_E_SUCCESS) {
					if (!errorNoti) {
						errorNoti = true;
						kit->errState = AWS_EX_MQTT_FAILURE;
						aws_user_exception_init_timer(kit);
					}
					nextState = CLIENT_STATE_INVALID;
				} else {
					kit->quitMQTT = false;
					nextState = CLIENT_STATE_INIT_MQTT_CLIENT;
					vTaskSuspend(clientTaskHandler);
				}
			} else {
				ret = aws_client_mqtt_wait_msg(kit);
				/* Set infinite loop to get the Delta topic. */
				if (ret == AWS_E_SUCCESS) {
					nextState = CLIENT_STATE_MQTT_WAIT_MESSAGE;
				} else {
					if (!errorNoti) {
						errorNoti = true;
						kit->errState = AWS_EX_MQTT_FAILURE;
						aws_user_exception_init_timer(kit);
					}
					nextState = CLIENT_STATE_INVALID;
				}
			}
		}
		break;

		case CLIENT_STATE_INVALID:
		{
			AWS_ERROR("Invalid client state");
			nextState = CLIENT_STATE_INVALID;
		}
		break;

		default:
			break;
	}

	currState = kit->clientState = nextState;
}

/**
 * \brief This Client task runs MQTT client application based on MQTT library under SSL/TLS library which is WolfSSL.
 * Before getting start this task, ATECC508 should be provisioned for certificate assignment to WolfSSL library.
 * This task is basically to implement the AWS JITR feature.(Just In Time Registation.)
 * Each connected device needs a credential to access the Thing Shadow service. All traffic to and from AWS IoT
 * MUST be kept safe in order to send data securely to the message broker.
 *
 * \param[in] params          Parameters for the task (Not used.)
 */
void aws_client_task(void* params)
{
	t_aws_kit* kit = aws_kit_get_instance();

	for (;;) {

		/* Run state machine for Client task. */
		aws_client_state_machine(kit);

		/* Block for 100ms. */
		vTaskDelay(AWS_CLIENT_TASK_DELAY);
	}
}
