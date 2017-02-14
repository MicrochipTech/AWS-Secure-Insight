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

#ifndef AWS_NET_INTERFACE_H_
#define AWS_NET_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/time.h>
#include "common/include/nm_common.h"
#include "driver/include/m2m_wifi.h"
#include "socket/include/socket.h"
#include "tls/atcatls.h"
#include <wolfssl/ssl.h>
#include <wolfssl/internal.h>
#include "cryptoauthlib.h"
#include "atcacert/atcacert_def.h"
#include "aws_kit_timer.h"
#include "aws_kit_object.h"

/**
 * \defgroup WIFI interface to communicate between ATWINC1500 host driver and MQTT client.
 *
 * @{
 */
 
/** \name WIFI configuration
   @{ */
#define MAIN_WLAN_AUTH							M2M_WIFI_SEC_WPA_PSK
#define MAIN_WIFI_M2M_BUFFER_SIZE				(1460)
#define MAIN_WLAN_POLL_TIMEOUT_SEC				(10)
/** @} */

/** \name Max timeout configuration
   @{ */
#define AWS_NET_CONN_TIMEOUT_MS					(5000)
#define AWS_NET_NTP_TIMEOUT_MS					(5000)
#define AWS_NET_SUBSCRIBE_TIMEOUT_MS			(1000)
/** @} */

/** \name TCP socket event definition
   @{ */
#define	SOCKET_STATUS_BIND						(1 << 0)	
#define	SOCKET_STATUS_LISTEN					(1 << 1)
#define	SOCKET_STATUS_ACCEPT					(1 << 2)
#define	SOCKET_STATUS_CONNECT					(1 << 3)	
#define	SOCKET_STATUS_RECEIVE					(1 << 4)	
#define	SOCKET_STATUS_SEND			    		(1 << 5)	
#define	SOCKET_STATUS_RECEIVE_FROM	    		(1 << 6)	
#define	SOCKET_STATUS_SEND_TO					(1 << 7)	
/** @} */

/** \name NTP socket event definition
   @{ */
#define	NTP_SOCKET_STATUS_BIND					(1 << 0)	
#define	NTP_SOCKET_STATUS_LISTEN				(1 << 1)	
#define	NTP_SOCKET_STATUS_ACCEPT				(1 << 2)	
#define	NTP_SOCKET_STATUS_CONNECT				(1 << 3)
#define	NTP_SOCKET_STATUS_RECEIVE				(1 << 4)	
#define	NTP_SOCKET_STATUS_SEND			    	(1 << 5)	
#define	NTP_SOCKET_STATUS_RECEIVE_FROM	    	(1 << 6)
#define	NTP_SOCKET_STATUS_SEND_TO				(1 << 7)	
/** @} */

/** \name NTP server definition
   @{ */
#define MAIN_WORLDWIDE_NTP_POOL_HOSTNAME		"pool.ntp.org"
#define MAIN_SERVER_PORT_FOR_UDP				(123)
#define MAIN_DEFAULT_ADDRESS					0xFFFFFFFF /* "255.255.255.255" */
/** @} */

extern uint16_t tls_socket_status;
extern uint16_t tcp_ntp_socket_status;

/**
 * Saves current tls socket status.
 */
#define ENABLE_SOCKET_STATUS(index)        		(tcp_socket_status |= index)
#define DISABLE_SOCKET_STATUS(index)        	(tcp_socket_status &= ~index)
#define GET_SOCKET_STATUS(index)        		(tcp_socket_status & index)

/**
 * Saves current ntp socket status.
 */
#define ENABLE_NTP_SOCKET_STATUS(index)        	(tcp_ntp_socket_status |= index)
#define DISABLE_NTP_SOCKET_STATUS(index)        (tcp_ntp_socket_status &= ~index)
#define GET_NTP_SOCKET_STATUS(index)        	(tcp_ntp_socket_status & index)

/**
 * Defines time & date structure.
 */
typedef struct time_date {
	int tm_sec;				//!< seconds after the minute [0-60]
	int tm_min;				//!< minutes after the hour [0-59]
	int tm_hour;			//!< hours since midnight [0-23]
	int tm_mday;			//!< day of the month [1-31]
	int tm_mon;				//!< months since January [0-11]
	int tm_year;			//!< years since 1900
	int tm_wday;			//!< days since Sunday [0-6]
	int tm_yday;			//!< days since January 1 [0-365]
	int tm_isdst;			//!< Daylight Savings Time flag
	long    tm_gmtoff;		//!< offset from CUT in seconds
	char    *tm_zone;		//!< timezone abbreviation
} t_time_date;

extern tstrSocketRecvMsg *pstrRecv;

int _gettimeofday(struct timeval *__p, void *__tz);

/** Function prototype.	*/
const char* aws_net_get_socket_string(int msg);
void aws_net_set_wifi_status(bool status);
bool aws_net_get_wifi_status(void);
void aws_net_wifi_cb(uint8_t u8MsgType, void* pvMsg);
int aws_net_init_wifi(t_aws_kit* kit, int timeout_sec);
int aws_net_get_time(t_aws_kit* kit);
void aws_net_socket_cb(SOCKET sock, uint8_t u8Msg, void *pvMsg);
void aws_net_set_ntp_socket(SOCKET socket);
SOCKET aws_net_get_ntp_socket(void);
int aws_net_set_current_time(uint32_t secs);
int aws_net_get_current_time(t_time_date* tm);
time_t aws_net_get_ntp_seconds(void);
int aws_net_get_ntp_time(uint32_t timeout_ms);
void aws_net_ntp_socket_cb(SOCKET sock, uint8_t u8Msg, void* pvMsg);
void aws_net_ntp_resolve_cb(uint8_t* pu8DomainName, uint32_t u32ServerIP);
int aws_net_compare_date(t_time_date* local, atcacert_tm_utc_t* cert);
void aws_net_dns_resolve_cb(uint8_t* pu8DomainName, uint32_t u32ServerIP);
uint32_t aws_net_get_host_addr(void);
void aws_net_set_host_addr(uint32_t addr);
int aws_net_connect_cb(void* context, const char* host, uint16_t port, int timeout_ms);
int aws_net_send_packet_cb(WOLFSSL* ssl, void* ctx, const byte* packet, int sz, int timeout_ms);
int aws_net_receive_packet_cb(WOLFSSL* ssl, void* ctx, byte* packet, int sz, int timeout_ms);
int aws_net_disconnect_cb(void* ctx);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AWS_NET_INTERFACE_H_ */
