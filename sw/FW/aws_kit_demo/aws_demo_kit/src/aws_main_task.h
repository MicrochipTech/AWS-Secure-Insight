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

#ifndef AWS_MAIN_TASK_H_
#define AWS_MAIN_TASK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asf.h>
#include "aws_kit_object.h"

/**
 * \defgroup Main Task Definition
 *
 * @{
 */

/** \name Main Task configuration
   @{ */
#define AWS_MAIN_TASK_PRIORITY					(tskIDLE_PRIORITY + 1)
#define AWS_MAIN_TASK_DELAY						(100 / portTICK_RATE_MS)
#define AWS_MAIN_TASK_STACK_SIZE				(1024)
/** @} */

#define AWS_GET_USER_DATA_LEN(data, idx) 		((data[idx + 3] << 24) | (data[idx + 2] << 16) | (data[idx + 1] << 8) | data[idx])
#define AWS_CHECK_USER_DATA_LEN(len, max) 		((len == 0) || (len >= max))
#define AWS_CHECK_USER_DATA(data, idx) 			((data[idx] == 0) || (data[idx] == 0xFF))

int aws_main_wait_notification(t_aws_kit* kit);
int aws_main_init_kit(t_aws_kit* kit);
int aws_main_build_certificate(t_aws_kit* kit);
int aws_main_reset_user_data(t_aws_kit* kit);
int aws_main_check_kit_state(t_aws_kit* kit);
void aws_main_state_machine(t_aws_kit* kit);
void aws_main_task(void *params);
void aws_demo_tasks_init(void);

extern xTaskHandle mainTaskHandler;
extern xTaskHandle provTaskHandler;
extern xTaskHandle userTaskHandler;
extern xTaskHandle clientTaskHandler;

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AWS_MAIN_TASK_H_ */
