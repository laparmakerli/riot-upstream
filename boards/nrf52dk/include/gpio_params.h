/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_nrf52dk
 * @{
 *
 * @file
 * @brief       Configuration of SAUL mapped GPIO pins
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 */

#ifndef GPIO_PARAMS_H
#define GPIO_PARAMS_H

#include "board.h"
#include "saul/periph.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief    LED configuration
 */
static const  saul_gpio_params_t saul_gpio_params[] =
{
    {
        .name = "LED 1",
        .pin = LED0_PIN,
        .dir = GPIO_DIR_OUT,
        .pull = GPIO_NOPULL
    },
    {
        .name = "LED 2",
        .pin = LED1_PIN,
        .dir = GPIO_DIR_OUT,
        .pull = GPIO_NOPULL
    },
    {
        .name = "LED 3",
        .pin = LED2_PIN,
        .dir = GPIO_DIR_OUT,
        .pull = GPIO_NOPULL
    },
    {
        .name = "LED 4",
        .pin = LED3_PIN,
        .dir = GPIO_DIR_OUT,
        .pull = GPIO_NOPULL
    },
    {
        .name = "Button 1",
        .pin = BUTTON1_PIN,
        .dir = GPIO_DIR_IN,
        .pull = GPIO_PULLUP
    },
    {
        .name = "Button 2",
        .pin = BUTTON2_PIN,
        .dir = GPIO_DIR_IN,
        .pull = GPIO_PULLUP
    },
    {
        .name = "Button 3",
        .pin = BUTTON3_PIN,
        .dir = GPIO_DIR_IN,
        .pull = GPIO_PULLUP
    },
    {
        .name = "Button 4",
        .pin = BUTTON4_PIN,
        .dir = GPIO_DIR_IN,
        .pull = GPIO_PULLUP
    }
};


#ifdef __cplusplus
}
#endif

#endif /* GPIO_PARAMS_H */
/** @} */
