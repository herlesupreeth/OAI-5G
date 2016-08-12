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
 * OAI configurations related technology abstraction implementation for emage.
 */

#include "emoai_config.h"
#include "emoai_common.h"

int emoai_ue_config_reply (EmageMsg * request, EmageMsg ** reply) {

	// m_id a OAI specific identifier.
	mid_t m_id = request->head->m_id;
	UeConfigRequest *conf_req = request->mconfs->ue_conf_req;
	LayerConfig layer;

	/* Parameters required to form the config reply. */
	uint32_t *ue_RNTIs = NULL;
	uint32_t n_ue_RNTIs = 0;

	/* For loop variables. */
	int ue_id;
	int i;

	Header *header;
	/* Initialize header message. */
	/* Assign the same transaction id as the request message. */
	if (emoai_create_header(
			request->head->b_id,
			m_id,
			request->head->seq,
			request->head->t_id,
			MSG_TYPE__CONF_REP, 
			&header) != 0)
		goto error;

	/* Initialize the configuration message. */
	Configs *conf_reply = (Configs *) malloc(sizeof(Configs));
	if (conf_reply == NULL)
		goto error;
	configs__init(conf_reply);

	/* Filling the configuration message. */
	conf_reply->type = CONFIG_MSG_TYPE__UE_CONF_REPLY;
	conf_reply->has_type = 1;	
	conf_reply->config_msg_case = CONFIGS__CONFIG_MSG_UE_CONF_REPL;

	/* Find out the layers for which configuration was requested. */
	if (!conf_req->has_layer) {
		layer = LAYER_CONFIG__LC_ALL;
	} else {
		layer = conf_req->layer;
	}

	/* 
	 * Find out if list of UE RNTIs were provided or not. If not, get all the
	 * UEs RNTIs in system.
	 */
	if (conf_req->n_rnti > 0){
		/* 
		 * Filter the UEs based on whether the UEs are still registered in \
		 * system or not.
		*/
		ue_RNTIs = malloc(sizeof(uint32_t));
		for (ue_id = 0; ue_id < conf_req->n_rnti; ue_id++) {
			/* 
			 * Find array index corresponding to RNTI of UE. 
			 * Returns -1 if the OAI system does not have that UE RNTI.
			*/
			if (!(find_UE_id(m_id, conf_req->rnti[ue_id]) < 0)) {
				++n_ue_RNTIs;
				ue_RNTIs = realloc(ue_RNTIs, n_ue_RNTIs);
				ue_RNTIs[n_ue_RNTIs -1] = conf_req->rnti[ue_id];				
			}
		}
	} else {
		/* Fill in the number of UEs in system and its RNTIs. */
		n_ue_RNTIs = emoai_get_num_ues(m_id);
		ue_RNTIs = malloc(n_ue_RNTIs * sizeof(uint32_t));
		for (ue_id = 0; ue_id < n_ue_RNTIs; ue_id++) {
			ue_RNTIs[ue_id] = emoai_get_ue_crnti(m_id, ue_id);
		}
	}

	/* UE configuration reply message. */
	UeConfigReply *ue_conf_repl = malloc(sizeof(UeConfigReply));
	if (ue_conf_repl == NULL)
		goto error;
	/* Initialize the UE configuration reply message. */	
	ue_config_reply__init(ue_conf_repl);
	ue_conf_repl->n_ue_conf = n_ue_RNTIs;
	
	/* Filling the UE configuration. */
	UeConfig **ue_conf = NULL;

	if (n_ue_RNTIs > 0) {
		ue_conf = malloc(sizeof(UeConfig *) * n_ue_RNTIs);
		if (ue_conf == NULL)
			goto error;
	}

	for (i = 0; i < n_ue_RNTIs; i++) {
		/* 
		 * Find array index corresponding to RNTI of UE. 
		 * Returns -1 if the OAI system does not have that UE RNTI.
		*/
		ue_id = find_UE_id(m_id, ue_RNTIs[i]);
		/* Filling the UE configuration for each UE. */
		ue_conf[i] = (UeConfig *) malloc(sizeof(UeConfig));
		ue_config__init(ue_conf[i]);
		/* Set the RNTI of the UE. */
		ue_conf[i]->rnti = ue_RNTIs[i];
		ue_conf[i]->has_rnti = 1;
		ue_conf[i]->has_state = 1;
		ue_conf[i]->state = emoai_get_ue_state(m_id, ue_id);

		/* Check flag for PHY layer configuration. */
		if ((layer & LAYER_CONFIG__LC_ALL) || (layer & LAYER_CONFIG__LC_PHY)) {
			/* Add Physical layer related configuration to the UE conf. */
			if (emoai_get_ue_phy_conf (&ue_conf[i]->phy_conf,
									   m_id,
									   ue_id) < 0) {
				goto error;
			}
		}

		/* Check flag for MAC layer configuration. */
		if ((layer & LAYER_CONFIG__LC_ALL) || (layer & LAYER_CONFIG__LC_MAC)) {
			/* Add MAC layer related configuration to the UE conf. */
			if (emoai_get_ue_mac_conf (&ue_conf[i]->mac_conf,
									   m_id,
									   ue_id) < 0) {
				goto error;
			}
		}

		/* Check flag for RRC layer configuration. */
		if ((layer & LAYER_CONFIG__LC_ALL) || (layer & LAYER_CONFIG__LC_RRC)) {
			/* Add MAC layer related configuration to the UE conf. */
			if (emoai_get_ue_mac_conf (&ue_conf[i]->rrc_conf,
									   m_id,
									   ue_id) < 0) {
				goto error;
			}
		}
	}
	/* Add the UE configuration of all the UEs to final UE config report. */
	ue_conf_repl->ue_conf = ue_conf;
	/* Add the collective UE configuration report to configuration reply. */
	conf_reply->ue_conf_repl = ue_conf_repl;	

	/* Form the main Emage message here. */
	*reply = (EmageMsg *) malloc(sizeof(EmageMsg));
	if(*reply == NULL)
		goto error;
	emage_msg__init(*reply);
	(*reply)->head = header;
	(*reply)->message_case = EMAGE_MSG__MESSAGE_M_CONFS;
	(*reply)->mconfs = conf_reply;

	if (n_ue_RNTIs > 0) {
		free(ue_RNTIs);
	}

	return 0;

	error:
		EMLOG("Error forming UE configuration reply message!");
		if (n_ue_RNTIs > 0) {
			free(ue_RNTIs);
		}
		return -1;
}

int emoai_get_ue_phy_conf (UePhyConfig ** phy_conf, mid_t m_id, ueid_t ue_id) {
	/* 
	 * Fill in the PHY configuration for the UE.
	*/
	*phy_conf = (UePhyConfig *) malloc(sizeof(UePhyConfig));
	UePhyConfig *conf;
	conf = *phy_conf;
	if (conf == NULL)
		goto error;
	ue_phy_config__init(conf);

	/* Set the SR configuration. But info is not available. */

	/* Set UE transmission antenna. */
	if(emoai_get_ue_trx_antenna(m_id, ue_id) != -1){
		conf->has_ue_trx_antenna = 1;
		conf->ue_trx_antenna = emoai_get_ue_trx_antenna(m_id, ue_id);
	}
	/* Set the transmission mode. */
	if(emoai_get_ue_trx_mode(m_id, ue_id) != -1){
		conf->transmission_mode = emoai_get_ue_trx_mode(m_id, ue_id);
		conf->has_transmission_mode = 1;
	}

	return 0;

	error:
		EMDBG("Error getting UE PHY configuration!");
		return -1;
}

int emoai_get_ue_mac_conf (UeMacConfig ** mac_conf, mid_t m_id, ueid_t ue_id) {
	/* 
	 * Fill in the MAC configuration for the UE.
	*/
	*mac_conf = (UeMacConfig *) malloc(sizeof(UeMacConfig));
	UeMacConfig *conf;
	conf = *mac_conf;
	if (conf == NULL)
		goto error;
	ue_mac_config__init(conf);

	/* Set the time_alignment_timer. */
	if(emoai_get_time_alignment_timer(m_id, ue_id) != -1){
		conf->time_alignment_timer = 
									emoai_get_time_alignment_timer(m_id, ue_id);
		conf->has_time_alignment_timer = 1;
	}
	/* Set tti bundling flag (Refer ts 36.321). */
	if(emoai_get_tti_bundling(m_id, ue_id) != -1){
		conf->has_tti_bundling = 1;
		conf->tti_bundling = emoai_get_tti_bundling(m_id, ue_id);
	}
	/* Set the max HARQ retransmission for the UL. */
	if(emoai_get_maxHARQ_TX(m_id, ue_id) != -1){
		conf->has_max_harq_tx = 1;
		conf->max_harq_tx = emoai_get_maxHARQ_TX(m_id, ue_id);
	}
	/* Set ack_nack_simultaneous_trans (Refer TS 36.213). */
	if(emoai_get_ack_nack_simult_trans(m_id, ue_id) != -1){
		conf->has_ack_nack_simultaneous_trans = 1;
		conf->ack_nack_simultaneous_trans = 
								emoai_get_ack_nack_simult_trans(m_id, ue_id);
	}
	/* Set simultaneous_ack_nack_cqi (Refer TS 36.213). */
	if(emoai_get_simult_ack_nack_cqi(m_id, ue_id) != -1){
		conf->has_simultaneous_ack_nack_cqi = 1;
		conf->simultaneous_ack_nack_cqi = 
									emoai_get_simult_ack_nack_cqi(m_id, ue_id);
	}
	/* Set type of aperiodic CQI reporting mode. */
	if(emoai_get_aperiodic_cqi_rep_mode(m_id, ue_id) != -1){
		conf->has_aper_cqi_rep = 1;
		int mode = emoai_get_aperiodic_cqi_rep_mode(m_id, ue_id);
		if (mode > 4) {
			conf->aper_cqi_rep = APERIODIC_CQI_REPORT_MODE__ACRM_NONE;
		} else {
			conf->aper_cqi_rep = mode;
		}
	}

	return 0;

	error:
		EMDBG("Error getting UE MAC configuration!");
		return -1;
}

int emoai_get_ue_rrc_conf (UeRrcConfig ** rrc_conf, mid_t m_id, ueid_t ue_id) {
	/* 
	 * Fill in the RRC configuration for the UE.
	*/
	*rrc_conf = (UeRrcConfig *) malloc(sizeof(UeRrcConfig));
	UeRrcConfig *conf;
	conf = *rrc_conf;
	if (conf == NULL)
		goto error;
	ue_rrc_config__init(conf);
	
	/* Set the DRX configuration. But its not supported yet. */

	/* Set the measurement gap configuration pattern. */
	if(emoai_get_meas_gap_config(m_id, ue_id) != -1){
		conf->meas_gap_patt = emoai_get_meas_gap_config(m_id, ue_id);
		conf->has_meas_gap_patt = 1;
	}
	/* Set the measurement gap offset if applicable. */
	if(conf->has_meas_gap_patt == 1 &&
		conf->meas_gap_patt != MEAS_GAP_PATTERN__MGP_OFF) {
		conf->meas_gap_config_sf_offset = 
								emoai_get_meas_gap_config_offset(m_id, ue_id);
		conf->has_meas_gap_config_sf_offset = 1;
	}

	/* Set the SPS configuration. But its not supported yet. */

	/* Set the CQI configuration. But info is not available yet. */

	/* Set the aggregated bit-rate of the non-gbr bearer (UL). */
	conf->ue_aggreg_max_bitrate_ul = 
							emoai_get_ue_aggreg_max_bitrate_ul(m_id, ue_id);
	conf->has_ue_aggreg_max_bitrate_ul = 1;
	/* Set the aggregated bit-rate of the non-gbr bearer (DL). */
	conf->ue_aggreg_max_bitrate_dl = 
							emoai_get_ue_aggreg_max_bitrate_dl(m_id, ue_id);
	conf->has_ue_aggreg_max_bitrate_dl = 1;
	/* Set the UE capabilities. */
	UeCapabilities *capab;
	capab = malloc(sizeof(UeCapabilities));
	ue_capabilities__init(capab);
	/* Set half duplex (FDD operation). */
	capab->has_half_duplex = 1;
	capab->half_duplex = 1;
	/* Set intra-frame hopping flag. */
	capab->has_intra_sf_hopping = 1;
	capab->intra_sf_hopping = 1;
	/* Set support for type 2 hopping with n_sb > 1. */
	capab->has_type2_sb_1 = 1;
	capab->type2_sb_1 = 1;
	/* Set ue category. */
	capab->has_ue_category = 1;
	capab->ue_category = 1;
	/* Set UE support for resource allocation type 1. */
	capab->has_res_alloc_type1 = 1;
	capab->res_alloc_type1 = 1;
	/* Set the capabilites to the message. */
	conf->capabilities = capab;
	/* Set offset index value for HARQ-ACK. */
	if(emoai_get_beta_offset_ack_index(m_id, ue_id) != -1){
		conf->has_beta_offset_ack_index = 1;
		conf->beta_offset_ack_index = 
								emoai_get_beta_offset_ack_index(m_id, ue_id);
	}
	/* Set offset index value for Rank Indication. */
	if(emoai_get_beta_offset_ri_index(m_id, ue_id) != -1){
		conf->has_beta_offset_ri_index = 1;
		conf->beta_offset_ri_index = 
								emoai_get_beta_offset_ri_index(m_id, ue_id);
	}
	/* Set offset index value for CQI. */
	if(emoai_get_beta_offset_cqi_index(m_id, ue_id) != -1){
		conf->has_beta_offset_cqi_index = 1;
		conf->beta_offset_cqi_index = 
								emoai_get_beta_offset_cqi_index(m_id, ue_id);
	}
	/* Set type of ACK/NACK feedback mode in TDD. */
	if(emoai_get_tdd_ack_nack_feedback(m_id, ue_id) >= 0 && 
		emoai_get_tdd_ack_nack_feedback(m_id, ue_id) < 2){
		conf->has_tdd_ack_nack_feedb = 1;
		conf->tdd_ack_nack_feedb = emoai_get_tdd_ack_nack_feedback(m_id, ue_id);
	}
	/* Set repition factor set for ACK/NACK. */
	if(emoai_get_ack_nack_repetition_factor(m_id, ue_id) != -1){
		conf->has_ack_nack_rep_factor = 1;
		conf->ack_nack_rep_factor = 
							emoai_get_ack_nack_repetition_factor(m_id, ue_id);
	}
	/* Set extended BSR size. */
	if(emoai_get_extended_bsr_size(m_id, ue_id) != -1){
		conf->has_extended_bsr_size = 1;
		conf->extended_bsr_size = emoai_get_extended_bsr_size(m_id, ue_id);
	}
	/* Set carrier aggregation support (boolean). */
	conf->has_ca_support = 0;
	conf->ca_support = 0;
	if(conf->has_ca_support){
		/* Set cross carrier scheduling support (boolean). */
		conf->has_cross_carrier_sched_supp = 1;
		conf->cross_carrier_sched_supp = 0;
		/* Set index of primary cell. */
		conf->has_pcell_cc_id = 1;
		conf->pcell_cc_id = 0;
		/* 
		 * Set the secondary cells configuration. But since carrier 
		 * aggregation is not supported yet.
		 * We do not set scell_conf field.
		*/
		/*Set deactivation timer for secondary cell. */
		conf->has_scell_deactivation_timer = 1;
		conf->scell_deactivation_timer = 1;
	}

	return 0;

	error:
		EMDBG("Error getting UE RRC configuration!");
		return -1;
}

int emoai_eNB_config_reply (EmageMsg * request, EmageMsg ** reply) {

	// m_id a OAI specific identifier.
	mid_t m_id = request->head->m_id;
	EnbConfigRequest *conf_req = request->mconfs->enb_conf_req;
	LayerConfig layer;

	/* Parameters required to form the config reply. */
	uint32_t *CC_IDs = NULL;
	uint32_t n_CC_IDs = 0;

	/* For loop variables. */
	int cc_id;
	int i;

	Header *header;
	/* Initialize header message. */
	/* Assign the same transaction id as the request message. */
	if (emoai_create_header(
			request->head->b_id,
			m_id,
			request->head->seq,
			request->head->t_id,
			MSG_TYPE__CONF_REP, 
			&header) != 0)
		goto error;

	/* Initialize the configuration message. */
	Configs *conf_reply = (Configs *) malloc(sizeof(Configs));
	if (conf_reply == NULL)
		goto error;
	configs__init(conf_reply);

	/* Filling the configuration message. */
	conf_reply->type = CONFIG_MSG_TYPE__ENB_CONF_REPLY;
	conf_reply->has_type = 1;	
	conf_reply->config_msg_case = CONFIGS__CONFIG_MSG_ENB_CONF_REPL;

	/* Find out the layers for which configuration was requested. */
	if (!conf_req->has_layer) {
		layer = LAYER_CONFIG__LC_ALL;
	} else {
		layer = conf_req->layer;
	}

	/* 
	 * Find out if list of CC IDs were provided or not. If not, get all the
	 * CC IDs in system.
	 */
	if (conf_req->n_cc_id > 0){
		/* 
		 * Filter the CC IDs based on whether the CC ID exists in the
		 * system or not.
		*/
		CC_IDs = malloc(sizeof(uint32_t));
		for (cc_id = 0; cc_id < conf_req->n_cc_id; cc_id++) {
			/* 
			 * For now OAI supports only one CC, therefore the value of
			 * CC ID provided in request must be less than MAX_NUM_CCs.
			*/			
			if (conf_req->cc_id[cc_id] >= 0 && 
							conf_req->cc_id[cc_id] < MAX_NUM_CCs) {
				++n_CC_IDs;
				CC_IDs = realloc(CC_IDs, n_CC_IDs);
				CC_IDs[n_CC_IDs -1] = conf_req->cc_id[cc_id];				
			}
		}
	} else {
		/* Fill in the number of CCs in system and its CC ids. */		
		n_CC_IDs = MAX_NUM_CCs;
		CC_IDs = malloc(n_CC_IDs * sizeof(uint32_t));
		for (cc_id = 0; cc_id < n_CC_IDs; cc_id++) {
			CC_IDs[cc_id] = cc_id;
		}
	}

	/* eNB configuration reply message. */
	EnbConfigReply *eNB_conf_repl = malloc(sizeof(EnbConfigReply));
	if (eNB_conf_repl == NULL)
		goto error;
	/* Initialize the eNB configuration reply message. */	
	enb_config_reply__init(eNB_conf_repl);
	eNB_conf_repl->n_cell_conf = n_CC_IDs;
	
	/* Filling the eNB configuration. */
	CellConfig **cell_conf;

	if (n_CC_IDs > 0) {
		cell_conf = malloc(sizeof(CellConfig *) * n_CC_IDs);
		if (cell_conf == NULL)
			goto error;
	}

	for (i = 0; i < n_CC_IDs; i++) {
		/* Filling the eNB configuration for each CC ID. */
		cell_conf[i] = (CellConfig *) malloc(sizeof(CellConfig));
		cell_config__init(cell_conf[i]);
		/* Set the CC ID of the cell. */
		cell_conf[i]->cc_id = CC_IDs[i];
		cell_conf[i]->has_cc_id = 1;

		/* Check flag for PHY layer configuration. */
		if ((layer & LAYER_CONFIG__LC_ALL) || (layer & LAYER_CONFIG__LC_PHY)) {
			/* Add Physical layer related configuration to the Cell conf. */
			if (emoai_get_cell_phy_conf (&cell_conf[i]->phy_conf,
										 m_id,
										 CC_IDs[i]) < 0) {
				goto error;
			}
		}

		/* Check flag for MAC layer configuration. */
		if ((layer & LAYER_CONFIG__LC_ALL) || (layer & LAYER_CONFIG__LC_MAC)) {
			/* Add MAC layer related configuration to the Cell conf. */
			if (emoai_get_cell_mac_conf (&cell_conf[i]->mac_conf,
										 m_id,
										 CC_IDs[i]) < 0) {
				goto error;
			}
		}

		/* Check flag for RRC layer configuration. */
		if ((layer & LAYER_CONFIG__LC_ALL) || (layer & LAYER_CONFIG__LC_RRC)) {
			/* Add MAC layer related configuration to the Cell conf. */
			if (emoai_get_cell_rrc_conf (&cell_conf[i]->rrc_conf,
										 m_id,
										 CC_IDs[i]) < 0) {
				goto error;
			}
		}
	}
	/* Add the configuration of all the Cells to final Cell config report. */
	eNB_conf_repl->cell_conf = cell_conf;
	/* Add the collective eNB configuration report to configuration reply. */
	conf_reply->enb_conf_repl = eNB_conf_repl;	

	/* Form the main Emage message here. */
	*reply = (EmageMsg *) malloc(sizeof(EmageMsg));
	if(*reply == NULL)
		goto error;
	emage_msg__init(*reply);
	(*reply)->head = header;
	(*reply)->message_case = EMAGE_MSG__MESSAGE_M_CONFS;
	(*reply)->mconfs = conf_reply;

	if (n_CC_IDs > 0) {
		free(CC_IDs);
	}

	return 0;

	error:
		EMLOG("Error forming eNB configuration reply message!");
		if (n_CC_IDs > 0) {
			free(CC_IDs);
		}
		return -1;
}

int emoai_get_cell_phy_conf (CellPhyConfig ** phy_conf,
							 mid_t m_id,
							 ccid_t cc_id) {
	int j;
	/* 
	 * Fill in the PHY configuration for the Cell.
	*/
	*phy_conf = (CellPhyConfig *) malloc(sizeof(CellPhyConfig));
	CellPhyConfig *conf;
	conf = *phy_conf;
	if (conf == NULL)
		goto error;
	cell_phy_config__init(conf);

	/* Set the PCI of this cell. */
	conf->pci = 0;
	conf->has_pci = 1;
	/* Set the PLMN cell id of this cell. */
	conf->cell_id = emoai_get_cell_id(m_id, cc_id);
	conf->has_cell_id = 1;
	/* Set PUSCH resources in RBs for hopping. */
	conf->pusch_hopp_offset = emoai_get_hopp_offset(m_id, cc_id);
	conf->has_pusch_hopp_offset = 1;
	/* Set the type of hopping mode used. */
	if (emoai_get_hopp_mode(m_id, cc_id) >= 0 && 
										emoai_get_hopp_mode(m_id, cc_id) < 2) {
		conf->hopp_mode = emoai_get_hopp_mode(m_id, cc_id);
		conf->has_hopp_mode = 1;
	}
	/* Set the number of subbands. */
	conf->n_sb = emoai_get_n_SB(m_id, cc_id);
	conf->has_n_sb = 1;
	/* Set the number of resource element groups used for PHICH. */
	if (emoai_get_phich_res(m_id, cc_id) >= 0 && 
										emoai_get_phich_res(m_id, cc_id) < 4) {
		conf->phich_res = emoai_get_phich_res(m_id, cc_id);
		conf->has_phich_res = 1;
	}
	/* Set the PHICH duration used. */
	if (emoai_get_phich_dur(m_id, cc_id) >= 0 && 
										emoai_get_phich_dur(m_id, cc_id) < 2) {
		conf->phich_dur = emoai_get_phich_dur(m_id, cc_id);
		conf->has_phich_dur = 1;
	}
	/* Set the number of OFDM symbols possible to use for PDCCH in a subframe.*/
	// conf->init_nr_pdcch_ofdm_sym = emoai_get_num_pdcch_symb(m_id, cc_id);
	conf->init_nr_pdcch_ofdm_sym = 1;
	conf->has_init_nr_pdcch_ofdm_sym = 1;
	/* Fill the configuration used for Signalling Information messages. */
	SgInfoConfig *si_config;
	si_config = malloc(sizeof(SgInfoConfig));
	if(si_config == NULL)
		goto error;
	sg_info_config__init(si_config);
	/* Set the frame number to apply the SI configuration. */
	si_config->sfn = 1;
	si_config->has_sfn = 1;
	/* Set the length of SIB1 in bytes. */
	si_config->sib1_len = emoai_get_sib1_len(m_id, cc_id);
	si_config->has_sib1_len = 1;
	/* Set the scheduling window for all SIs in subframes. */
	si_config->si_window_len = (uint32_t) emoai_get_si_window_len(m_id, cc_id);
	si_config->has_si_window_len = 1;
	/* Set the number of SI messages. */
	si_config->n_si_message = 1;
	SgInfoMsg **si_msg;
	si_msg = malloc(sizeof(SgInfoMsg *) * si_config->n_si_message);
	if(si_msg == NULL)
		goto error;
	for (j = 0; j < si_config->n_si_message; j++) {
		si_msg[j] = (SgInfoMsg *) malloc(sizeof(SgInfoMsg));
		if(si_msg[j] == NULL)
			goto error;
		sg_info_msg__init(si_msg[j]);
		/* Set the periodicity of SI msg in radio frames. */
		si_msg[j]->periodicity = 1;
		si_msg[j]->has_periodicity = 1;
		/* Set the length of the SI message in bytes. */
		si_msg[j]->length = 10;
		si_msg[j]->has_length = 1;
	}
	if(si_config->n_si_message > 0){
		si_config->si_message = si_msg;
	}
	conf->si_conf = si_config;
	/* Set the DL transmission bandwidth in RBs. */
	conf->dl_bw = emoai_get_N_RB_DL(m_id, cc_id);
	conf->has_dl_bw = 1;
	/* Set the UL transmission bandwidth in RBs. */
	conf->ul_bw = emoai_get_N_RB_UL(m_id, cc_id);
	conf->has_ul_bw = 1;
	/* Set the cyclic prefix length used in uplink. */
	if (emoai_get_ul_cyc_prefix_len(m_id, cc_id) >= 0 && 
								emoai_get_ul_cyc_prefix_len(m_id, cc_id) < 2) {
		conf->ul_cyc_prefix_len = emoai_get_ul_cyc_prefix_len(m_id, cc_id);
		conf->has_ul_cyc_prefix_len = 1;
	}
	/* Set the cyclic prefix length used in downlink. */
	if (emoai_get_dl_cyc_prefix_len(m_id, cc_id) >= 0 && 
							emoai_get_dl_cyc_prefix_len(m_id, cc_id) < 2) {
		conf->dl_cyc_prefix_len = emoai_get_dl_cyc_prefix_len(m_id, cc_id);
		conf->has_dl_cyc_prefix_len = 1;
	}
	/* Set the number of cell specific antenna ports. */
	conf->antenna_ports_count = 1;
	conf->has_antenna_ports_count = 1;
	/* Set the type of duplex mode used. */
	if (emoai_get_dupl_mode(m_id, cc_id) >= 0 && 
										emoai_get_dupl_mode(m_id, cc_id) < 2) {
		conf->dupl_mode = emoai_get_dupl_mode(m_id, cc_id);
		conf->has_dupl_mode = 1;
	}
	/*
	 * Set the Physical Random Access Channel (PRACH) configuration index.
	 * Refer TS 36.211, section 5.7.1
	*/
	conf->prach_conf_index = emoai_get_prach_ConfigIndex(m_id, cc_id);
	conf->has_prach_conf_index = 1;
	/*
	 * Set the Physical Random Access Channel (PRACH) frequency offset.
	 * Refer TS 36.211, section 5.7.1
	*/
	conf->prach_freq_offset = emoai_get_prach_FreqOffset(m_id, cc_id);
	conf->has_prach_freq_offset = 1;
	/* Set the duration of RA response window in subframes. */
	conf->ra_resp_window_size = emoai_get_ra_resp_window_size(m_id, cc_id);
	conf->has_ra_resp_window_size = 1;
	/* Set the resource index for ACK/NACK. Refer TS 36.213, section 10.1. */
	conf->n1pucch_an = emoai_get_n1pucch_an(m_id, cc_id);
	conf->has_n1pucch_an = 1;
	/* Set the number of equally spaced cyclic time shifts. */
	conf->deltapucch_shift = emoai_get_deltaPUCCH_Shift(m_id, cc_id);
	conf->has_deltapucch_shift = 1;
	/* Set whether 64 QAM is enabled or not. */
	conf->enable_64qam = emoai_get_enable64QAM(m_id, cc_id);
	conf->has_enable_64qam = 1;

	return 0;

	error:
		EMDBG("Error getting Cell PHY configuration!");
		return -1;
}

int emoai_get_cell_mac_conf (CellMacConfig ** mac_conf,
							 mid_t m_id,
							 ccid_t cc_id) {
	/* 
	 * Fill in the MAC configuration for the Cell.
	*/
	*mac_conf = (CellMacConfig *) malloc(sizeof(CellMacConfig));
	CellMacConfig *conf;
	conf = *mac_conf;
	if (conf == NULL)
		goto error;
	cell_mac_config__init(conf);

	/* Set the DL/UL subframe assignment. TDD only. */
	conf->sf_assign = emoai_get_sf_assign(m_id, cc_id);
	conf->has_sf_assign = 1;
	/* Set the special subframe pattern. TDD only. See TS 36.211,table 4.2.1. */
	conf->special_subframe_patterns = emoai_get_special_sf_pattern(m_id, cc_id);
	conf->has_special_subframe_patterns = 1;
	/* Set the timer for RA. MAC contention resolution timer. */
	conf->mac_cont_resol_timer = 
						emoai_get_mac_ContentionResolutionTimer(m_id, cc_id);
	conf->has_mac_cont_resol_timer = 1;
	/* Set the Maximum Hybrid ARQ for Msg3 transmission. Refer TS 36.321. */
	conf->max_harq_msg3tx = emoai_get_maxHARQ_Msg3Tx(m_id, cc_id);
	conf->has_max_harq_msg3tx = 1;
	/* Set the n Resource Blocks CQI. Refer TS 36.211, section 5.4. */
	conf->nrb_cqi = emoai_get_nRB_CQI(m_id, cc_id);
	conf->has_nrb_cqi = 1;

	return 0;

	error:
		EMDBG("Error getting Cell MAC configuration!");
		return -1;
}

int emoai_get_cell_rrc_conf (CellRrcConfig ** rrc_conf,
							 mid_t m_id,
							 ccid_t cc_id) {
	int j;
	/* 
	 * Fill in the RRC configuration for the Cell.
	*/
	*rrc_conf = (CellRrcConfig *) malloc(sizeof(CellRrcConfig));
	CellRrcConfig *conf;
	conf = *rrc_conf;
	if (conf == NULL)
		goto error;
	cell_rrc_config__init(conf);

	/* Set the configuration of MBSFN radio frame period in SIB2. */
	conf->n_mbsfn_conf_rfperiod = 5;
	uint32_t *rfperiod;
	rfperiod = (uint32_t *) malloc(sizeof(uint32_t) * 
												conf->n_mbsfn_conf_rfperiod);
	if(rfperiod == NULL)
		goto error;
	for(j = 0; j < conf->n_mbsfn_conf_rfperiod; j++){
		rfperiod[j] = 1;
	}
	conf->mbsfn_conf_rfperiod = rfperiod;
	/* Set the configuration of MBSFN radio frame offset in SIB2. */
	conf->n_mbsfn_conf_rfoffset = 5;
	uint32_t *rfoffset;
	rfoffset = (uint32_t *) malloc(sizeof(uint32_t) * 
												conf->n_mbsfn_conf_rfoffset);
	if(rfoffset == NULL)
		goto error;
	for(j = 0; j < conf->n_mbsfn_conf_rfoffset; j++){
		rfoffset[j] = 1;
	}
	conf->mbsfn_conf_rfoffset = rfoffset;
	/*
	 * Set the bitmap indicating subframes that are allocated for MBSFN 
	 * within the MBSFN frame.
	*/
	conf->n_mbsfn_conf_sfalloc = 5;
	uint32_t *sfalloc;
	sfalloc = (uint32_t *) malloc(sizeof(uint32_t) * 
													conf->n_mbsfn_conf_sfalloc);
	if(sfalloc == NULL)
		goto error;
	for(j = 0; j < conf->n_mbsfn_conf_sfalloc; j++){
		sfalloc[j] = 1;
	}
	conf->mbsfn_conf_sfalloc = sfalloc;
	/* Set the SRS subframe configuration in SIB2. */
	conf->srs_sf_conf = emoai_get_srs_SubframeConfig(m_id, cc_id);
	conf->has_srs_sf_conf = 1;
	/* Set the SRS bandwidth configuration in SIB2. */
	conf->srs_bw_conf = emoai_get_srs_BandwidthConfig(m_id, cc_id);
	conf->has_srs_bw_conf = 1;
	/* Set the SRS maximum uplink pilot time slot. TDD only. */
	conf->srs_max_up_pts = emoai_get_srs_MaxUpPts(m_id, cc_id);
	conf->has_srs_max_up_pts = 1;

	return 0;

	error:
		EMDBG("Error getting Cell RRC configuration!");
		return -1;
}