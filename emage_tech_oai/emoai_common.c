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
#include "emoai_config.h"
#include "emoai_rrc_measurements.h"

#include "RRC/LITE/extern.h"
#include "openair2/LAYER2/MAC/extern.h"
#include "openair1/PHY/extern.h"

int emoai_create_new_thread (void * (*func)(void *), void * arg) {

	int s = 0;
	/* POSIX thread variable. */
	pthread_t thread;
	/* POSIX thread attribute variable. */
	pthread_attr_t attr;

	s = pthread_attr_init(&attr);
	if (s != 0) {
		EMLOG("Error initializing pthread attribute!");
		return s;
	}

	s = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (s != 0) {
		EMLOG("Error setting pthread attribute!");
		return s;
	}

	s = pthread_create(&thread, &attr, func, arg);
	if (s != 0) {
		EMLOG("Error creating thread!");
		return s;
	}

	s = pthread_attr_destroy(&attr);
	if (s != 0) {
		EMLOG("Error destroying pthread attribute!");
		return s;
	}

	return s;
}

int emoai_create_header(
		uint32_t b_id,
		seq_t seq,
		tid_t t_id,
		Header **header) {

	*header = malloc(sizeof(Header));
	if(*header == NULL)
		goto error;

	/* Initialize the header message. */
	header__init(*header);

	/*
	 * Filling the header.
	*/

	(*header)->vers = 1;

	(*header)->b_id = b_id;

	(*header)->seq = seq;

	(*header)->t_id = t_id;

	return 0;

	error:
		EMLOG("Error forming header!");
		return -1;
}

int emoai_get_num_ues (void) {
	UE_list_t *UE_list;
	UE_list = &(&eNB_mac_inst[DEFAULT_ENB_ID])->UE_list;
	return UE_list->num_UEs;
}

uint32_t emoai_get_b_id (void) {
	const Enb_properties_array_t* enb_properties = enb_config_get();
	return enb_properties->properties[DEFAULT_ENB_ID]->eNB_id;
}

float emoai_get_operating_dl_freq (ccid_t cc_id) {
	const Enb_properties_array_t* enb_properties = enb_config_get();
	return (enb_properties->properties[DEFAULT_ENB_ID]->
										downlink_frequency[cc_id] / 1000000);
}

int emoai_get_operating_band (ccid_t cc_id) {
	const Enb_properties_array_t* enb_properties = enb_config_get();
	return enb_properties->properties[DEFAULT_ENB_ID]->eutra_band[cc_id];
}

int emoai_get_eNB_dupl_mode (ccid_t cc_id) {
	const Enb_properties_array_t* enb_properties = enb_config_get();
	if (enb_properties->properties[DEFAULT_ENB_ID]->frame_type[cc_id] ==
																		FDD) {
		return DD_MODE__DDM_FDD;
	}
	else {
		return DD_MODE__DDM_TDD;
	}
}

uint32_t emoai_get_ue_crnti (ueid_t ue_id) {
	return UE_RNTI(DEFAULT_ENB_ID, ue_id);
}

char* emoai_get_ue_imsi (ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = emoai_get_ue_context(ue_id);

	if (ue_context_p != NULL && (&ue_context_p->ue_context != NULL)) {
		/* IMSI length is 15. */
		char *imsi = calloc(16, sizeof(char));
		strcpy(imsi, ue_context_p->ue_context.ue_imsi);
		return imsi;
	}
	return NULL;
}

char* emoai_get_selected_plmn_id (ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = emoai_get_ue_context(ue_id);

	if (ue_context_p != NULL && (&ue_context_p->ue_context != NULL)) {
		/* PLMN ID max length is 6. */
		char *plmn_id = calloc(7, sizeof(char));
		strcpy(plmn_id, ue_context_p->ue_context.plmn_id);
		return plmn_id;
	}
	return NULL;
}

UE_TEMPLATE emoai_get_ue_template (ueid_t ue_id) {
	UE_list_t *UE_list;
	UE_list = &(&eNB_mac_inst[DEFAULT_ENB_ID])->UE_list;
	return UE_list->UE_template[UE_PCCID(DEFAULT_ENB_ID, ue_id)][ue_id];
}

double emoai_get_ue_time_distance (ccid_t cc_id, ueid_t ue_id) {
	#ifdef LOCALIZATION
		uint32_t rnti = emoai_get_ue_crnti(ue_id);
		LTE_eNB_UE_stats *stats = get_eNB_UE_stats(DEFAULT_ENB_ID, cc_id, rnti);
		return stats->distance.time_based;
	#else
		return 0;
	#endif
}

double emoai_get_ue_power_distance (ccid_t cc_id, ueid_t ue_id) {
	#ifdef LOCALIZATION
		uint32_t rnti = emoai_get_ue_crnti(ue_id);
		LTE_eNB_UE_stats *stats = get_eNB_UE_stats(DEFAULT_ENB_ID, cc_id, rnti);
		return stats->distance.power_based;
	#else
		return 0;
	#endif
}

struct rrc_eNB_ue_context_s* emoai_get_ue_context (ueid_t ue_id) {
	uint32_t rnti = emoai_get_ue_crnti(ue_id);
	return rrc_eNB_get_ue_context(&eNB_rrc_inst[DEFAULT_ENB_ID], rnti);
}

int emoai_get_ue_state (ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = emoai_get_ue_context(ue_id);

	if (ue_context_p != NULL && (&ue_context_p->ue_context != NULL)) {
		return ue_context_p->ue_context.Status;
	}
	return RRC_STATE__RS_RRC_INACTIVE;
}

int emoai_get_meas_gap_config (ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = emoai_get_ue_context(ue_id);

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

int emoai_get_meas_gap_config_offset (ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = emoai_get_ue_context(ue_id);

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

int emoai_get_num_bands (ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = emoai_get_ue_context(ue_id);

	if(ue_context_p != NULL &&
		ue_context_p->ue_context.UE_EUTRA_Capability != NULL) {
		return ue_context_p->ue_context.UE_EUTRA_Capability->
								rf_Parameters.supportedBandListEUTRA.list.count;
	}
	return 0;
}

uint32_t* emoai_get_bands (ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = emoai_get_ue_context(ue_id);
	uint32_t *bands = NULL;

	if(ue_context_p != NULL &&
						ue_context_p->ue_context.UE_EUTRA_Capability != NULL) {
		int bands_to_scan = emoai_get_num_bands(ue_id);
		bands = (uint32_t *) malloc(sizeof(uint32_t) * bands_to_scan);
		for (int i = 0; i < bands_to_scan; i++) {
			bands[i] = ue_context_p->ue_context.UE_EUTRA_Capability->
							rf_Parameters.supportedBandListEUTRA.list.array[i]->
																	bandEUTRA;
		}
	}
	return bands;
}

int emoai_get_access_release_vers (ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = emoai_get_ue_context(ue_id);

	if(ue_context_p != NULL &&
						ue_context_p->ue_context.UE_EUTRA_Capability != NULL) {
		switch (ue_context_p->ue_context.UE_EUTRA_Capability->
														accessStratumRelease) {
		case AccessStratumRelease_rel8:
			return 8;
		case AccessStratumRelease_rel9:
			return 9;
		case AccessStratumRelease_rel10:
			return 10;
		}
	}
	return -1;
}

uint32_t emoai_get_feature_grp_ind (ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = emoai_get_ue_context(ue_id);
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
			/* fgi in rel.8 and 9 are 32 bits, hence size must not exceed 4. */
			if (size > 4) {
				/* fgi > 32 bits is not handled for now. */
				size = 4;
			}
			for (int i = 0; i < size; i++) {
				fgi = fgi << 8;
				fgi += ue_context_p->ue_context.UE_EUTRA_Capability->
												featureGroupIndicators->buf[i];
			}
			return fgi;
		} else {
			return 0xFFFFFFFF;
		}
	}
	return 0;
}

int emoai_get_ue_category (ueid_t ue_id) {
	struct rrc_eNB_ue_context_s* ue_context_p = emoai_get_ue_context(ue_id);

	if(ue_context_p != NULL &&
						ue_context_p->ue_context.UE_EUTRA_Capability != NULL) {
		return ue_context_p->ue_context.UE_EUTRA_Capability->ue_Category;
	}
	return -1;
}

int emoai_is_A5A4_supp (ueid_t ue_id) {
	uint32_t fgi = emoai_get_feature_grp_ind (ue_id);
	/* 14th bit in Feature group Indicator provides this info. */
	return ((fgi >> (32 - 14)) & 0x01);
}

int emoai_is_intraF_refs_per_meas_supp (ueid_t ue_id) {
	uint32_t fgi = emoai_get_feature_grp_ind (ue_id);
	/* 16th bit in Feature group Indicator provides this info. */
	return ((fgi >> (32 - 16)) & 0x01);
}

int emoai_is_interF_meas_supp (ueid_t ue_id) {
	uint32_t fgi = emoai_get_feature_grp_ind (ue_id);
	/* 25th bit in Feature group Indicator provides this info. */
	return ((fgi >> (32 - 25)) & 0x01);
}

int emoai_is_interF_refs_per_meas_supp (ueid_t ue_id) {
	uint32_t fgi = emoai_get_feature_grp_ind (ue_id);
	/* 16th bit in Feature group Indicator provides this info. */
	return (emoai_is_interF_meas_supp(ue_id) & ((fgi >> (32 - 16)) & 0x01));
}

int emoai_is_longDRX_DRX_supp (ueid_t ue_id) {
	uint32_t fgi = emoai_get_feature_grp_ind (ue_id);
	/* 5th bit in Feature group Indicator provides this info. */
	return ((fgi >> (32 - 5)) & 0x01);
}

int emoai_is_intraF_ANR_supp (ueid_t ue_id) {
	uint32_t fgi = emoai_get_feature_grp_ind (ue_id);
	/* 17th bit in Feature group Indicator provides this info. */
	return (emoai_is_longDRX_DRX_supp(ue_id) & ((fgi >> (32 - 17)) & 0x01));
}

int emoai_is_interF_HO_intra_DD_supp (ueid_t ue_id) {
	uint32_t fgi = emoai_get_feature_grp_ind (ue_id);
	/* 13th bit in Feature group Indicator provides this info. */
	return (emoai_is_interF_meas_supp(ue_id) & ((fgi >> (32 - 13)) & 0x01));
}

int emoai_is_interF_ANR_supp (ueid_t ue_id) {
	uint32_t fgi = emoai_get_feature_grp_ind (ue_id);
	/* 18th bit in Feature group Indicator provides this info. */
	return (emoai_is_interF_HO_intra_DD_supp(ue_id) &
												((fgi >> (32 - 18)) & 0x01));
}

int emoai_is_intraF_neighCellSIacq_supp (ueid_t ue_id) {
	if (emoai_get_access_release_vers(ue_id) > 8) {
		struct rrc_eNB_ue_context_s* ue = emoai_get_ue_context(ue_id);
		struct UE_EUTRA_Capability_v920_IEs	*nonCriticalExtension;
		nonCriticalExtension = ue->ue_context.UE_EUTRA_Capability->
														nonCriticalExtension;
		if ((nonCriticalExtension) &&
			(nonCriticalExtension->neighCellSI_AcquisitionParameters_r9.
											intraFreqSI_AcquisitionForHO_r9) &&
			(*nonCriticalExtension->neighCellSI_AcquisitionParameters_r9.
									intraFreqSI_AcquisitionForHO_r9 ==
 NeighCellSI_AcquisitionParameters_r9__intraFreqSI_AcquisitionForHO_r9_supported
			)) {
			return 1;
		}
	}
	return 0;
}

int emoai_is_interF_neighCellSIacq_supp (ueid_t ue_id) {
	if (emoai_get_access_release_vers(ue_id) > 8) {
		struct rrc_eNB_ue_context_s* ue = emoai_get_ue_context(ue_id);
		struct UE_EUTRA_Capability_v920_IEs	*nonCriticalExtension;
		nonCriticalExtension = ue->ue_context.UE_EUTRA_Capability->
														nonCriticalExtension;
		if ((nonCriticalExtension) &&
			(nonCriticalExtension->neighCellSI_AcquisitionParameters_r9.
											interFreqSI_AcquisitionForHO_r9) &&
			(*nonCriticalExtension->neighCellSI_AcquisitionParameters_r9.
									interFreqSI_AcquisitionForHO_r9 ==
 NeighCellSI_AcquisitionParameters_r9__interFreqSI_AcquisitionForHO_r9_supported
			)) {
			return 1;
		}
	}
	return 0;
}

int emoai_handle_ue_down (uint32_t * rnti) {

	struct rrc_m_conf_trigg *rrc_mconf_ctxt = NULL;

	/****** LOCK **********************************************************/
	pthread_spin_lock(&rrc_m_conf_t_lock);

	rrc_mconf_ctxt = rrc_m_conf_get_trigg(*rnti);

	pthread_spin_unlock(&rrc_m_conf_t_lock);
	/****** UNLOCK ********************************************************/
	if (rrc_mconf_ctxt != NULL) {
		/* UE no longer exists so remove its trigger. */
		rrc_m_conf_rem_trigg(rrc_mconf_ctxt);
	}

	/* Remove all the RRC measurement trigger contexts from tree for a
	 * particular UE.
	 */
	rrc_meas_rem_ue_all_trigg(*rnti);

	return 0;
}