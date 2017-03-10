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




#ifndef WOLFSSL_CALLBACKS_H
#define WOLFSSL_CALLBACKS_H

#include <sys/time.h>

#ifdef __cplusplus
    extern "C" {
#endif


enum { /* CALLBACK CONTSTANTS */
    MAX_PACKETNAME_SZ     =  24,
    MAX_CIPHERNAME_SZ     =  24,
    MAX_TIMEOUT_NAME_SZ   =  24,       
    MAX_PACKETS_HANDSHAKE =  14,       /* 12 for client auth plus 2 alerts */
    MAX_VALUE_SZ          = 128,       /* all handshake packets but Cert should
                                          fit here  */
};


typedef struct handShakeInfo_st {
    char   cipherName[MAX_CIPHERNAME_SZ + 1];    /* negotiated cipher */
    char   packetNames[MAX_PACKETS_HANDSHAKE][MAX_PACKETNAME_SZ + 1];
                                                 /* SSL packet names  */ 
    int    numberPackets;                        /* actual # of packets */
    int    negotiationError;                     /* cipher/parameter err */
} HandShakeInfo;


typedef struct timeval Timeval;


typedef struct packetInfo_st {
    char           packetName[MAX_PACKETNAME_SZ + 1]; /* SSL packet name */
    Timeval        timestamp;                       /* when it occurred    */
    unsigned char  value[MAX_VALUE_SZ];             /* if fits, it's here */ 
    unsigned char* bufferValue;                     /* otherwise here (non 0) */
    int            valueSz;                         /* sz of value or buffer */
} PacketInfo;


typedef struct timeoutInfo_st {
    char       timeoutName[MAX_TIMEOUT_NAME_SZ + 1]; /* timeout Name */
    int        flags;                              /* for future use */
    int        numberPackets;                      /* actual # of packets */
    PacketInfo packets[MAX_PACKETS_HANDSHAKE];     /* list of all packets  */
    Timeval    timeoutValue;                       /* timer that caused it */
} TimeoutInfo;



#ifdef __cplusplus
    }  /* extern "C" */
#endif


#endif /* WOLFSSL_CALLBACKS_H */

