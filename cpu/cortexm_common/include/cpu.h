/*
 * Copyright (C) 2014-2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    cpu_cortexm_common ARM Cortex-M common
 * @ingroup     cpu
 * @brief       Common implementations and headers for Cortex-M family based
 *              micro-controllers
 * @{
 *
 * @file
 * @brief       Basic definitions for the Cortex-M common module
 *
 * When ever you want to do something hardware related, that is accessing MCUs
 * registers, just include this file. It will then make sure that the MCU
 * specific headers are included.
 *
 * @author      Stefan Pfeiffer <stefan.pfeiffer@fu-berlin.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Joakim Gebart <joakim.gebart@eistec.se>
 */

#ifndef CORTEXM_COMMON_H_
#define CORTEXM_COMMON_H_


#include "cpu_conf.h"

/**
 * TODO: remove once core was adjusted
 */
#include "irq.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Deprecated interrupt control function for backward compatibility
 * @{
 */
#define eINT            enableIRQ
#define dINT            disableIRQ
/** @} */

/**
 * @brief   Some members of the Cortex-M family have architecture specific atomic
 *          operations in atomic_arch.c
 */
#if defined(CPU_ARCH_CORTEX_M3) || defined(CPU_ARCH_CORTEX_M4) || \
    defined(CPU_ARCH_CORTEX_M4F)
#define ARCH_HAS_ATOMIC_COMPARE_AND_SWAP 1
#endif

/**
 * @brief   Initialization of the CPU
 */
void cpu_init(void);

#ifdef __cplusplus
}
#endif

#endif /* CORTEXM_COMMON_H_ */
/** @} */
