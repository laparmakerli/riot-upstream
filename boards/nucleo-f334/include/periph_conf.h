/*
 * Copyright (C) 2015  Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_nucleo-f334
 * @{
 *
 * @file
 * @brief       Peripheral MCU configuration for the nucleo-f334 board
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Kaspar Schleiser <kaspar.schleiser@fu-berlin.de>
 */

#ifndef PERIPH_CONF_H_
#define PERIPH_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Clock system configuration
 * @{
 **/
#define CLOCK_HSE           (8000000U)          /* external oscillator */
#define CLOCK_CORECLOCK     (72000000U)         /* desired core clock frequency */

/* the actual PLL values are automatically generated */
#define CLOCK_PLL_MUL       (CLOCK_CORECLOCK / CLOCK_HSE)
#define CLOCK_AHB_DIV       RCC_CFGR_HPRE_DIV1
#define CLOCK_APB2_DIV      RCC_CFGR_PPRE2_DIV1
#define CLOCK_APB1_DIV      RCC_CFGR_PPRE1_DIV2
#define CLOCK_FLASH_LATENCY FLASH_ACR_LATENCY_1
/** @} */

/**
 * @brief Timer configuration
 * @{
 */
#define TIMER_NUMOF         (1U)
#define TIMER_0_EN          1
#define TIMER_IRQ_PRIO      1

/* Timer 0 configuration */
#define TIMER_0_DEV         TIM2
#define TIMER_0_CHANNELS    4
#define TIMER_0_PRESCALER   (71U)
#define TIMER_0_MAX_VALUE   (0xffffffff)
#define TIMER_0_CLKEN()     (RCC->APB1ENR |= RCC_APB1ENR_TIM2EN)
#define TIMER_0_ISR         isr_tim2
#define TIMER_0_IRQ_CHAN    TIM2_IRQn
/** @} */

/**
 * @brief UART configuration
 * @}
 */
#define UART_NUMOF          (1U)
#define UART_0_EN           1
#define UART_IRQ_PRIO       1

/* UART 0 device configuration */
#define UART_0_DEV          USART2
#define UART_0_CLKEN()      (RCC->APB1ENR |= RCC_APB1ENR_USART2EN)
#define UART_0_CLK          (CLOCK_CORECLOCK / 2)   /* UART clock runs with 32MHz (F_CPU / 1) */
#define UART_0_IRQ_CHAN     USART2_IRQn
#define UART_0_ISR          isr_usart2
/* UART 0 pin configuration */
#define UART_0_PORT         GPIOA
#define UART_0_PORT_CLKEN() (RCC->AHBENR |= RCC_AHBENR_GPIOAEN)
#define UART_0_RX_PIN       3
#define UART_0_TX_PIN       2
#define UART_0_AF           7
/** @} */

/**
 * @name SPI configuration
 * @{
 */
#define SPI_NUMOF           (1U)
#define SPI_0_EN            1
#define SPI_IRQ_PRIO        1

/* SPI 0 device config */
#define SPI_0_DEV               SPI1
#define SPI_0_CLKEN()           (RCC->APB2ENR |= RCC_APB2ENR_SPI1EN)
#define SPI_0_CLKDIS()          (RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN)
#define SPI_0_IRQ               SPI1_IRQn
#define SPI_0_IRQ_HANDLER       isr_spi1
/* SPI 0 pin configuration */
#define SPI_0_SCK_PORT          GPIOA
#define SPI_0_SCK_PIN           5
#define SPI_0_SCK_AF            5
#define SPI_0_SCK_PORT_CLKEN()  (RCC->AHBENR |= RCC_AHBENR_GPIOAEN)
#define SPI_0_MISO_PORT         GPIOA
#define SPI_0_MISO_PIN          6
#define SPI_0_MISO_AF           5
#define SPI_0_MISO_PORT_CLKEN() (RCC->AHBENR |= RCC_AHBENR_GPIOAEN)
#define SPI_0_MOSI_PORT         GPIOA
#define SPI_0_MOSI_PIN          7
#define SPI_0_MOSI_AF           5
#define SPI_0_MOSI_PORT_CLKEN() (RCC->AHBENR |= RCC_AHBENR_GPIOAEN)
/** @} */

/**
 * @brief GPIO configuration
 * @{
 */
#define GPIO_NUMOF          3
#define GPIO_0_EN           1
#define GPIO_1_EN           1
#define GPIO_2_EN           1
#define GPIO_IRQ_PRIO       1

/* IRQ config */
#define GPIO_IRQ_0          (-1)    /* not configured */
#define GPIO_IRQ_1          (-1)    /* not configured */
#define GPIO_IRQ_2          GPIO_2
#define GPIO_IRQ_3          (-1)    /* not configured */
#define GPIO_IRQ_4          (-1)    /* not configured */
#define GPIO_IRQ_5          (-1)    /* not configured */
#define GPIO_IRQ_6          (-1)    /* not configured */
#define GPIO_IRQ_7          (-1)    /* not configured */
#define GPIO_IRQ_8          (-1)    /* not configured */
#define GPIO_IRQ_9          (-1)    /* not configured */
#define GPIO_IRQ_10         GPIO_1
#define GPIO_IRQ_11         (-1)    /* not configured */
#define GPIO_IRQ_12         (-1)    /* not configured */
#define GPIO_IRQ_13         GPIO_0
#define GPIO_IRQ_14         (-1)    /* not configured */
#define GPIO_IRQ_15         (-1)    /* not configured */

/* GPIO channel 0 config */
#define GPIO_0_PORT         GPIOC                   /* Used for user button 1 */
#define GPIO_0_PIN          13
#define GPIO_0_CLKEN()      (RCC->AHBENR |= RCC_AHBENR_GPIOCEN)
#define GPIO_0_EXTI_CFG1()  (SYSCFG->EXTICR[3] &= ~(SYSCFG_EXTICR4_EXTI13))
#define GPIO_0_EXTI_CFG2()  (SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC)
#define GPIO_0_IRQ          EXTI15_10_IRQn

/* GPIO channel 1 config */
#define GPIO_1_PORT         GPIOC
#define GPIO_1_PIN          10
#define GPIO_1_CLKEN()      (RCC->AHBENR |= RCC_AHBENR_GPIOCEN)
#define GPIO_1_EXTI_CFG1()  (SYSCFG->EXTICR[2] &= ~(SYSCFG_EXTICR3_EXTI10))
#define GPIO_1_EXTI_CFG2()  (SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI10_PC)
#define GPIO_1_IRQ          EXTI15_10_IRQn

/* GPIO channel 2 config */
#define GPIO_2_PORT         GPIOD
#define GPIO_2_PIN          2
#define GPIO_2_CLKEN()      (RCC->AHBENR |= RCC_AHBENR_GPIODEN)
#define GPIO_2_EXTI_CFG1()  (SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI2))
#define GPIO_2_EXTI_CFG2()  (SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PD)
#define GPIO_2_IRQ          EXTI2_TSC_IRQn

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H_ */
/** @} */
