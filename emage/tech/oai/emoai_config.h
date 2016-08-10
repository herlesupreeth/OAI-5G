/* Copyright (c) 2016 Supreeth Herle <s.herle@create-net.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/* OAI configurations related technology abstractions for emage.
 *
 * This element provides OAI related configurations to the controller.
 *
 */
#ifndef __EMOAI_CONFIG_H
#define __EMOAI_CONFIG_H

#include <main.pb-c.h>
#include <configs.pb-c.h>
#include <emlog.h>

/* UE identifier template. */
typedef int ueid_t;
/* Module identifier template. */
typedef uint32_t mid_t;
/* Component Carrier (CC) identifier template. */
typedef int ccid_t;

/* Refers to the UE configuration request and prepares the UE configuration
 * reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_ue_config_reply (EmageMsg * request, EmageMsg ** reply);

/* Prepares the physical layer related configuration message for UE
 * configuration reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_get_ue_phy_conf (UePhyConfig ** phy_conf, mid_t m_id, ueid_t ue_id);

/* Prepares the MAC layer related configuration message for UE configuration
 * reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_get_ue_mac_conf (UeMacConfig ** mac_conf, mid_t m_id, ueid_t ue_id);

/* Prepares the RRC layer related configuration message for UE configuration
 * reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_get_ue_rrc_conf (UeRrcConfig ** rrc_conf, mid_t m_id, ueid_t ue_id);

/* Refers to the eNB configuration request and prepares the eNB configuration
 * reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_eNB_config_reply (EmageMsg * request, EmageMsg ** reply);

/* Prepares the physical layer related configuration message for eNB
 * configuration reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_get_cell_phy_conf (CellPhyConfig ** phy_conf,
							 mid_t m_id,
							 ccid_t cc_id);

/* Prepares the MAC layer related configuration message for eNB configuration
 * reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_get_cell_mac_conf (CellMacConfig ** mac_conf,
							 mid_t m_id,
							 ccid_t cc_id);

/* Prepares the RRC layer related configuration message for eNB configuration
 * reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_get_cell_rrc_conf (CellRrcConfig ** rrc_conf,
							 mid_t m_id,
							 ccid_t cc_id);

#endif