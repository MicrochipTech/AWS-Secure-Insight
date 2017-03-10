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



/* submitted by eof */


#ifndef WOLFSSL_MEMORY_H
#define WOLFSSL_MEMORY_H

#include <stdlib.h>
#include <wolfssl/wolfcrypt/types.h>

#ifdef __cplusplus
    extern "C" {
#endif

typedef void *(*wolfSSL_Malloc_cb)(size_t size);
typedef void (*wolfSSL_Free_cb)(void *ptr);
typedef void *(*wolfSSL_Realloc_cb)(void *ptr, size_t size);


/* Public set function */
WOLFSSL_API int wolfSSL_SetAllocators(wolfSSL_Malloc_cb  malloc_function,
                                    wolfSSL_Free_cb    free_function,
                                    wolfSSL_Realloc_cb realloc_function);

/* Public in case user app wants to use XMALLOC/XFREE */
WOLFSSL_API void* wolfSSL_Malloc(size_t size);
WOLFSSL_API void  wolfSSL_Free(void *ptr);
WOLFSSL_API void* wolfSSL_Realloc(void *ptr, size_t size);


#ifdef __cplusplus
    }  /* extern "C" */
#endif

#endif /* WOLFSSL_MEMORY_H */

