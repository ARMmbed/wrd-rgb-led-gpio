/*
 * Copyright (c) 2016, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __WRD_RGB_LED_GPIO_H__
#define __WRD_RGB_LED_GPIO_H__

#include "mbed-drivers/mbed.h"

#include "core-util/CriticalSectionLock.h"

using namespace mbed::util;

#define MINIMUM_WIDTH 10

class RGBLEDImplementation
{
public:
    /**
     * @brief Constructor. Implementation of the WRD RGB API.
     * @details Pin configurations are pulled from Yotta config.
     */
    RGBLEDImplementation()
        :   code(0x00),
            gpioRed(YOTTA_CFG_HARDWARE_WRD_RGB_LED_RED_GPIO_PIN,
                    YOTTA_CFG_HARDWARE_WRD_RGB_LED_RED_GPIO_INVERSE),
            gpioGreen(YOTTA_CFG_HARDWARE_WRD_RGB_LED_GREEN_GPIO_PIN,
                      YOTTA_CFG_HARDWARE_WRD_RGB_LED_GREEN_GPIO_INVERSE),
            gpioBlue(YOTTA_CFG_HARDWARE_WRD_RGB_LED_BLUE_GPIO_PIN,
                     YOTTA_CFG_HARDWARE_WRD_RGB_LED_BLUE_GPIO_INVERSE)
    { }

    /**
     * @brief Set RGB color.
     *
     * @param red uint8_t red intensity.
     * @param green uint8_t green intensity.
     * @param blue uint8_t blue intensity.
     * @param callback FunctionPointer callback for when setting has been received.
     */
    void set(uint8_t newRed,
             uint8_t newGreen,
             uint8_t newBlue,
             FunctionPointer0<void> callback = (void(*)(void)) NULL)
    {
        if (callback)
        {
            minar::Scheduler::postCallback(callback)
                .tolerance(1);
        }

        /* set value */
        {
            CriticalSectionLock lock;

            // round up vaues 1-3 up to 4
            valueRed   = ((newRed   < 0x04) && (newRed   != 0)) ? 0x04 : newRed;
            valueGreen = ((newGreen < 0x04) && (newGreen != 0)) ? 0x04 : newGreen;
            valueBlue  = ((newBlue  < 0x04) && (newBlue  != 0)) ? 0x04 : newBlue;

            // set pin according to code and bitmap
            gpioRed   = (!!(valueRed   & code)) ^ YOTTA_CFG_HARDWARE_WRD_RGB_LED_RED_GPIO_INVERSE;
            gpioGreen = (!!(valueGreen & code)) ^ YOTTA_CFG_HARDWARE_WRD_RGB_LED_GREEN_GPIO_INVERSE;
            gpioBlue  = (!!(valueBlue  & code)) ^ YOTTA_CFG_HARDWARE_WRD_RGB_LED_BLUE_GPIO_INVERSE;

            if ((valueRed > 0) || (valueGreen > 0) || (valueBlue > 0))
            {
                // set countdown, use maximum width
                countdown.attach_us(this, &RGBLEDImplementation::timeoutHandler, MINIMUM_WIDTH);
            }
            else
            {
                countdown.detach();
            }
        }
    }

private:
    void timeoutHandler(void)
    {
        // skip the lowest 2 bits for better performance
        code += 4;

        switch (code)
        {
//            case (1 << 0):
//            case (1 << 1):
            case (1 << 2):
            case (1 << 3):
            case (1 << 4):
            case (1 << 5):
            case (1 << 6):
            case (1 << 7):
                        // set pin according to code and bitmap
                        gpioRed   = (!!(valueRed   & code)) ^ YOTTA_CFG_HARDWARE_WRD_RGB_LED_RED_GPIO_INVERSE;
                        gpioGreen = (!!(valueGreen & code)) ^ YOTTA_CFG_HARDWARE_WRD_RGB_LED_GREEN_GPIO_INVERSE;
                        gpioBlue  = (!!(valueBlue  & code)) ^ YOTTA_CFG_HARDWARE_WRD_RGB_LED_BLUE_GPIO_INVERSE;
                        break;
            default:
                        break;
        }
    }

    uint8_t code;

    DigitalOut gpioRed;
    DigitalOut gpioGreen;
    DigitalOut gpioBlue;

    volatile uint8_t valueRed;
    volatile uint8_t valueGreen;
    volatile uint8_t valueBlue;

    Ticker countdown;
};

#endif // __WRD_RGB_LED_GPIO_H__
