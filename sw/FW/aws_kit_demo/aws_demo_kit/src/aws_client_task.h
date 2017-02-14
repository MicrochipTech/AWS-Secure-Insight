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

#ifndef AWS_CLIENT_TASK_H_
#define AWS_CLIENT_TASK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "aws_kit_object.h"

/**
 * \defgroup Client Task Definition
 *
 * @{
 */

/** \name Main Task configuration
   @{ */
#define AWS_CLIENT_TASK_PRIORITY				(tskIDLE_PRIORITY + 2)
#define AWS_CLIENT_TASK_DELAY					(100 / portTICK_RATE_MS)
#define AWS_CLIENT_TASK_STACK_SIZE				(4096)
/** @} */

/** \name Definition for timer interval of MQTT subscriber, MQTT message max length
   @{ */
#define AWS_MQTT_CMD_TIMEOUT_MS					(2000)
#define AWS_MQTT_PAYLOAD_MAX					(128)
/** @} */

int aws_client_mqtt_packet_id(void);
bool aws_client_scan_button(t_aws_kit* kit);
int aws_client_mqtt_msg_cb(MqttClient* mqttCli, MqttMessage* msg, uint8_t new, uint8_t done);
int aws_client_tls_cb(MqttClient* mqttCli);
int aws_client_mqtt_disconnect(t_aws_kit* kit);
int aws_client_init_mqtt_client(t_aws_kit* kit);
int aws_client_mqtt_subscribe(t_aws_kit* kit);
int aws_client_mqtt_publish(t_aws_kit* kit);
int aws_client_mqtt_wait_msg(t_aws_kit* kit);
void aws_client_state_machine(t_aws_kit* kit);
void aws_client_task(void *params);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AWS_CLIENT_TASK_H_ */
