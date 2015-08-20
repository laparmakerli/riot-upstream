/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gnrc_udp UDP
 * @ingroup     net_gnrc
 * @brief       GNRC's implementation of the UDP protocol
 *
 * @{
 *
 * @file
 * @brief       UDP GNRC definition
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef GNRC_UDP_H_
#define GNRC_UDP_H_

#include <stdint.h>

#include "byteorder.h"
#include "net/gnrc.h"
#include "net/udp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Default message queue size for the UDP thread
 */
#ifndef GNRC_UDP_MSG_QUEUE_SIZE
#define GNRC_UDP_MSG_QUEUE_SIZE (8U)
#endif

/**
 * @brief   Priority of the pktdump thread
 */
#ifndef GNRC_UDP_PRIO
#define GNRC_UDP_PRIO           (THREAD_PRIORITY_MAIN - 2)
#endif

/**
 * @brief   Default stack size to use for the UDP thread
 */
#ifndef GNRC_UDP_STACK_SIZE
#define GNRC_UDP_STACK_SIZE     (THREAD_STACKSIZE_DEFAULT)
#endif

/**
 * @brief   Calculate the checksum for the given packet
 *
 * @param[in] hdr           Pointer to the UDP header
 * @param[in] pseudo_hdr    Pointer to the network layer header
 *
 * @return  0 on success
 * @return  -EBADMSG if @p hdr is not of type GNRC_NETTYPE_UDP
 * @return  -EFAULT if @p hdr or @p pseudo_hdr is NULL
 * @return  -ENOENT if gnrc_pktsnip_t::type of @p pseudo_hdr is not known
 */
int gnrc_udp_calc_csum(gnrc_pktsnip_t *hdr, gnrc_pktsnip_t *pseudo_hdr);

/**
 * @brief   Allocate and initialize a fresh UDP header in the packet buffer
 *
 * @param[in] payload       Payload contained in the UDP packet
 * @param[in] src           Source port in host byte order
 * @param[in] src_len       Length of @p src, must be 2
 * @param[in] dst           Destination port in host byte order
 * @param[in] dst_len       Length of @p dst, must be 2
 *
 * @return  pointer to the newly created (and allocated) header
 * @return  NULL on `src == NULL`, `dst == NULL`, `src_len != 2`, `dst_len != 2`
 *          or on allocation error
 */
gnrc_pktsnip_t *gnrc_udp_hdr_build(gnrc_pktsnip_t *payload,
                                   uint8_t *src, size_t src_len,
                                   uint8_t *dst, size_t dst_len);

/**
 * @brief   Initialize and start UDP
 *
 * @return  PID of the UDP thread
 * @return  negative value on error
 */
int gnrc_udp_init(void);

#ifdef __cplusplus
}
#endif

#endif /* GNRC_UDP_H_ */
/** @} */
