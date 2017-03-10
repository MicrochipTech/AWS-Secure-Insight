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

#include "asf.h"
#include "cryptoauthlib.h"
#include "socket/include/socket.h"
#include "aws_net_interface.h"
#include "aws_kit_debug.h"

uint16_t tcp_socket_status = 0;
uint16_t tcp_ntp_socket_status = 0;
static bool gIsWifiConnected = false;
static SOCKET gNtpSocket = -1;
static t_time_date curr_time_date;
static time_t gSecSince1900 = 0;
static uint8_t ntp_dns_address[HOSTNAME_MAX_SIZE];
static uint32_t hostAddress = 0;
tstrSocketRecvMsg *pstrRecv = NULL;

/**
 * \brief Return event strings corresponding to input event for debugging.
 *
 * \param msg[in]            Type of socket event
 * \return message string
 */
const char* aws_net_get_socket_string(int msg)
{
    int evtType = msg;

    if (evtType < SOCKET_MSG_BIND && evtType > SOCKET_MSG_RECVFROM) {
        return "UNKNOWN socket event";
    }

    switch (evtType) {

    case SOCKET_MSG_BIND :
        return "BIND";

    case SOCKET_MSG_LISTEN :
        return "LISTEN";

    case SOCKET_MSG_DNS_RESOLVE :
        return "RESOLVE";

    case SOCKET_MSG_ACCEPT :
        return "ACCEPT";

    case SOCKET_MSG_CONNECT :
        return "CONNECT";

    case SOCKET_MSG_RECV :
        return "RECV";

    case SOCKET_MSG_SEND :
        return "SEND";

    case SOCKET_MSG_SENDTO :
        return "SENDTO";

    case SOCKET_MSG_RECVFROM :
        return "RECVFROM";		

    default :
        return "UNKNOWN";		
    }
}

/**
 * \brief If ATWINC1500 is successfully connected to a WIFI router, set the state to true.
 *
 * \param status[in]         True : connected
 */
void aws_net_set_wifi_status(bool status)
{
    gIsWifiConnected = status;
}

/**
 * \brief Return connection state.
 *
 * \return true, if connected.
 */
bool aws_net_get_wifi_status(void)
{
    return gIsWifiConnected;
}

/**
 * \brief WIFI callback to be called by ATWINC1500.
 *
 * \param u8MsgType[in]      Message type
 * \param pvMsg[in]          Message corresponding to message type
 */
void aws_net_wifi_cb(uint8_t u8MsgType, void* pvMsg)
{
	t_aws_kit* kit = aws_kit_get_instance();

	switch (u8MsgType) {
		case M2M_WIFI_RESP_CON_STATE_CHANGED:
		{
			tstrM2mWifiStateChanged *pstrWifiState = (tstrM2mWifiStateChanged *)pvMsg;
            
			if (pstrWifiState->u8CurrState == M2M_WIFI_CONNECTED) {
				AWS_INFO("M2M_WIFI_RESP_CON_STATE_CHANGED: CONNECTED");
			} else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED) {
				AWS_INFO("M2M_WIFI_RESP_CON_STATE_CHANGED: DISCONNECTED");
				aws_net_set_wifi_status(M2M_WIFI_DISCONNECTED);
				m2m_wifi_connect((char *)kit->user.ssid, kit->user.ssidLen, MAIN_WLAN_AUTH, (char *)kit->user.psk, M2M_WIFI_CH_ALL);
			}
			break;
		}

		case M2M_WIFI_REQ_DHCP_CONF:
		{
			uint8_t *pu8IPAddress = (uint8_t *)pvMsg;
			aws_net_set_wifi_status(M2M_WIFI_CONNECTED);
			AWS_INFO("M2M_WIFI_REQ_DHCP_CONF: IP is %u.%u.%u.%u", pu8IPAddress[0], pu8IPAddress[1], pu8IPAddress[2], pu8IPAddress[3]);
			memcpy(ntp_dns_address, (uint8_t *)MAIN_WORLDWIDE_NTP_POOL_HOSTNAME, strlen(MAIN_WORLDWIDE_NTP_POOL_HOSTNAME));
			gethostbyname((uint8_t *)ntp_dns_address);
			break;
		}

		default:
			break;
	}
}

/**
 * \brief Initialize ATWINC1500 driver, and then access to WIFI router with SSID and password.
 *
 * \param kit[in]            Pointer to an instance of AWS Kit
 * \param timeout_sec[in]    Wait for input seconds
 * \return AWS_E_SUCCESS     On success
 */
int aws_net_init_wifi(t_aws_kit* kit, int timeout_sec)
{
	int ret = AWS_E_FAILURE;
	tstrWifiInitParam param;
	Timer wifiTimer;

	do {
		/* Initialize WIFI parameters structure. */
		memset((uint8_t *)&param, 0, sizeof(tstrWifiInitParam));

		/* Initialize Wi-Fi driver with data and status callbacks. */
		param.pfAppWifiCb = aws_net_wifi_cb;
		ret = m2m_wifi_init(&param);	
		if (ret != M2M_SUCCESS) {
			AWS_ERROR("Failed to register wifi callback!(%d)", ret);
			ret = AWS_E_WIFI_INVALID;
			break;
		}

		/* Initialize socket module. */
		network_socket_init();

		/* Connect to router. */
		ret = m2m_wifi_connect((char *)kit->user.ssid, kit->user.ssidLen, MAIN_WLAN_AUTH, 
					(char *)kit->user.psk, M2M_WIFI_CH_ALL);
		if (ret != M2M_SUCCESS) {
			AWS_ERROR("Failed to connect to router!(%d)", ret);
			ret = AWS_E_WIFI_CONN_FAILURE;
		}

		TimerInit(&wifiTimer);
		TimerCountdown(&wifiTimer, timeout_sec);
		/* Monitor connection state until input time. */
		while (aws_net_get_wifi_status() != M2M_WIFI_CONNECTED) {
			if(TimerIsExpired(&wifiTimer)) {
				ret = AWS_E_WIFI_CONN_TIMEOUT;
				AWS_ERROR("Expired WIFI connection time!(%d)", ret);
				return ret;
			}
			m2m_wifi_handle_events(NULL);		
		}
		AWS_INFO("WINC is connected to %s successfully!", kit->user.ssid);

	} while(0);

	return ret;
}

/**
 * \brief Get current time using Network Time Protocol.
 *
 * \param kit[in]            Not used
 * \return AWS_E_SUCCESS     On success
 */
int aws_net_get_time(t_aws_kit* kit)
{
	int ret = AWS_E_FAILURE;

	/* Set both socket and resolve callbacks. */
	registerSocketCallback(aws_net_ntp_socket_cb, aws_net_ntp_resolve_cb);
	if (aws_net_get_ntp_socket() < 0) {
		/* Get current time in 5 seconds over UDP. */
		ret = aws_net_get_ntp_time(AWS_NET_NTP_TIMEOUT_MS);
		if (ret != AWS_E_SUCCESS) {
			AWS_ERROR("Failed to get time and date!(%d)", ret);
			ret = AWS_E_NET_SOCKET_INVALID;
		}
	} else {
		ret = AWS_E_NET_SOCKET_INVALID;
		aws_net_set_ntp_socket((SOCKET)-1);
		AWS_ERROR("Failed to get NTP socket!(%d)", ret);
	}

	return ret;
}

/**
 * \brief Main interface bewteen application and ATWINC1500 driver.
 *
 * \param sock[in]           Socket number
 * \param u8Msg[in]          Event came from ATWINC1500
 * \param pvMsg[in]          Message corresponding to the event
 */
void aws_net_socket_cb(SOCKET sock, uint8_t u8Msg, void *pvMsg)
{
	AWS_INFO("Socket Event : %s", aws_net_get_socket_string(u8Msg));
	switch (u8Msg) {
		case SOCKET_MSG_BIND:
		{
			tstrSocketBindMsg *pstrBind = (tstrSocketBindMsg *)pvMsg;
			if (pstrBind && pstrBind->status == 0) {
				ENABLE_SOCKET_STATUS(SOCKET_STATUS_BIND);
			} else {
				DISABLE_SOCKET_STATUS(SOCKET_STATUS_BIND);
				close(sock);
			}
		}
		break;		

		case SOCKET_MSG_LISTEN:
		{
			tstrSocketListenMsg *pstrListen = (tstrSocketListenMsg *)pvMsg;
			if (pstrListen && pstrListen->status == 0) {
				ENABLE_SOCKET_STATUS(SOCKET_STATUS_LISTEN);
			} else {
				DISABLE_SOCKET_STATUS(SOCKET_MSG_LISTEN);
				close(sock);
			}
		}
		break;

		case SOCKET_MSG_ACCEPT:
		{
			tstrSocketAcceptMsg *pstrAccept = (tstrSocketAcceptMsg *)pvMsg;
			if (pstrAccept) {
				ENABLE_SOCKET_STATUS(SOCKET_STATUS_ACCEPT);
			} else {
				DISABLE_SOCKET_STATUS(SOCKET_STATUS_ACCEPT);
				close(sock);
			}
		}
		break;

		case SOCKET_MSG_CONNECT:
		{
			tstrSocketConnectMsg *pstrConnect = (tstrSocketConnectMsg *)pvMsg;
			if (pstrConnect && pstrConnect->s8Error >= 0) {
				ENABLE_SOCKET_STATUS(SOCKET_STATUS_CONNECT);
			} else {
				DISABLE_SOCKET_STATUS(SOCKET_STATUS_CONNECT);
				close(sock);
			}
		}
		break;

		case SOCKET_MSG_RECV:
		{
			pstrRecv = (tstrSocketRecvMsg *)pvMsg;
			if (pstrRecv && pstrRecv->s16BufferSize > 0) {
				ENABLE_SOCKET_STATUS(SOCKET_STATUS_RECEIVE);
			} else {
				DISABLE_SOCKET_STATUS(SOCKET_STATUS_RECEIVE);
				DISABLE_SOCKET_STATUS(SOCKET_STATUS_CONNECT);
				close(sock);
			}
		}
		break;

		case SOCKET_MSG_SEND:
		{
			sint16 sent_size = *(sint16*)pvMsg;
			if (sent_size > 0) {
				ENABLE_SOCKET_STATUS(SOCKET_STATUS_SEND);
			} else {
				DISABLE_SOCKET_STATUS(SOCKET_STATUS_SEND);
				close(sock);
			}
		}
		break;

	default:
		break;
	}
}

/**
 * \brief Set a socket number to connect to NTP server.
 *
 * \param socket[in]         Socket ID
 */
void aws_net_set_ntp_socket(SOCKET socket)
{
    gNtpSocket = socket;    
}

/**
 * \brief Return the socket ID.
 *
 * \return the socket ID.
 */
SOCKET aws_net_get_ntp_socket(void)
{
    return gNtpSocket;
}

/**
 * \brief Set local time and date came from the NTP server.
 *
 * \param secsSince1900[in]  Unix time
 */
int aws_net_set_current_time(uint32_t secsSince1900)
{
    #define YEAR0          1900
    #define EPOCH_YEAR     1970
    #define SECS_DAY       (24L * 60L * 60L)
    #define LEAPYEAR(year) (!((year) % 4) && (((year) % 100) || !((year) %400)))
    #define YEARSIZE(year) (LEAPYEAR(year) ? 366 : 365)

    int ret = 0;
    time_t secs = secsSince1900;
    unsigned long dayclock, dayno;
    int year = EPOCH_YEAR;
    static const int _ytab[2][12] =
    {
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };

	gSecSince1900 = secs;
    dayclock = (unsigned long)secs % SECS_DAY;
    dayno    = (unsigned long)secs / SECS_DAY;

    curr_time_date.tm_sec  = (int) dayclock % 60;
    curr_time_date.tm_min  = (int)(dayclock % 3600) / 60;
    curr_time_date.tm_hour = (int) dayclock / 3600;
    curr_time_date.tm_wday = (int) (dayno + 4) % 7;        /* day 0 a Thursday */

    while(dayno >= (unsigned long)YEARSIZE(year)) {
        dayno -= YEARSIZE(year);
        year++;
    }

    curr_time_date.tm_year = year - YEAR0;
    curr_time_date.tm_yday = (int)dayno;
    curr_time_date.tm_mon  = 0;

    while(dayno >= (unsigned long)_ytab[LEAPYEAR(year)][curr_time_date.tm_mon]) {
        dayno -= _ytab[LEAPYEAR(year)][curr_time_date.tm_mon];
        curr_time_date.tm_mon++;
    }

    curr_time_date.tm_mday  = (int)++dayno;
    curr_time_date.tm_isdst = 0;

    curr_time_date.tm_year += 1900;
    curr_time_date.tm_mon += 1;

	AWS_INFO("Date of Today : %d / %d / %d", curr_time_date.tm_year, curr_time_date.tm_mon, curr_time_date.tm_mday);
    return ret;

}

/**
 * \brief Assign received time and date to out param.
 *
 * \param t_time_date[out]   For WolfSSL to check the date validity in certificate
 * \return zero
 */
int aws_net_get_current_time(t_time_date* tm)
{
	int ret = 0;

	if (tm == NULL) {
		AWS_ERROR("Invalid param!");
		return -1;
	} 

	tm->tm_year = curr_time_date.tm_year;
	tm->tm_mon = curr_time_date.tm_mon;
	tm->tm_mday = curr_time_date.tm_mday;
	tm->tm_hour = curr_time_date.tm_hour;
	tm->tm_min = curr_time_date.tm_min;
	tm->tm_sec = curr_time_date.tm_sec;

	return ret;
}

/**
 * \brief Return current seconds received from NTP server.
 *
 * \return the seconds
 */
time_t aws_net_get_ntp_seconds(void)
{
	return gSecSince1900;
}

/**
 * \brief Wait for socket event to get current date from NTP server.
 *
 * \param timeout_ms[in]     Wait until input milliseconds
 * \return AWS_E_SUCCESS     On success
 */
int aws_net_get_ntp_time(uint32_t timeout_ms)
{
	int ret = AWS_E_SUCCESS;
	SOCKET ntp_socket = -1;
	struct sockaddr_in addr;
	Timer ntpTimer;

	ntp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (ntp_socket < 0) {
		AWS_ERROR("Failed to create UDP Client Socket!");
		return AWS_E_NET_SOCKET_INVALID;
	} else {
		aws_net_set_ntp_socket(ntp_socket);
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = _htonl(MAIN_DEFAULT_ADDRESS);
	addr.sin_port = _htons(6666);
	delay_ms(50);
	if (bind((SOCKET)aws_net_get_ntp_socket(), (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) != SOCK_ERR_NO_ERROR) {
		close(ntp_socket);
		AWS_ERROR("Failed to bind socket!");
		return AWS_E_NET_SOCKET_INVALID;        
	}

	TimerInit(&ntpTimer);
	TimerCountdownMS(&ntpTimer, timeout_ms);
	while (!GET_NTP_SOCKET_STATUS(NTP_SOCKET_STATUS_RECEIVE_FROM)) {
		if(TimerIsExpired(&ntpTimer)) {
			close(ntp_socket);
			AWS_ERROR("Expired connection time!(%d)", ret);
			return AWS_E_NET_SOCKET_TIMEOUT;
		}
		m2m_wifi_handle_events(NULL);
	}

	close(ntp_socket);
	return ret;    
}

/**
 * \brief Callback to get the Data from socket.
 *
 * \param sock[in]           Socket ID
 * \param u8Msg[in]          Type of Socket event
 * \param pvMsg[in]          Message of the type
 */
void aws_net_ntp_socket_cb(SOCKET sock, uint8_t u8Msg, void* pvMsg)
{
	/* Check for socket event on socket. */
	int16_t ret = SOCK_ERR_NO_ERROR;
  
	switch (u8Msg) {
		case SOCKET_MSG_BIND:
		{
			tstrSocketBindMsg *pstrBind = (tstrSocketBindMsg *)pvMsg;
			if (pstrBind && pstrBind->status == 0) {
				ENABLE_NTP_SOCKET_STATUS(NTP_SOCKET_STATUS_BIND);
				ret = recvfrom(sock, ntp_dns_address, sizeof(ntp_dns_address), 0);
				if (ret != SOCK_ERR_NO_ERROR) {
					AWS_ERROR("socket_cb: recv error(%d)", ret);
				}
			} else {
			    DISABLE_NTP_SOCKET_STATUS(NTP_SOCKET_STATUS_BIND);
				AWS_ERROR("socket_cb: bind error!");
			}
			break;
		}

		case SOCKET_MSG_RECVFROM:
		{
			tstrSocketRecvMsg *pstrRx = (tstrSocketRecvMsg *)pvMsg;
			if (pstrRx->pu8Buffer && pstrRx->s16BufferSize) {
				uint8_t packetBuffer[48];
				memcpy(&packetBuffer, pstrRx->pu8Buffer, sizeof(packetBuffer));

	   			ENABLE_NTP_SOCKET_STATUS(NTP_SOCKET_STATUS_RECEIVE_FROM);
				if ((packetBuffer[0] & 0x7) != 4) {                   /* expect only server response */
					AWS_ERROR("socket_cb: Expecting response from Server Only!");
					return;                    /* MODE is not server, abort */
				} else {
					uint32_t secsSince1900 = packetBuffer[40] << 24 | packetBuffer[41] << 16 | packetBuffer[42] << 8 | packetBuffer[43];

					/* Now convert NTP time into everyday time.
					 * Unix time starts on Jan 1 1970. In seconds, that's 2208988800.
					 * Subtract seventy years.
					 */
					const uint32_t seventyYears = 2208988800UL;
					uint32_t epoch = secsSince1900 - seventyYears;
					/* Print the hour, minute and second.
					 * GMT is the time at Greenwich Meridian.
					 */
					aws_net_set_current_time(epoch);
					ret = close(sock);
				}
			} else {
	            DISABLE_NTP_SOCKET_STATUS(NTP_SOCKET_STATUS_RECEIVE_FROM);
			}
			break;
		}

		default:
			break;
	}
}

/**
 * \brief Query date and time to NTP server over UDP.
 *
 * \param pu8DomainName[in]  Not used
 * \param u32ServerIP[in]    IP address of NTP server(pool.ntp.org)
 */
void aws_net_ntp_resolve_cb(uint8_t* pu8DomainName, uint32_t u32ServerIP)
{
	struct sockaddr_in addr;
	int8_t cDataBuf[48];
	memset(cDataBuf, 0, sizeof(cDataBuf));
	cDataBuf[0] = 0x1B; /* time query */

	if (aws_net_get_ntp_socket() >= 0) {
		/* Set NTP server socket address structure. */
		addr.sin_family = AF_INET;
		addr.sin_port = _htons(MAIN_SERVER_PORT_FOR_UDP);
		addr.sin_addr.s_addr = u32ServerIP;

		/*Send an NTP time query to the NTP server. */
		if (sendto((SOCKET)aws_net_get_ntp_socket(), (int8_t *)&cDataBuf, sizeof(cDataBuf), 0, (struct sockaddr *)&addr, sizeof(addr)) != M2M_SUCCESS)
			AWS_ERROR("Failed to time query!\r\n");
	}
}

/**
 * \brief Compare current time to the time in certificate.
 *
 * \param local[in]          Current time and date
 * \param cert[in]           Time and date in certificate
 * \return 1                 Current time is greater than the time issued by issuer
 */
int aws_net_compare_date(t_time_date* local, atcacert_tm_utc_t* cert)
{
	uint8_t ret = 0;

	if (local->tm_year > cert->tm_year)
		return 1;

	if (local->tm_year == cert->tm_year && local->tm_mon > cert->tm_mon)
		return 1;

	if (local->tm_year == cert->tm_year && local->tm_mon == cert->tm_mon && local->tm_mday > cert->tm_mday)
		return 1;

	return ret;
}

/**
 * \brief Get IP address from DNS server over ATWINC1500 driver.
 *
 * \param pu8DomainName[in] Not used
 * \param u32ServerIP[in]   IP address
 */
void aws_net_dns_resolve_cb(uint8_t* pu8DomainName, uint32_t u32ServerIP)
{
	hostAddress = u32ServerIP;
}

/**
 * \brief Get IP address from DNS server over ATWINC1500 driver.
 *
 * \return the IP address
 */
uint32_t aws_net_get_host_addr(void)
{
	return hostAddress;
}

/**
 * \brief Set host IP address with input param in hex.
 *
 * \param addr[in]           IP address
 */
void aws_net_set_host_addr(uint32_t addr)
{
	hostAddress = addr;
}

int aws_net_disconnect_cb(void* ctx)
{
	SOCKET* sock = (SOCKET*)ctx;

	return close(*sock);
}

