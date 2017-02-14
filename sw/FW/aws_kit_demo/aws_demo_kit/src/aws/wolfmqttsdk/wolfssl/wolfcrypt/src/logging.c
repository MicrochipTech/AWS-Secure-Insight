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



#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

#include <wolfssl/wolfcrypt/settings.h>

/* submitted by eof */

#include <wolfssl/wolfcrypt/logging.h>
#include <wolfssl/wolfcrypt/error-crypt.h>


#ifdef __cplusplus
    extern "C" {
#endif
    WOLFSSL_API int  wolfSSL_Debugging_ON(void);
    WOLFSSL_API void wolfSSL_Debugging_OFF(void);
#ifdef __cplusplus
    } 
#endif


#ifdef DEBUG_WOLFSSL

/* Set these to default values initially. */
static wolfSSL_Logging_cb log_function = 0;
static int loggingEnabled = 0;

#endif /* DEBUG_WOLFSSL */


int wolfSSL_SetLoggingCb(wolfSSL_Logging_cb f)
{
#ifdef DEBUG_WOLFSSL
    int res = 0;

    if (f)
        log_function = f;
    else
        res = BAD_FUNC_ARG;

    return res;
#else
    (void)f;
    return NOT_COMPILED_IN;
#endif
}


int wolfSSL_Debugging_ON(void)
{
#ifdef DEBUG_WOLFSSL
    loggingEnabled = 1;
    return 0;
#else
    return NOT_COMPILED_IN;
#endif
}


void wolfSSL_Debugging_OFF(void)
{
#ifdef DEBUG_WOLFSSL
    loggingEnabled = 0;
#endif
}


#ifdef DEBUG_WOLFSSL

#if defined(FREESCALE_MQX) || defined(FREESCALE_KSDK_MQX)
    #if MQX_USE_IO_OLD
        #include <fio.h>
    #else
        #include <nio.h>
    #endif
#else
    #include <stdio.h>   /* for default printf stuff */
#endif

#ifdef THREADX
    int dc_log_printf(char*, ...);
#endif

static void wolfssl_log(const int logLevel, const char *const logMessage)
{
    if (log_function)
        log_function(logLevel, logMessage);
    else {
        if (loggingEnabled) {
#ifdef THREADX
            dc_log_printf("%s\n", logMessage);
#elif defined(MICRIUM)
        #if (NET_SECURE_MGR_CFG_EN == DEF_ENABLED)
            NetSecure_TraceOut((CPU_CHAR *)logMessage);
        #endif
#elif defined(WOLFSSL_MDK_ARM)
            fflush(stdout) ;
            printf("%s\n", logMessage);
            fflush(stdout) ;
#elif defined(WOLFSSL_LOG_PRINTF)
            printf("%s\n", logMessage);
#else
            fprintf(stderr, "%s\r\n", logMessage);
#endif
        }
    }
}


void WOLFSSL_MSG(const char* msg)
{
    if (loggingEnabled)
        wolfssl_log(INFO_LOG , msg);
}


void WOLFSSL_BUFFER(byte* buffer, word32 length)
{
    #define LINE_LEN 16

    if (loggingEnabled) {
        word32 i;
        char line[80];

        if (!buffer) {
            wolfssl_log(INFO_LOG, "\tNULL");

            return;
        }

        sprintf(line, "\t");

        for (i = 0; i < LINE_LEN; i++) {
            if (i < length)
                sprintf(line + 1 + i * 3,"%02x ", buffer[i]);
            else
                sprintf(line + 1 + i * 3, "   ");
        }

        sprintf(line + 1 + LINE_LEN * 3, "| ");

        for (i = 0; i < LINE_LEN; i++)
            if (i < length)
                sprintf(line + 3 + LINE_LEN * 3 + i,
                     "%c", 31 < buffer[i] && buffer[i] < 127 ? buffer[i] : '.');

        wolfssl_log(INFO_LOG, line);

        if (length > LINE_LEN)
            WOLFSSL_BUFFER(buffer + LINE_LEN, length - LINE_LEN);
    }
}


void WOLFSSL_ENTER(const char* msg)
{
    if (loggingEnabled) {
        char buffer[80];
        sprintf(buffer, "wolfSSL Entering %s", msg);
        wolfssl_log(ENTER_LOG , buffer);
    }
}


void WOLFSSL_LEAVE(const char* msg, int ret)
{
    if (loggingEnabled) {
        char buffer[80];
        sprintf(buffer, "wolfSSL Leaving %s, return %d", msg, ret);
        wolfssl_log(LEAVE_LOG , buffer);
    }
}


void WOLFSSL_ERROR(int error)
{
    if (loggingEnabled) {
        char buffer[80];
        sprintf(buffer, "wolfSSL error occurred, error = %d", error);
        wolfssl_log(ERROR_LOG , buffer);
    }
}

#endif  /* DEBUG_WOLFSSL */
