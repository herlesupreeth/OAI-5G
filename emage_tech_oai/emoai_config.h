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
#include <emage/emage.h>
#include "emoai_common.h"

#include "MeasObjectEUTRA.h"
#include "ReportConfigEUTRA.h"
#include "openair2/RRC/LITE/defs.h"

/* UE identifier template. */
typedef int ueid_t;
/* Component Carrier (CC) identifier template. */
typedef int ccid_t;
/* Radio Network Temporary identifier template. */
typedef uint16_t rnti_t;
/* Transaction identifier template. */
typedef uint32_t tid_t;

/* Trigger to send UE IDs to controller. Triggered when a UE is attached or
 * detached from the network.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_trig_UEs_ID_report (void);

/* Parses the UEs ID request and prepares the list of UE identifiers of
 * UEs attached to network.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_UEs_ID_report (
	EmageMsg * request,
	EmageMsg ** reply,
	unsigned int trigger_id);

/* Trigger to send UE's RRC measurement configuration to controller.
 * Triggered whenever an UE sends reconfiguration complete or connection
 * restablishment.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_trig_RRC_meas_conf_report (rnti_t * rnti);

/* Helps in forming EUTRA measurement object for RRC measurement configuration
 * reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_form_EUTRA_meas_obj (MeasObjectEUTRA_t m_obj, MeasObjEUTRA ** m);

/* Helps in forming EUTRA report configuration object for RRC measurement
 * configuration reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_form_EUTRA_rep_conf (ReportConfigEUTRA_t r_c, RepConfEUTRA ** r);

/* Parses the UE's RRC measurement configuration request and prepares
 * reply with UEs RRC measurement configuration.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_RRC_meas_conf_report (
	EmageMsg * request,
	EmageMsg ** reply,
	unsigned int trigger_id);

/* Request parameters related to RRC measurements configuration trigger.
 */
struct rrc_m_conf_trigg {
	/* Tree related data. */
	RB_ENTRY(rrc_m_conf_trigg) rrc_m_c_t;
	/* Transaction identifier. */
	tid_t t_id;
	/* RNTI of the UE. */
	uint32_t rnti;
};

/* Compares two triggers based on UE rnti.
 */
int rrc_m_conf_comp_trigg (
	struct rrc_m_conf_trigg* t1,
	struct rrc_m_conf_trigg* t2);

/* Fetches RRC measurement configuration trigger context based on UE rnti.
 */
struct rrc_m_conf_trigg* rrc_m_conf_get_trigg (uint32_t rnti);

/* Removes RRC measurement configuration trigger context from tree.
 */
int rrc_m_conf_rem_trigg (struct rrc_m_conf_trigg* ctxt);

/* Insert RRC measurement configuration trigger context into tree.
 */
int rrc_m_conf_add_trigg (struct rrc_m_conf_trigg* ctxt);

/* RB Tree holding all request parameters related to RRC measurements
 * configuration trigger.
 */
struct rrc_m_conf_trigg_tree;

#endif