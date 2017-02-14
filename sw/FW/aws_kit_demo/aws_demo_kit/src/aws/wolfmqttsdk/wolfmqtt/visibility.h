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

/* Visibility control macros */

#ifndef WOLFMQTT_VISIBILITY_H
#define WOLFMQTT_VISIBILITY_H

/* WOLFMQTT_API is used for the public API symbols.
        It either imports or exports (or does nothing for static builds)

   WOLFMQTT_LOCAL is used for non-API symbols (private).
*/

#if defined(BUILDING_WOLFMQTT)
    #if defined(HAVE_VISIBILITY) && HAVE_VISIBILITY
        #define WOLFMQTT_API   __attribute__ ((visibility("default")))
        #define WOLFMQTT_LOCAL __attribute__ ((visibility("hidden")))
    #elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
        #define WOLFMQTT_API   __global
        #define WOLFMQTT_LOCAL __hidden
    #elif defined(_MSC_VER)
        #ifdef _WINDLL
            #define WOLFMQTT_API __declspec(dllexport)
        #else
            #define WOLFMQTT_API
        #endif
        #define WOLFMQTT_LOCAL
    #else
        #define WOLFMQTT_API
        #define WOLFMQTT_LOCAL
    #endif /* HAVE_VISIBILITY */
#else /* BUILDING_WOLFMQTT */
    #if defined(_MSC_VER)
        #define WOLFMQTT_API __declspec(dllimport)
        #define WOLFMQTT_LOCAL
    #else
        #define WOLFMQTT_API
        #define WOLFMQTT_LOCAL
    #endif
#endif /* BUILDING_WOLFMQTT */

#endif /* WOLFMQTT_VISIBILITY_H */
