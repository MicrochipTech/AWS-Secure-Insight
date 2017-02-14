/**
 * \file
 *
 * \brief OLED1 Xplained Pro LED and button driver for FreeRTOS demo.
 *
 * Copyright (C) 2014-2016 Atmel Corporation. All rights reserved.
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

#include "aws_kit_debug.h"
#include "aws_main_task.h"
#include "aws_user_task.h"
#include "aws_kit_object.h"
#include "cryptoauthlib.h"

static OLED1_CREATE_INSTANCE(oled1, OLED1_EXT_HEADER);

/**
 * \brief Turn on LED corresponding to the input ID in case of receiving Delta topic.
 *
 * \param id[in]              Peripheral ID of each button
 * \return AWS_E_SUCCESS      On success
 */
int aws_user_led_on(uint8_t id)
{
	int ret = AWS_E_SUCCESS;

	if (id < OLED1_LED1_ID || id > OLED1_LED3_ID)
		return AWS_E_FAILURE;

	if (id == OLED1_LED1_ID) 
		ioport_set_pin_level(OLED1_LED1_PIN, OLED1_LED1_ACTIVE);
	else if (id == OLED1_LED2_ID)
		ioport_set_pin_level(OLED1_LED2_PIN, OLED1_LED1_ACTIVE);
	else if (id == OLED1_LED3_ID)
		ioport_set_pin_level(OLED1_LED3_PIN, OLED1_LED1_ACTIVE);
	else
		ret = AWS_E_FAILURE;

	return ret;
}

/**
 * \brief Turn off LED corresponding to the input ID in case of receiving Delta topic.
 *
 * \param id[in]              Peripheral ID of each button
 * \return AWS_E_SUCCESS      On success
 */
int aws_user_led_off(uint8_t id)
{
	int ret = AWS_E_SUCCESS;

	if (id < OLED1_LED1_ID || id > OLED1_LED3_ID)
		return AWS_E_FAILURE;

	if (id == OLED1_LED1_ID) 
		ioport_set_pin_level(OLED1_LED1_PIN, OLED1_LED1_INACTIVE);
	else if (id == OLED1_LED2_ID)
		ioport_set_pin_level(OLED1_LED2_PIN, OLED1_LED1_INACTIVE);
	else if (id == OLED1_LED3_ID)
		ioport_set_pin_level(OLED1_LED3_PIN, OLED1_LED1_INACTIVE);
	else
		ret = AWS_E_FAILURE;

	return ret;
}

/**
 * \brief SW0 button ISR of ATSAMG55 Xplained pro board.
 * If a user press this button for 3 seconds, all data will be reset to zero.
 * In case of resetting WIFI credential, host address, and provisining, this feature would be useful.
 *
 * \param id[in]              Peripheral ID of each button
 * \param mask[in]            Bitmast of each button
 */
void aws_user_sw0_cb(uint32_t id, uint32_t mask)
{
	t_aws_kit* kit = aws_kit_get_instance();
	
	AWS_INFO("Pressed SW0 button");
	kit->pushButtonState = AWS_BUTTON_PRESSED;
	ioport_set_pin_level(LED_0_PIN, LED_0_ACTIVE);

	aws_kit_init_timer(&kit->resetISR);
	aws_kit_countdown_sec(&kit->resetISR, AWS_USER_RESET_TIMEOUT_SEC);
}

/**
 * \brief Button ISR of OLED1 board.
 * If a same button is pressed again in 900 ms due to debounce issue, Disregard it.
 * Queue a pressed button ID.
 *
 * \param id[in]              Peripheral ID of each button
 * \param mask[in]            Bitmast of each button
 */
void aws_user_oled1_button_cb(uint32_t id, uint32_t mask)
{
	t_aws_kit* kit = aws_kit_get_instance();
	uint8_t buttonId = 0;
	static uint8_t prevButtonId = 0;
	
	AWS_INFO("Pressed one of the buttons : %d %ld", kit->clientState, id);
	if (kit->clientState != CLIENT_STATE_MQTT_WAIT_MESSAGE)
		return;

	/* Set a pressed button. */
	if ((OLED1_PIN_PUSHBUTTON_1_ID == id) && (OLED1_PIN_PUSHBUTTON_1_MASK == mask))
		buttonId = OLED1_BUTTON1_ID;
	else if ((OLED1_PIN_PUSHBUTTON_2_ID == id) && (OLED1_PIN_PUSHBUTTON_2_MASK == mask)) 
		buttonId = OLED1_BUTTON2_ID;
	else if ((OLED1_PIN_PUSHBUTTON_3_ID == id) && (OLED1_PIN_PUSHBUTTON_3_MASK == mask)) 
		buttonId = OLED1_BUTTON3_ID;
	else
		return;

	/* Check for debounce issue. */
	if (!(aws_kit_timer_expired(&kit->buttonISR)) && (buttonId == prevButtonId)) {
		aws_kit_init_timer(&kit->buttonISR);
		aws_kit_countdown_ms(&kit->buttonISR, 900);
		return;
	} else {
		aws_kit_init_timer(&kit->buttonISR);
		aws_kit_countdown_ms(&kit->buttonISR, 900);
	}

	/* Queue the button ID pressed by a user. */
	prevButtonId = buttonId;	
	if (!xQueueSendFromISR(kit->buttonQueue, (uint8_t *)&buttonId,	NULL)) {
		AWS_ERROR("Failed to send button ID!");
		// Error: could not enqueue pressed button
	}
}

/**
 * \brief Get buttons state. (Not used.)
 */
void aws_user_scan_oled1_button(void)
{
	bool button[AWS_KIT_BUTTON_MAX];

	for (uint8_t i = 0; i < sizeof(button); i++) {
		button[i] = oled1_get_button_state(&oled1, i + 1);
	}
}

/**
 * \brief Initialize SW0 button IOPORT in ATSAMG55 Xplained pro board.
 */
void aws_user_sw0_init(void)
{
	/* Configure Pushbutton 1 */
	pmc_enable_periph_clk(PIN_PUSHBUTTON_1_ID);
	pio_set_debounce_filter(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_MASK, 10);
	/* Interrupt on rising edge  */
	pio_handler_set(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_ID, PIN_PUSHBUTTON_1_MASK, PIN_PUSHBUTTON_1_ATTR, aws_user_sw0_cb);
	NVIC_EnableIRQ((IRQn_Type) PIN_PUSHBUTTON_1_ID);
	pio_handler_set_priority(PIN_PUSHBUTTON_1_PIO, (IRQn_Type) PIN_PUSHBUTTON_1_ID, 0);
	pio_enable_interrupt(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_MASK);
}

/**
 * \brief Initialize all LED IOPORT in OLED1 board.
 */
void aws_user_oled1_init(void)
{
	/* Initialize EXT3 LED0, LED1 & LED2, turned off */
	ioport_set_pin_level(OLED1_LED1_PIN, OLED1_LED1_INACTIVE);
	ioport_set_pin_dir(OLED1_LED1_PIN, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(OLED1_LED2_PIN, OLED1_LED1_INACTIVE);
	ioport_set_pin_dir(OLED1_LED2_PIN, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(OLED1_LED3_PIN, OLED1_LED1_INACTIVE);
	ioport_set_pin_dir(OLED1_LED3_PIN, IOPORT_DIR_OUTPUT);

	/* Configure Pushbutton 1. */
	pmc_enable_periph_clk(OLED1_PIN_PUSHBUTTON_1_ID);
	pio_set_debounce_filter(OLED1_PIN_PUSHBUTTON_1_PIO, OLED1_PIN_PUSHBUTTON_1_MASK, 200);
	pio_handler_set(OLED1_PIN_PUSHBUTTON_1_PIO, OLED1_PIN_PUSHBUTTON_1_ID,
			OLED1_PIN_PUSHBUTTON_1_MASK, OLED1_PIN_PUSHBUTTON_1_ATTR, aws_user_oled1_button_cb);
	NVIC_EnableIRQ((IRQn_Type) OLED1_PIN_PUSHBUTTON_1_ID);
	pio_handler_set_priority(OLED1_PIN_PUSHBUTTON_1_PIO, (IRQn_Type) OLED1_PIN_PUSHBUTTON_1_ID, 0);
	pio_enable_interrupt(OLED1_PIN_PUSHBUTTON_1_PIO, OLED1_PIN_PUSHBUTTON_1_MASK);

	/* Configure Pushbutton 2. */
	pmc_enable_periph_clk(OLED1_PIN_PUSHBUTTON_2_ID);
	pio_set_debounce_filter(OLED1_PIN_PUSHBUTTON_2_PIO, OLED1_PIN_PUSHBUTTON_2_MASK, 200);
	pio_handler_set(OLED1_PIN_PUSHBUTTON_2_PIO, OLED1_PIN_PUSHBUTTON_2_ID,
			OLED1_PIN_PUSHBUTTON_2_MASK, OLED1_PIN_PUSHBUTTON_2_ATTR, aws_user_oled1_button_cb);
	NVIC_EnableIRQ((IRQn_Type) OLED1_PIN_PUSHBUTTON_2_ID);
	pio_handler_set_priority(OLED1_PIN_PUSHBUTTON_2_PIO, (IRQn_Type) OLED1_PIN_PUSHBUTTON_2_ID, 0);
	pio_enable_interrupt(OLED1_PIN_PUSHBUTTON_2_PIO, OLED1_PIN_PUSHBUTTON_2_MASK);

	/* Configure Pushbutton 3. */
	pmc_enable_periph_clk(OLED1_PIN_PUSHBUTTON_3_ID);
	pio_set_debounce_filter(OLED1_PIN_PUSHBUTTON_3_PIO, OLED1_PIN_PUSHBUTTON_3_MASK, 200);
	pio_handler_set(OLED1_PIN_PUSHBUTTON_3_PIO, OLED1_PIN_PUSHBUTTON_3_ID,
			OLED1_PIN_PUSHBUTTON_3_MASK, OLED1_PIN_PUSHBUTTON_3_ATTR, aws_user_oled1_button_cb);
	NVIC_EnableIRQ((IRQn_Type) OLED1_PIN_PUSHBUTTON_3_ID);
	pio_handler_set_priority(OLED1_PIN_PUSHBUTTON_3_PIO, (IRQn_Type) OLED1_PIN_PUSHBUTTON_3_ID, 0);
	pio_enable_interrupt(OLED1_PIN_PUSHBUTTON_3_PIO, OLED1_PIN_PUSHBUTTON_3_MASK);
}

/**
 * \brief Initialize a exception timer, and then count down the timer.
 *
 * \param kit[in]             Pointer to an instance of AWS Kit
 */
void aws_user_exception_init_timer(t_aws_kit* kit)
{
	aws_kit_init_timer(&kit->exceptionTimer);
	aws_kit_countdown_sec(&kit->exceptionTimer, AWS_USER_ERROR_TIMEOUT_SEC);
}

/**
 * \brief Blink LED every 1 second, if a exception happens.
 * Blinking LED1 on not configured ATECC508A.
 * Blinking LED2 on inaccessible WIFI router.
 * Blinking LED3 on TLS failure.
 *
 * \param kit[in]             Pointer to an instance of AWS Kit
 */
void aws_user_exception_blink_led(t_aws_kit* kit)
{
	if (kit->errState == AWS_EX_NONE || kit->errState == AWS_EX_MAX) 
		return;

	/* Don't blink, if the timer is not expired. */
	if (!aws_kit_timer_expired(&kit->exceptionTimer))
		return;

	/* Toggle IOPORT pin corresponding to exception state. */
	AWS_ERROR("Exception happened!(%d)", kit->errState);
	if (kit->errState == AWS_EX_UNPROVISIONED_CRYPTO) {
		ioport_toggle_pin_level(OLED1_LED1_PIN);
	} else if(kit->errState == AWS_EX_UNAVAILABLE_WIFI) {
		ioport_toggle_pin_level(OLED1_LED2_PIN);
	} else if(kit->errState == AWS_EX_MQTT_FAILURE) {
		ioport_toggle_pin_level(OLED1_LED3_PIN);
	} else {
		return;
	}

	/* Initialze a exception timer, and then count down the timer. */
	aws_kit_init_timer(&kit->exceptionTimer);
	aws_kit_countdown_sec(&kit->exceptionTimer, AWS_USER_ERROR_TIMEOUT_SEC);
}

/**
 * \brief This User task monitors all button state from ISR to make user MQTT client reports the state.
 *
 * \param[in] params          Parameters for the task (Not used)
 */
void aws_user_task(void* params)
{
	int ret = AWS_E_FAILURE;
	t_aws_kit* kit = aws_kit_get_instance();
	uint8_t butBuffer[1];

	kit->buttonQueue = xQueueCreate(1, sizeof(uint8_t));
	kit->led.turn_on = aws_user_led_on;
	kit->led.turn_off = aws_user_led_off;

	/* Initialize OLED1 board and SW0 button. */
	aws_user_sw0_init();
	aws_user_oled1_init();

	for (;;) {

		while (xQueueReceive(kit->buttonQueue, butBuffer, 0)) {
			AWS_INFO("Pressed a button on the OLED1 : %d", butBuffer[0]);
			if (butBuffer[0] == OLED1_BUTTON1_ID)
				kit->button.isPressed[AWS_KIT_BUTTON_1] = AWS_BUTTON_PRESSED;
			else if(butBuffer[0] == OLED1_BUTTON2_ID)
				kit->button.isPressed[AWS_KIT_BUTTON_2] = AWS_BUTTON_PRESSED;
			else if(butBuffer[0] == OLED1_BUTTON3_ID)
				kit->button.isPressed[AWS_KIT_BUTTON_3] = AWS_BUTTON_PRESSED;
		}

		/* If a user presses the SW0 button for 3 seconds, then reset all user data including WIFI credential, host address, etc. */ 
		if (kit->pushButtonState == AWS_BUTTON_PRESSED) {
			if (ioport_get_pin_level(BUTTON_0_PIN) == BUTTON_0_ACTIVE) {
				if (aws_kit_timer_expired(&kit->resetISR)) {
					/* Turn on all LEDs to notify reboot. */
					kit->pushButtonState = AWS_BUTTON_RELEASED;
					ioport_set_pin_level(LED_0_PIN, LED_0_INACTIVE);
					ioport_set_pin_level(OLED1_LED1_PIN, LED_0_ACTIVE);
					ioport_set_pin_level(OLED1_LED2_PIN, LED_0_ACTIVE);
					ioport_set_pin_level(OLED1_LED3_PIN, LED_0_ACTIVE);
					aws_kit_init_timer(&kit->resetISR);
					ret = aws_main_reset_user_data(kit);
					if (ret != ATCA_SUCCESS) {
						AWS_ERROR("Failed to reset user data!(%d)", ret);
						ret = AWS_E_CRYPTO_FAILURE;
						break;
					}
					/* Reboot the kit. */
					aws_kit_software_reset();
				}
			} else {
				kit->pushButtonState = AWS_BUTTON_RELEASED;
				ioport_set_pin_level(LED_0_PIN, LED_0_INACTIVE);
				aws_kit_init_timer(&kit->resetISR);
			}
		}

		/* Once a exception happens, then blink the specified LED. */
		if ((kit->errState != AWS_EX_NONE) && (kit->errState > AWS_EX_NONE && kit->errState < AWS_EX_MAX)) {
			aws_user_exception_blink_led(kit); 
		}

		/* Block for 100ms. */
		vTaskDelay(AWS_USER_TASK_DELAY);
	}
}
