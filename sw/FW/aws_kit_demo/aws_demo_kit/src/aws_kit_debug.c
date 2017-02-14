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

#include <asf.h>
#include "aws_kit_debug.h"

/**
 * \brief Return string to print it onto LCD of OLED1. (Not used.)
 *
 * \param info[in]                  Indicates string ID
 * \return string                   String to draw
 */
const char* aws_kit_get_string(AWS_KIT_LCD_INFO info)
{
    if (info <= AWS_KIT_MODE_INVALID && info >= AWS_KIT_MODE_MQTT_MAX) {
        return (const char*)"UNKNOWN Mode";
    }

    switch (info) {

    case AWS_KIT_MODE_PROVISIONING:
        return (const char*)"AWS KIT Mode\r\nProvisioning";

    case AWS_KIT_MODE_INITIALIZATION:
        return (const char*)"AWS KIT Mode\r\nInitialization";

    case AWS_KIT_MODE_MQTT_CLIENT:
        return (const char*)"AWS KIT Mode\r\nAWS Client";

    case AWS_KIT_SETUP_USER_DATA:
        return (const char*)"AWS KIT Mode\r\nCofigure data";

    default:
        return (const char*)"AWS KIT Mode\r\nUnknown";		
    }
}

/**
 * \brief Draw strings using a graphic library. (Not used.)
 *
 * \param info[in]                  Indicates string ID
 */
void aws_kit_lcd_print(AWS_KIT_LCD_INFO info)
{
	gfx_mono_draw_string(aws_kit_get_string(info), GFX_MONO_DISPLAY_X_POSITION, GFX_MONO_DISPLAY_Y_POSITION, &sysfont);
}

/**
 * \brief Reset the ATSAMG55 by force.
 */
void aws_kit_software_reset(void)
{
	AWS_INFO("Reset system");
	delay_ms(500);
	rstc_start_software_reset(RSTC);
}