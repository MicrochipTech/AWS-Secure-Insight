/**
 *
 * \file
 *
 * \brief Platform network interface
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

#include <wolfssl/ssl.h>
#include <wolfssl/internal.h>

#include "aws_kit_object.h"
#include "network_interface.h"
#include "aws_net_interface.h"
#include "common/include/nm_common.h"
#include "driver/include/m2m_wifi.h"
#include "cryptoauthlib.h"


static uint8_t *tlsSocketBuf = NULL;

/**
 * \brief Reads data from the WolfSSL library.
 *
 * \param network[in]               The Eclipse Paho MQTT network information
 * \param read_buffer[in]           The buffer
 * \param length[in]                The buffer length
 * \param timeout_ms[in]            The timeout
 *
 * \return    The MQTT status
 */
int mqtt_packet_read(Network *network, unsigned char *read_buffer, int length, int timeout_ms)
{
	int ret;
	int error;
	int position = 0;
	int count = length;
	t_aws_kit* kit = aws_kit_get_instance();	

	do {
		ret = wolfSSL_read(kit->tls.ssl, (char*)&read_buffer[position], count);
		error = wolfSSL_get_error(kit->tls.ssl, 0);
		
		if (ret > 0) {
			position += ret;
			count -= ret;
		} else {
			break;
		}
	} while (count > 0);
	
	if (error == SSL_ERROR_WANT_READ)
		return FAILURE;
	
	return ret;
}

/**
 * \brief Writes data to the WolfSSL library.
 *
 * \param network[in]               The Eclipse Paho MQTT network information
 * \param send_buffer[in]           The buffer
 * \param length[in]                The buffer length
 * \param timeout_ms[in]            The timeout
 *
 * \return    The MQTT status
 */
int mqtt_packet_write(Network *network, unsigned char *send_buffer, int length, int timeout_ms)
{
	int ret;
	int error;
	t_aws_kit* kit = aws_kit_get_instance();
	
	ret = wolfSSL_write(kit->tls.ssl, (void*)send_buffer, length);
	error = wolfSSL_get_error(kit->tls.ssl, 0);
	
	if (error == SSL_ERROR_WANT_WRITE)
		return FAILURE;
		
	return ret;
}


/**
 * \brief Initializes the network socket library.
 */
void network_socket_init(void)
{
	socketInit();	
}

/**
 * \brief Connects the network socket library to the IP address and port.
 *
 * \param network_socket[out]       The network socket
 * \param address[in]               The network IP address
 * \param port[in]                  The network port
 * \param timeout_ms[in]            The timeout
 *
 * \return    The network socket status
 */
int network_socket_connect(SOCKET *network_socket, uint32_t address, uint16_t port, int timeout_ms)
{
	int ret = SOCK_ERR_INVALID;
	struct sockaddr_in socket_address;
	Timer connection_timer;

	if (network_socket == NULL)
		return SOCK_ERR_INVALID_ARG;
	
	/* Create the socket */
	*network_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (*network_socket != SOCK_ERR_NO_ERROR)
		return SOCK_ERR_INVALID;
	
	/* Set the socket address information */
	socket_address.sin_family      = AF_INET;
	socket_address.sin_addr.s_addr = address;
	socket_address.sin_port        = _htons(port);

	ret = connect(*network_socket, (struct sockaddr*)&socket_address, sizeof(struct sockaddr));
	if (ret != SOCK_ERR_NO_ERROR)
		return ret;	

	
	TimerInit(&connection_timer);
	TimerCountdownMS(&connection_timer, timeout_ms);

	while (!GET_SOCKET_STATUS(SOCKET_STATUS_CONNECT)) {
		if(TimerIsExpired(&connection_timer))
			return SOCK_ERR_TIMEOUT;

		m2m_wifi_handle_events(NULL);
	}
	
	return SOCK_ERR_NO_ERROR;
}

/**
 * \brief Disconnects the network socket library.
 *
 * \param network_socket[in]        The network socket
 *
 * \return    The network socket status
 */
int network_socket_disconnect(SOCKET *network_socket)
{
	if (network_socket == NULL)
		return SOCK_ERR_INVALID_ARG;
		
	close(*network_socket);
	
	return SOCK_ERR_NO_ERROR;
}

/**
 * \brief Reads data from the network socket library.
 *
 * \param network[in]               The network socket
 * \param read_buffer[in]           The buffer
 * \param length[in]                The buffer length
 * \param flags[in]                 The network socket read flags
 * \param timeout_ms[in]            The timeout
 *
 * \return    The MQTT status
 */
int network_socket_read(SOCKET *socket, unsigned char *read_buffer, int length, int flags, int timeout_ms)
{
    int i = length, totalLen = 0, bodyLen = 0, read_count = 0, left_size = 0, copy_count = 0;
	Timer waitTimer;
	t_aws_kit* kit = aws_kit_get_instance();

#ifdef AWS_KIT_DEBUG
	AWS_INFO("aws_net_receive_packet_cb = %d, %d, %d, %d", timeout_ms, length, kit->tls.ssl->options.processReply, kit->tls.ssl->options.handShakeState);
#endif
	if (kit->tls.ssl->options.connectState == CLIENT_HELLO_SENT) {
		if (kit->tls.ssl->options.processReply == 0 ) {
			tlsSocketBuf = (uint8_t*)malloc(MAIN_WIFI_M2M_BUFFER_SIZE * 3);
			if (tlsSocketBuf == NULL) {
				AWS_ERROR("Failed to allocate socket heap!\r\n");
				return WOLFSSL_CBIO_ERR_GENERAL;
			}

			if (recv(*socket, tlsSocketBuf, MAIN_WIFI_M2M_BUFFER_SIZE, flags)) {
				AWS_ERROR("Failed to receive packet!");
				return WOLFSSL_CBIO_ERR_CONN_CLOSE;
			}

			while (!GET_SOCKET_STATUS(SOCKET_STATUS_RECEIVE)) {
				m2m_wifi_handle_events(NULL);
			}
			DISABLE_SOCKET_STATUS(SOCKET_STATUS_RECEIVE);

			bodyLen = (tlsSocketBuf[3] << 8) | (tlsSocketBuf[4]);
			totalLen = bodyLen + RECORD_HEADER_SZ;

			read_count = totalLen / pstrRecv->s16BufferSize;
			left_size = totalLen % pstrRecv->s16BufferSize;
			copy_count = 1 * pstrRecv->s16BufferSize;

			while (read_count > 0) {
				if (read_count == 1) {
					if (recv(*socket, tlsSocketBuf + copy_count, MAIN_WIFI_M2M_BUFFER_SIZE, flags)) {
						AWS_ERROR("Failed to receive packet!");
						return WOLFSSL_CBIO_ERR_CONN_CLOSE;
					}

					while (!GET_SOCKET_STATUS(SOCKET_STATUS_RECEIVE)) {
						m2m_wifi_handle_events(NULL);
					}
					DISABLE_SOCKET_STATUS(SOCKET_STATUS_RECEIVE);
					copy_count += left_size;
				} else {
					if (recv(*socket, tlsSocketBuf + copy_count, MAIN_WIFI_M2M_BUFFER_SIZE, flags)) {
						AWS_ERROR("Failed to receive packet!");
						return WOLFSSL_CBIO_ERR_CONN_CLOSE;
					}

					while (!GET_SOCKET_STATUS(SOCKET_STATUS_RECEIVE)) {
						m2m_wifi_handle_events(NULL);
					}
					DISABLE_SOCKET_STATUS(SOCKET_STATUS_RECEIVE);
					copy_count += pstrRecv->s16BufferSize;
				}
				--read_count;
			}
			memcpy(read_buffer, &tlsSocketBuf[0], length);
		} else {
			memcpy(read_buffer, &tlsSocketBuf[RECORD_HEADER_SZ], length);
			if (tlsSocketBuf) {			
				free(tlsSocketBuf);
				tlsSocketBuf = NULL;
			}
		}
	} else if (kit->tls.ssl->options.connectState == FINISHED_DONE) {
		if (kit->tls.ssl->options.processReply == 0 ) {
			if (kit->tls.ssl->curRL.type == 22) {
				tlsSocketBuf = (uint8_t*)malloc(MAIN_WIFI_M2M_BUFFER_SIZE * 3);
				if (tlsSocketBuf == NULL) {
					AWS_ERROR("Failed to allocate heap!");
					return WOLFSSL_CBIO_ERR_GENERAL;
				}

				if (recv(*socket, tlsSocketBuf, MAIN_WIFI_M2M_BUFFER_SIZE, flags)) {
					AWS_ERROR("Failed to receive packet!");
					return WOLFSSL_CBIO_ERR_CONN_CLOSE;
				}

				while (!GET_SOCKET_STATUS(SOCKET_STATUS_RECEIVE)) {
					m2m_wifi_handle_events(NULL);
				}
				DISABLE_SOCKET_STATUS(SOCKET_STATUS_RECEIVE);
				memcpy(read_buffer, &tlsSocketBuf[0], length);

			} else {
				memcpy(read_buffer, &tlsSocketBuf[RECORD_HEADER_SZ + 1], length);
			}

		} else {

			if (kit->tls.ssl->curRL.type == 20) {
				memcpy(read_buffer, &tlsSocketBuf[RECORD_HEADER_SZ], length);
			} else {
				memcpy(read_buffer, &tlsSocketBuf[RECORD_HEADER_SZ * 2 + 1], length);
				if (tlsSocketBuf) {			
					free(tlsSocketBuf);
					tlsSocketBuf = NULL;
				}
			}
		}
	} else {
		if (kit->tls.ssl->options.processReply == 0 ) {
			tlsSocketBuf = (uint8_t*)malloc(MAIN_WIFI_M2M_BUFFER_SIZE);
			if (tlsSocketBuf == NULL) {
				AWS_ERROR("Failed to allocate heap!");
				return WOLFSSL_CBIO_ERR_GENERAL;
			}

			if (recv(*socket, tlsSocketBuf, MAIN_WIFI_M2M_BUFFER_SIZE, flags)) {
				AWS_ERROR("Failed to receive packet!");
				return WOLFSSL_CBIO_ERR_CONN_CLOSE;
			}

			TimerInit(&waitTimer);
			if (kit->clientState == CLIENT_STATE_MQTT_WAIT_MESSAGE)
				TimerCountdownMS(&waitTimer, AWS_NET_SUBSCRIBE_TIMEOUT_MS);
			else
				TimerCountdownMS(&waitTimer, timeout_ms);
			while (!GET_SOCKET_STATUS(SOCKET_STATUS_RECEIVE)) {
				if(TimerIsExpired(&waitTimer) && !kit->blocking) {
					AWS_INFO("Expired MQTT waiting timer to receive!");
					DISABLE_SOCKET_STATUS(SOCKET_STATUS_RECEIVE);
					if (tlsSocketBuf) {			
						free(tlsSocketBuf);
						tlsSocketBuf = NULL;
					}
					return 0;
				}
				m2m_wifi_handle_events(NULL);
			}
			DISABLE_SOCKET_STATUS(SOCKET_STATUS_RECEIVE);
			memcpy(read_buffer, &tlsSocketBuf[0], length);

		} else {
			memcpy(read_buffer, &tlsSocketBuf[RECORD_HEADER_SZ], length);
			if (tlsSocketBuf) {			
				free(tlsSocketBuf);
				tlsSocketBuf = NULL;
			}
		}
	}
#ifdef AWS_KIT_DEBUG
    atcab_printbin_label((const uint8_t*)"RECEIVED PACKET\r\n", (uint8_t*)read_buffer, length);
#endif
    return i;
}
	
/**
 * \brief Writes data to the network socket library.
 *
 * \param network[in]               The network socket
 * \param send_buffer[in]           The buffer
 * \param length[in]                The buffer length
 * \param flags[in]                 The network socket write flags
 * \param timeout_ms[in]            The timeout
 *
 * \return    The MQTT status
 */
int network_socket_write(SOCKET *socket, unsigned char *send_buffer, int length, int flags, int timeout_ms)
{
	int sent = 0;
	int count = 0;
	Timer wait_timer;
	t_aws_kit* kit = aws_kit_get_instance();
	
	if ((socket == NULL) || (send_buffer == NULL))
		return SOCK_ERR_INVALID_ARG;

#ifdef AWS_KIT_DEBUG
	AWS_INFO("aws_net_send_packet_cb = %d, %d, %d, %d", timeout_ms, length, kit->tls.ssl->options.processReply, kit->tls.ssl->options.handShakeState);
#endif
	while (count < length) {
		/* Write data to TCP socket buffer. */
		sent = ((length - count) > MAIN_WIFI_M2M_BUFFER_SIZE) ? MAIN_WIFI_M2M_BUFFER_SIZE : length;
		
		if (send(*socket, (void*)&send_buffer[count], sent, flags) < 0) {
			AWS_ERROR("Failed to send packet!");
			return SOCK_ERR_CONN_ABORTED;
		}

		TimerInit(&wait_timer);
		TimerCountdownMS(&wait_timer, timeout_ms);
		
		while (!GET_SOCKET_STATUS(SOCKET_STATUS_SEND) && !(kit->nonBlocking)) {
			if(TimerIsExpired(&wait_timer)) {
				DISABLE_SOCKET_STATUS(SOCKET_STATUS_SEND);
				return SOCK_ERR_TIMEOUT;
			}

			m2m_wifi_handle_events(NULL);
		}

		DISABLE_SOCKET_STATUS(SOCKET_STATUS_SEND);
		count += sent;
	}

#ifdef AWS_KIT_DEBUG
	atcab_printbin_label((const uint8_t*)"SENT PACKET\r\n", (uint8_t*)send_buffer, length);
#endif
	return count;
}
