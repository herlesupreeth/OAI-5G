/* Copyright (c) 2016 Supreeth Herle <s.herle@create-net.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* OAI configurations related technology abstractions for emage.
 *
 * This element provides OAI related configurations to the controller.
 *
 */

#ifndef __EMOAI_CONFIG_H
#define __EMOAI_CONFIG_H

#include <emage/pb/main.pb-c.h>
#include <emage/pb/configs.pb-c.h>
#include <emage/emlog.h>

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