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
 * \brief Subscribe the Delta topic if a difference is detected between the Reported and Desired sections of a Thing shadow.
 * aws_client_mqtt_msg_cb will be called, if there is a difference. 
 *
 * \param kit[in]             Pointer to an instance of AWS Kit
 * \return AWS_E_SUCCESS      On success
 */
int aws_client_mqtt_msg_cb(MqttClient* mqttCli, MqttMessage* msg, uint8_t new, uint8_t done)
{
	int ret = AWS_E_SUCCESS;
	t_aws_kit* kit = aws_kit_get_instance();

	if (!mqttCli || !msg) return AWS_E_CLI_SUB_FAILURE;

#ifdef AWS_KIT_DEBUG
	AWS_INFO("Subscribed Topic = %s\r\nMessage = %s", msg->topic_name, msg->buffer);
#else
	AWS_INFO("Subscribed delta topic");
#endif
	/* Check if a topic name is same with "$aws/things/%s/shadow/update/delta".
	   Turn a specified LED on/off. */
	if(strncmp((const char*)kit->topic.updateDeltaTopic, (const char*)msg->topic_name, 
				strlen((const char*)kit->topic.updateDeltaTopic)) == 0) {

		char reportedMsg[AWS_MQTT_PAYLOAD_MAX];
		char intBuf[0], desiredBuf[16], reportedBuf[32];
		char* serializedStr = NULL;
		JSON_Value* jPubVal = NULL;
		JSON_Value* jSubVal = json_parse_string((const char*)msg->buffer);
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

		MqttPublish publish;
		memset(&publish, 0, sizeof(MqttPublish));
		publish.retain = 0;
		publish.qos = 0;
		publish.duplicate = 0;
		publish.topic_name = (const char*)kit->topic.updateTopic;
		publish.packet_id = aws_client_mqtt_packet_id();
		publish.buffer = (byte*)reportedMsg;
		publish.total_len = strlen((char*)publish.buffer);
		ret = MqttClient_Publish(&kit->client, &publish);
		if (ret != MQTT_CODE_SUCCESS) {
			AWS_ERROR("Failed to publish update topic!(%d)", ret);
			ret = AWS_E_CLI_PUB_FAILURE;
		}

		json_value_free(jSubVal);
		json_value_free(jPubVal);

    }  

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
int aws_client_tls_cb(MqttClient* mqttCli)
{
	int ret = AWS_E_SUCCESS;
	uint8_t* cert_chain = NULL;
	t_aws_kit* kit = aws_kit_get_instance();

	do {

#ifdef AWS_KIT_DEBUG
		wolfSSL_Debugging_ON();
#else
		wolfSSL_Debugging_OFF();
		delay_ms(500);
#endif
		/* Initialize SSL context. */
		mqttCli->tls.ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
		if (mqttCli->tls.ctx == NULL) {
			AWS_ERROR("Failed to init context!");
			break;
		}

		/* ATECC508A supports ECC(ECDH, ECDSA) hardware acceleration. 
		   ECDHE-ECDSA-AES128-GCM-SHA256 cipher suite recommende by AWS IoT is set for authentication and data encryption. */
		ret = wolfSSL_CTX_set_cipher_list(mqttCli->tls.ctx, AWS_IOT_CIPHER_SPEC);
		if (ret != SSL_SUCCESS) {
			AWS_ERROR("Failed to set cipher!");
			break;
		}

		/* Since AWS IoT server was signed by the VeriSign root CA, this root CA certificate should be loaded to WolfSSL to verify AWS ioT. */
		ret = wolfSSL_CTX_load_verify_buffer(mqttCli->tls.ctx, AWS_IOT_ROOT_CERT, sizeof(AWS_IOT_ROOT_CERT), SSL_FILETYPE_PEM);
		if (ret != SSL_SUCCESS) {
			AWS_ERROR("Failed to set root cert!");
			break;
		}

		/* As the AT88CKECCSIGNER already signed ATECC508A of Thing, There are the Signer and Device certifictes in the ATECC508A. 
		Both certificates should be set to WolfSSL for the JITR achievement. */
		cert_chain = (uint8_t*)malloc(kit->cert.signerCertLen + kit->cert.devCertLen);
		memcpy(&cert_chain[0], kit->cert.devCert, kit->cert.devCertLen);
		memcpy(&cert_chain[kit->cert.devCertLen], kit->cert.signerCert, kit->cert.signerCertLen);
		ret = wolfSSL_CTX_use_certificate_chain_buffer(mqttCli->tls.ctx, cert_chain, kit->cert.signerCertLen + kit->cert.devCertLen);
		if (ret != SSL_SUCCESS) {
			AWS_ERROR("Failed to set cert chain!");
			break;
		}

		/* This Device private key is not actual private key of ATECC508A, and WolfSSL never use it as own Device key. 
		   This temporaty key has been set to make sure Device owns a private key. */
		ret = wolfSSL_CTX_use_PrivateKey_buffer(mqttCli->tls.ctx, AWS_TEMP_DEV_KEY, sizeof(AWS_TEMP_DEV_KEY), SSL_FILETYPE_PEM);
		if (ret != SSL_SUCCESS) {
			AWS_ERROR("Failed to set fake key!");
			break;
		}

		/* Turn on a certificate request from the server to the client. */
		wolfSSL_CTX_set_verify(mqttCli->tls.ctx, SSL_VERIFY_PEER, NULL);
		/* Set the Public key Callback for ECC Signing. */
		wolfSSL_CTX_SetEccSignCb(mqttCli->tls.ctx, atca_tls_sign_certificate_cb);
		/* Set the Public key Callback for ECC Verification. */
		wolfSSL_CTX_SetEccVerifyCb(mqttCli->tls.ctx, atca_tls_verify_signature_cb);
		/* Set the Public key Callback for Pre-Master Secret creation. */
		wolfSSL_CTX_SetEccPmsCb(mqttCli->tls.ctx, atca_tls_create_pms_cb);

	} while(0);

	if (cert_chain) free(cert_chain);

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
	MqttUnsubscribe mqttUnSub;
	MqttTopic topics[1];

	do {

		if (&kit->client == NULL) {
			AWS_ERROR("Error(%d) : Invaild param!", ret);
			return ret;
		}

		snprintf((char*)kit->topic.updateDeltaTopic, sizeof(kit->topic.updateDeltaTopic), AWS_IOT_UPDATE_DELTA_TOPIC, kit->user.thing);
		topics[0].topic_filter = (const char*)kit->topic.updateDeltaTopic;
		topics[0].qos = MQTT_QOS_0;

		memset(&mqttUnSub, 0, sizeof(MqttSubscribe));
		mqttUnSub.packet_id = aws_client_mqtt_packet_id();
		mqttUnSub.topic_count = sizeof(topics)/sizeof(MqttTopic);
		mqttUnSub.topics = topics;
		kit->blocking = true;
		kit->nonBlocking = true;
		/* Unsubscribe the Update topic anymore. */
		ret = MqttClient_Unsubscribe(&kit->client, &mqttUnSub);
		if (ret != MQTT_CODE_SUCCESS) {
			AWS_ERROR("Error(%d) : Failed to unsubscribe delta topic!", ret);
			kit->blocking = false;
			kit->nonBlocking = false;
			break;
		}
		kit->blocking = false;
		kit->nonBlocking = false;

		/* Send the disconnect MQTT packet. */
		ret = MqttClient_Disconnect(&kit->client);
		if (ret != MQTT_CODE_SUCCESS) {
			AWS_ERROR("Error(%d) : Failed to send disconnection packet!", ret);
			break;
		}

		/* Disconnect from AWS MQTT broker. */
		ret = MqttClient_NetDisconnect(&kit->client);
		if (ret != MQTT_CODE_SUCCESS) {
			AWS_ERROR("Error(%d) : Failed to disconnect with Host!", ret);
			break;
		}

		if (kit->net.context) {
			free(kit->net.context);
			kit->net.context = NULL;
		}

	} while(0);

	return ret;
}

/**
 * \brief This function initializes WolfMQTT client, and tries to connect to destination address depending on a user AWS account.
 * The host address should be installed in ATECC508A through Insight GUI.
 *
 * \param kit[inout]          Pointer to an instance of AWS Kit
 * \return AWS_E_SUCCESS      On success
 */
int aws_client_init_mqtt_client(t_aws_kit* kit)
{
	int ret = AWS_E_FAILURE;
	MqttConnect mqttCon;
	MqttMessage mqttMsg;

	do {

		/* Set callbacks to be communicated with ATWINC1500. */
		kit->net.connect = aws_net_connect_cb;
		kit->net.read = aws_net_receive_packet_cb;
		kit->net.write = aws_net_send_packet_cb;
		kit->net.disconnect = aws_net_disconnect_cb;

		if (kit->net.context == NULL)
			kit->net.context = malloc(sizeof(SOCKET));
		if (kit->net.context == NULL) { 
			AWS_ERROR("Failed to allocate heap!");
			break;
		}

		/* Initialize MQTT client object. */
		memset(kit->net.context, 0, sizeof(SOCKET));
		ret = MqttClient_Init(&kit->client, &kit->net, aws_client_mqtt_msg_cb, kit->buffer.mqttTxBuf, sizeof(kit->buffer.mqttTxBuf), 
						kit->buffer.mqttRxBuf, sizeof(kit->buffer.mqttRxBuf),  AWS_MQTT_CMD_TIMEOUT_MS);
		if (ret != MQTT_CODE_SUCCESS) {
			AWS_ERROR("Error(%d) : Failed to initialize MQTT client!", ret);
			break;
		}

		/* Connect to AWS IoT over TLS handshaking with ECDHE-ECDSA-AES128-GCM-SHA256 cipher suite. 
		   If TLS connection would be failed by AWS ioT during JITR, then return normal failure for the retry. */
		ret = MqttClient_NetConnect(&kit->client, (const char *)kit->user.host, AWS_IOT_MQTT_PORT, 
						AWS_NET_CONN_TIMEOUT_MS, TRUE, aws_client_tls_cb);
		if (ret != MQTT_CODE_SUCCESS) {
			AWS_ERROR("Error(%d) : Failed to connect to Host!", ret);
			if (ret == MQTT_CODE_ERROR_NETWORK) 
				ret = AWS_E_NET_TLS_FAILURE;
			break;
		}

		/* If both JITR and TLS session establishment have been successfully made, Send MQTT CONN packet. */
		memset(&mqttCon, 0, sizeof(MqttConnect));
		memset(&mqttMsg, 0, sizeof(MqttMessage));
		mqttCon.keep_alive_sec = AWS_IOT_KEEP_ALIVE_SEC;
		mqttCon.clean_session = 1;
		/* Client ID represents a serial number of ATECC508A. */
		mqttCon.client_id = (const char*)kit->user.clientID;
		mqttCon.lwt_msg = &mqttMsg;
		ret = MqttClient_Connect(&kit->client, &mqttCon);
		if (ret != MQTT_CODE_SUCCESS) {
			AWS_ERROR("Error(%d) : Failed to receive CONNACK!", ret);
			if (ret == MQTT_CODE_ERROR_NETWORK || ret == MQTT_CODE_ERROR_TIMEOUT) 
				ret = AWS_E_NET_JITR_RETRY;
			break;
		}

		/* According to AWS message broker requirements, by default, MQTT client connection is disconnected 
		   after 30 minutes of inactivity. When the client sends a PUBLISH, SUBSCRIBE, PING, or PUBACK message, 
		   the inactivity timer is reset. The keep-alive timer begins immediately after the server returns 
		   the client's sending of a CONNECT message and start of keep-alive behavior. 
		   For this AWS Kit Things, the keep-alive timer is initialized to 20 minutes, and then count down to zero seconds. */
		aws_kit_init_timer(&kit->keepAlive);
		aws_kit_countdown_sec(&kit->keepAlive, AWS_IOT_KEEP_ALIVE_SEC);

	} while(0);

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
	MqttSubscribe mqttSub;
	MqttTopic topics[1];

	snprintf((char*)kit->topic.updateDeltaTopic, sizeof(kit->topic.updateDeltaTopic), AWS_IOT_UPDATE_DELTA_TOPIC, kit->user.thing);
	topics[0].topic_filter = (const char*)kit->topic.updateDeltaTopic;
	topics[0].qos = MQTT_QOS_0;

	/* Subscribe the Delta topic. */
	memset(&mqttSub, 0, sizeof(MqttSubscribe));
	mqttSub.packet_id = aws_client_mqtt_packet_id();
	mqttSub.topic_count = sizeof(topics)/sizeof(MqttTopic);
	mqttSub.topics = topics;
	ret = MqttClient_Subscribe(&kit->client, &mqttSub);
	if (ret != MQTT_CODE_SUCCESS) {
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
	MqttPublish mqttPub;
	char pubMsg[AWS_MQTT_PAYLOAD_MAX * 2];	

	snprintf((char*)kit->topic.updateTopic, sizeof(kit->topic.updateTopic), AWS_IOT_UPDATE_TOPIC, kit->user.thing);
	mqttPub.retain = 0;
	mqttPub.qos = MQTT_QOS_0;
	mqttPub.duplicate = 0;
	mqttPub.topic_name = (const char*)kit->topic.updateTopic;
	mqttPub.packet_id = aws_client_mqtt_packet_id();
	sprintf(pubMsg, AWS_IOT_LED_PUB_MESSAGE, kit->led.state[AWS_KIT_LED_1] ? "on" : "off", 
			kit->led.state[AWS_KIT_LED_2] ? "on" : "off", kit->led.state[AWS_KIT_LED_3] ? "on" : "off");
	mqttPub.buffer = (byte *)pubMsg;
	mqttPub.total_len = strlen((char *)mqttPub.buffer);
	/* Publish LEDs state. */
	ret = MqttClient_Publish(&kit->client, &mqttPub);
	if (ret != MQTT_CODE_SUCCESS) {
		AWS_ERROR("Failed to publish the update topic(%d)", ret);
		return AWS_E_CLI_PUB_FAILURE;
	}
#ifdef AWS_KIT_DEBUG
	AWS_INFO("Published LED Message : %s", pubMsg);
#else
	AWS_INFO("Published LED Message");
#endif

	sprintf(pubMsg, AWS_IOT_BUT_PUB_MESSAGE, kit->button.state[AWS_KIT_BUTTON_1] ? "down" : "up",
			kit->button.state[AWS_KIT_BUTTON_2] ? "down" : "up", kit->button.state[AWS_KIT_BUTTON_3] ? "down" : "up");
	mqttPub.buffer = (byte*)pubMsg;
	mqttPub.total_len = strlen((char*)mqttPub.buffer);
	/* Publish buttons state. */
	ret = MqttClient_Publish(&kit->client, &mqttPub);
	if (ret != MQTT_CODE_SUCCESS) {
		AWS_ERROR("Failed to publish the update topic(%d)", ret);
		ret = AWS_E_CLI_PUB_FAILURE;
	}
#ifdef AWS_KIT_DEBUG
	AWS_INFO("Published BUTTON Message : %s", pubMsg);
#else
	AWS_INFO("Published BUTTON Message");
#endif

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
	int ret = AWS_E_FAILURE;

	/* Send PING packet to maintain connection with AWS Broker, if 20 minutes timer expired. */
	if (aws_kit_timer_expired(&kit->keepAlive)) {
		kit->nonBlocking = true;
		ret = MqttClient_Ping(&kit->client);
		if (ret != MQTT_CODE_SUCCESS) {
			AWS_ERROR("Failed to send PING packet!(%d)", ret);
			return ret;
		}
		kit->nonBlocking = false;
		/* Count down keep-alive timer again. */
		aws_kit_init_timer(&kit->keepAlive);
		aws_kit_countdown_sec(&kit->keepAlive, AWS_IOT_KEEP_ALIVE_SEC);
	}

	/* Wait for Publish packets to arrive. */
	ret = MqttClient_WaitMessage(&kit->client, AWS_MQTT_CMD_TIMEOUT_MS);
	if (ret == MQTT_CODE_ERROR_TIMEOUT) {
		AWS_INFO("Polling message to subscribe");
		/* Check for a button state of OLED1 board. */
		if (aws_client_scan_button(kit)) {

			char* serializedStr = NULL;
			char reportedMsg[AWS_MQTT_PAYLOAD_MAX], intBuf[0], reportedBuf[32] = "state.reported.button";
			JSON_Value* jPubVal = json_value_init_object();
			JSON_Object* jObject = json_value_get_object(jPubVal);
			MqttPublish publish;

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

			memset(&publish, 0, sizeof(MqttPublish));
			publish.retain = 0;
			publish.qos = 0;
			publish.duplicate = 0;
			publish.topic_name = (const char*)kit->topic.updateTopic;
			publish.packet_id = aws_client_mqtt_packet_id();
			publish.buffer = (byte*)reportedMsg;
			publish.total_len = strlen((char*)publish.buffer);
			kit->nonBlocking = true;
			/* Encode and send the MQTT Publish packet with a button state. */
			ret = MqttClient_Publish(&kit->client, &publish);
			if (ret != MQTT_CODE_SUCCESS) {
				AWS_ERROR("Failed to publish the update topic!(%d)", ret);
			}
			kit->nonBlocking = false;
			json_value_free(jPubVal);
#ifdef AWS_KIT_DEBUG
			AWS_INFO("Published Topic = %s\r\nMessage = %s", publish.topic_name, publish.buffer);
#else
			AWS_INFO("Published the update topic");
#endif
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
			/* After initializing WolfMQTT, try to connect to host address. */
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
			/* For the Just In Time Regstration, hold on seconds, and retry */
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
				if (ret == MQTT_CODE_ERROR_TIMEOUT || ret == AWS_E_SUCCESS) {
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
 * \brief This Client task runs MQTT client application based on WolfMQTT library under SSL/TLS library which is WolfSSL.
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
