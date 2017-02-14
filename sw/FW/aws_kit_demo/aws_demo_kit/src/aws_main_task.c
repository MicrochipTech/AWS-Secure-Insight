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

#include "aws_client_task.h"
#include "aws_user_task.h"
#include "aws_prov_task.h"
#include "aws_main_task.h"
#include "aws_net_interface.h"
#include "aws_kit_debug.h"
#include "cryptoauthlib.h"
#include "tls/atcatls_cfg.h"
#include "atecc508cb.h"

//! Handle for about Main task
xTaskHandle mainTaskHandler;
//! Handle for about Provisioning task
xTaskHandle provTaskHandler;
//! Handle for about User task
xTaskHandle userTaskHandler;
//! Handle for about Client task
xTaskHandle clientTaskHandler;

/**
 * \brief Notification receiver from provisioning task.
 *
 * Wait for a notification which means provisioning has been completed.
 * Once provisioning task gets 'aws:sw' command from Insight GUI,
 * Then the notification is queued.
 *
 * \param kit[inout]        Pointer to an instance of AWS Kit
 * \return 0                on success
 */
int aws_main_wait_notification(t_aws_kit* kit)
{
	int ret = AWS_E_FAILURE;
	uint8_t queueBuf[1];

	/* Check if there is queued data or not. */
	while (xQueueReceive(kit->notiQueue, queueBuf, 0)) {

		if (queueBuf[0] == NOTI_RUN_MQTT_CLIENT) {
			ret = AWS_E_SUCCESS;
		} else if (queueBuf[0] == NOTI_QUIT_MQTT_CLIENT) {
			kit->quitMQTT = true;
			ret = AWS_E_MQTT_REINITIALIZE;
		}
		AWS_INFO("aws_main_wait_notification[%d][%d]", kit->mainState, ret);
	}

	return ret;
}

/**
 * \brief Build certificates.
 *
 * Now that both signer and device certificate was saved onto specified slot with certificate definition,
 * Both certificates can be built based on certificate definition in order to pass them to TLS library.
 *
 * \param kit[inout]        Pointer to an instance of AWS Kit
 * \return AWS_E_SUCCESS    On success
 */
int aws_main_init_kit(t_aws_kit* kit)
{
	int ret = AWS_E_FAILURE;
	bool lockstate = false;
	uint8_t revision[INFO_SIZE];

	do {

		kit->errState = AWS_EX_NONE;

		/* Initialize CryptoAuthLib to communicate with ATECC508A over I2C interface. */
		cfg_ateccx08a_i2c_default.atcai2c.slave_address = DEVICE_I2C;
		atcab_init( &cfg_ateccx08a_i2c_default );

		/* Check for personalization state of configuration & data zone. */
		ret = atcab_is_locked(LOCK_ZONE_CONFIG, &lockstate);
		if (ret != ATCA_SUCCESS) {
			AWS_ERROR("Failed to read config zone of ATECC508(%d)", ret);
			ret = AWS_E_CRYPTO_FAILURE;
			break;
		}
		if (!lockstate)	{
			AWS_ERROR("Un-Provisioned ATECC508");
			ret = AWS_E_CRYPTO_FAILURE;
			break;
		}

		ret = atcab_is_locked(LOCK_ZONE_DATA, &lockstate);
		if (ret != ATCA_SUCCESS) {
			AWS_ERROR("Failed to read data zone of ATECC508(%d)", ret);
			ret = AWS_E_CRYPTO_FAILURE;
			break;
		}
		if (!lockstate)	{
			AWS_ERROR("Un-Provisioned ATECC508");
			ret = AWS_E_CRYPTO_FAILURE;
			break;
		}

		/* Check if CryptoAuthLib works or not. */
		ret = atcab_info(revision);
		if (ret != ATCA_SUCCESS) {
			AWS_ERROR("Failed to initialize ATECC508A!(%d)", ret);
			ret = AWS_E_CRYPTO_FAILURE;
			break;
		}

        /* Read serial number of ATECC508A to use it as unique MQTT client ID. */
		ret = atcab_read_serial_number(kit->user.clientID);
		if (ret != ATCA_SUCCESS) {
			AWS_ERROR("Failed to read serial number of ATECC508A!(%d)", ret);
			ret = AWS_E_CRYPTO_FAILURE;
			break;
		}
		kit->user.clientIDLen = (uint32_t)ATCA_SERIAL_NUM_SIZE;

        /* Set a key as parent key to be used for encrypted read, when reading premaster secret. */
		ret = atca_tls_init_enc_key();
		if (ret != ATCA_SUCCESS) {
			AWS_ERROR("Failed to set parent key!(%d)", ret);
			ret = AWS_E_CRYPTO_FAILURE;
			break;
		}

		/* initialize IOs bewteen ATSAMG55 and ATWINC1500. */
		ret = nm_bsp_init();
		if (ret != M2M_SUCCESS) {
			AWS_ERROR("Failed to initialize Wireless module!(%d)", ret);
			ret = AWS_E_WIFI_INVALID;
			break;
		}

		/* initialize flags. */
		kit->quitMQTT = false;
		kit->blocking = false;
		kit->nonBlocking = false;
		kit->pushButtonState = AWS_BUTTON_RELEASED;
	} while(0);

	return ret;
}

/**
 * \brief Build certificates.
 *
 * Now that both signer and device certificate was saved onto specified slot with certificate definition,
 * Both certificates can be built based on certificate definition in order to pass them to TLS library.
 *
 * \param kit[inout]        Pointer to an instance of AWS Kit
 * \return AWS_E_SUCCESS    On success
 */
int aws_main_build_certificate(t_aws_kit* kit)
{
	int ret = AWS_E_FAILURE;
	t_atcert cert;

	/* Allocate heap to obtain signer DER, PEM certificates and public key space. */
	cert.signer_der = (uint8_t*)malloc(DER_CERT_INIT_SIZE);
	if (cert.signer_der == NULL) goto free_cert;
	cert.signer_der_size = DER_CERT_INIT_SIZE;
	cert.signer_pem = (uint8_t*)malloc(PEM_CERT_INIT_SIZE);
	if (cert.signer_pem == NULL) goto free_cert;
	cert.signer_pem_size = PEM_CERT_INIT_SIZE;
	cert.signer_pubkey= (uint8_t*)malloc(ATCERT_PUBKEY_SIZE);
	if (cert.signer_pubkey == NULL) goto free_cert;

	/* Build signer certificate */
	ret = atca_tls_build_signer_cert(&cert);
	if (ret != ATCA_SUCCESS) {
		ret = AWS_E_CRYPTO_CERT_FAILURE;
		AWS_ERROR("Failed to build signer certificate!(%d)", ret);
		goto free_cert; 
	}

	/* Allocate heap to secure device DER, PEM certificates and public key space. */
	cert.device_der = (uint8_t*)malloc(DER_CERT_INIT_SIZE);
	if (cert.device_der == NULL) goto free_cert;
	cert.device_der_size = DER_CERT_INIT_SIZE;
	cert.device_pem = (uint8_t*)malloc(PEM_CERT_INIT_SIZE);
	if (cert.device_pem == NULL) goto free_cert;
	cert.device_pem_size = PEM_CERT_INIT_SIZE;
	cert.device_pubkey= (uint8_t*)malloc(ATCERT_PUBKEY_SIZE);
	if (cert.device_pubkey == NULL) goto free_cert;

	/* Build device certificate. */
	ret = atca_tls_build_device_cert(&cert);
	if (ret != ATCA_SUCCESS) {
		ret = AWS_E_CRYPTO_CERT_FAILURE;
		AWS_ERROR("Failed to build device certificate!(%d)", ret);
		goto free_cert; 
	}

	/* Copy both PEM certificates to corresponding fields to be used for TLS library. */
	kit->cert.signerCertLen = cert.signer_pem_size;
	memcpy(kit->cert.signerCert, cert.signer_pem, kit->cert.signerCertLen);
	kit->cert.devCertLen = cert.device_pem_size;
	memcpy(kit->cert.devCert, cert.device_pem, kit->cert.devCertLen);

	/* Release temporary heap region */
free_cert:
	if (cert.signer_der) free(cert.signer_der);
	if (cert.signer_pem) free(cert.signer_pem);
	if (cert.signer_pubkey) free(cert.signer_pubkey);
	if (cert.device_der) free(cert.device_der);
	if (cert.device_pem) free(cert.device_pem);
	if (cert.device_pubkey) free(cert.device_pubkey);

	return ret;
}

/**
 * \brief User data initialization.
 *
 * If SW0 button has been pressing for more than 3 seconds, all user data should be set to initial value.
 * This allows a user to reset host address depending on AWS account as well as WIFI credential.
 *
 * \param kit[in]           Pointer to an instance of AWS Kit
 * \return AWS_E_SUCCESS    On success
 */
int aws_main_reset_user_data(t_aws_kit* kit)
{
	int ret = AWS_E_FAILURE;
	uint8_t userData[AWS_USER_DATA_OFFSET_HOST_LEN];

	do {

		memset(userData, 0x00, sizeof(userData));
		/* Write zero values to slot8 of ATECC508A to reset all user data. */
		ret = atcab_write_bytes_zone(ATCA_ZONE_DATA, TLS_SLOT8_ENC_STORE, 0, &userData[0], sizeof(userData));
		if (ret != ATCA_SUCCESS) {
			ret = AWS_E_CRYPTO_FAILURE;
			break;
		}

		/* Write zero values to slot8 of ATECC508A to reset button state. */
		ret = atcab_write_bytes_zone(ATCA_ZONE_DATA, TLS_SLOT8_ENC_STORE, AWS_USER_DATA_BUTTON_STATE, &userData[0], 4);
		if (ret != ATCA_SUCCESS) {
			ret = AWS_E_CRYPTO_FAILURE;
			break;
		}

	} while(0);

	return ret;
}

/**
 * \brief User data verification and loading certificate.
 *
 * Now that user data depending on each of Things is saved in slot8 of ATECC508A,
 * The data can be pull out so that Thing have to connect to WIFI router, and to get destination IP address.
 * Both signer and device certificates should be built in advance to pass them to TLS library.
 *
 * \param kit[inout]        Pointer to an instance of AWS Kit
 * \return AWS_E_SUCCESS    On success
 */
int aws_main_check_kit_state(t_aws_kit* kit)
{
	int ret = AWS_E_FAILURE;
	uint8_t userData[AWS_USER_DATA_OFFSET_MAX];

	do {

		memset(userData, 0x00, sizeof(userData));
		/* Read a bunch of user data from slot8 of ATECC508A. */
		ret = atcab_read_bytes_zone(ATCA_ZONE_DATA, TLS_SLOT8_ENC_STORE, 0x00, userData, sizeof(userData));
		if (ret != ATCA_SUCCESS) {
			AWS_ERROR("Failed to get user data!(%d)", ret);
			ret = AWS_E_CRYPTO_FAILURE;
			break;
		}

		/* Set length for each item. */
		kit->user.ssidLen = AWS_GET_USER_DATA_LEN(userData, AWS_USER_DATA_OFFSET_SSID_LEN);
		kit->user.pskLen = AWS_GET_USER_DATA_LEN(userData, AWS_USER_DATA_OFFSET_PSK_LEN);
		kit->user.hostLen = AWS_GET_USER_DATA_LEN(userData, AWS_USER_DATA_OFFSET_HOST_LEN);
		kit->user.thingLen = AWS_GET_USER_DATA_LEN(userData, AWS_USER_DATA_OFFSET_THING_LEN);

		/* Check if length is valid. */
		if (AWS_CHECK_USER_DATA_LEN(kit->user.ssidLen, AWS_WIFI_SSID_MAX) 
			|| AWS_CHECK_USER_DATA_LEN(kit->user.pskLen, AWS_WIFI_PSK_MAX)
			|| AWS_CHECK_USER_DATA_LEN(kit->user.hostLen, AWS_HOST_ADDR_MAX)
			|| AWS_CHECK_USER_DATA_LEN(kit->user.thingLen, AWS_THING_NAME_MAX)
			|| AWS_CHECK_USER_DATA(userData, AWS_USER_DATA_OFFSET_SSID)
			|| AWS_CHECK_USER_DATA(userData, AWS_USER_DATA_OFFSET_PSK)
			|| AWS_CHECK_USER_DATA(userData, AWS_USER_DATA_OFFSET_HOST)
			|| AWS_CHECK_USER_DATA(userData, AWS_USER_DATA_OFFSET_THING)) {
			ret = AWS_E_USER_DATA_INVALID;
			AWS_ERROR("Invalid user data, try to setup again!(%d)", ret);
			break;
		}

		/* Copy each of items to corresponding variables. */
		memcpy(kit->user.ssid, &userData[AWS_USER_DATA_OFFSET_SSID], kit->user.ssidLen);
		memcpy(kit->user.psk, &userData[AWS_USER_DATA_OFFSET_PSK], kit->user.pskLen);
		memcpy(kit->user.host, &userData[AWS_USER_DATA_OFFSET_HOST], kit->user.hostLen);
		memcpy(kit->user.thing, &userData[AWS_USER_DATA_OFFSET_THING], kit->user.thingLen);

		/* Set button state with previously saved state to keep it. */
		for (uint8_t i = 0; i < AWS_KIT_BUTTON_MAX; i++) {
			if (userData[AWS_USER_DATA_BUTTON_STATE + i] == true)
				kit->button.state[i] = true;
			else
				kit->button.state[i] = false;
		}

		/* Initialize WIFI host driver, and register a socket callback to be interfaced with the driver. 
		   Try to conntect to a WIFI router. */
		if (aws_net_get_wifi_status() != M2M_WIFI_CONNECTED) {
			ret = aws_net_init_wifi(kit, MAIN_WLAN_POLL_TIMEOUT_SEC);
			if (ret != AWS_E_SUCCESS) break;
		}

		/* Get time information from NTP server. 
		   If a WIFI router cannot get to connect the server, reboot device. */
		if (aws_net_get_ntp_seconds() <= 0) {
			ret = aws_net_get_time(kit);
			if (ret != AWS_E_SUCCESS) {
				AWS_ERROR("Reset kit to reconnect to router to get correct Time info!(%d)", ret);
				aws_kit_software_reset();
				// Never come back here
			}
		}

		/* Build signer & device certificates to be set for TLS library. */
		ret = aws_main_build_certificate(kit);
		if (ret != AWS_E_SUCCESS) break;
#ifdef AWS_KIT_DEBUG
		AWS_INFO("SSID : %s, PWD : %s", kit->user.ssid, kit->user.psk);
#endif
		AWS_INFO("HOST : %s, THING : %s", kit->user.host, kit->user.thing);
	} while(0);

	return ret;
}

/**
 * \brief State machine for Main task.
 *
 * This task initializes ATECC508A and ATWINC1500 to build certificates, and connect to WIFI AP.
 * The WIFI credential should be saved in slot8 to be able to make connection.
 * If there is no either valid WIFI credential or compressed cert data, LED2 continues to blink.
 *
 * \param kit[in]           Pointer to an instance of AWS Kit
 * \return AWS_E_SUCCESS    On success
 */
void aws_main_state_machine(t_aws_kit* kit)
{
	int ret = AWS_E_SUCCESS;
	static uint8_t currState = MAIN_STATE_INIT_KIT;
	uint8_t nextState = MAIN_STATE_INVALID;

	switch (currState)
	{
		case MAIN_STATE_INIT_KIT:
			/* Check if ATECC508A has been configured, and initialize BSP for ATWINC1500. */
			ret = aws_main_init_kit(kit);
			if (ret == AWS_E_CRYPTO_FAILURE) {
				kit->errState = AWS_EX_UNPROVISIONED_CRYPTO;
				aws_user_exception_init_timer(kit);
				nextState = MAIN_STATE_PROVISIONING;
			} else {
				/* The kit has been initialized without any problem, go to next state */
				nextState = MAIN_STATE_CHECK_KIT;
			}
			break;

		case MAIN_STATE_CHECK_KIT:
			/* Check if all WIFI credential, AWS IoT host address and Thing name are available, 
			and build signer & device certificates. */
			ret = aws_main_check_kit_state(kit);
			if (ret != AWS_E_SUCCESS) {
				kit->errState = AWS_EX_UNAVAILABLE_WIFI;
				aws_user_exception_init_timer(kit);
				nextState = MAIN_STATE_PROVISIONING;
			} else {
				kit->errState = AWS_EX_NONE;
				nextState = MAIN_STATE_RUN_KIT;
				kit->quitMQTT = false;
				/* Resume Client task, if kit has been provisioned by using Insight GUI without any invalid configuration. */
				vTaskResume(clientTaskHandler);
			}
			break;

		case MAIN_STATE_PROVISIONING:
			/* Wait for a notification from Insight GUI to launch , 
			if kit has been provisioned using Insight GUI without any invalid configuration. */
			ret = aws_main_wait_notification(kit);
			if (ret != AWS_E_SUCCESS) {
				nextState = MAIN_STATE_PROVISIONING;
			} else {
				ret = aws_main_check_kit_state(kit);
				if (ret != AWS_E_SUCCESS) {
					nextState = MAIN_STATE_PROVISIONING;
				} else {
					kit->errState = AWS_EX_NONE;
					nextState = MAIN_STATE_RUN_KIT;
					kit->quitMQTT = false;
					/* Resume Client task, if kit has been provisioned by using Insight GUI without any invalid configuration. */
					vTaskResume(clientTaskHandler);
				}
			}
			break;

		case MAIN_STATE_RUN_KIT:
			/* Wait for a notification again to support multiple provisioning. */
			ret = aws_main_wait_notification(kit);
			if (ret == AWS_E_MQTT_REINITIALIZE) {
				nextState = MAIN_STATE_PROVISIONING;
			} else {
				nextState = MAIN_STATE_RUN_KIT;
			}
			break;

		case MAIN_STATE_INVALID:
			AWS_ERROR("Invalid main state");
			nextState = MAIN_STATE_INVALID;
			break;

		default:
			break;
	}

	currState = kit->mainState = nextState;
}

/**
 * \brief Main task.
 *
 * This task initializes ATECC508A, and then ATWINC1500 for provisioning.
 *
 * \param[in] params        Parameters for the task (Not used.)
 */
void aws_main_task(void* params)
{
	t_aws_kit* kit = aws_kit_get_instance();
	kit->notiQueue = xQueueCreate(1, sizeof(uint8_t));
	
	for (;;) {

		/* Run state machine for Main task. */
		aws_main_state_machine(kit);

		/* Block for 100ms. */
		vTaskDelay(AWS_MAIN_TASK_DELAY);
	}
}

/**
 * \brief Tasks initialization
 *
 * Create all new tasks to add them to the list of tasks.
 *
 */
void aws_demo_tasks_init(void)
{

	/* Create Main task to initialize ATECC508 and ATWINC1500. */
	xTaskCreate(aws_main_task,
			(const char *) "Main",
			AWS_MAIN_TASK_STACK_SIZE,
			NULL,
			AWS_MAIN_TASK_PRIORITY,
			&mainTaskHandler);

	/* Create Provisioning task to communicate with the Insight GUI. */
	xTaskCreate(aws_prov_task,
			(const char *) "Prov",
			AWS_PROV_TASK_STACK_SIZE,
			NULL,
			AWS_PROV_TASK_PRIORITY,
			&provTaskHandler);

	/* Create User task to control OLED1 board. */
	xTaskCreate(aws_user_task,
			(const char *) "User",
			AWS_USER_TASK_STACK_SIZE,
			NULL,
			AWS_USER_TASK_PRIORITY,
			&userTaskHandler);

	/* Create Client task to drive MQTT & TLS library. */
	xTaskCreate(aws_client_task,
			(const char *) "Client",
			AWS_CLIENT_TASK_STACK_SIZE,
			NULL,
			AWS_CLIENT_TASK_PRIORITY,
			&clientTaskHandler);

	/* Suspend Client task to be resumed by Main task. */
	vTaskSuspend(clientTaskHandler);

}

