/**
 *
 * \file
 *
 * \brief Platform timer interface
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

#include <asf.h>

#include "timer_interface.h"


/**
 * \brief Interrupt handler for the RTT.
 *
 * To do something on MS tick increment.
 */
void RTT_Handler(void)
{
	uint32_t ul_status;
	/* Get RTT status */
	ul_status = rtt_get_status(RTT);

	/* Time has changed, to do something */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
	}

	/* Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
	}
}

/**
 * \brief RTT configuration function.
 *
 * Configure the RTT to generate a one second tick, which triggers the RTTINC interrupt.
 */
void configure_rtt(void)
{
	uint32_t previous_time;

	/* Configure RTT for a 1ms tick interrupt */
	rtt_init(RTT, 32); //32768

	previous_time = rtt_read_timer_value(RTT);
	while (previous_time == rtt_read_timer_value(RTT));

	/* Enable RTT interrupt */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 0);
	NVIC_EnableIRQ(RTT_IRQn);
	rtt_enable_interrupt(RTT, RTT_MR_RTTINCIEN);
}

/**
 * \brief Get current value of RTT timer.
 *
 * \param tv[out]                   Set time values in seconds and microseconds
 * \return 0                        On success
 */
int _gettimeofday(struct timeval* tv, void* tzvp)
{
	uint32_t ms_tick = 0;

	if (!tv) return -1;

	ms_tick = rtt_read_timer_value(RTT);
	tv->tv_sec =  ms_tick / 1000;
	tv->tv_usec = ms_tick * 1000;

	return 0;
}

/**
 * \brief Initialize a timer.
 * Performs any initialization required to the timer passed in.
 *
 * \param Timer[out]                Pointer to the timer to be initialized
 */
void TimerInit(Timer *timer)
{
	timer->end_time.tv_sec  = 0;
	timer->end_time.tv_usec = 0;
}

/**
 * \brief Check if a timer is expired.
 * Call this function passing in a timer to check if that timer has expired.
 *
 * \param timer[in]                 Pointer to the timer to be checked for expiration
 * \return                          True = timer expired, False = timer not expired
 */
char TimerIsExpired(Timer *timer)
{
	struct timeval now, res;

	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);

	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}

/**
 * \brief Create a timer. (milliseconds)
 * Sets the timer to expire in a specified number of milliseconds.
 *
 * \param Timer[inout]              Pointer to the timer to be set to expire in milliseconds
 * \param timeout_ms[in]            Set the timer to expire in this number of milliseconds
 */
void TimerCountdownMS(Timer *timer, unsigned int timeout_ms)
{
	struct timeval now, interval = {timeout_ms / 1000, (int)((timeout_ms % 1000) * 1000)};

	gettimeofday(&now, NULL);
	timeradd(&now, &interval, &timer->end_time);
}

/**
 * \brief Create a timer. (seconds)
 * Sets the timer to expire in a specified number of seconds.
 *
 * \param timer[inout]              Pointer to the timer to be set to expire in seconds
 * \param timeout[in]               Set the timer to expire in this number of seconds
 */
void TimerCountdown(Timer *timer, unsigned int timeout)
{
	struct timeval now;
	struct timeval interval = {timeout, 0};

	gettimeofday(&now, NULL);
	timeradd(&now, &interval, &timer->end_time);
}

/**
 * \brief Check the time remaining on a given timer.
 * Checks the input timer and returns the number of milliseconds remaining on the timer.
 *
 * \param timer[inout]              Pointer to the timer to be set to checked
 * \return Milliseconds left on the countdown timer
 */
int TimerLeftMS(Timer *timer)
{
	uint32_t result_ms = 0;
	struct timeval now, res;

	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);
	if(res.tv_sec >= 0) {
		result_ms = (uint32_t) (res.tv_sec * 1000 + res.tv_usec / 1000);
	}

	return result_ms;
}
