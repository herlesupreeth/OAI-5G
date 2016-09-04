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

/*
 * OAI commonly used function.
 */

#include "emoai_common.h"

/* The following arrays hold most of the layer 2 and 3 statistics. */
eNB_MAC_INST * enb[MAX_NUM_MODs];
UE_list_t * enb_ue[MAX_NUM_MODs];
eNB_RRC_INST * enb_rrc[MAX_NUM_MODs];

int emoai_create_header(
		uint32_t b_id,
		mid_t m_id,
		seq_t seq,
		tid_t t_id,
		MsgType type,
		Header **header) {

	*header = malloc(sizeof(Header));
	if(*header == NULL)
		goto error;

	/* Initialize the header message. */
	header__init(*header);

	/*
	 * Filling the header.
 	*/

	(*header)->has_type = 1;
	(*header)->type = type;

	(*header)->has_vers = 1;
	(*header)->vers = 1;

	(*header)->has_b_id = 1;
	(*header)->b_id = b_id;

	(*header)->has_m_id = 1;
	(*header)->m_id = m_id;

	(*header)->has_seq = 1;
	(*header)->seq = seq;

	(*header)->has_t_id = 1;
	(*header)->t_id = t_id;

	return 0;

	error:
		EMLOG("Error forming header!");
		return -1;
}

void emoai_update_eNB_UE_inst (mid_t m_id) {
	enb_ue[m_id] = &eNB_mac_inst[m_id].UE_list;
}

void emoai_update_eNB_MAC_inst (mid_t m_id) {
	enb[m_id] = &eNB_mac_inst[m_id];
}

void emoai_update_eNB_RRC_inst (mid_t m_id) {
	enb_rrc[m_id] = &eNB_rrc_inst[m_id];
}

int emoai_get_num_ues (mid_t m_id) {
	emoai_update_eNB_UE_inst(m_id);
	return (enb_ue[m_id])->num_UEs;
}

uint32_t emoai_get_ue_crnti (mid_t m_id, ueid_t ue_id) {
	return  UE_RNTI(m_id, ue_id);
}

UE_TEMPLATE emoai_get_ue_template (mid_t m_id, ueid_t ue_id) {
	emoai_update_eNB_UE_inst(m_id);
	return (enb_ue[m_id])->UE_template[UE_PCCID(m_id, ue_id)][ue_id];
}

///////////////////////////////////////////////////////////////////////////////
///				L2 statistics helper functions								///
//////////////////////////////////////////////////////////////////////////////

int emoai_get_ue_bsr (mid_t m_id, ueid_t ue_id, lcid_t lcid) {
	UE_TEMPLATE ue_temp = emoai_get_ue_template(m_id, ue_id);
	return ue_temp.bsr_info[lcid];
}

int emoai_get_ue_phr (mid_t m_id, ueid_t ue_id) {
	UE_TEMPLATE ue_temp = emoai_get_ue_template(m_id, ue_id);
	return ue_temp.phr_info;
}

unsigned int emoai_get_current_frame (mid_t m_id) {
	emoai_update_eNB_MAC_inst(m_id);
	return (enb[m_id])->frame;
}

int emoai_get_tx_queue_size (mid_t m_id,
					         ueid_t ue_id,
					         logical_chan_id_t channel_id) {
	rnti_t rnti = emoai_get_ue_crnti(m_id, ue_id);
	uint16_t frame = (uint16_t) emoai_get_current_frame(m_id);
	mac_rlc_status_resp_t rlc_status = mac_rlc_status_ind(m_id,
														  rnti,
														  m_id,
														  frame,
														  ENB_FLAG_YES,
														  MBMS_FLAG_NO,
														  channel_id,
														  0);
	return rlc_status.bytes_in_buffer;
}

int emoai_get_MAC_CE_bitmap_TA (mid_t m_id, ueid_t ue_id) {
	emoai_update_eNB_UE_inst(m_id);
	if(((enb_ue[m_id])->UE_sched_ctrl[ue_id].ta_update) > 0)
		return 1;
	else
		return 0;
}

unsigned int emoai_get_current_system_frame_num(mid_t m_id) {
	/* System Frame Number : ranges from 0 to 1023. */
	return (emoai_get_current_frame(m_id) % 1024);
}

unsigned int emoai_get_current_subframe (mid_t m_id) {

	emoai_update_eNB_MAC_inst(m_id);
	return (enb[m_id])->subframe;
}

uint16_t emoai_get_sfn_sf (mid_t m_id) {

	frame_t frame;
	sub_frame_t subframe;
	uint16_t sfn_sf, frame_mask, sf_mask;

	frame = (frame_t) emoai_get_current_system_frame_num(m_id);
	subframe = (sub_frame_t) emoai_get_current_subframe(m_id);
	frame_mask = ((1 << 12) - 1);
	sf_mask = ((1 << 4) -1);
	sfn_sf = (subframe & sf_mask) | ((frame & frame_mask) << 4);

	return sfn_sf;
}

int emoai_get_active_CC (mid_t m_id, ueid_t ue_id) {
	emoai_update_eNB_UE_inst(m_id);
	return (enb_ue[m_id])->numactiveCCs[ue_id];
}

int emoai_get_current_RI (mid_t m_id, ueid_t ue_id, ccid_t cc_id) {
	LTE_eNB_UE_stats *eNB_UE_stats = NULL;

	int pCCid = UE_PCCID(m_id, ue_id);
	rnti_t rnti = emoai_get_ue_crnti(m_id, ue_id);

	eNB_UE_stats = mac_xface->get_eNB_UE_stats(m_id, cc_id, rnti);
	return eNB_UE_stats[cc_id].rank;
}

int emoai_get_ue_wcqi (mid_t m_id, ueid_t ue_id) {
	emoai_update_eNB_UE_inst(m_id);
	int pCCid = UE_PCCID(m_id, ue_id);
	return (enb_ue[m_id])->eNB_UE_stats[pCCid][ue_id].dl_cqi;
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
///				UE Configuration helper functions							///
//////////////////////////////////////////////////////////////////////////////

int emoai_get_ue_state (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);

	if (ue_context_p != NULL && (&ue_context_p->ue_context != NULL)) {
		return ue_context_p->ue_context.Status;
	}
	return UE_STATE__UES_RRC_INACTIVE;
}

int emoai_get_ue_trx_antenna (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);

	if (ue_context_p != NULL &&
					ue_context_p->ue_context.physicalConfigDedicated != NULL) {
		if (ue_context_p->ue_context.physicalConfigDedicated->antennaInfo->
			choice.explicitValue.ue_TransmitAntennaSelection.choice.setup ==
			AntennaInfoDedicated__ue_TransmitAntennaSelection__setup_closedLoop)
			return 2;
		else if (
			ue_context_p->ue_context.physicalConfigDedicated->antennaInfo->
			choice.explicitValue.ue_TransmitAntennaSelection.choice.setup ==
			AntennaInfoDedicated__ue_TransmitAntennaSelection__setup_openLoop)
			return 1;
		else
			return 0;
	}
	return -1;
}

int emoai_get_ue_trx_mode (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);

	if (ue_context_p != NULL &&
		ue_context_p->ue_context.physicalConfigDedicated != NULL) {
		return ue_context_p->ue_context.physicalConfigDedicated->antennaInfo->
										choice.explicitValue.transmissionMode;
	}
	return -1;
}

int emoai_get_time_alignment_timer (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);
	if (ue_context_p != NULL &&
		ue_context_p->ue_context.mac_MainConfig != NULL) {
		return ue_context_p->ue_context.mac_MainConfig->
													timeAlignmentTimerDedicated;
	}
	return -1;
}


int emoai_get_tti_bundling (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);
	if(ue_context_p != NULL &&
		ue_context_p->ue_context.mac_MainConfig != NULL) {
		return ue_context_p->ue_context.mac_MainConfig->
													ul_SCH_Config->ttiBundling;
	}
	return -1;
}

int emoai_get_maxHARQ_TX (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);
	if(ue_context_p != NULL &&
		ue_context_p->ue_context.mac_MainConfig != NULL) {
		return *ue_context_p->ue_context.mac_MainConfig->
													ul_SCH_Config->maxHARQ_Tx;
	}
	return -1;
}

int emoai_get_simult_ack_nack_cqi (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);
	if(ue_context_p != NULL &&
		ue_context_p->ue_context.physicalConfigDedicated != NULL &&
		ue_context_p->ue_context.physicalConfigDedicated->cqi_ReportConfig->
												cqi_ReportPeriodic != NULL) {
		return ue_context_p->ue_context.physicalConfigDedicated->
								cqi_ReportConfig->cqi_ReportPeriodic->
										choice.setup.simultaneousAckNackAndCQI;
	}
	return -1;
}

int emoai_get_ack_nack_simult_trans (mid_t m_id, ueid_t ue_id) {
	emoai_update_eNB_RRC_inst(m_id);
	return ((enb_rrc[m_id])->carrier[0].sib2->
			radioResourceConfigCommon.soundingRS_UL_ConfigCommon.choice.
									setup.ackNackSRS_SimultaneousTransmission);
}

int emoai_get_aperiodic_cqi_rep_mode (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);
	if(ue_context_p != NULL &&
		ue_context_p->ue_context.physicalConfigDedicated != NULL) {
		return *ue_context_p->ue_context.physicalConfigDedicated->
									cqi_ReportConfig->cqi_ReportModeAperiodic;
	}
	return -1;
}

int emoai_get_meas_gap_config (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id,ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);
	if(ue_context_p != NULL &&
		ue_context_p->ue_context.measGapConfig != NULL &&
		ue_context_p->ue_context.measGapConfig->present ==
													MeasGapConfig_PR_setup) {
		if (ue_context_p->ue_context.measGapConfig->
				choice.setup.gapOffset.present ==
									MeasGapConfig__setup__gapOffset_PR_gp0) {
			return MEAS_GAP_PATTERN__MGP_GP0;
		} else if (ue_context_p->ue_context.measGapConfig->
				choice.setup.gapOffset.present ==
									MeasGapConfig__setup__gapOffset_PR_gp1) {
			return MEAS_GAP_PATTERN__MGP_GP1;
		} else {
			return MEAS_GAP_PATTERN__MGP_OFF;
		}
	}
	return -1;
}

int emoai_get_meas_gap_config_offset (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id,ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);
	if(ue_context_p != NULL &&
		ue_context_p->ue_context.measGapConfig != NULL &&
		ue_context_p->ue_context.measGapConfig->present ==
													MeasGapConfig_PR_setup) {
		if (ue_context_p->ue_context.measGapConfig->
			choice.setup.gapOffset.present ==
									MeasGapConfig__setup__gapOffset_PR_gp0) {
			return ue_context_p->ue_context.measGapConfig->
											choice.setup.gapOffset.choice.gp0;
		} else if (ue_context_p->ue_context.measGapConfig->
				choice.setup.gapOffset.present ==
									MeasGapConfig__setup__gapOffset_PR_gp1) {
			return ue_context_p->ue_context.measGapConfig->
											choice.setup.gapOffset.choice.gp1;
		}
	}
	return -1;
}

int emoai_get_ue_aggreg_max_bitrate_dl (mid_t m_id, ueid_t ue_id) {
	emoai_update_eNB_UE_inst(m_id);
	return (enb_ue[m_id])->UE_sched_ctrl[ue_id].ue_AggregatedMaximumBitrateDL;
}

int emoai_get_ue_aggreg_max_bitrate_ul (mid_t m_id, ueid_t ue_id) {
	emoai_update_eNB_UE_inst(m_id);
	return (enb_ue[m_id])->UE_sched_ctrl[ue_id].ue_AggregatedMaximumBitrateUL;
}

uint32_t* emoai_get_half_duplex (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);
	uint32_t *halfduplex = NULL;

	if(ue_context_p != NULL &&
						ue_context_p->ue_context.UE_EUTRA_Capability != NULL) {
		int bands_to_scan = emoai_get_num_bands(m_id, ue_id);
		halfduplex = (uint32_t *) malloc(sizeof(uint32_t) * bands_to_scan);
		for (int i = 0; i < bands_to_scan; i++) {
			if (ue_context_p->ue_context.UE_EUTRA_Capability->
				rf_Parameters.supportedBandListEUTRA.list.array[i]->
															halfDuplex > 0) {
				halfduplex[i] = 1;
			} else {
				halfduplex[i] = 0;
			}
		}
	}
	return halfduplex;
}

int emoai_get_num_bands (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);

	if(ue_context_p != NULL &&
		ue_context_p->ue_context.UE_EUTRA_Capability != NULL) {
		return ue_context_p->ue_context.UE_EUTRA_Capability->
								rf_Parameters.supportedBandListEUTRA.list.count;
	}
	return 0;
}

uint32_t* emoai_get_bands (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);
	uint32_t *bands = NULL;

	if(ue_context_p != NULL &&
						ue_context_p->ue_context.UE_EUTRA_Capability != NULL) {
		int bands_to_scan = emoai_get_num_bands(m_id, ue_id);
		bands = (uint32_t *) malloc(sizeof(uint32_t) * bands_to_scan);
		for (int i = 0; i < bands_to_scan; i++) {
			bands[i] = ue_context_p->ue_context.UE_EUTRA_Capability->
							rf_Parameters.supportedBandListEUTRA.list.array[i]->
																	bandEUTRA;
		}
	}
	return bands;
}

uint32_t emoai_get_feature_grp_ind (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);
	uint32_t fgi = 0;

	if(ue_context_p != NULL &&
						ue_context_p->ue_context.UE_EUTRA_Capability != NULL) {
		/*
		 * If "featureGroupIndicators" is not included it means UE supports
		 * everything.
		*/
		if (ue_context_p->ue_context.UE_EUTRA_Capability->
											featureGroupIndicators != NULL) {
			uint32_t size = ue_context_p->ue_context.UE_EUTRA_Capability->
												featureGroupIndicators->size;
			for (int i = 0; i < size; i++) {
				fgi = fgi << 8;
				fgi += ue_context_p->ue_context.UE_EUTRA_Capability->
												featureGroupIndicators->buf[i];
			}
			return fgi;
		} else {
			return 1;
		}
	}
	return 0;
}

int emoai_get_intra_sf_hopping (mid_t m_id, ueid_t ue_id) {
	uint32_t fgi = emoai_get_feature_grp_ind (m_id, ue_id);
	return ((fgi & (1 << 31)) >> 31);
}

int emoai_get_type2_sb_1 (mid_t m_id, ueid_t ue_id) {
	uint32_t fgi = emoai_get_feature_grp_ind (m_id, ue_id);
	return ((fgi & (1 << 11)) >> 11);
}

int emoai_get_ue_category (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);

	if(ue_context_p != NULL &&
						ue_context_p->ue_context.UE_EUTRA_Capability != NULL) {
		return ue_context_p->ue_context.UE_EUTRA_Capability->ue_Category;
	}
	return -1;
}

int emoai_get_res_alloc_type1 (mid_t m_id, ueid_t ue_id) {
	uint32_t fgi = emoai_get_feature_grp_ind (m_id, ue_id);
	return ((fgi & (1 << 30)) >> 30);
}

int emoai_get_tdd_ack_nack_feedback (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);
	if(ue_context_p != NULL &&
		ue_context_p->ue_context.physicalConfigDedicated != NULL) {
		return ue_context_p->ue_context.physicalConfigDedicated->
								pucch_ConfigDedicated->tdd_AckNackFeedbackMode;
	}
	return -1;
}

int emoai_get_ack_nack_repetition_factor (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);
	if(ue_context_p != NULL &&
		ue_context_p->ue_context.physicalConfigDedicated != NULL) {
		return ue_context_p->ue_context.physicalConfigDedicated->
							pucch_ConfigDedicated->
								ackNackRepetition.choice.setup.repetitionFactor;
	}
	return -1;
}

int emoai_get_extended_bsr_size (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);
	if(ue_context_p != NULL &&
		ue_context_p->ue_context.mac_MainConfig != NULL &&
			ue_context_p->ue_context.mac_MainConfig->ext2 != NULL) {
		long val = (*(ue_context_p->ue_context.mac_MainConfig->ext2->
								mac_MainConfig_v1020->extendedBSR_Sizes_r10));
		if (val > 0) {
			return 1;
		}
	}
	return -1;
}

int emoai_get_beta_offset_ack_index (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);
	if(ue_context_p != NULL &&
		ue_context_p->ue_context.physicalConfigDedicated != NULL) {
		return ue_context_p->ue_context.physicalConfigDedicated->
									pusch_ConfigDedicated->betaOffset_ACK_Index;
	}
	return -1;
}

int emoai_get_beta_offset_ri_index (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);
	if(ue_context_p != NULL &&
		ue_context_p->ue_context.physicalConfigDedicated != NULL) {
		return ue_context_p->ue_context.physicalConfigDedicated->
									pusch_ConfigDedicated->betaOffset_RI_Index;
	}
	return -1;
}

int emoai_get_beta_offset_cqi_index (mid_t m_id, ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = NULL;
	uint32_t rntiP = emoai_get_ue_crnti(m_id, ue_id);
	emoai_update_eNB_RRC_inst(m_id);
	ue_context_p = rrc_eNB_get_ue_context(enb_rrc[m_id], rntiP);
	if(ue_context_p != NULL &&
		ue_context_p->ue_context.physicalConfigDedicated != NULL) {
		return ue_context_p->ue_context.physicalConfigDedicated->
									pusch_ConfigDedicated->betaOffset_CQI_Index;
	}
	return -1;
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
///				eNB Configuration helper functions							///
//////////////////////////////////////////////////////////////////////////////

int emoai_get_cell_id (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->Nid_cell;
}

int emoai_get_hopp_offset (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->pusch_config_common.pusch_HoppingOffset;
}

int emoai_get_hopp_mode (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->pusch_config_common.hoppingMode;
}

int emoai_get_n_SB (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->pusch_config_common.n_SB;
}

int emoai_get_enable64QAM (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->pusch_config_common.enable64QAM;
}

int emoai_get_phich_dur (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->phich_config_common.phich_duration;
}

int emoai_get_phich_res (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	if(frame_parms->phich_config_common.phich_resource == oneSixth)
		return 0;
	else if(frame_parms->phich_config_common.phich_resource == half)
		return 1;
	else if(frame_parms->phich_config_common.phich_resource == one)
		return 2;
	else if(frame_parms->phich_config_common.phich_resource == two)
		return 3;

	return -1;
}

int emoai_get_n1pucch_an (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->pucch_config_common.n1PUCCH_AN;
}

int emoai_get_deltaPUCCH_Shift (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->pucch_config_common.deltaPUCCH_Shift;
}

int emoai_get_prach_ConfigIndex (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
return frame_parms->prach_config_common.prach_ConfigInfo.prach_ConfigIndex;
}

int emoai_get_prach_FreqOffset (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->prach_config_common.prach_ConfigInfo.prach_FreqOffset;
}

int emoai_get_ul_cyc_prefix_len (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->Ncp_UL;
}

int emoai_get_dl_cyc_prefix_len (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->Ncp;
}

int emoai_get_N_RB_DL (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->N_RB_DL;
}

int emoai_get_N_RB_UL (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->N_RB_UL;
}

int emoai_get_ra_resp_window_size (mid_t m_id, ccid_t cc_id) {
	Enb_properties_array_t * enb_properties;
	enb_properties = enb_config_get();
	return enb_properties->properties[m_id]->
											rach_raResponseWindowSize[cc_id];
}

int emoai_get_dupl_mode (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	if(frame_parms->frame_type == 0)
		return 1;
	else if (frame_parms->frame_type == 1)
		return 0;

	return -1;
}

int emoai_get_num_pdcch_symb (mid_t m_id, ccid_t cc_id) {
	/* //Add these values to some struct in MAC.
	// Buggy. Will be corrected later.
	LTE_UE_PDCCH *lte_ue_pdcch;
	lte_ue_pdcch = mac_xface->get_lte_ue_pdcch(m_id, cc_id, m_id);

	return (PHY_vars_UE_g[m_id][cc_id]->
								lte_ue_pdcch_vars[m_id]->num_pdcch_symbols);*/
}

long emoai_get_si_window_len (mid_t m_id, ccid_t cc_id) {
	emoai_update_eNB_RRC_inst(m_id);
	return (enb_rrc[m_id])->carrier[cc_id].sib1->si_WindowLength;
}

int emoai_get_sib1_len (mid_t m_id, ccid_t cc_id) {
	emoai_update_eNB_RRC_inst(m_id);
	return (enb_rrc[m_id])->carrier[cc_id].sizeof_SIB1;
}

int emoai_get_sf_assign (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->tdd_config;
}

int emoai_get_special_sf_pattern (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->tdd_config_S;
}

int emoai_get_mac_ContentionResolutionTimer (mid_t m_id, ccid_t cc_id) {
	Enb_properties_array_t * enb_properties;
	enb_properties = enb_config_get();
	return enb_properties->properties[m_id]->
									rach_macContentionResolutionTimer[cc_id];
}

int emoai_get_maxHARQ_Msg3Tx (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->maxHARQ_Msg3Tx;
}

int emoai_get_nRB_CQI (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->pucch_config_common.nRB_CQI;
}

int emoai_get_srs_BandwidthConfig (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->soundingrs_ul_config_common.srs_BandwidthConfig;
}

int emoai_get_srs_SubframeConfig (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->soundingrs_ul_config_common.srs_SubframeConfig;
}

int emoai_get_srs_MaxUpPts (mid_t m_id, ccid_t cc_id) {
	LTE_DL_FRAME_PARMS * frame_parms;
	frame_parms = mac_xface->get_lte_frame_parms(m_id, cc_id);
	return frame_parms->soundingrs_ul_config_common.srs_MaxUpPts;
}