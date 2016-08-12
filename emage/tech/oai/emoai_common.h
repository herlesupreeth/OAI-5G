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

#include <emage.h>
#include <main.pb-c.h>
#include <statistics.pb-c.h>
#include <emlog.h>
#include "RRC/LITE/extern.h"
#include "LAYER2/MAC/extern.h"
#include "rrc_eNB_UE_context.h"
#include "enb_config.h"

/* OAI supports only 1 Component Carrier in current implementation. */
#define MAX_NUM_CCs 1
/* OAI supports only 1 module in current implementation. */
/* In OAI the term module refer to component carrier. */
#define MAX_NUM_MODs 1

/* UE identifier template. */
typedef int ueid_t;
/* Component Carrier (CC) identifier template. */
typedef int ccid_t;
/* Module identifier template. */
typedef uint32_t mid_t;
/* Logical channel identifier template. */
typedef uint8_t lcid_t;
/* Message sequence number template. */
typedef uint32_t seq_t;
/* Transaction identifier template. */
typedef uint32_t tid_t;

/* Refers to the layer 2 statistics request and prepares the requested layer 2
 * statistics reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_create_header (
	/* Base station identifier. */
	uint32_t b_id,
	/* Module identifier. */
	mid_t m_id,
	/* Sequence number. */
	seq_t seq,
	/* Transaction identifier. */
	tid_t t_id,
	/* Type of 1st level message. */
	MsgType type,
	/* Header message to be formed. */
	Header **header);

/* 
 * Fetches the UE instances maintained in OAI eNB MAC and updates the 
 * array maintained in the agent.
 */
void emoai_update_eNB_UE_inst (mid_t m_id);

/* 
 * Fetches the OAI eNB MAC instances and updates the 
 * array maintained in the agent.
 */
void emoai_update_eNB_MAC_inst (mid_t m_id);

/* 
 * Fetches the OAI eNB RRC instances and updates the  
 * array maintained in the agent.
 */
void emoai_update_eNB_RRC_inst (mid_t m_id);

/* 
 * Returns number of UEs connected to the particular base station module.
 */
int emoai_get_num_ues (mid_t m_id);

/* 
 * Returns C-RNTI of UE given the ue id in the system.
 */
uint32_t emoai_get_ue_crnti (mid_t m_id, ueid_t ue_id);

/* 
 * Fetches the template maintained for each UE in OAI.
 * This template holds UE context information.
 */
UE_TEMPLATE emoai_get_ue_template (mid_t m_id, ueid_t ue_id);

/* 
 * Returns the Buffer Status Report for a UE in OAI.
 */
int emoai_get_ue_bsr (mid_t m_id, ueid_t ue_id, lcid_t lcid);

/* 
 * Returns the Power Headroom Report for a UE in OAI.
 */
int emoai_get_ue_phr (mid_t m_id, ueid_t ue_id);

/* 
 * Returns the frame number on which OAI is currently operating.
 */
unsigned int emoai_get_current_frame (mid_t m_id);

/* 
 * Returns the number of bytes scheduled for transmission 
 * by the RLC instance corresponding to the radio bearer identifier.
 */
int emoai_get_tx_queue_size (mid_t m_id, 
					         ueid_t ue_id,
					         logical_chan_id_t channel_id);

/* 
 * Returns Timing Advance from scheduling control info maintained for a UE.
 */
int emoai_get_MAC_CE_bitmap_TA (mid_t m_id, ueid_t ue_id);

/* 
 * Returns the System Frame Number on which OAI is currently operating.
 */
unsigned int emoai_get_current_system_frame_num(mid_t m_id);

/* 
 * Returns the Subframe number on which OAI is currently operating.
 */
unsigned int emoai_get_current_subframe (mid_t m_id);

/* 
 * Returns System Frame Number and Subframe at which the report was received.
 */
uint16_t emoai_get_sfn_sf (mid_t m_id);

/* 
 * Returns the number of active Component Carriers in UE.
 */
int emoai_get_active_CC (mid_t m_id, ueid_t ue_id);

/* 
 * Returns the Rank Indicator of UE.
 */
int emoai_get_current_RI (mid_t m_id, ueid_t ue_id, ccid_t cc_id);

/* 
 * Returns UEs wideband Channel Quality Indicator.
 */
int emoai_get_ue_wcqi (mid_t m_id, ueid_t ue_id);

/* 
 * Returns UE's RRC state.
 */
int emoai_get_ue_state (mid_t m_id, ueid_t ue_id);

/* 
 * Returns UE transmission antenna type. Related to power control.
 */
int emoai_get_ue_trx_antenna (mid_t m_id, ueid_t ue_id);

/* 
 * Returns Transmission mode 1 (single antenna), 2 (transmit diversity), etc.
 */
int emoai_get_ue_trx_mode (mid_t m_id, ueid_t ue_id);

/* 
 * Returns timer in SF. Control the synchronization status of the UE, not the 
 * actual advance procedure.
 */
int emoai_get_time_alignment_timer (mid_t m_id, ueid_t ue_id);

/* 
 * Returns Transmission Time Interval bundling value (enabled or disabled).
 */
int emoai_get_tti_bundling (mid_t m_id, ueid_t ue_id);

/* 
 * Returns Maximum HARQ retransmission for UL.
 */
int emoai_get_maxHARQ_TX (mid_t m_id, ueid_t ue_id);

/* 
 * Returns value of simultaneous ACK/NACK CQI reporting.
 */
int emoai_get_simult_ack_nack_cqi (mid_t m_id, ueid_t ue_id);

/* 
 * Returns value of simultaneous ACK/NACK transmissions.
 */
int emoai_get_ack_nack_simult_trans (mid_t m_id, ueid_t ue_id);

/* 
 * Returns type of aperiodic CQI reporting mode.
 */
int emoai_get_aperiodic_cqi_rep_mode (mid_t m_id, ueid_t ue_id);

/* 
 * Returns Measurement gap configuration of UE.
 */
int emoai_get_meas_gap_config (mid_t m_id, ueid_t ue_id);

/* 
 * Returns Measurement gap offset if measurement gap is configured.
 */
int emoai_get_meas_gap_config_offset (mid_t m_id, ueid_t ue_id);

/* 
 * Returns Aggregated bit-rate of non-gbr bearer per UE in DL.
 */
int emoai_get_ue_aggreg_max_bitrate_dl (mid_t m_id, ueid_t ue_id);

/* 
 * Returns Aggregated bit-rate of non-gbr bearer per UL.
 */
int emoai_get_ue_aggreg_max_bitrate_ul (mid_t m_id, ueid_t ue_id);

/* 
 * Returns whether only half duplex support or not. FDD operation
 */
int emoai_get_half_duplex (ueid_t ue_id);

/* 
 * Returns whether intra-subframe hopping is supported or not.
 */
int emoai_get_intra_sf_hopping (ueid_t ue_id);

/* 
 * Returns whether type 2 hopping with n_sb > 1 is supported or not.
 */
int emoai_get_type2_sb_1 (ueid_t ue_id);

/* 
 * Returns category of the UE.
 */
int emoai_get_ue_category (ueid_t ue_id);

/* 
 * Returns whether support for resource allocation type 1 is there or not.
 */
int emoai_get_res_alloc_type1 (ueid_t ue_id);

/* 
 * Returns type of ACK/NACK feedback mode in TDD.
 */
int emoai_get_tdd_ack_nack_feedback (mid_t m_id, ueid_t ue_id);

/* 
 * Returns repition factor set for ACK/NACK. 
 */
int emoai_get_ack_nack_repetition_factor (mid_t m_id, ueid_t ue_id);

/* 
 * Returns whether Extended BSR size is enabled or disabled.
 */
int emoai_get_extended_bsr_size (mid_t m_id, ueid_t ue_id);

/* 
 * Returns offset index value for HARQ-ACK.
 */
int emoai_get_beta_offset_ack_index (mid_t m_id, ueid_t ue_id);

/* 
 * Returns offset index value for Rank Indication.
 */
int emoai_get_beta_offset_ri_index (mid_t m_id, ueid_t ue_id);

/* 
 * Returns offset index value for CQI.
 */
int emoai_get_beta_offset_cqi_index (mid_t m_id, ueid_t ue_id);

///////////////////////////////////////////////////////////////////////////////
///				eNB Configuration helper functions							///
//////////////////////////////////////////////////////////////////////////////

/* 
 * Returns the PLMN cell id of the cell.
 */
int emoai_get_cell_id (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the PUSCH resources in RBs for hopping.
 */
int emoai_get_hopp_offset (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the type of hopping mode used.
 */
int emoai_get_hopp_mode (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the number of subbands.
 */
int emoai_get_n_SB (mid_t m_id, ccid_t cc_id);

/* 
 * Returns whether 64 QAM is enabled or not.
 */
int emoai_get_enable64QAM (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the PHICH duration used.
 */
int emoai_get_phich_dur (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the number of resource element groups used for PHICH.
 */
int emoai_get_phich_res (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the resource index for ACK/NACK.
 */
int emoai_get_n1pucch_an (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the number of equally spaced cyclic time shifts.
 */
int emoai_get_deltaPUCCH_Shift (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the Physical Random Access Channel (PRACH) configuration index.
 */
int emoai_get_prach_ConfigIndex (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the Physical Random Access Channel (PRACH) frequency offset.
 */
int emoai_get_prach_FreqOffset (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the cyclic prefix length used in uplink.
 */
int emoai_get_ul_cyc_prefix_len (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the cyclic prefix length used in downlink.
 */
int emoai_get_dl_cyc_prefix_len (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the DL transmission bandwidth in RBs.
 */
int emoai_get_N_RB_DL (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the UL transmission bandwidth in RBs.
 */
int emoai_get_N_RB_UL (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the duration of RA response window in subframes.
 */
int emoai_get_ra_resp_window_size (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the type of duplex mode used.
 */
int emoai_get_dupl_mode (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the number of OFDM symbols possible to use for PDCCH in a subframe.
 */
int emoai_get_num_pdcch_symb (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the scheduling window for all SIs in subframes.
 */
long emoai_get_si_window_len (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the length of SIB1 in bytes.
 */
int emoai_get_sib1_len (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the DL/UL subframe assignment. TDD only.
 */
int emoai_get_sf_assign (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the special subframe pattern. TDD only.
 */
int emoai_get_special_sf_pattern (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the timer for RA. MAC contention resolution timer.
 */
int emoai_get_mac_ContentionResolutionTimer (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the Maximum Hybrid ARQ for Msg3 transmission.
 */
int emoai_get_maxHARQ_Msg3Tx (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the n Resource Blocks CQI.
 */
int emoai_get_nRB_CQI (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the SRS bandwidth configuration in SIB2.
 */
int emoai_get_srs_BandwidthConfig (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the SRS subframe configuration in SIB2.
 */
int emoai_get_srs_SubframeConfig (mid_t m_id, ccid_t cc_id);

/* 
 * Returns the SRS maximum uplink pilot time slot. TDD only.
 */
int emoai_get_srs_MaxUpPts (mid_t m_id, ccid_t cc_id);

#endif
