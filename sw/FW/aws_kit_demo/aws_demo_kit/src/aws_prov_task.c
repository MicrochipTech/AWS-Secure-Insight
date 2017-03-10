/**
 *
 * \file
 *
 * \brief AWS IoT Demo kit.
 *
 * Copyright (C) 2014-2016 Atmel Corporation. All rights reserved.
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
 */

#include <ctype.h>
#include <string.h>
#include "conf_usb.h"
#include "cryptoauthlib.h"
#include "tls/atcatls_cfg.h"
#include "aws_prov_task.h"
#include "aws_kit_object.h"
#include "cert_def_1_signer.h"
#include "cert_def_2_device.h"

/** 
 * \brief version of development kit firmware that contains AES132 and SHA204 library.
 */
const char VersionKit[] = {1, 1, 9};		//!< AWS kit version
const char VersionSha204[] = {1, 3, 0};		//!< SHA204 version
const char VersionAes132[] = {1, 1, 0};		//!< AES132 version
const char VersionEcc508[] = {1, 1, 0};		//!< ECC108 string

const char StringSha204[] = "SHA204 ";		//!< SHA204 string
const char StringAes132[] = "AES132 ";		//!< AES132 string
const char StringEcc508[] = "ECC108 ";		//!< ECC108 string

const char StringKitShort[] = "CK590 ";		//!< short string of Microbase kit
const char StringKit[] = "ATSAMG55 ";		//!< long string of Microbase kit

device_info_t device_info[DISCOVER_DEVICE_COUNT_MAX];	//!< Information of Crypto Authentication device
uint8_t device_count = 0;								//!< Number of Crypto Authentication device

static uint8_t pucUsbRxBuffer[USB_BUFFER_SIZE_RX];	//!< USB CDC RX buffer
static uint8_t pucUsbTxBuffer[USB_BUFFER_SIZE_TX];	//!< USB CDC TX buffer
static uint8_t rxPacketStatus = KIT_STATUS_SUCCESS;	//!< USB RX packet status
static uint16_t rxBufferIndex = 0;					//!< Index of USB RX buffer

/**
 * \brief This function returns the pointer of USB RX buffer.
 *
 * \return pointer             Pointer of USB RX buffer
 */
uint8_t* aws_prov_get_rx_buffer(void)
{
	return pucUsbRxBuffer;
}

/**
 * \brief This function returns the pointer of USB TX buffer.
 *
 * \return pointer             Pointer of USB TX buffer
 */
uint8_t* aws_prov_get_tx_buffer(void)
{
	return pucUsbTxBuffer;
}

/**
 * \brief This function converts a nibble to Hex-ASCII.
 *
 * \param[in] nibble           Nibble value to be converted
 * \return ASCII value
 */
uint8_t aws_prov_convert_nibble_to_ascii(uint8_t nibble)
{
    nibble &= 0x0F;
    if (nibble <= 0x09 )
        nibble += '0';
    else
        nibble += ('A' - 10);
    return nibble;
}

/**
 * \brief This function converts an ASCII character to a nibble.
 *
 * \param[in] ascii            ASCII value to be converted
 * \return nibble value
 */
uint8_t aws_prov_convert_ascii_to_nibble(uint8_t ascii)
{
    if ((ascii <= '9') && (ascii >= '0'))
        ascii -= '0';
    else if ((ascii <= 'F' ) && (ascii >= 'A'))
        ascii -= ('A' - 10);
    else if ((ascii <= 'f') && (ascii >= 'a'))
        ascii -= ('a' - 10);
    else
        ascii = 0;
    return ascii;
}

/**
 * \brief This function converts ASCII to binary.
 *
 * \param[in] length           Number of bytes in buffer
 * \param[inout] buffer        Pointer to buffer
 * \return number of bytes in buffer
 */
uint16_t aws_prov_convert_ascii_to_binary(uint16_t length, uint8_t *buffer)
{
	if (length < 2)
		return 0;

	uint16_t i, binIndex;

	for (i = 0, binIndex = 0; i < length; i += 2)
	{
		buffer[binIndex] = aws_prov_convert_ascii_to_nibble(buffer[i]) << 4;
		buffer[binIndex++] |= aws_prov_convert_ascii_to_nibble(buffer[i + 1]);
	}

	return binIndex;
}

/**
 * \brief Return pointer of device_info with specified index.
 *
 * \param[in] index            Index of device
 * \return pointer             Pointer of device_info
 */
device_info_t* aws_prov_get_device_info(uint8_t index) 
{
	if (index >= device_count)
		return NULL;
	return &device_info[index];
}

/**
 * \brief Return type of device with specified index.
 *
 * \param[in] index            Index of device
 * \return device_type_t
 */
device_type_t aws_prov_get_device_type(uint8_t index) 
{
	if (index >= device_count)
		return DEVICE_TYPE_UNKNOWN;
	return device_info[index].device_type;
}

/**
 * \brief Initialize CryptoAuthLib to detect ATECC508A.
 *
 * \return ATCA_STATUS
 */
ATCA_STATUS aws_prov_detect_I2c_devices(void)
{
	ATCA_STATUS status = ATCA_NO_DEVICES;
	uint8_t revision[4] = { 0 };

	/* Initialize CryptoAuthLib. */
	cfg_ateccx08a_i2c_default.atcai2c.slave_address = DEVICE_I2C;
	status = atcab_init( &cfg_ateccx08a_i2c_default );
	if (status != ATCA_SUCCESS) RETURN(status, "Failed: Initialize interface");

	/* Check whether communication does work or not. */
	status = atcab_info(revision);
	if (status != ATCA_SUCCESS) {
		atcab_release();

		/* If failed, try to initialize again with another I2C address. */
		cfg_ateccx08a_i2c_default.atcai2c.slave_address = FACTORY_INIT_I2C;
		status = atcab_init( &cfg_ateccx08a_i2c_default );
		if (status != ATCA_SUCCESS) return status;

		status = atcab_info(revision);
		if (status != ATCA_SUCCESS) RETURN(status, "Failed: Not connected to ECC508");
	}

	/* Set device information to be used as interface of Kit Protocol. */
	device_info[device_count].address = cfg_ateccx08a_i2c_default.atcai2c.slave_address;
	device_info[device_count].bus_type = DEVKIT_IF_I2C;
	device_info[device_count].device_type = DEVICE_TYPE_ECC108;
	memcpy(device_info[device_count].dev_rev, revision, sizeof(revision));

	device_count++;
	
	return status;
}

/**
 * \brief Try to find Crypto Authentication devices(only ATECC508A, except for ATSHA204A and ATAES132A).
 * For the AWS Kit firmware, Don't care SHA204 and AES128.
 * ATECC508A should be configured in advance using a configuration utility.
 * In addition to it, ATECC508A should be set I2C address to 0xB0 for AWS Kit firmware.
 * 
 * \return interface_id_t
 */
interface_id_t aws_prov_discover_devices(void)
{
	ATCA_STATUS status = ATCA_NO_DEVICES;
	interface_id_t bus_type;

	device_count = 0;
	memset(device_info, 0, sizeof(device_info));

	/* Try to dectect ATECC508A over I2C. */
	status = aws_prov_detect_I2c_devices();

	if (device_count == 0 || status != ATCA_SUCCESS)
		return DEVKIT_IF_UNKNOWN;

	bus_type = device_info[0].bus_type;

	return bus_type;
}

/**
 * \brief This function parses kit commands (ASCII) received from a PC host and returns an ASCII response.
 *
 * \param[in] commandLength    Number of bytes in command buffer
 * \param[in] command          Pointer to ASCII command buffer
 * \param[out] responseLength  Pointer to number of bytes in response buffer
 * \param[out] response        Pointer to binary response buffer
 * \param[out] responseIsAscii Pointer to response type
 * \return the status of the operation
 */
uint8_t aws_prov_parse_board_commands(uint16_t commandLength, uint8_t *command, 
										uint16_t *responseLength, uint8_t *response, uint8_t *responseIsAscii)
{
	uint8_t status = KIT_STATUS_UNKNOWN_COMMAND;
	uint16_t responseIndex = 0;
	uint16_t deviceIndex;
	uint16_t dataLength = 1;
	uint8_t *rxData[1];
	interface_id_t device_interface = DEVKIT_IF_UNKNOWN;
	device_info_t* dev_info;
	const char *StringInterface[] = {"no_device ", "SPI ", "TWI ", "SWI "};
	const char *pToken = strchr((char *) command, ':');

	if (!pToken)
		return status;

	*responseIsAscii = 1;

	switch(pToken[1]) {

		case 'v':
			/* Gets abbreviated board name and, if found, first device type and interface type.
			   response (no device): <kit version>, "no_devices"<status>()
			   response (device found): <kit version>, <device type>, <interface><status>(<address>). */
			break;
		
		case 'f':
			status = aws_prov_extract_data_load((const char*)pToken, &dataLength, rxData);
			if (status != KIT_STATUS_SUCCESS)
				break;

			dataLength = 4; // size of versions + status byte

			switch (*rxData[0]) {
				case 0: // kit version
					strcpy((char *) response, StringKit);
					responseIndex = strlen((char *) response);
					memcpy((char *) (response + responseIndex + 1), VersionKit, dataLength);
					break;

				case 1: // SHA204 library version
					strcpy((char *) response, StringSha204);
					responseIndex = strlen((char *) response);
					memcpy((char *) (response + responseIndex + 1), VersionSha204, dataLength);
					break;

				case 2: // AES132 library version
					strcpy((char *) response, StringAes132);
					responseIndex = strlen((char *) response);
					memcpy((char *) (response + responseIndex + 1), VersionAes132, dataLength);
					break;

				case 3: // ECC508 library version
					strcpy((char *) response, StringEcc508);
					responseIndex = strlen((char *) response);
					memcpy((char *) (response + responseIndex + 1), VersionEcc508, dataLength);
					break;

				default:
					status = KIT_STATUS_INVALID_PARAMS;
					break;
			}
			break;

		case 'd':
			status = aws_prov_extract_data_load((const char*)pToken, &dataLength, rxData);
			if (status != KIT_STATUS_SUCCESS)
				break;

			device_interface = aws_prov_discover_devices();
			deviceIndex = *rxData[0];
			dev_info = aws_prov_get_device_info(deviceIndex);
			if (!dev_info) {
				status = KIT_STATUS_NO_DEVICE;
				break;
			}

			switch (dev_info->device_type) {
				case DEVICE_TYPE_SHA204:
					strcpy((char *) response, StringSha204);
					break;

				case DEVICE_TYPE_AES132:
					strcpy((char *) response, StringAes132);
					break;

				case DEVICE_TYPE_ECC108:
					strcpy((char *) response, StringEcc508);
					break;

				case DEVICE_TYPE_UNKNOWN:
					strcpy((char *) response, StringInterface[0]);
					status = KIT_STATUS_NO_DEVICE;
					break;

				default:
					strcpy((char *) response, "unknown_device");
					break;
			}


			if (dev_info->bus_type == DEVKIT_IF_UNKNOWN) {
				responseIndex = strlen((char *) response);
				break;
			}
			
			/* Append interface type to response. */
			strcat((char*)response, StringInterface[device_interface]);
			responseIndex = strlen((char *) response);

			/* Append the address (TWI) / index (SWI) of the device.
			   Skip one byte for status. */
			dataLength++;
			response[responseIndex + 1] = dev_info->bus_type == DEVKIT_IF_I2C ? dev_info->address : dev_info->device_index;
			break;

		default:
			status = KIT_STATUS_UNKNOWN_COMMAND;
			break;
			
	}
	
	/* Append <status>(<data>). */
	response[responseIndex] = status;
	*responseLength = aws_prov_create_usb_packet(dataLength, &response[responseIndex]) + responseIndex;
	
	return status;
}

/**
 * \brief Give index of command and response length based on received command.  
 *
 * \param[in] tx_buffer includes command to be sent to device 
 * \param[out] cmd_index       Index corresponding to opcode
 * \param[out] rx_length       Length of response to be came to device
 * \return ATCA_SUCCESS
 */
uint8_t aws_prov_get_commands_info(uint8_t *tx_buffer, uint8_t *cmd_index, uint16_t *rx_length)
{
	uint8_t status = ATCA_SUCCESS;
	uint8_t opCode = tx_buffer[1];
	uint8_t param1 = tx_buffer[2];
	
	switch (opCode) {
		
		case ATCA_CHECKMAC:
			*cmd_index = CMD_CHECKMAC;
			*rx_length = CHECKMAC_RSP_SIZE;
			break;
		
		case ATCA_COUNTER:
			*cmd_index = CMD_COUNTER;
			*rx_length = COUNTER_RSP_SIZE;
			break;
		
		case ATCA_DERIVE_KEY:
			*cmd_index = CMD_DERIVEKEY;
			*rx_length = DERIVE_KEY_RSP_SIZE;
			break;
		
		case ATCA_ECDH:
			*cmd_index = CMD_ECDH;
			*rx_length = ECDH_RSP_SIZE;
			break;
		
		case ATCA_GENDIG:
			*cmd_index = CMD_GENDIG;
			*rx_length = GENDIG_RSP_SIZE;
			break;
		
		case ATCA_GENKEY:
			*cmd_index = CMD_GENKEY;
			*rx_length = (param1 == GENKEY_MODE_DIGEST)	? GENKEY_RSP_SIZE_SHORT : GENKEY_RSP_SIZE_LONG;
			break;
		
		case ATCA_HMAC:
			*cmd_index = CMD_HMAC;
			*rx_length = HMAC_RSP_SIZE;
			break;
		
		case ATCA_INFO:
			*cmd_index = CMD_INFO;
			*rx_length = INFO_RSP_SIZE;
			break;

		case ATCA_LOCK:
			*cmd_index = CMD_LOCK;
			*rx_length = LOCK_RSP_SIZE;
			break;
		
		case ATCA_MAC:
			*cmd_index = CMD_MAC;
			*rx_length = MAC_RSP_SIZE;
			break;
		
		case ATCA_NONCE:
			*cmd_index = CMD_NONCE;
			*rx_length = (param1 == NONCE_MODE_PASSTHROUGH)	? NONCE_RSP_SIZE_SHORT : NONCE_RSP_SIZE_LONG;
			break;
		
		case ATCA_PAUSE:
			*cmd_index = CMD_PAUSE;
			*rx_length = PAUSE_RSP_SIZE;
			break;
		
		case ATCA_PRIVWRITE:
			*cmd_index = CMD_PRIVWRITE;
			*rx_length = PRIVWRITE_RSP_SIZE;
			break;
		
		case ATCA_RANDOM:
			*cmd_index = CMD_RANDOM;
			*rx_length = RANDOM_RSP_SIZE;
			break;
		
		case ATCA_READ:
			*cmd_index = CMD_READMEM;
			*rx_length = (param1 & 0x80)	? READ_32_RSP_SIZE : READ_4_RSP_SIZE;
			break;
		
		case ATCA_SHA:
			*cmd_index = CMD_SHA;
			*rx_length = (param1 == SHA_MODE_SHA256_END) ? ATCA_RSP_SIZE_32 : ATCA_RSP_SIZE_4;
			break;

		case ATCA_SIGN:
			*cmd_index = CMD_SIGN;
			*rx_length = SIGN_RSP_SIZE;
			break;
		
		case ATCA_UPDATE_EXTRA:
			*cmd_index = CMD_UPDATEEXTRA;
			*rx_length = UPDATE_RSP_SIZE;
			break;
		
		case ATCA_VERIFY:
			*cmd_index = CMD_VERIFY;
			*rx_length = VERIFY_RSP_SIZE;
			break;
		
		case ATCA_WRITE:
			*cmd_index = CMD_WRITEMEM;
			*rx_length = WRITE_RSP_SIZE;
			break;
		
		default:
			break;
		
	}

	return status;
}

/**
 * \brief Send a command array, and then receive a result from ATECC508A.  
 *
 * \param[in] tx_buffer        TX buffer to send command
 * \param[out] rx_buffer       RX buffer to receive response
 * \return ATCA_SUCCESS        On success
 */
uint8_t aws_prov_send_and_receive(uint8_t *tx_buffer, uint8_t *rx_buffer)
{
	uint8_t status = ATCA_SUCCESS;
	uint8_t cmd_index;
	uint16_t rx_length;
	uint16_t execution_time = 0;
	uint8_t *cmd_buffer;
	ATCADevice  _gDevice = NULL;
	ATCACommand _gCommandObj = NULL;
	ATCAIface   _gIface = NULL;

	do {

		if (tx_buffer == NULL || rx_buffer == NULL)
			break;

		/* Collect command information from TX buffer. */
		if (aws_prov_get_commands_info( tx_buffer, &cmd_index, &rx_length) != ATCA_SUCCESS)
			break;

		cmd_buffer = (uint8_t *)malloc(tx_buffer[0] + 1);
		memcpy(&cmd_buffer[1], tx_buffer, tx_buffer[0]);

		/* Initialize every objects. */
		_gDevice= atcab_getDevice();
		_gCommandObj = atGetCommands(_gDevice);
		_gIface = atGetIFace(_gDevice);

		/* Get command execution time. */
		execution_time = atGetExecTime(_gCommandObj, cmd_index);

		if ((status = atcab_wakeup()) != ATCA_SUCCESS)
			break;
	
		/* Send command. */
		if ((status = atsend(_gIface, (uint8_t *)cmd_buffer, tx_buffer[0])) != ATCA_SUCCESS)
			break;

		/* delay the appropriate amount of time for command to execute. */
		atca_delay_ms(execution_time);

		/* receive the response from ATECC508. */
		if ((status = atreceive(_gIface, rx_buffer, &rx_length)) != ATCA_SUCCESS)
			break;

		atcab_idle();

		free((void *)cmd_buffer);

	} while(0);
	
	return status;
	
}

/**
 * \brief Send a command array to ATECC508A over I2C.
 *
 * \param[in] tx_buffer        Buffer to be sent
 * \return ATCA_SUCCESS        On success
 */
uint8_t aws_prov_send_command(uint8_t *tx_buffer)
{
	uint8_t status = ATCA_SUCCESS;
	uint8_t cmd_index;
	uint16_t rx_length;
	uint16_t execution_time = 0;
	uint8_t *cmd_buffer;
	ATCADevice  _gDevice = NULL;
	ATCACommand _gCommandObj = NULL;
	ATCAIface   _gIface = NULL;

	do {

		if (tx_buffer == NULL)
			break;

		/* Collect command information from TX buffer. */
		if (aws_prov_get_commands_info(tx_buffer, &cmd_index, &rx_length) != ATCA_SUCCESS)
			break;

		cmd_buffer = (uint8_t *)malloc(tx_buffer[0] + 1);
		memcpy(&cmd_buffer[1], tx_buffer, tx_buffer[0]);

		/* Initialize every objects. */
		_gDevice= atcab_getDevice();
		_gCommandObj = atGetCommands(_gDevice);
		_gIface = atGetIFace(_gDevice);

		/* Get command execution time. */
		execution_time = atGetExecTime(_gCommandObj, cmd_index);

		if ((status = atcab_wakeup()) != ATCA_SUCCESS )
			break;
		
		/* Send command. */
		if ((status = atsend( _gIface, (uint8_t *)cmd_buffer, tx_buffer[0])) != ATCA_SUCCESS)
			break;

		/* delay the appropriate amount of time for command to execute. */
		atca_delay_ms(execution_time);

		atcab_idle();

		free((void *)cmd_buffer);

	} while(0);
	
	return status;
	
}

/**
 * \brief Only receive a command array.
 *
 * \param[in] size             Expected number to receive
 * \param[out] rx_buffer       Buffer that includes data to be received from a device 
 * \return ATCA_SUCCESS        On success
 */
uint8_t aws_prov_receive_response(uint8_t size, uint8_t *rx_buffer)
{
	uint8_t status = ATCA_SUCCESS;
	uint16_t rxlength = size;	
	ATCADevice  _gDevice = NULL;
	ATCAIface   _gIface = NULL;

	do {

		if ( rx_buffer == NULL )
			break;

		/* Initialize every objects. */
		_gDevice= atcab_getDevice();
		
		if ( (status = atcab_wakeup()) != ATCA_SUCCESS )
			break;

		_gDevice= atcab_getDevice();
		_gIface = atGetIFace(_gDevice);

		/* Receive the response from ATECC508. */
		if ( (status = atreceive( _gIface, rx_buffer, &rxlength)) != ATCA_SUCCESS )
			break;

		atcab_idle();

	} while(0);
	
	return status;
	
}

/**
 * \brief This function parses communication commands (ASCII) received from a
 *         PC host and returns a binary response.
 *
 *         protocol syntax:\n\n
 *         functions for command sequences:\n
 *            v[erify]                            several Communication and Command Marshaling layer functions
 *            a[tomic]                            Wraps "talk" into a Wakeup / Idle.
 *         functions in sha204_comm.c (Communication layer):\n
 *            w[akeup]                            sha204c_wakeup\n
 *            t[alk](command)                     sha204c_send_and_receive\n
 *         functions in sha204_i2c.c / sha204_swi.c (Physical layer):\n
 *            [physical:]s[leep]                  sha204p_sleep\n
 *            [physical:]i[dle]                   sha204p_idle\n
 *            p[hysical]:r[esync]                 sha204p_resync\n
 *            p[hysical]:e[nable]                 sha204p_init\n
 *            p[hysical]:d[isable]                sha204p_disable_interface\n
 *            c[ommand](data)                     sha204p_send_command\n
 *            r[esponse](size)                    sha204p_receive_response\n
 * \param[in] commandLength number of bytes in command buffer
 * \param[in] command pointer to ASCII command buffer
 * \param[out] responseLength pointer to number of bytes in response buffer
 * \param[out] response pointer to binary response buffer
 * \return the status of the operation
 */
uint8_t aws_prov_parse_ecc_commands(uint16_t commandLength, uint8_t *command, uint16_t *responseLength, uint8_t *response)
{
	uint8_t status = KIT_STATUS_SUCCESS;
	uint16_t dataLength;
	uint8_t *data_load[1];
	uint8_t *dataLoad;
	char *pToken = strchr((char *) command, ':');

	*responseLength = 0;

	if (!pToken)
		return status;

	switch (pToken[1]) {
		/* Talk (send command and receive response) */
		case 't':
			status = aws_prov_extract_data_load((const char*)pToken + 2, &dataLength, data_load);
			if (status != KIT_STATUS_SUCCESS)
				return status;

			response[SHA204_BUFFER_POS_COUNT] = 0;
			status = aws_prov_send_and_receive(data_load[0], &response[0]);
			if (status != KIT_STATUS_SUCCESS)
				return status;

			*responseLength = response[SHA204_BUFFER_POS_COUNT];
			break;

		/* Wakeup. */
		case 'w':
			status = atcab_wakeup();
			if (status != KIT_STATUS_SUCCESS)
				return status;
			break;

		/* Sleep. */
		case 's':
			status = atcab_sleep();
			if (status != KIT_STATUS_SUCCESS)
				return status;
			break;

		/* Idle. */
		case 'i':
			status = atcab_idle();
			if (status != KIT_STATUS_SUCCESS)
				return status;			
			break;
		
		/* Switch whether to wrap a Wakeup / Idle around a "talk" message. */
		case 'a':
			status = aws_prov_extract_data_load((const char*)pToken + 2, &dataLength, data_load);
			if (status != KIT_STATUS_SUCCESS)
				return status;
			break;

		/*  Calls physical functions. */
		case 'p':
			/*"s[ha204]:p[hysical]:" */
			pToken = strchr(&pToken[1], ':');
			if (!pToken)
				return status;

			switch (pToken[1]) {
				/* Wake-up without receive. */
				case 'w':
					status = atcab_wakeup();
					if (status != KIT_STATUS_SUCCESS)
						return status;					
					break;

				/* Send command. */
				case 'c':
					status = aws_prov_extract_data_load((const char*)pToken + 2, &dataLength, data_load);
					if (status != KIT_STATUS_SUCCESS)
						return status;
					dataLoad = data_load[0];
					status = aws_prov_send_command(dataLoad);				
					break;

				/* Receive response. */
				case 'r':
					status = aws_prov_extract_data_load((const char*)pToken + 2, &dataLength, data_load);
					if (status != KIT_STATUS_SUCCESS)
						return status;
					// Reset count byte.
					response[SHA204_BUFFER_POS_COUNT] = 0;
					status = aws_prov_receive_response(*data_load[0], response);
					if (status != KIT_STATUS_SUCCESS)
						return status;					
					*responseLength = response[SHA204_BUFFER_POS_COUNT];
					break;

				/* "s[elect](device index | TWI address)" or "s[leep]" */
				case 's':
					status = aws_prov_extract_data_load((const char*)pToken + 2, &dataLength, data_load);
					if (status == KIT_STATUS_SUCCESS) {
						// Select device (I2C: address; SWI: index into GPIO array).
						dataLoad = data_load[0];
					} else {
						// Sleep command
						status = atcab_idle();
						if (status != KIT_STATUS_SUCCESS)
							return status;						
					}
					break;

				default:
					status = KIT_STATUS_UNKNOWN_COMMAND;
					break;
					
				} /* end physical. */
			break;
			
		default:
			status = KIT_STATUS_UNKNOWN_COMMAND;
			break;
	}
	
	return status;
}

/**
 * \brief This function extracts data from a command string and	converts them to binary. *
 * The data load is expected to be in Hex-Ascii and surrounded by parentheses.
 *
 * \param[in] command          Command string
 * \param[out] dataLength      Number of bytes extracted
 * \param[out] data            Pointer of binary data
 * \return KIT_STATUS_SUCCESS  On success
 */
uint8_t aws_prov_extract_data_load(const char* command, uint16_t* dataLength, uint8_t** data)
{
	uint8_t status = KIT_STATUS_INVALID_PARAMS;

	if (!command || !dataLength || !data)
		return status;

	char* pToken = strchr(command, '(');
	if (!pToken)
		return status;

	char* dataEnd = strchr(pToken, ')');
	if (!dataEnd)
		// Allow a missing closing parenthesis.
		dataEnd = (char *) command + strlen(command);
	else
		dataEnd--;

	uint16_t asciiLength = (uint16_t) (dataEnd - pToken);
	*data = (uint8_t *) pToken + 1;
	*dataLength = aws_prov_convert_ascii_to_binary(asciiLength, *data);

	return KIT_STATUS_SUCCESS;
}

/**
 * \brief Extract valid data between start and end token from a entire command string.
 * Convert it ASCII to binary.
 *
 * \param[in] command          Command string
 * \param[out] dataLength      Number of bytes extracted
 * \param[out] data            Pointer to pointer to binary data
 * \param[in] start_token      The first character
 * \param[in] end_token        The last character
 * \return KIT_STATUS_SUCCESS  On success
 */
uint8_t aws_prov_extract_data_load_tokens(const char *command, uint16_t *dataLength, uint8_t **data, uint8_t start_token, uint8_t end_token)
{
	uint8_t status = KIT_STATUS_INVALID_PARAMS;

	if (!command || !dataLength || !data)
	return status;

	char *pToken = strchr(command, start_token);
	if (!pToken)
	return status;

	char *dataEnd = strchr(pToken, end_token);
	if (!dataEnd)
	/* Allow a missing closing parenthesis. */
	dataEnd = (char *) command + strlen(command);
	else
	dataEnd--;

	uint16_t asciiLength = (uint16_t) (dataEnd - pToken);
	*data = (uint8_t *) pToken + 1;
	*dataLength = aws_prov_convert_ascii_to_binary(asciiLength, *data);

	return KIT_STATUS_SUCCESS;
}

/**
 * \brief This function converts binary response data to hex-ascii and packs it into a protocol response.
 *        <status byte> <'('> <hex-ascii data> <')'> <'\n'>
 * \param[in] length           Number of bytes in data load plus one status byte
 * \param[out] buffer          Pointer to data
 * \return length              Length of ASCII data
 */
uint16_t aws_prov_create_usb_packet(uint16_t length, uint8_t *buffer)
{
	uint16_t binBufferIndex = length - 1;
	/* Size of data load is length minus status byte. */
	uint16_t asciiLength = 2 * (length - 1) + 5; /* + 5: 2 status byte characters + '(' + ")\n". */
	uint16_t asciiBufferIndex = asciiLength - 1;
	uint8_t byteValue;

	/* Terminate ASCII packet. */
	buffer[asciiBufferIndex--] = KIT_EOP;

	/* Append ')'. */
	buffer[asciiBufferIndex--] = ')';

	/* Convert binary data to hex-ascii starting with the last byte of data. */
	while (binBufferIndex)
	{
		byteValue = buffer[binBufferIndex--];
		buffer[asciiBufferIndex--] = aws_prov_convert_nibble_to_ascii(byteValue);
		buffer[asciiBufferIndex--] = aws_prov_convert_nibble_to_ascii(byteValue >> 4);
	}

	/* Start data load with open parenthesis. */
	buffer[asciiBufferIndex--] = '(';

	/* Convert first byte (function return value) to hex-ascii. */
	byteValue = buffer[0];
	buffer[asciiBufferIndex--] = aws_prov_convert_nibble_to_ascii(byteValue);
	buffer[asciiBufferIndex] = aws_prov_convert_nibble_to_ascii(byteValue >> 4);

	return asciiLength;
}

/**
 * \brief This function converts binary data to Hex-ASCII.
 *
 * \param[in]                  Tx length of bytes to send
 * \param[out]                 Buffer pointer to Tx buffer
 * \return                     New length of data
 */
uint16_t aws_prov_convert_data(uint16_t length, uint8_t *buffer)
{
	if (length > DEVICE_BUFFER_SIZE_MAX_RX) {
		buffer[0] = KIT_STATUS_USB_TX_OVERFLOW;
		length = DEVICE_BUFFER_SIZE_MAX_RX;
	}

	return aws_prov_create_usb_packet(length, buffer);
}

/**
 * \brief Interpret Rx packet, and then execute received command.
 *
 * \param[in] rx_length        Length of received packet
 * \param[inout] txLength      Tx length to be sent to Insight GUI
 * return pointer              Pointer of Tx buffer to be sent to Insight GUI
 */
uint8_t* aws_prov_process_usb_packet(uint16_t rx_length, uint16_t *txLength)
{
	uint8_t status = KIT_STATUS_SUCCESS;
	uint8_t responseIsAscii = 0;
	uint16_t rxLength = rx_length - 1;	/* except for a line feed character */
	uint8_t* txBuffer = aws_prov_get_tx_buffer();
	uint8_t* pRxBuffer = aws_prov_get_rx_buffer();

	if (rxPacketStatus != KIT_STATUS_SUCCESS) {
		pucUsbTxBuffer[0] = rxPacketStatus;
		*txLength = 1;
		*txLength = aws_prov_convert_data(*txLength, pucUsbTxBuffer);
	}
	
	if (pRxBuffer[0] == 'l') {	/* lib. */
		/* "lib" as the first field is optional. Move rx pointer to the next field. */
		pRxBuffer = memchr(pRxBuffer, ':', rxBufferIndex);
		if (!pRxBuffer)
			status = KIT_STATUS_UNKNOWN_COMMAND;
		else
			pRxBuffer++;
	}
		
	switch (pRxBuffer[0]) {
			
		case 's':
		case 'e':			
			status = aws_prov_parse_ecc_commands(rxLength, (uint8_t *)pRxBuffer, txLength, pucUsbTxBuffer + 1);
			break;

		case 'B':
		case 'b':
			/* board level commands ("b[oard]"). */
			status = aws_prov_parse_board_commands((uint8_t) rxLength, (uint8_t *)pRxBuffer, txLength, txBuffer, &responseIsAscii);
			break;

		case 'A':
		case 'a':
				switch (pRxBuffer[1])
				{
					case 'W':
					case 'w':
						status = aws_prov_parse_aws_commands(rxLength, (uint8_t *) pRxBuffer, txLength, pucUsbTxBuffer + 1);
					break;
					default:
					break;
				}
		break;
		default:
			status = KIT_STATUS_UNKNOWN_COMMAND;
			*txLength = 1;			
			break;
	}

	if (!responseIsAscii) {
		/* Copy leading function return byte. */
		pucUsbTxBuffer[0] = status;
		/* Tell aws_prov_convert_data the correct txLength. */
		if (*txLength < DEVICE_BUFFER_SIZE_MAX_RX)
			(*txLength)++;
		*txLength = aws_prov_convert_data(*txLength, pucUsbTxBuffer);
	}
	
	return txBuffer;
}

/**
 * \brief Write the Root CA public key to the slot 0x0B of ATECC508A. (Not used.)
 *
 * \param[out] public_key      Buffer for the Root public key to write it
 * \return ATCA_SUCCESS        On success
 */
uint8_t aws_prov_save_root_public_key(char* command, uint8_t* response, uint16_t* response_length)
{
	uint8_t ret = ATCA_SUCCESS;
	uint8_t *data_buffer = NULL;
	uint16_t buffer_length = 0;
	size_t end_block = 3, start_block = 0;
	uint8_t padded_public_key[96];

	*response_length = 0;

	/* Extract the Root public key passed by Insight GUI */
	ret = aws_prov_extract_data_load((const char*)command, &buffer_length, &data_buffer);
	if (ret != KIT_STATUS_SUCCESS)
		return ret;

	memset(padded_public_key, 0x00, sizeof(padded_public_key));
	memmove(&padded_public_key[40], &data_buffer[32], 32);
	memset(&padded_public_key[36], 0, 4);
	memmove(&padded_public_key[4], &data_buffer[0], 32);
	memset(&padded_public_key[0], 0, 4);

	/* Write the Root public key */
	for (; start_block < end_block; start_block++) {
		ret = atcab_write_zone(DEVZONE_DATA, TLS_SLOT_PKICA_PUBKEY, 
							start_block, 0, &padded_public_key[(start_block - 0) * 32], 32);
		if (ret != ATCA_SUCCESS) return ret;
	}
	
	return ret;
}

/**
 * \brief Read the Root CA public key from the slot 0x0E of ATECC508A.
 *
 * \param[out] public_key      Buffer for the Root public key to read it
 * \return ATCA_SUCCESS        On success
 */
uint8_t aws_prov_get_root_public_key(uint8_t* response, uint16_t* response_length)
{
	uint8_t ret = ATCA_SUCCESS;
	size_t end_block = 3, start_block = 0;
	uint8_t padded_public_key[96];

	/* Read the Root public key */
	memset(padded_public_key, 0x00, sizeof(padded_public_key));
	for (; start_block < end_block; start_block++) {
		ret = atcab_read_zone(DEVZONE_DATA, TLS_SLOT_PKICA_PUBKEY, 
							start_block, 0, &padded_public_key[(start_block - 0) * 32], 32);
		if (ret != ATCA_SUCCESS) return ret;
	}

	memcpy(&response[32], &padded_public_key[40], 32);
	memcpy(&response[0], &padded_public_key[4], 32);
	*response_length = ATCA_PUB_KEY_SIZE;

	return ret;
}

/**
 * \brief Write the Signer public key to the slot 0x0B of ATECC508A. (Not used.)
 *
 * \param[out] public_key      Buffer for the Signer public key to write it
 * \return ATCA_SUCCESS        On success
 */
uint8_t aws_prov_save_signer_public_key(uint8_t* public_key)
{
	uint8_t ret = ATCA_SUCCESS;
	size_t end_block = 3, start_block = 0;
	uint8_t padded_public_key[96];

	memset(padded_public_key, 0x00, sizeof(padded_public_key));
	memmove(&padded_public_key[40], &public_key[32], 32);
	memset(&padded_public_key[36], 0, 4);
	memmove(&padded_public_key[4], &public_key[0], 32);
	memset(&padded_public_key[0], 0, 4);

	/* Write the Signer public key */
	for (; start_block < end_block; start_block++) {
		ret = atcab_write_zone(DEVZONE_DATA, TLS_SLOT_SIGNER_PUBKEY, 
							start_block, 0, &padded_public_key[(start_block - 0) * 32], 32);
		if (ret != ATCA_SUCCESS) return ret;
	}

	return ret;
}

/**
 * \brief Read the Signer public key from the slot 0x0B of ATECC508A.
 *
 * \param[out] public_key      Buffer for the Signer public key to read it
 * \return ATCA_SUCCESS        On success
 */
uint8_t aws_prov_get_signer_public_key(uint8_t* public_key)
{
	uint8_t ret = ATCA_SUCCESS;
	size_t end_block = 3, start_block = 0;
	uint8_t padded_public_key[96];

	/* Read the Signer public key */
	memset(padded_public_key, 0x00, sizeof(padded_public_key));
	for (; start_block < end_block; start_block++) {
		ret = atcab_read_zone(DEVZONE_DATA, TLS_SLOT_SIGNER_PUBKEY, 
							start_block, 0, &padded_public_key[(start_block - 0) * 32], 32);
		if (ret != ATCA_SUCCESS) return ret;
	}

	memcpy(&public_key[32], &padded_public_key[40], 32);
	memcpy(&public_key[0], &padded_public_key[4], 32);

	return ret;
}

/**
 * \brief Except for the Device signature, build the Device certificate, and then compute it using SHA256
 * to create a digest to be signed by the Signer.
 *
 * \param[in] cert_def         The Device certificate definition to be built
 * \param[in] cert             The Device certificate buffer
 * \param[in] cert_size        Size of cert buffer
 * \param[in] ca_public_key    Parent public key to create Auth Key ID
 * \param[in] public_key       Own public key to set in the Device certificate
 * \param[in] signer_id        Set the signer ID in the issuer field of the Device certificate
 * \param[in] issue_date       Validity between Not Before and Not After in the Device certificate
 * \param[in] config32         Device information came from configuration zone in ATECC508A
 * \param[out] tbs_digest      Digest of the Device to be signed by the Signer
 * \return ATCACERT_E_SUCCESS  On success
 */
uint8_t aws_prov_build_tbs_cert_digest(
	const atcacert_def_t*    cert_def,
	uint8_t*                 cert,
	size_t*                  cert_size,
	const uint8_t            ca_public_key[64],
	const uint8_t            public_key[64],
	const uint8_t            signer_id[2],
	const atcacert_tm_utc_t* issue_date,
	const uint8_t            config32[32],
	uint8_t*                 tbs_digest)
{
	int ret = ATCACERT_E_SUCCESS;
	atcacert_build_state_t build_state;
	atcacert_tm_utc_t expire_date = {
		.tm_year = issue_date->tm_year + cert_def->expire_years,
		.tm_mon = issue_date->tm_mon,
		.tm_mday = issue_date->tm_mday,
		.tm_hour = issue_date->tm_hour,
		.tm_min = 0,
		.tm_sec = 0
	};
	const atcacert_device_loc_t config32_dev_loc = {
		.zone = DEVZONE_CONFIG,
		.offset = 0,
		.count = 32
	};

	if (cert_def->expire_years == 0)
	{
		ret = atcacert_date_get_max_date(cert_def->expire_date_format, &expire_date);
		if (ret != ATCACERT_E_SUCCESS) return ret;
	}

	ret = atcacert_cert_build_start(&build_state, cert_def, cert, cert_size, ca_public_key);
	if (ret != ATCACERT_E_SUCCESS) return ret;

	ret = atcacert_set_subj_public_key(build_state.cert_def, build_state.cert, *build_state.cert_size, public_key);
	if (ret != ATCACERT_E_SUCCESS) return ret;
	ret = atcacert_set_issue_date(build_state.cert_def, build_state.cert, *build_state.cert_size, issue_date);
	if (ret != ATCACERT_E_SUCCESS) return ret;
	ret = atcacert_set_expire_date(build_state.cert_def, build_state.cert, *build_state.cert_size, &expire_date);
	if (ret != ATCACERT_E_SUCCESS) return ret;
	ret = atcacert_set_signer_id(build_state.cert_def, build_state.cert, *build_state.cert_size, signer_id);
	if (ret != ATCACERT_E_SUCCESS) return ret;
	ret = atcacert_cert_build_process(&build_state, &config32_dev_loc, config32);
	if (ret != ATCACERT_E_SUCCESS) return ret;

	ret = atcacert_cert_build_finish(&build_state);
	if (ret != ATCACERT_E_SUCCESS) return ret;

	ret = atcacert_get_tbs_digest(build_state.cert_def, build_state.cert, *build_state.cert_size, tbs_digest);
	if (ret != ATCACERT_E_SUCCESS) return ret;

	return ret;
}

/**
 * \brief Write compressed certificate and signagure into specified slot.
 * Stored certificate would be used for the purpose of rebuilding to pass it to TLS library. 
 *
 * \param[in] signature        Either Signer or Device signature signed by a parent private key
 * \param[in] cert_id          Certificate ID to identify Signer or Device certificate definition
 * \return KIT_STATUS_SUCCESS  On success
 */
uint8_t aws_prov_build_and_save_cert(uint8_t* signature, uint8_t cert_id)
{
	uint8_t ret = ATCA_SUCCESS;
	uint8_t cert[AWS_CERT_LENGH_MAX] = {0}, tbs_digest[ATCA_SHA_DIGEST_SIZE];
	const atcacert_def_t* cert_def = (cert_id == AWS_SIGNER_CERT_ID) ? &g_cert_def_1_signer : &g_cert_def_2_device;
	size_t cert_size = sizeof(cert);
	size_t max_cert_size = cert_size;
	uint8_t pub_key[ATCA_PUB_KEY_SIZE] = { 0 };
	uint8_t signer_pub_key[ATCA_PUB_KEY_SIZE] = { 0 };
	uint8_t signer_id[2] = {0x00, 0x00};
	uint8_t configdata[ATCA_CONFIG_SIZE];
	atcacert_device_loc_t device_locs[4];
	size_t device_locs_count = 0;
	size_t i;
	const atcacert_tm_utc_t issue_date = {
		.tm_year = 2016 - 1900,
		.tm_mon  = 7 - 1,
		.tm_mday = 19,
		.tm_hour = 20,
		.tm_min  = 0,
		.tm_sec  = 0
	};
	
	do {

		/* Read data from Configuration zone. */
		ret = atcab_read_config_zone(configdata);
		if (ret != ATCA_SUCCESS) break;

		/* Read the device public key from the zeroth slot of ATECC508A. */
		ret = atcab_get_pubkey(TLS_SLOT_AUTH_PRIV, pub_key);
		if (ret != ATCA_SUCCESS) break;

		/* Read Signer public key from slot B. */
		ret = aws_prov_get_signer_public_key(signer_pub_key);
		if (ret != ATCA_SUCCESS) break;

		/* Get the TBS digest to be generated by SHA2. */
		ret = aws_prov_build_tbs_cert_digest(cert_def, cert, &cert_size, signer_pub_key, 
				pub_key, signer_id, &issue_date, configdata, tbs_digest);
		if (ret != ATCACERT_E_SUCCESS) break;

		/* Set the signature to a certificate. */
		ret = atcacert_set_signature(cert_def, cert, &cert_size, max_cert_size, signature);
		if (ret != ATCACERT_E_SUCCESS) return ret;

		/* Get device list from certificate definition to write data into specified slot of ATECC508A. */
		ret = atcacert_get_device_locs(cert_def, device_locs, &device_locs_count, sizeof(device_locs) / sizeof(device_locs[0]), 32);
		if (ret != ATCACERT_E_SUCCESS) return ret;

		/* Write compressed certificate and signature to ATECC508A. */
		for (i = 0; i < device_locs_count; i++)	{

			size_t end_block, start_block, block;
			uint8_t data[96];

			if (device_locs[i].zone == DEVZONE_CONFIG)
				continue;
			if (device_locs[i].zone == DEVZONE_DATA && device_locs[i].is_genkey)
				continue;

			ret = atcacert_get_device_data(cert_def, cert, cert_size, &device_locs[i], data);
			if (ret != ATCACERT_E_SUCCESS) return ret;

			start_block = device_locs[i].offset / 32;
			end_block = (device_locs[i].offset + device_locs[i].count) / 32;
			for (block = start_block; block < end_block; block++) {
				ret = atcab_write_zone(device_locs[i].zone, device_locs[i].slot, (uint8_t)block, 0, &data[(block - start_block) * 32], 32);
				if (ret != ATCA_SUCCESS) return ret;
			}
		}		

	} while (0);

	return ret;
}

/**
 * \brief Store WIFI credential, Host address and Thing name into slot 8 of ATECC508A.
 *
 * \param[in] offset           Offset of slot 8
 * \param[in] data             Buffer pointer to be saved into specified offset
 * \param[in] len              Number of data parameter
 * \return KIT_STATUS_SUCCESS  On success
 */
uint8_t aws_prov_write_user_data(uint8_t offset, uint8_t* data, uint32_t len)
{
	uint8_t status = KIT_STATUS_SUCCESS;
	uint8_t userData[AWS_HOST_ADDR_MAX];
	size_t write_size = 0;

	memset(userData, 0x00, sizeof(userData));
	memcpy(userData, data, len);

	/* Make sure that number of bytes to be written should be multiple of 4 bytes. */
	if (offset == AWS_USER_DATA_OFFSET_SSID_LEN || offset == AWS_USER_DATA_OFFSET_PSK_LEN
		|| offset == AWS_USER_DATA_OFFSET_HOST_LEN 	|| offset == AWS_USER_DATA_OFFSET_THING_LEN) {
		write_size = sizeof(size_t);
	} else if (offset == AWS_USER_DATA_OFFSET_HOST) {
		write_size = sizeof(userData);
	} else {
		write_size = sizeof(userData) / 2;
	}

	/* Write the data with specified address and size. */
	status = atcab_write_bytes_zone(ATCA_ZONE_DATA, TLS_SLOT8_ENC_STORE, offset, userData, write_size);
	if (status != ATCA_SUCCESS) {
		AWS_ERROR("Failed to write user data!(%d)", status);
		status = AWS_E_CRYPTO_FAILURE;
	}

	return status;
}

/**
 * \brief Sign the device signature, if self-signed signature would be supported
 * In current, Chain of trust is only supported for AWS Kit firmware. (Not used.)
 *
 * \param[in] command          Command string
 * \param[out] response        Pointer of self-signed signature
 * \param[out] response_length Fixed 64 bytes length of the response packet
 * \return KIT_STATUS_SUCCESS  On success
 */
uint8_t aws_prov_sign_digest(char* command, uint8_t* response, uint16_t* response_length)
{
	uint8_t status = KIT_STATUS_SUCCESS;
	uint8_t *data_buffer = NULL;
	uint16_t buffer_length = 0;
	
	*response_length = 0;

	do {

		/* Extract the slot number of ATECC508A. */
		status = aws_prov_extract_data_load_tokens((const char*)command, &buffer_length, &data_buffer, '(', ',');
		if (status != KIT_STATUS_SUCCESS)
			break;

		/* Check for valid private key to use it for signing. */
		if (*data_buffer != TLS_SLOT_AUTH_PRIV) {
			status = KIT_STATUS_INVALID_PARAMS;
			break;
		}

		/* Extract the digest to sing it with own private key. */
		status = aws_prov_extract_data_load_tokens((const char*)command + (buffer_length * 2 + 1), &buffer_length, &data_buffer, ',', ')');
		if (status != KIT_STATUS_SUCCESS)
			break;

		if (buffer_length != ATCA_SHA_DIGEST_SIZE) {
			status = KIT_STATUS_INVALID_PARAMS;
			break;
		}

		/* Sign the digest to produce signature. */
		status = atcab_sign(TLS_SLOT_AUTH_PRIV, data_buffer, response);
		if (status != ATCA_SUCCESS)
			break;
		
		*response_length = VERIFY_256_SIGNATURE_SIZE;

	} while (0);
	
	return status;
}

/**
 * \brief Save Host address and Thing name for MQTT communication with AWS IoT.
 * Host address depending on AWS account is passed by Insight GUI.
 * Thing name will be typed by a user from Insight GUI.
 *
 * \param[in] command          Command string
 * \param[out] response        Number of bytes (Not used.)
 * \param[out] response_length Length of the response packet
 * \return KIT_STATUS_SUCCESS  On success
 */
uint8_t aws_prov_save_host_thing(char* command, uint8_t* response, uint16_t* response_length)
{
	uint8_t status = KIT_STATUS_SUCCESS;
	uint8_t *data_buffer = NULL, dataLen[ATCA_WORD_SIZE];
	uint16_t buffer_length = 0;
	uint32_t bufLen = 0;

	*response_length = 0;

	do {

		/* Extract Host address */
		status = aws_prov_extract_data_load_tokens((const char*)command, &buffer_length, &data_buffer, '(', ',');
		if (status != KIT_STATUS_SUCCESS)
			break;

		/* Write length of Host address to specified offset of slot 8 */
		bufLen = buffer_length;
		memset(dataLen, 0x00, sizeof(dataLen));
		dataLen[0] = (bufLen >> 0) & 0xFF;
		dataLen[1] = (bufLen >> 8) & 0xFF;
		dataLen[2] = (bufLen >> 16) & 0xFF;
		dataLen[3] = (bufLen >> 24) & 0xFF;
		status = aws_prov_write_user_data(AWS_USER_DATA_OFFSET_HOST_LEN, dataLen, bufLen);
		if (status != KIT_STATUS_SUCCESS)
			break;

		/* Write actual Host address to specified offset of slot 8 */
		status = aws_prov_write_user_data(AWS_USER_DATA_OFFSET_HOST, data_buffer, buffer_length);
		if (status != KIT_STATUS_SUCCESS)
			break;

		/* Extract Thing name */
		status = aws_prov_extract_data_load_tokens((const char*)command + (buffer_length * 2 + 1), &buffer_length, &data_buffer, ',', ')');
		if (status != KIT_STATUS_SUCCESS)
			break;

		/* Write length of Thing name to specified offset of slot 8 */
		bufLen = buffer_length;
		memset(dataLen, 0x00, sizeof(dataLen));
		dataLen[0] = (bufLen >> 0) & 0xFF;
		dataLen[1] = (bufLen >> 8) & 0xFF;
		dataLen[2] = (bufLen >> 16) & 0xFF;
		dataLen[3] = (bufLen >> 24) & 0xFF;
		status = aws_prov_write_user_data(AWS_USER_DATA_OFFSET_THING_LEN, dataLen, bufLen);
		if (status != KIT_STATUS_SUCCESS)
			break;

		/* Write actual Thing name to specified offset of slot 8 */
		status = aws_prov_write_user_data(AWS_USER_DATA_OFFSET_THING, data_buffer, buffer_length);
		if (status != KIT_STATUS_SUCCESS)
			break;

	} while (0);
	
	return status;
}

/**
 * \brief Save WIFI credential, for ATWINC1500 to access to WIFI router.
 * this credential is saved into slot 8 of ATECC508A.
 * Insight GUI will pass them typed by a user.
 *
 * \param[in] command          Command string
 * \param[out] response        Number of bytes (Not used.)
 * \param[out] response_length Length of the response packet
 * \return KIT_STATUS_SUCCESS  On success
 */
uint8_t aws_prov_save_wifi_credential(char* command, uint8_t* response, uint16_t* response_length)
{
	uint8_t status = KIT_STATUS_SUCCESS;
	uint8_t *data_buffer, dataLen[ATCA_WORD_SIZE];
	uint16_t buffer_length = 0;
	uint32_t bufLen = 0;

	*response_length = 0;

	do {

		/* Extract WIFI SSID */
		status = aws_prov_extract_data_load_tokens((const char*)command, &buffer_length, &data_buffer, '(', ',');
		if (status != KIT_STATUS_SUCCESS)
			break;

		/* Write length of SSID to specified offset of slot 8 */
		bufLen = buffer_length;
		memset(dataLen, 0x00, sizeof(dataLen));
		dataLen[0] = (bufLen >> 0) & 0xFF;
		dataLen[1] = (bufLen >> 8) & 0xFF;
		dataLen[2] = (bufLen >> 16) & 0xFF;
		dataLen[3] = (bufLen >> 24) & 0xFF;
		status = aws_prov_write_user_data(AWS_USER_DATA_OFFSET_SSID_LEN, dataLen, bufLen);
		if (status != KIT_STATUS_SUCCESS)
			break;

		/* Write actual SSID to specified offset of slot 8 */
		status = aws_prov_write_user_data(AWS_USER_DATA_OFFSET_SSID, data_buffer, buffer_length);
		if (status != KIT_STATUS_SUCCESS)
			break;

		/* Extract WIFI Password */
		status = aws_prov_extract_data_load_tokens((const char*)command + (buffer_length * 2 + 1), &buffer_length, &data_buffer, ',', ')');
		if (status != KIT_STATUS_SUCCESS)
			break;

		/* Write length of Password to specified offset of slot 8 */
		bufLen = buffer_length;
		memset(dataLen, 0x00, sizeof(dataLen));
		dataLen[0] = (bufLen >> 0) & 0xFF;
		dataLen[1] = (bufLen >> 8) & 0xFF;
		dataLen[2] = (bufLen >> 16) & 0xFF;
		dataLen[3] = (bufLen >> 24) & 0xFF;
		status = aws_prov_write_user_data(AWS_USER_DATA_OFFSET_PSK_LEN, dataLen, bufLen);
		if (status != KIT_STATUS_SUCCESS)
			break;

		/* Write actual Password to specified offset of slot 8 */
		status = aws_prov_write_user_data(AWS_USER_DATA_OFFSET_PSK, data_buffer, buffer_length);
		if (status != KIT_STATUS_SUCCESS)
			break;

	} while (0);
	
	return status;
}

/**
 * \brief Save the certificate to slot specified by certificate definition.
 * Both Signer and Device certificates will be passed to TLS library for session establishment.
 *
 * \param[in] command          Command string
 * \param[out] response        Number of bytes (Not used.)
 * \param[out] response_length Length of the response packet
 * \return KIT_STATUS_SUCCESS  On success
 */
uint8_t aws_prov_save_cert(char* command, uint8_t* response, uint16_t* response_length)
{
	uint8_t status = KIT_STATUS_SUCCESS;
	const atcacert_def_t* cert_def;
	uint8_t der_cert[AWS_CERT_LENGH_MAX];
	size_t der_cert_size = sizeof(der_cert);
	uint8_t *data_buffer;
	char* start_pem = NULL;
	char* end_pem = NULL;
	uint16_t buffer_length = 0;

	*response_length = 0;
	
	do {

		/* Extract identifier to designate either Signer or Device certificate definition. */
		status = aws_prov_extract_data_load_tokens((const char*)command, &buffer_length, &data_buffer, '(', ',');
		if (status != KIT_STATUS_SUCCESS)
			break;

		/* Set certificate definition with the identifier. */
		if (*data_buffer == AWS_SIGNER_CERT_ID)
			cert_def = &g_cert_def_1_signer;
		else if (*data_buffer == AWS_DEVICE_CERT_ID)
			cert_def = &g_cert_def_2_device;
		else
			break;

		start_pem = strstr(command, PEM_CERT_BEGIN);
		end_pem = strstr(command, PEM_CERT_END);
		buffer_length = end_pem - start_pem + sizeof(PEM_CERT_END) + 1;

		/* Convert PEM to DER certificate. */
		status = atcacert_decode_pem_cert((const char*)start_pem, buffer_length, der_cert, &der_cert_size);
		if (status != ATCA_SUCCESS)
			break;

		/* Write the certificate into ATECC508A based on the certificate definition. */
		status = atcacert_write_cert(cert_def, der_cert, der_cert_size);
		if (status != ATCA_SUCCESS)
			break;

	} while (0);
	
	return status;
}

/**
 * \brief Save the signature for the Signer or Device certificate.
 * The signature will be added to the certificate that was used to generate TBS for Device.
 * Then stored appropriately for reconstruction.
 *
 * \param[in] command          Command string
 * \param[out] response        Number of bytes (Not used.)
 * \param[out] response_length Length of the response packet
 * \return KIT_STATUS_SUCCESS  On success
 */
uint8_t aws_prov_save_signature(char* command, uint8_t* response, uint16_t* response_length)
{
	uint8_t status = KIT_STATUS_SUCCESS;
	uint8_t *data_buffer;
	uint16_t buffer_length = 0;
	uint8_t cert_id;

	*response_length = 0;
	
	do {

		/* Extract identifier to make sure that Device needs to save only Signer and Device certificate. */
		status = aws_prov_extract_data_load_tokens((const char*)command, &buffer_length, &data_buffer, '(', ',');
		if (status != KIT_STATUS_SUCCESS)
			break;

		/* Check for if the identifier is correct. */
		if (*data_buffer != AWS_SIGNER_CERT_ID && *data_buffer != AWS_DEVICE_CERT_ID)
			break;
		else
			cert_id = *data_buffer;

		/* Extract signature signed by Root or Signer. */
		status = aws_prov_extract_data_load_tokens((const char*)command + (buffer_length * 2 + 1), &buffer_length, &data_buffer, ',', ')');
		if (status != KIT_STATUS_SUCCESS)
			break;
		if (buffer_length != ATCA_SIG_SIZE) 	{
			status = KIT_STATUS_INVALID_PARAMS;
			break;
		}

		/* Extract signature signed by Root or Signer. */
		status = aws_prov_build_and_save_cert(data_buffer, cert_id);
		if (status != ATCA_SUCCESS)
			break;

	} while (0);
	
	return status;
}

/**
 * \brief Read Device public key associated with the private key of slot 0.
 * The signature will be added to the certificate that was used to generate TBS for Device.
 *
 * \param[in] command          Command string
 * \param[out] response        Number of bytes (Not used.)
 * \param[out] response_length Length of the response packet
 * \return KIT_STATUS_SUCCESS  On success
 */
uint8_t aws_prov_get_public_key(char* command, uint8_t* response, uint16_t* response_length)
{
	uint8_t status = KIT_STATUS_SUCCESS;
	uint8_t *data_buffer;
	uint16_t buffer_length = 0;
		
	*response_length = 0;

	do {

		/* Extract slot ID to read public key from the slot. */
		status = aws_prov_extract_data_load((const char*)command, &buffer_length, &data_buffer);
		if (status != KIT_STATUS_SUCCESS)
			break;

		/* Zero slot should be passed from Insight GUI. */
		if (*data_buffer != TLS_SLOT_AUTH_PRIV) {
			status = KIT_STATUS_INVALID_PARAMS;
			break;
		}

		/* Read the device public key from the zeroth slot of ATECC508A. */
		status = atcab_get_pubkey(*data_buffer, response);
		if (status != ATCA_SUCCESS)
			break;

		/* Set RX length to 64 bytes. */
		*response_length = ATCA_PUB_KEY_SIZE;

	} while (0);
	
	return status;
}

/**
 * \brief Save the signature for a certificate.
 * The signature will be added to the certificate that was used to generate TBS signature for Device.
 *
 * \param[in] command          Command string
 * \param[out] response        Number of bytes extracted
 * \param[out] response_length Length of the response packet
 * \return KIT_STATUS_SUCCESS  On success
 */
uint8_t aws_prov_get_cert(char* command, uint8_t* response, uint16_t* response_length)
{
	uint8_t status = KIT_STATUS_SUCCESS;
	uint8_t der_cert[512], pem_cert[AWS_CERT_LENGH_MAX];
	size_t der_cert_size = sizeof(der_cert);
	size_t pem_cert_size = sizeof(pem_cert);
	uint8_t *data_buffer, signer_pub_key[ATCA_PUB_KEY_SIZE] = { 0 };
	uint16_t buffer_length = 0;

	do {

		/* Extract identifier from command string. */
		status = aws_prov_extract_data_load((const char*)command, &buffer_length, &data_buffer);
		if (status != KIT_STATUS_SUCCESS) {
			break;
		}

		/* Check for the identifier. */
		if (*data_buffer != AWS_DEVICE_CERT_ID) {
			status = KIT_STATUS_INVALID_PARAMS;
			break;
		}

		/* Read Signer public key from slot B. */
		status = aws_prov_get_signer_public_key(signer_pub_key);
		if (status != ATCA_SUCCESS) break;

		/* Read Device certificate specified by certificate definition from ATECC508A.  */
		status = atcacert_read_cert(&g_cert_def_2_device, signer_pub_key, der_cert, &der_cert_size);
		if (status != ATCA_SUCCESS) 
			break;

		/* Convert DER to PEM certificate. */
		status = atcacert_encode_pem_cert(der_cert, der_cert_size, (char*)pem_cert, &pem_cert_size);
		if (status != ATCA_SUCCESS)
			break;

		/* Copy PEM certificate to USB TX buffer. */
		*response_length = (uint16_t)pem_cert_size;
		memcpy(response, pem_cert, pem_cert_size);

	} while (0);
	
	return status;
}

/**
 * \brief Build 32 bytes hash of TBS portion of the certificate.
 * These 32 bytes digest can be signed by Signer(PKI parent).
 *
 * \param[in] command          Command string
 * \param[out] tbs_digest      32 bytes digest
 * \param[out] tbs_size        Length of the digest
 * \return KIT_STATUS_SUCCESS  On success
 */
uint8_t aws_prov_build_device_tbs(char* command, uint8_t* tbs_digest, uint16_t* tbs_size)
{
	uint8_t status = KIT_STATUS_SUCCESS;
	uint8_t *data_buffer;
	uint16_t buffer_length;
	bool lockstate = false;
	uint8_t device_public_key[ATCA_PUB_KEY_SIZE] = { 0 };
	uint8_t device_cert[AWS_CERT_LENGH_MAX];
	size_t  device_cert_size = sizeof(device_cert);
	uint8_t signer_id[] = { 0x00, 0x00 };
	uint8_t configdata[ATCA_CONFIG_SIZE];
	const atcacert_tm_utc_t device_issue_date = {
		.tm_year = 2016 - 1900,
		.tm_mon  = 7 - 1,
		.tm_mday = 19,
		.tm_hour = 20,
		.tm_min  = 0,
		.tm_sec  = 0
	};

	do {

        /* Extract the signer public key from command string. */
		status = aws_prov_extract_data_load((const char*)command, &buffer_length, &data_buffer);
		if (status != KIT_STATUS_SUCCESS) break;

		/* Return failure, if ATECC508A has not been configured. */
		status = atcab_is_locked(LOCK_ZONE_CONFIG, &lockstate);
		if (status != ATCA_SUCCESS || !lockstate) break;

		/* Read data from Configuration zone. */
		status = atcab_read_config_zone(configdata);
		if (status != ATCA_SUCCESS) break;

		/* Generate a pair of key from slot 0, and get the Device public key to be included in certificate. */
		status = atcab_genkey(TLS_SLOT_AUTH_PRIV, device_public_key);
		if (status != ATCA_SUCCESS) break;

		/* Get the TBS digest to be generated by SHA2. */
		status = aws_prov_build_tbs_cert_digest(&g_cert_def_2_device, device_cert, &device_cert_size, data_buffer,
				device_public_key, signer_id, &device_issue_date, configdata, tbs_digest);
		if (status != ATCA_SUCCESS) break;
		*tbs_size = ATCA_SHA_DIGEST_SIZE;

	} while(0);

	return status;
}

/**
 * \brief This function parses communication commands received from Insight GUI in the context of an AWS application.
 * aws: Indicates that this is a command for the AWS Starter Kit.
 * The second parameter indicates the command to be executed.
 *
 * Kit Protocol
 * b[oard:version]             returns kit string and version string
 * aw[s]:i[nit]                aws_init
 * aw[s]:si[gn]                aws_sign
 * aw[s]:g[etcert]             aws_get_cert
 * aw[s]:p[ubkey]              aws_get_pubkey
 * aw[s]:ss[ignature]          aws_save_cert
 * aw[s]:sw[ifissid]           aws_save_wifi_ssid
 * aw[s]:sh[ost]               aws_save_host_address
 *
 * \param[in] commandLength    Length of command (Not used.)
 * \param[in] command          The second parameter of entire command
 * \param[out] resposeLength   Number of bytes of TX data
 * \param[out] response        Pointer of TX data
 * \return KIT_STATUS_SUCCESS  On success
 */
uint8_t aws_prov_parse_aws_commands(uint16_t commandLength, uint8_t *command, uint16_t *responseLength, uint8_t *response)
{
	uint8_t status = KIT_STATUS_SUCCESS;
	t_aws_kit* kit = aws_kit_get_instance();
	char* pToken = strchr((char *) command, ':');
	*responseLength = 0;
	
	if (!pToken)
	return KIT_STATUS_UNKNOWN_COMMAND;
	
	switch (pToken[1]) 
	{
		/* "aw[s]:i(signer public key)" */
		case 'I':
		case 'i':
			status = aws_prov_build_device_tbs(pToken + 1, response, responseLength);
			if (status == KIT_STATUS_SUCCESS && kit->clientState == CLIENT_STATE_MQTT_WAIT_MESSAGE)
				kit->noti = NOTI_QUIT_MQTT_CLIENT;
			break;
		/* "aw[s]:g(identifier)" */
		case 'G':
		case 'g':
			if (pToken[2] == 'r' || pToken[2] == 'R')
				status = aws_prov_get_root_public_key(response, responseLength);
			else
				status = aws_prov_get_cert(pToken + 1, response, responseLength);
			break;
		/* "aw[s]:p(slotId)" */
		case 'p':
		case 'P':
			status = aws_prov_get_public_key(pToken + 1, response, responseLength);
			break;
		break;
		case 'S':
		case 's':
			switch (pToken[2])
			{
				/* aw[s]:ss(certId,signature) */
				case 'S':
				case 's':	
					status = aws_prov_save_signature(pToken + 2, response, responseLength);
				break;
				/* aw[s]:sc(Id,Cert) */
				case 'C':
				case 'c':
					status = aws_prov_save_cert(pToken + 2, response, responseLength);
				break;
				/* aw[s]:sh(Host,Thing) */
				case 'H':
				case 'h':
					status = aws_prov_save_host_thing(pToken + 2, response, responseLength);
				break;
				/* aw[s]:sw(SSID,Password) */
				case 'W':
				case 'w':
					status = aws_prov_save_wifi_credential(pToken + 2, response, responseLength);
					if (status == KIT_STATUS_SUCCESS)
						kit->noti = NOTI_RUN_MQTT_CLIENT;
				break;
				/* aw[s]:si(slotId,digest) */
				case 'I':
				case 'i':
					status = aws_prov_sign_digest(pToken + 2, response, responseLength);
				break;
				/* aw[s]:sr(Root Public Key) */
				case 'R':
				case 'r':
					status = aws_prov_save_root_public_key(pToken + 2, response, responseLength);
				break;
				default:
					status = KIT_STATUS_UNKNOWN_COMMAND;
				break;
			} 
		break;
		default:
			status = KIT_STATUS_UNKNOWN_COMMAND;
	}

	return status;
}

/**
 * \brief USB packet handler is to receive AWS Kit commands, and send a result back over CDC interface.
 * For the device provisioning and certificate registration, Insight GUI will be sending AWS Kit commands. 
 *
 */
void aws_prov_handler(void)
{
	uint16_t rx_length = 0, tx_length = 0;
	uint8_t* tx_buffer = NULL;

	/* Check for received data */
   	if ((udi_cdc_is_rx_ready()) && ((rx_length = udi_cdc_get_nb_received_data()) > 0)) {
		
		memset(pucUsbRxBuffer, 0, sizeof(pucUsbRxBuffer));
		memset(pucUsbTxBuffer, 0, sizeof(pucUsbTxBuffer));

		/* Read received data into RX buffer */
		rx_length = udi_cdc_get_nb_received_data();
		udi_cdc_read_buf((void *)pucUsbRxBuffer, rx_length);

		/* Parse received data and execute command */
		tx_buffer = aws_prov_process_usb_packet(rx_length, &tx_length);

		/* Write a result of command execution into TX buffer to notify it to Insight GUI */
		if(udi_cdc_is_tx_ready() && tx_length > 0) {
			udi_cdc_write_buf((const void *)tx_buffer, tx_length);				
		}
   	}	
}

/**
 * \brief Provisioning task runs in the background to parse & execute USB kit extension commands come from Insight GUI.
 * Typically the kit protocol has been defined to primarily pass commands, parameters and data through to various crypto 
 * security devices. The complexity of these commands are at a higher level and therefore the AWS commands will trigger 
 * a subroutine made up of device level kit protocol commands.
 *
 * \param[in] params           Parameters for the task (Not used)
 */
void aws_prov_task(void* params)
{
	uint8_t notiBuffer[1];
	t_aws_kit* kit = aws_kit_get_instance();

	/* Enable USB device controller. */
	udc_start();

	for (;;) {

		ioport_toggle_pin_level(LED0_GPIO);

		/* Monitor USB CDC packet, and then execute a command. */
		aws_prov_handler();

		/* Check for the last aws kit command to resume Client task. */
		if (kit->noti == NOTI_RUN_MQTT_CLIENT || kit->noti == NOTI_QUIT_MQTT_CLIENT) {
			notiBuffer[0] = kit->noti;
			kit->noti = NOTI_INVALID;
			/* Notify to Main task that provisioning has been completed. */ 
			xQueueSendToFront(kit->notiQueue, notiBuffer, 1);
		}
		ioport_toggle_pin_level(LED0_GPIO);

		/* Block for 100ms. */
		vTaskDelay(AWS_PROV_TASK_DELAY);
	}
}
