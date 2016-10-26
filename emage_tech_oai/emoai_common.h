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

/* OAI commonly used function.
 *
 * This element implements commonly used functions and functions that fetch
 * some values from OAI.
 *
 */

#ifndef __EMOAI_COMMON_H
#define __EMOAI_COMMON_H

#include <emage/emage.h>
#include <emage/pb/main.pb-c.h>
#include <emage/pb/statistics.pb-c.h>
#include <emage/pb/configs.pb-c.h>
#include <emage/emlog.h>
#include "emoai.h"

#include "openair2/LAYER2/MAC/defs.h"
#include "openair1/SCHED/defs.h"
#include "enb_config.h"
#include "rrc_eNB_UE_context.h"

/* UE identifier template. */
typedef int ueid_t;
/* Component Carrier (CC) identifier template. */
typedef int ccid_t;
/* Message sequence number template. */
typedef uint32_t seq_t;
/* Transaction identifier template. */
typedef uint32_t tid_t;

/* Create a thread with detached state.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_create_new_thread (void * (*func)(void *), void * arg);

/* Create the header for the main message.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_create_header (
	/* Base station identifier. */
	uint32_t b_id,
	/* Sequence number. */
	seq_t seq,
	/* Transaction identifier. */
	tid_t t_id,
	/* Header message to be formed. */
	Header **header);

/*
 * Returns number of UEs connected to the particular base station module.
 */
int emoai_get_num_ues (void);

/*
 * Returns base station identifier specified in eNB config file.
 */
uint32_t emoai_get_b_id (void);

/*
 * Returns operating downlink frequency specified in eNB config file.
 */
float emoai_get_operating_dl_freq (ccid_t cc_id);

/*
 * Returns duplexing mode of eNB from frame type parameter in eNB config file.
 */
int emoai_get_eNB_dupl_mode (ccid_t cc_id);

/*
 * Returns operating EUTRA frequency band specified in eNB config file.
 */
int emoai_get_operating_band (ccid_t cc_id);

/*
 * Returns C-RNTI of UE given the ue id in the system.
 */
uint32_t emoai_get_ue_crnti (ueid_t ue_id);

/*
 * Returns IMSI of UE intercepted at eNB.
 */
char* emoai_get_ue_imsi (ueid_t ue_id);

/*
 * Returns PLMN ID of CN to which UE is attached.
 */
char* emoai_get_selected_plmn_id (ueid_t ue_id);

/*
 * Fetches the template maintained for each UE in OAI.
 * This template holds UE context information.
 */
UE_TEMPLATE emoai_get_ue_template (ueid_t ue_id);

/*
 * Fetches the time-based localization of UE, relying on TA and TOA.
 */
double emoai_get_ue_time_distance (ccid_t cc_id, ueid_t ue_id);

/*
 * Fetches the power-based localization of UE, relying on RSS and RSSI.
 */
double emoai_get_ue_power_distance (ccid_t cc_id, ueid_t ue_id);

/*
 * Fetches the UE context based on UE id (Array index number).
 */
struct rrc_eNB_ue_context_s* emoai_get_ue_context (ueid_t ue_id);

/*
 * Returns UE's RRC state.
 */
int emoai_get_ue_state (ueid_t ue_id);

/*
 * Returns Measurement gap configuration of UE.
 */
int emoai_get_meas_gap_config (ueid_t ue_id);

/*
 * Returns Measurement gap offset if measurement gap is configured.
 */
int emoai_get_meas_gap_config_offset (ueid_t ue_id);

/*
 * Returns number of EUTRA bands supported by the UE.
 */
int emoai_get_num_bands (ueid_t ue_id);

/*
 * Returns the EUTRA bands supported by the UE.
 */
uint32_t* emoai_get_bands (ueid_t ue_id);

/*
 * Returns the 3GPP Access release implemented in UE.
 */
int emoai_get_access_release_vers (ueid_t ue_id);

/*
 * Returns the features group indicator set by UE.
 * This indicates the list of features supported by UE.
 */
uint32_t emoai_get_feature_grp_ind (ueid_t ue_id);

/*
 * Returns category of the UE.
 */
int emoai_get_ue_category (ueid_t ue_id);

/*
 * Returns whether UE supports event measurement reporting A4 and A5.
 */
int emoai_is_A5A4_supp (ueid_t ue_id);

/*
 * Returns whether UE supports periodical measurement, reportStrongestCells:
 * EUTRA Intra-frequency.
 */
int emoai_is_intraF_refs_per_meas_supp (ueid_t ue_id);

/*
 * Returns whether UE supports Inter-frequency measurements and reporting in
 * E-UTRA connected mode.
 */
int emoai_is_interF_meas_supp (ueid_t ue_id);

/*
 * Returns whether UE supports periodical measurement, reportStrongestCells:
 * EUTRA Inter-frequency.
 */
int emoai_is_interF_refs_per_meas_supp (ueid_t ue_id);

/*
 * Returns whether UE supports Long DRX cycle, DRX command MAC control element.
 */
int emoai_is_longDRX_DRX_supp (ueid_t ue_id);

/*
 * Returns whether UE supports Intra-frequency ANR features periodical
 * measurement, reportStrongestCells and reportCGI.
 */
int emoai_is_intraF_ANR_supp (ueid_t ue_id);

/*
 * Returns whether UE supports Inter-frequency handover within FDD or TDD.
 */
int emoai_is_interF_HO_intra_DD_supp (ueid_t ue_id);

/*
 * Returns whether UE supports Inter-frequency ANR features periodical
 * measurement, reportStrongestCells and reportCGI.
 */
int emoai_is_interF_ANR_supp (ueid_t ue_id);

/*
 * Returns whether UE supports Intra-frequency neighbor cell signalling info.
 * aquisition.
 */
int emoai_is_intraF_neighCellSIacq_supp (ueid_t ue_id);

/*
 * Returns whether UE supports Inter-frequency neighbor cell signalling info.
 * aquisition.
 */
int emoai_is_interF_neighCellSIacq_supp (ueid_t ue_id);

/*
 * Procedure used for clean up when an UE becomes inactive.
 */
int emoai_handle_ue_down (uint32_t * rnti);

#endif
