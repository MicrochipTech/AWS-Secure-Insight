/*
 * Copyright (C) 2006-2016 wolfSSL Inc.  All rights reserved.
 *
 * This license is only for commercial evaluation purposes.  
 * 
 * Redistribution of the source and binary forms, with or without
 * modification, is not permitted.
 * 
 * To obtain a commercial license from wolfSSL for redistribution please contact licensing @wolfssl.com
 *
 * THIS SOFTWARE IS PROVIDED BY WOLFSSL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL WOLFSSL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Contact licensing@wolfssl.com with any questions or comments.
 *
 * http://www.wolfssl.com
 */

/* Implementation by: David Garske
 * Based on specification for MQTT v3.1.1
 * See http://mqtt.org/documentation for additional MQTT documentation.
 */

#ifndef WOLFMQTT_SOCKET_H
#define WOLFMQTT_SOCKET_H

#ifdef __cplusplus
    extern "C" {
#endif


#include "wolfmqtt/mqtt_types.h"
#ifdef ENABLE_MQTT_TLS
//#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/error-crypt.h>
#endif

/* Default Port Numbers */
#define MQTT_DEFAULT_PORT   1883
#define MQTT_SECURE_PORT    8883


struct _MqttClient;

/* Function callbacks */
typedef int (*MqttTlsCb)(struct _MqttClient* client);

typedef int (*MqttNetConnectCb)(void *context,
    const char* host, word16 port, int timeout_ms);
typedef int (*MqttNetWriteCb)(WOLFSSL* ssl, void *context,
    const byte* buf, int buf_len, int timeout_ms);
typedef int (*MqttNetReadCb)(WOLFSSL* ssl, void *context,
    byte* buf, int buf_len, int timeout_ms);
typedef int (*MqttNetDisconnectCb)(void *context);

/* Strucutre for Network Security */
#ifdef ENABLE_MQTT_TLS
typedef struct _MqttTls {
    WOLFSSL_CTX         *ctx;
    WOLFSSL             *ssl;
} MqttTls;
#endif

/* Structure for Network callbacks */
typedef struct _MqttNet {
    void                *context;
    MqttNetConnectCb    connect;
    MqttNetReadCb       read;
    MqttNetWriteCb      write;
    MqttNetDisconnectCb disconnect;
} MqttNet;


/* MQTT SOCKET APPLICATION INTERFACE */
int MqttSocket_Init(struct _MqttClient *client, MqttNet* net);
int MqttSocket_Write(struct _MqttClient *client, const byte* buf, int buf_len,
    int timeout_ms);
int MqttSocket_Read(struct _MqttClient *client, byte* buf, int buf_len,
    int timeout_ms);

int MqttSocket_Connect(struct _MqttClient *client, const char* host,
    word16 port, int timeout_ms, int use_tls, MqttTlsCb cb);
int MqttSocket_Disconnect(struct _MqttClient *client);


#ifdef __cplusplus
    } /* extern "C" */
#endif

#endif /* WOLFMQTT_SOCKET_H */
