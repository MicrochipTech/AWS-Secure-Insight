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

#ifndef AWS_PROV_TASK_H_
#define AWS_PROV_TASK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asf.h>
#include <stdint.h>
#include "cryptoauthlib.h"
#include "atcacert/atcacert_client.h"

/**
 * \defgroup Provisioning Task Definition
 *
 * @{
 */

/** \name Provisioning Task configuration
   @{ */
#define AWS_PROV_TASK_PRIORITY					(tskIDLE_PRIORITY + 4)
#define AWS_PROV_TASK_DELAY						(100 / portTICK_RATE_MS)
#define AWS_PROV_TASK_STACK_SIZE				(2048)
/** @} */

#define SHA204_RSP_SIZE_MIN						((uint8_t)  4)	//!< minimum number of bytes in response
#define SHA204_RSP_SIZE_MAX						((uint8_t) 100)	//!< maximum size of response packet

#define SHA204_BUFFER_POS_COUNT					(0)				//!< buffer index of count byte in command or response
#define SHA204_BUFFER_POS_DATA					(1)				//!< buffer index of data in response

#define USB_BUFFER_SIZE_TX						(2048)			//!< max USB tx buffer size
#define USB_BUFFER_SIZE_RX						(2048)			//!< max USB rx buffer size

//! Commands and responses are terminated by this character.
#define KIT_EOP									'\n'

//! number of characters in response excluding data
#define KIT_RESPONSE_COUNT_NO_DATA				(5)

//! Every device response byte is converted into two hex-ascii characters.
#define KIT_CHARS_PER_BYTE						(2)

#define DEVICE_BUFFER_SIZE_MAX_RX				(uint16_t) ((USB_BUFFER_SIZE_TX - KIT_RESPONSE_COUNT_NO_DATA) / KIT_CHARS_PER_BYTE)

#define DISCOVER_DEVICE_COUNT_MAX				(1)

// I2C address for device programming and initial communication
#define FACTORY_INIT_I2C						(uint8_t)(0xC0)	//!< Initial I2C address is set to 0xC0 in the factory
#define DEVICE_I2C								(uint8_t)(0xB0)	//!< Device I2C Address to program device to

#define AWS_ROOT_CERT_ID						(uint8_t)(0x00)	//!< AWS Root Certificate Identifier		
#define AWS_SIGNER_CERT_ID						(uint8_t)(0x01)	//!< AWS Signer Certificate Identifier
#define AWS_VERIF_CERT_ID						(uint8_t)(0x02)	//!< AWS Verification Certificate Identifier
#define AWS_DEVICE_CERT_ID						(uint8_t)(0x03)	//!< AWS Device Certificate Identifier

// AES132 library occupies codes between 0x00 and 0xB4.
// SHA204 library occupies codes between 0xD0 and 0xF7.
enum {
	KIT_STATUS_SUCCESS             = 0x00,
	KIT_STATUS_UNKNOWN_COMMAND     = 0xC0,
	KIT_STATUS_USB_RX_OVERFLOW     = 0xC1,
	KIT_STATUS_USB_TX_OVERFLOW     = 0xC2,
	KIT_STATUS_INVALID_PARAMS      = 0xC3,
	KIT_STATUS_INVALID_IF_FUNCTION = 0xC4,
	KIT_STATUS_NO_DEVICE           = 0xC5
};

//! USB packet state.
enum usb_packet_state
{
	PACKET_STATE_IDLE,
	PACKET_STATE_TAKE_DATA,
	PACKET_STATE_END_OF_VALID_DATA,
	PACKET_STATE_OVERFLOW
};

//! enumeration for device types
typedef enum {
	DEVICE_TYPE_UNKNOWN,   //!< unknown device
	DEVICE_TYPE_CM,        //!< CM, currently not supported
	DEVICE_TYPE_CRF,       //!< CRF, currently not supported
	DEVICE_TYPE_CMC,       //!< CMC, currently not supported
	DEVICE_TYPE_SA100S,    //!< SA100S, can be discovered, but is currently not supported
	DEVICE_TYPE_SA102S,    //!< SA102S, can be discovered, but is currently not supported
	DEVICE_TYPE_SA10HS,    //!< SA10HS, can be discovered, but is currently not supported
	DEVICE_TYPE_SHA204,    //!< SHA204 device
	DEVICE_TYPE_AES132,    //!< AES132 device
	DEVICE_TYPE_ECC108     //!< ECC108 device
} device_type_t;

//! enumeration for interface types
typedef enum {
	DEVKIT_IF_UNKNOWN,
	DEVKIT_IF_SPI,
	DEVKIT_IF_I2C,
	DEVKIT_IF_SWI,
	DEVKIT_IF_UART
} interface_id_t;

//! information about a discovered device
typedef struct {
	//! interface type (SWI, I2C, SPI)
	interface_id_t bus_type;
	//! device type
	device_type_t device_type;
	//! I2C address or selector byte
	uint8_t address;
	//! array index into GPIO structure for SWI and SPI (Microbase does not support this for SPI.)
	uint8_t device_index;
	//! revision bytes (four bytes for SHA204 and ECC108, first two bytes for AES132)
	uint8_t dev_rev[4];
} device_info_t;

uint8_t* aws_prov_get_rx_buffer(void);
uint8_t* aws_prov_get_tx_buffer(void);
uint8_t aws_prov_convert_nibble_to_ascii(uint8_t nibble);
uint8_t aws_prov_convert_ascii_to_nibble(uint8_t ascii);
uint16_t aws_prov_convert_ascii_to_binary(uint16_t length, uint8_t *buffer);
device_info_t* aws_prov_get_device_info(uint8_t index);
device_type_t aws_prov_get_device_type(uint8_t index);
ATCA_STATUS aws_prov_detect_I2c_devices(void);
interface_id_t aws_prov_discover_devices(void);
uint8_t aws_prov_parse_board_commands(uint16_t commandLength, uint8_t* command, uint16_t* responseLength, uint8_t* response, uint8_t* responseIsAscii);
uint8_t aws_prov_get_commands_info(uint8_t* tx_buffer, uint8_t* cmd_index, uint16_t* rx_length);
uint8_t aws_prov_send_and_receive(uint8_t* tx_buffer, uint8_t* rx_buffer);
uint8_t aws_prov_send_command(uint8_t* tx_buffer);
uint8_t aws_prov_receive_response(uint8_t size, uint8_t* rx_buffer);
uint8_t aws_prov_parse_ecc_commands(uint16_t commandLength, uint8_t* command, uint16_t* responseLength, uint8_t* response);
uint8_t aws_prov_extract_data_load(const char* command, uint16_t* dataLength, uint8_t** data);
uint8_t aws_prov_extract_data_load_tokens(const char* command, uint16_t* dataLength, uint8_t** data, uint8_t start_token, uint8_t end_token);
uint16_t aws_prov_create_usb_packet(uint16_t length, uint8_t *buffer);
uint16_t aws_prov_convert_data(uint16_t length, uint8_t *buffer);
uint8_t* aws_prov_process_usb_packet(uint16_t rx_length, uint16_t* txLength);
uint8_t aws_prov_save_root_public_key(char* command, uint8_t* response, uint16_t* response_length);
uint8_t aws_prov_get_root_public_key(uint8_t* response, uint16_t* response_length);
uint8_t aws_prov_save_signer_public_key(uint8_t* public_key);
uint8_t aws_prov_get_signer_public_key(uint8_t* public_key);
uint8_t aws_prov_build_tbs_cert_digest(const atcacert_def_t* cert_def, uint8_t* cert, size_t* cert_size, const uint8_t* ca_public_key,
		const uint8_t* public_key, const uint8_t* signer_id, const atcacert_tm_utc_t* issue_date, const uint8_t* config32, uint8_t* tbs_digest);
uint8_t aws_prov_build_and_save_cert(uint8_t* signature, uint8_t cert_id);
uint8_t aws_prov_write_user_data(uint8_t offset, uint8_t* data, uint32_t len);
uint8_t aws_prov_sign_digest(char* command, uint8_t* response, uint16_t* response_length);
uint8_t aws_prov_save_host_thing(char* command, uint8_t* response, uint16_t* response_length);
uint8_t aws_prov_save_wifi_credential(char* command, uint8_t* response, uint16_t* response_length);
uint8_t aws_prov_save_cert(char* command, uint8_t* response, uint16_t* response_length);
uint8_t aws_prov_save_signature(char* command, uint8_t* response, uint16_t* response_length);
uint8_t aws_prov_get_public_key(char* command, uint8_t* response, uint16_t* response_length);
uint8_t aws_prov_get_cert(char* command, uint8_t* response, uint16_t* response_length);
uint8_t aws_prov_build_device_tbs(char* command, uint8_t* tbs_digest, uint16_t* tbs_size);
uint8_t aws_prov_parse_aws_commands(uint16_t commandLength, uint8_t *command, uint16_t *responseLength, uint8_t *response);
void aws_prov_handler(void);
void aws_prov_task(void *params);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AWS_PROV_TASK_H_ */
