/* Copyright (c) 2016 Supreeth Herle <s.herle@create-net.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/* OAI Layer 2 related technology abstraction implementation for emage.
 */

#include "emoai_layer2.h"
#include "emoai_common.h"

int emoai_L2_stats_reply (EmageMsg * request, EmageMsg ** reply) {

	mid_t m_id = request->head->m_id;
	L2StatisticsRequest *sts_req = request->mstats->l2_stats_req;

	/* Parameters required to form the l2 stats reply. */
	uint32_t *ue_RNTIs = NULL;
	uint32_t n_ue_RNTIs = 0;
	uint32_t ue_report_flags = 0;
	uint32_t *CC_IDs = NULL;
	uint32_t n_CC_IDs = 0;
	uint32_t cell_report_flags = 0;

	/* For loop variables. */
	int ue_id, cc_id;
	int i;

	Header *header;
	/* Initialize header message. */
	/* Assign the same transaction id as the request message. */
	if (emoai_create_header(
			request->head->b_id,
			m_id,
			request->head->seq,
			request->head->t_id,
			MSG_TYPE__STATS_REP, 
			&header) != 0)
		goto error;

	/* Initialize the statistics message. */
	Statistics *stats_reply = (Statistics *) malloc(sizeof(Statistics));
	if (stats_reply == NULL)
		goto error;
	statistics__init(stats_reply);

	/* Filling the statistics message. */
	stats_reply->type = STATS_MSG_TYPE__L2_STATISTICS_REPLY;
	stats_reply->has_type = 1;	
	stats_reply->stats_msg_case = STATISTICS__STATS_MSG_L2_STATS_REPL;

	/* Reply message for Layer 2 statistics request. */
	L2StatisticsReply *l2_stats_repl = malloc(sizeof(L2StatisticsReply));
	if (l2_stats_repl == NULL)
		goto error;
	/* Initialize the Layer 2 statistics reply message. */
	l2_statistics_reply__init(l2_stats_repl);
	l2_stats_repl->n_cell_report = 0;
	l2_stats_repl->n_ue_report = 0;

	/* Filling the Layer 2 statistics reply message. */
	UeStatisticsReport **ue_reports;
	CellStatisticsReport **cell_reports;

	/* Both cell and UE related statistics.	*/
	if (sts_req->type == L2_STATS_TYPE__L2ST_COMPLETE) {
		/* Fill in the number of UEs in system and its RNTIs. */
		n_ue_RNTIs = emoai_get_num_ues(m_id);
		ue_RNTIs = malloc(n_ue_RNTIs * sizeof(uint32_t));
		for (ue_id = 0; ue_id < n_ue_RNTIs; ue_id++) {
			ue_RNTIs[ue_id] = emoai_get_ue_crnti(m_id, ue_id);
		}
		ue_report_flags = sts_req->comp_stats_req->ue_report_flags;

		/* Fill in the number of CCs in system and its CC ids. */		
		n_CC_IDs = MAX_NUM_CCs;
		CC_IDs = malloc(n_CC_IDs * sizeof(uint32_t));
		for (cc_id = 0; cc_id < n_CC_IDs; cc_id++) {
			CC_IDs[cc_id] = cc_id;
		}
		cell_report_flags = sts_req->comp_stats_req->cell_report_flags;
	} 
	/* Base station cell related statistics. */
	else if (sts_req->type == L2_STATS_TYPE__L2ST_CELL) {
		n_CC_IDs = sts_req->cell_stats_req->n_cc_id;
		CC_IDs = malloc(n_CC_IDs * sizeof(uint32_t));
		for (cc_id = 0; cc_id < n_CC_IDs; cc_id++) {
			CC_IDs[cc_id] = sts_req->cell_stats_req->cc_id[cc_id];
		}
		cell_report_flags = sts_req->cell_stats_req->report_flags;
	}
	/* UE related statistics. */
	else if (sts_req->type == L2_STATS_TYPE__L2ST_UE) {
		ue_RNTIs = malloc(sizeof(uint32_t));
		for (ue_id = 0; ue_id < sts_req->ue_stats_req->n_rnti; ue_id++) {
			/* 
			 * Find array index corresponding to RNTI of UE. 
			 * Returns -1 if the OAI system does not have that UE RNTI.
			*/
			if (!(find_UE_id(m_id, sts_req->ue_stats_req->rnti[ue_id]) < 0)) {
				++n_ue_RNTIs;
				ue_RNTIs = realloc(ue_RNTIs, n_ue_RNTIs);
				ue_RNTIs[n_ue_RNTIs -1] = sts_req->ue_stats_req->rnti[ue_id];				
			}
		}
		ue_report_flags = sts_req->ue_stats_req->report_flags;
	}

	/* UE statistics report is formed here. */
	if (n_ue_RNTIs > 0) {
		ue_reports = malloc(n_ue_RNTIs * sizeof(UeStatisticsReport *));
		if (ue_reports == NULL)
			goto error;

		for (i = 0; i < n_ue_RNTIs; i++) {
			/* 
			 * Find array index corresponding to RNTI of UE. 
			 * Returns -1 if the OAI system does not have that UE RNTI.
			*/
			ue_id = find_UE_id(m_id, ue_RNTIs[i]);
			/* Filling the UE statistics report for each UE. */
			ue_reports[i] = (UeStatisticsReport *) malloc(sizeof(
														UeStatisticsReport));
			ue_statistics_report__init(ue_reports[i]);
			ue_reports[i]->rnti = ue_RNTIs[i];
			ue_reports[i]->has_rnti = 1;
			ue_reports[i]->report_flags = ue_report_flags;
			ue_reports[i]->has_report_flags = 1;

			/* Check flag for buffer status report */
			if (ue_report_flags & UE_STATS_TYPE__UEST_BSR) {
				/* Add buffer status reports to the UE report. */
				ue_reports[i]->n_bsr = 4;
				if (emoai_prep_bsr (ue_reports[i]->n_bsr,
									&ue_reports[i]->bsr,
									m_id,
									ue_id) < 0) {
					goto error;
				}
			}

			/* Check flag for power headroom report */
			if (ue_report_flags & UE_STATS_TYPE__UEST_PRH) {
				/* Fill in the power headroom value for the UE. */
				ue_reports[i]->phr = emoai_get_ue_phr (m_id, ue_id);
				ue_reports[i]->has_phr = 1;
			}

			/* Check flag for RLC buffer status report */
			if (ue_report_flags & UE_STATS_TYPE__UEST_RLC_BS) {
				/* Add RLC buffer status reports to the UE report. */
				ue_reports[i]->n_rlc_bsr = 3;
				if (emoai_prep_rlc_bsr (ue_reports[i]->n_rlc_bsr,
										&ue_reports[i]->rlc_bsr,
										m_id,
										ue_id) < 0) {
					goto error;
				}
			}

			/* Check flag for MAC CE buffer status report */
			if (ue_report_flags & UE_STATS_TYPE__UEST_MAC_CE_BS) {
				/* Fill in the MAC CE buffer status report. */
				/* Use as bitmap. Set one or more of the MAC_CE_TYPE values. */
				int TA = emoai_get_MAC_CE_bitmap_TA(m_id, ue_id);
				uint32_t pend = (TA | (0 << 1) | (0 << 2) | (0 << 3)) & 15;
				ue_reports[i]->pending_mac_ces = pend;
				ue_reports[i]->has_pending_mac_ces = 1;
			}

			/* Check flag for DL CQI report */
			if (ue_report_flags & UE_STATS_TYPE__UEST_DL_CQI) {
				/* Add the DL CQI report to the stats report. */
				if (emoai_prep_dl_cqi (CSI_TYPE__CSIT_P10,
									   &ue_reports[i]->dl_cqi,
									   m_id,
									   ue_id)) {
					goto error;
				}
			}

			/* Check flag for paging buffer status report */
			if (ue_report_flags & UE_STATS_TYPE__UEST_PBS) {
				/* Add the paging report to the UE report. */
				if (emoai_prep_pbsr (&ue_reports[i]->pbr,
									 m_id,
									 ue_id)) {
					goto error;
				}
			}

			/* Check flag for UL CQI report */
			if (ue_report_flags & UE_STATS_TYPE__UEST_UL_CQI) {
				/* Add full UL CQI report to the UE report. */
				if (emoai_prep_ul_cqi (UL_CQI_TYPE__ULCT_SRS,
									   &ue_reports[i]->ul_cqi,
									   m_id,
									   ue_id)) {
					goto error;
				}
			}
			/* Increment the number of UE reports included in stats reply. */
			++l2_stats_repl->n_ue_report;
		}
		/* Add list of all UE reports to the stats message */
		l2_stats_repl->ue_report = ue_reports;
	}

	/* Cell statistics report is formed here. */
	if (n_CC_IDs > 0) {
		cell_reports = malloc(n_CC_IDs * sizeof(CellStatisticsReport *));
		if (cell_reports == NULL)
			goto error;
	    /* Fill in the Cell reports. */
		for (i = 0; i < n_CC_IDs; i++) {
			cell_reports[i] = (CellStatisticsReport *) malloc(
												sizeof(CellStatisticsReport));
			if(cell_reports[i] == NULL)
				goto error;
			cell_statistics_report__init(cell_reports[i]);
			/*
			 * Filling the Cell statistics report for each CC id.
			*/
			cell_reports[i]->cc_id = CC_IDs[i];
			cell_reports[i]->has_cc_id = 1;
			cell_reports[i]->report_flags = cell_report_flags;
			cell_reports[i]->has_report_flags = 1;

			/* Check flag for noise and interference report */
			if(cell_report_flags & CELL_STATS_TYPE__CST_NOISE_INTERFERENCE) {			
				/* Add the noise interference report to the cell stats report.*/
				if (emoai_prep_noise_interf (&cell_reports[i]->noise_interf,
											 m_id,
											 cell_reports[i]->cc_id)) {
					goto error;
				}
			}
			/* Increment the number of Cell reports included in stats reply. */
			++l2_stats_repl->n_cell_report;
		}
		/* Add list of all cell reports to the stats reply message.	*/
		l2_stats_repl->cell_report = cell_reports;
	}
	/* Attach the l2 stats reply to the main stats message. */
	stats_reply->l2_stats_repl = l2_stats_repl;

	/* Form the main Emage message here. */
	*reply = (EmageMsg *) malloc(sizeof(EmageMsg));
	if(*reply == NULL)
		goto error;
	emage_msg__init(*reply);
	(*reply)->head = header;
	(*reply)->message_case = EMAGE_MSG__MESSAGE_M_STATS;
	(*reply)->mstats = stats_reply;

	if (n_ue_RNTIs > 0) {
		free(ue_RNTIs);
	}
	if (n_CC_IDs > 0) {
		free(CC_IDs);
	}

	return 0;

	error:
		EMLOG("Error forming layer 2 statistics reply message!");
		if (n_ue_RNTIs > 0) {
			free(ue_RNTIs);
		}
		if (n_CC_IDs > 0) {
			free(CC_IDs);
		}
		return -1;
}

int emoai_prep_bsr (size_t n_bsr, uint32_t ** bsr, mid_t m_id, ueid_t ue_id) {

	int j;
	/* 
	 * Create a report for each LCG (4 elements). 
	 *  Logical Channel Group (LCG)
	*/
	*bsr = malloc(sizeof(uint32_t) * n_bsr);
	if (*bsr == NULL)
		goto error;
	
	for (j = 0; j < n_bsr; j++) {
		/* 
		 *	Set the BSR for each LCG element of the current UE
		 *  we need to know cc_id here, consider the 1st one.
		 *  Since OAI support only one CC for now.
		*/
		*((*bsr) + j) = emoai_get_ue_bsr (m_id, ue_id, j);
	}
	return 0;

	error:
		EMDBG("Error preparing buffer status report!");
		return -1;
}

int emoai_prep_rlc_bsr (size_t n_rlc_bsr,
						RlcBsrReport ***buff_stat_reports,
						mid_t m_id,
						ueid_t ue_id) {

	int j;
	/* Fill in the RLC buffer status reports. */
	/* Set this to the number of LCs for this UE. */
	*buff_stat_reports = malloc(n_rlc_bsr * sizeof(RlcBsrReport *));
	RlcBsrReport **BSRs;
	BSRs = *buff_stat_reports;
	if (BSRs == NULL)
		goto error;
	/* 
	 * Fill the buffer status report for each logical channel.
	 * Refer LAYER2/openair2_proc.c for rlc status.
	*/
	for (j = 0; j < n_rlc_bsr; j++) {
		BSRs[j] = (RlcBsrReport *) malloc(sizeof(RlcBsrReport));
		if (BSRs[j] == NULL)
			goto error;
		rlc_bsr_report__init(BSRs[j]);
		/* Set logical channel id. */
		BSRs[j]->lc_id = j + 1;
		BSRs[j]->has_lc_id = 1;
		/* Set tx queue size in bytes. */
		BSRs[j]->tx_queue_size = emoai_get_tx_queue_size(m_id, ue_id, j+1);
		BSRs[j]->has_tx_queue_size = 1;
		/* Set tx queue head of line delay in ms. */
		BSRs[j]->tx_queue_hol_delay = 100;
		BSRs[j]->has_tx_queue_hol_delay = 1;
		/* Set retransmission queue size in bytes. */
		BSRs[j]->retransmission_queue_size = 10;
		BSRs[j]->has_retransmission_queue_size = 1;
		/* Set retransmission queue head of line delay in ms. */
		BSRs[j]->retransmission_queue_hol_delay = 100;
		BSRs[j]->has_retransmission_queue_hol_delay = 1;
		/* Set current size of the pending message in bytes. */
		BSRs[j]->status_pdu_size = 100;
		BSRs[j]->has_status_pdu_size = 1;
	}
	return 0;

	error:
		EMDBG("Error preparing rlc buffer status report!");
		return -1;
}

int emoai_prep_dl_cqi (CsiType type,
					   DlCqiReport **dl_cqi_report,
					   mid_t m_id,
					   ueid_t ue_id) {
	int j;
	/* 
	 * Fill in the DL CQI report for the UE 
	 * based on its configuration.
	*/
	*dl_cqi_report = (DlCqiReport *) malloc(sizeof(DlCqiReport));
	DlCqiReport *dl_rep;
	dl_rep = *dl_cqi_report;
	if (dl_rep == NULL)
		goto error;
	dl_cqi_report__init(dl_rep);
	/* Set the SFN and SF of the report held in the agent. */
	dl_rep->sfn_sn = emoai_get_sfn_sf(m_id);
	dl_rep->has_sfn_sn = 1;
	/* 
	 * Set the number of DL CQI reports for this UE.
	 * One for each CC.
	*/
	dl_rep->n_dl_csi = emoai_get_active_CC(m_id, ue_id);

	/* Create the CSI reports. */
	DlCsiReport **csi_rep;
	csi_rep = malloc(dl_rep->n_dl_csi * sizeof(DlCsiReport *));
	if (csi_rep == NULL)
		goto error;

	for (j = 0; j < dl_rep->n_dl_csi; j++) {
		csi_rep[j] = (DlCsiReport *) malloc(sizeof(DlCsiReport));
		if (csi_rep[j] == NULL)
			goto error;
		dl_csi_report__init(csi_rep[j]);
		/* The servCellIndex for this report. */
		csi_rep[j]->serv_cell_index = j;
		csi_rep[j]->has_serv_cell_index = 1;
		/* The rank indicator value for this cc. */
		csi_rep[j]->ri = emoai_get_current_RI(m_id, ue_id, j);
		csi_rep[j]->has_ri = 1;
		/* 
		 * The type of CSI report based on the configuration of 
		 * the UE. Here, we use type only P10, which only needs 
		 * a wideband value.
		*/
		csi_rep[j]->type =  type;
		csi_rep[j]->has_type = 1;
		
		if (csi_rep[j]->type == CSI_TYPE__CSIT_P10){
			/* Fill in the CSI P10 report. */
			csi_rep[j]->report_case = DL_CSI_REPORT__REPORT_P10CSI;
			CsiP10 *p10csi;
			p10csi = (CsiP10 *) malloc(sizeof(CsiP10));
			if (p10csi == NULL)
				goto error;
			csi_p10__init(p10csi);
			/*
			 * Set the wideband value. This depends on cc_id.
			*/
			p10csi->wb_cqi = emoai_get_ue_wcqi (m_id, ue_id);
			p10csi->has_wb_cqi = 1;
			/* Add this report to CSI report. */
			csi_rep[j]->p10csi = p10csi;
		}
		else if (csi_rep[j]->type == CSI_TYPE__CSIT_P11){
			// csi_rep[j]->report_case == DL_CSI_REPORT__REPORT_P11CSI;
		}
		else if (csi_rep[j]->type == CSI_TYPE__CSIT_P20){
			// csi_rep[j]->report_case == DL_CSI_REPORT__REPORT_P20CSI;
		}
		else if (csi_rep[j]->type == CSI_TYPE__CSIT_P21){
			// csi_rep[j]->report_case == DL_CSI_REPORT__REPORT_P21CSI;
		}
		else if (csi_rep[j]->type == CSI_TYPE__CSIT_A12){
			// csi_rep[j]->report_case == DL_CSI_REPORT__REPORT_A12CSI;
		}
		else if (csi_rep[j]->type == CSI_TYPE__CSIT_A22){
			// csi_rep[j]->report_case == DL_CSI_REPORT__REPORT_A22CSI;
		}
		else if (csi_rep[j]->type == CSI_TYPE__CSIT_A20){
			// csi_rep[j]->report_case == DL_CSI_REPORT__REPORT_A20CSI;
		}
		else if (csi_rep[j]->type == CSI_TYPE__CSIT_A30){
			// csi_rep[j]->report_case == DL_CSI_REPORT__REPORT_A30CSI;
		}
		else if (csi_rep[j]->type == CSI_TYPE__CSIT_A31){
			// csi_rep[j]->report_case == DL_CSI_REPORT__REPORT_A31CSI;
		}
	}
	/* Add the csi reports to the full DL CQI report. */
	dl_rep->dl_csi = dl_rep->dl_csi;

	return 0;

	error:
		EMDBG("Error preparing downlink CQI report!");
		return -1;
}

int emoai_prep_pbsr (PagingBufferReport **paging_buff_stats_report,
					 mid_t m_id,
					 ueid_t ue_id) {
	int j;
	/* Fill in the paging buffer status report. */
	/*
	 * For this field to be valid, the RNTI
	 * set in the report must be a P-RNTI
	*/
	*paging_buff_stats_report = (PagingBufferReport *) malloc(
													sizeof(PagingBufferReport));
	PagingBufferReport *pg_rep;
	pg_rep = *paging_buff_stats_report;
	if (pg_rep == NULL)
		goto error;
	paging_buffer_report__init(pg_rep);
	/* Set the number of pending paging messages. */
	pg_rep->n_pg_info = 1;
	/* Provide a report for each pending paging message. */
	PagingInfo **p_info;
	p_info = malloc(pg_rep->n_pg_info * sizeof(PagingInfo *));
	if (p_info == NULL)
		goto error;

	for (j = 0; j < pg_rep->n_pg_info; j++) {
		p_info[j] = (PagingInfo *) malloc(sizeof(PagingInfo));
		if(p_info[j] == NULL)
			goto error;
		paging_info__init(p_info[j]);
		/*
		 * Set paging index. This index is the same that will be 
		 * used for the scheduling of the paging message.
		 * Paging is not supported in OAI for now.
		*/
		p_info[j]->paging_index = 10;
		p_info[j]->has_paging_index = 1;
		/* Set the paging message size. */
		p_info[j]->paging_message_size = 100;
		p_info[j]->has_paging_message_size = 1;
		/* Set the paging subframe. */
		p_info[j]->paging_subframe = 10;
		p_info[j]->has_paging_subframe = 1;
		/* Set carrier index for the pending paging message. */
		p_info[j]->cc_index = 0;
		p_info[j]->has_cc_index = 1;
	}
	/* Add all paging info to the paging buffer report.	*/
	pg_rep->pg_info = p_info;

	return 0;

	error:
		EMDBG("Error preparing paging buffer status report!");
		return -1;
}

int emoai_prep_ul_cqi (uint32_t type,
					   UlCqiReport **uplink_cqi_report,
					   mid_t m_id,
					   ueid_t ue_id) {
	int j, k;
	/* Fill in the full UL CQI report of the UE. */
	*uplink_cqi_report = (UlCqiReport *) malloc(sizeof(UlCqiReport));
	UlCqiReport *ul_cqi;
	ul_cqi = *uplink_cqi_report;
	if(ul_cqi == NULL)
		goto error;
	ul_cqi_report__init(ul_cqi);
	/* Set the SFN and SF of the generated report. */
	ul_cqi->has_sfn_sn = 1;
	ul_cqi->sfn_sn = emoai_get_sfn_sf(m_id);				
	/*
	 * Set the number of UL measurement reports based on 
	 * types of measurements configured for the UE on the
	 * serving cell index. "type" is a BITMAP of ul_cqi_type.
	*/
	ul_cqi->n_cqi_meas = 1;
	UlCqi **ul_rep;
	ul_rep = malloc(ul_cqi->n_cqi_meas * sizeof(UlCqi *));
	if(ul_rep == NULL)
		goto error;
	/* 
	 * Fill each UL report of the UE for each of 
	 * the configured report types
	*/
	for(j = 0; j < ul_cqi->n_cqi_meas; j++) {
		ul_rep[j] = (UlCqi *) malloc(sizeof(UlCqi));
		if(ul_rep[j] == NULL)
			goto error;
		ul_cqi__init(ul_rep[j]);
		/*
		 * Set the type of the UL report. 
		 * As an example set it to SRS UL report.
		*/
		ul_rep[j]->type = UL_CQI_TYPE__ULCT_SRS;
		ul_rep[j]->has_type = 1;
		/*
		 * Set the number of SINR measurements based on the 
		 * report type.
		*/
		ul_rep[j]->n_sinr = 10;
		uint32_t *sinr_meas;
		sinr_meas = (uint32_t *) malloc(ul_rep[j]->n_sinr 
												* sizeof(uint32_t));
		if (sinr_meas == NULL)
			goto error;					
		/* Set the SINR measurements for the specified type. */
		for (k = 0; k < ul_rep[j]->n_sinr; k++) {
			sinr_meas[k] = 10;
		}
		ul_rep[j]->sinr = sinr_meas;
		/* Set the servCellIndex for this report. */
		ul_rep[j]->serv_cell_index = 0;
		ul_rep[j]->has_serv_cell_index = 1;
	}
	/* Set the list of UL reports of this UE to final UL report. */
	ul_cqi->cqi_meas = ul_rep;

	return 0;

	error:
		EMDBG("Error preparing uplink CQI report!");
		return -1;
}



int emoai_prep_noise_interf (NoiseInterferenceReport **noise_interf_report,
							 mid_t m_id,
							 int CC_id) {
	/* Fill in the noise and interference report for this cell. */
	*noise_interf_report = malloc(sizeof(NoiseInterferenceReport));
	NoiseInterferenceReport *ni_rep;
	ni_rep = *noise_interf_report;
	if(ni_rep == NULL)
		goto error;
	noise_interference_report__init(ni_rep);
	/* Current frame and subframe number. */
	ni_rep->sfn_sf = emoai_get_sfn_sf(m_id);
	ni_rep->has_sfn_sf = 1;
	/* Received interference power in dbm. */
	ni_rep->rip = 0;
	ni_rep->has_rip = 1;
	/* Thermal noise power in dbm. */
	ni_rep->tnp = 0;
	ni_rep->has_tnp = 1;

	return 0;

	error:
		EMDBG("Error preparing noise interference report!");
		return -1;
}