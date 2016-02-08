/*
 * Copyright (C) 2014-2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @addtogroup  cpu_stm32f1
 * @{
 *
 * @file
 * @brief       Low-level CPUID driver implementation
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 *
 * @}
 */

#include <string.h>

#include "periph/cpuid.h"

void cpuid_get(void *id)
{
    memcpy(id, (void *)(0x1ffff7e8), CPUID_LEN);
}
