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
 * OAI RRC measurements abstraction implementation for emage.
 */

#include "emoai_rrc_measurements.h"

#include "RRC/LITE/extern.h"
#include "openair2/LAYER2/MAC/extern.h"
#include "platform_types.h"
#include "msc.h"
#include "asn1_msg.h"

/* This disables EXMIMO_IOT flag and enables RRC measurements. */
#if defined(OAI_USRP)
  #undef EXMIMO_IOT
#endif

/* Holds the context of all the UEs maintained at RRC OAI eNB for performing
 * RRC operations.
 */
struct UE_RRC_proto_ctxt * UE_RRC_proto_ctxt_list = NULL;

/* Number maintained at RRC OAI eNB for lower layer signalling. */
mui_t * emoai_rrc_eNB_mui;

/* List of all LTE TDD band numbers. */
int eutra_tdd_bands [14] = {
	33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46
};

/* Lookup for each of EUTRA TDD bands bandwidth.
 */
struct tdd_bands_bw t_bands_bw[14] = {
	{.n = 33, .bw = 20},
	{.n = 34, .bw = 15},
	{.n = 35, .bw = 60},
	{.n = 36, .bw = 60},
	{.n = 37, .bw = 20},
	{.n = 38, .bw = 50},
	{.n = 39, .bw = 40},
	{.n = 40, .bw = 100},
	{.n = 41, .bw = 194},
	{.n = 42, .bw = 200},
	{.n = 43, .bw = 200},
	{.n = 44, .bw = 100},
	{.n = 45, .bw = 20},
	{.n = 46, .bw = 775}
};

/* Lookup for each of EUTRA TDD bands.
 */
struct tdd_bands_i tdd_bands[14] = {
	{.n = 33, .cn_l = 36000, .cn_h = 36199, .f_l = 1900},
	{.n = 34, .cn_l = 36200, .cn_h = 36349, .f_l = 2010},
	{.n = 35, .cn_l = 36350, .cn_h = 36949, .f_l = 1850},
	{.n = 36, .cn_l = 36950, .cn_h = 37549, .f_l = 1930},
	{.n = 37, .cn_l = 37550, .cn_h = 37749, .f_l = 1910},
	{.n = 38, .cn_l = 37750, .cn_h = 38249, .f_l = 2570},
	{.n = 39, .cn_l = 38250, .cn_h = 38649, .f_l = 1880},
	{.n = 40, .cn_l = 38650, .cn_h = 39649, .f_l = 2300},
	{.n = 41, .cn_l = 39650, .cn_h = 41589, .f_l = 2496},
	{.n = 42, .cn_l = 41590, .cn_h = 43589, .f_l = 3400},
	{.n = 43, .cn_l = 43590, .cn_h = 45589, .f_l = 3600},
	{.n = 44, .cn_l = 45590, .cn_h = 46589, .f_l = 703},
	{.n = 45, .cn_l = 46590, .cn_h = 46789, .f_l = 1447},
	{.n = 46, .cn_l = 46590, .cn_h = 54339, .f_l = 5150}
};

/* List of all LTE FDD band numbers. */
int eutra_fdd_bands [38] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30, 31, 32, 65, 66, 67, 68, 69, 70, 252, 255
};

/* Lookup for each of EUTRA FDD bands bandwidth.
 */
struct fdd_bands_bw f_bands_bw[38] = {
	{.n = 1, .bw = 60},
	{.n = 2, .bw = 60},
	{.n = 3, .bw = 75},
	{.n = 4, .bw = 45},
	{.n = 5, .bw = 25},
	{.n = 6, .bw = 10},
	{.n = 7, .bw = 70},
	{.n = 8, .bw = 35},
	{.n = 9, .bw = 35},
	{.n = 10, .bw = 60},
	{.n = 11, .bw = 20},
	{.n = 12, .bw = 17},
	{.n = 13, .bw = 10},
	{.n = 14, .bw = 10},
	{.n = 17, .bw = 12},
	{.n = 18, .bw = 15},
	{.n = 19, .bw = 15},
	{.n = 20, .bw = 30},
	{.n = 21, .bw = 15},
	{.n = 22, .bw = 80},
	{.n = 23, .bw = 20},
	{.n = 24, .bw = 34},
	{.n = 25, .bw = 65},
	{.n = 26, .bw = 35},
	{.n = 27, .bw = 17},
	{.n = 28, .bw = 45},
	{.n = 29, .bw = 11},
	{.n = 30, .bw = 10},
	{.n = 31, .bw = 5},
	{.n = 32, .bw = 44},
	{.n = 65, .bw = 90},
	{.n = 66, .bw = 70},
	{.n = 67, .bw = 20},
	{.n = 68, .bw = 30},
	{.n = 69, .bw = 95},
	{.n = 70, .bw = 15},
	{.n = 252, .bw = 100},
	{.n = 255, .bw = 125}
};

/* Lookup for DL of each of EUTRA FDD bands.
 */
struct fdd_bands_dl_i fdd_bands_dl[38] = {
	{.n = 1, .cn_DLl = 0, .cn_DLh = 599, .f_DLl = 2110},
	{.n = 2, .cn_DLl = 600, .cn_DLh = 1199, .f_DLl = 1930},
	{.n = 3, .cn_DLl = 1200, .cn_DLh = 1949, .f_DLl = 1805},
	{.n = 4, .cn_DLl = 1950, .cn_DLh = 2399, .f_DLl = 2110},
	{.n = 5, .cn_DLl = 2400, .cn_DLh = 2649, .f_DLl = 869},
	{.n = 6, .cn_DLl = 2650, .cn_DLh = 2749, .f_DLl = 875},
	{.n = 7, .cn_DLl = 2750, .cn_DLh = 3449, .f_DLl = 2620},
	{.n = 8, .cn_DLl = 3450, .cn_DLh = 3799, .f_DLl = 925},
	{.n = 9, .cn_DLl = 3800, .cn_DLh = 4149, .f_DLl = 1844.9},
	{.n = 10, .cn_DLl = 4150, .cn_DLh = 4749, .f_DLl = 2110},
	{.n = 11, .cn_DLl = 4750, .cn_DLh = 4949, .f_DLl = 1475.9},
	{.n = 12, .cn_DLl = 5010, .cn_DLh = 5179, .f_DLl = 729},
	{.n = 13, .cn_DLl = 5180, .cn_DLh = 5279, .f_DLl = 746},
	{.n = 14, .cn_DLl = 5280, .cn_DLh = 5379, .f_DLl = 758},
	{.n = 17, .cn_DLl = 5730, .cn_DLh = 5849, .f_DLl = 734},
	{.n = 18, .cn_DLl = 5850, .cn_DLh = 5999, .f_DLl = 860},
	{.n = 19, .cn_DLl = 6000, .cn_DLh = 6149, .f_DLl = 875},
	{.n = 20, .cn_DLl = 6150, .cn_DLh = 6449, .f_DLl = 791},
	{.n = 21, .cn_DLl = 6450, .cn_DLh = 6599, .f_DLl = 1495.9},
	{.n = 22, .cn_DLl = 6600, .cn_DLh = 7399, .f_DLl = 3510},
	{.n = 23, .cn_DLl = 7500, .cn_DLh = 7699, .f_DLl = 2180},
	{.n = 24, .cn_DLl = 7700, .cn_DLh = 8039, .f_DLl = 1525},
	{.n = 25, .cn_DLl = 8040, .cn_DLh = 8689, .f_DLl = 1930},
	{.n = 26, .cn_DLl = 8690, .cn_DLh = 9039, .f_DLl = 859},
	{.n = 27, .cn_DLl = 9040, .cn_DLh = 9209, .f_DLl = 852},
	{.n = 28, .cn_DLl = 9210, .cn_DLh = 9659, .f_DLl = 758},
	{.n = 29, .cn_DLl = 9660, .cn_DLh = 9769, .f_DLl = 717},
	{.n = 30, .cn_DLl = 9770, .cn_DLh = 9869, .f_DLl = 2350},
	{.n = 31, .cn_DLl = 9870, .cn_DLh = 9919, .f_DLl = 462.5},
	{.n = 32, .cn_DLl = 9920, .cn_DLh = 10359, .f_DLl = 1452},
	{.n = 65, .cn_DLl = 65536, .cn_DLh = 66435, .f_DLl = 2110},
	{.n = 66, .cn_DLl = 66436, .cn_DLh = 67335, .f_DLl = 2110},
	{.n = 67, .cn_DLl = 67336, .cn_DLh = 67535, .f_DLl = 738},
	{.n = 68, .cn_DLl = 67536, .cn_DLh = 67835, .f_DLl = 753},
	{.n = 69, .cn_DLl = 67386, .cn_DLh = 68335, .f_DLl = 2570},
	{.n = 70, .cn_DLl = 68336, .cn_DLh = 68585, .f_DLl = 1995},
	{.n = 252, .cn_DLl = 255144, .cn_DLh = 256143, .f_DLl = 5150},
	{.n = 255, .cn_DLl = 260894, .cn_DLh = 262143, .f_DLl = 5725}
};


/* RB Tree holding all request parameters related to RRC measurements trigger.
 */
RB_HEAD(rrc_meas_trigg_tree, rrc_meas_trigg) rrc_meas_t_head =
											RB_INITIALIZER(&rrc_meas_t_head);

RB_PROTOTYPE(
	rrc_meas_trigg_tree,
	rrc_meas_trigg,
	rrc_m_t,
	rrc_meas_comp_trigg);

/* Generate the tree. */
RB_GENERATE(
	rrc_meas_trigg_tree,
	rrc_meas_trigg,
	rrc_m_t,
	rrc_meas_comp_trigg);

/******************************************************************************/

int rrc_meas_comp_trigg (
	struct rrc_meas_trigg * t1,
	struct rrc_meas_trigg * t2) {

	if (t1->t_id > t2->t_id) {
		return 1;
	}
	if (t1->t_id < t2->t_id) {
		return -1;
	}
	return 0;
}

struct rrc_meas_trigg* rrc_meas_get_trigg (uint32_t rnti, int measId) {

	struct rrc_meas_trigg *ctxt = NULL;

	/* Compare with each of the RRC measurement trigger context stored. */
	RB_FOREACH(ctxt, rrc_meas_trigg_tree, &rrc_meas_t_head) {
		if ((ctxt->rnti == rnti) && (ctxt->measId == measId)) {
			return ctxt;
		}
	}
	return NULL;
}

int rrc_meas_rem_trigg (struct rrc_meas_trigg* ctxt) {

	if (ctxt == NULL)
		return -1;

	RB_REMOVE(rrc_meas_trigg_tree, &rrc_meas_t_head, ctxt);
	/* Free the measurement context. */
	if (ctxt) {
		free(ctxt);
	}

	return 0;
}

int rrc_meas_rem_ue_all_trigg (uint32_t rnti) {

	struct rrc_meas_trigg *rrc_meas_ctxt = NULL;

	/* Compare with each of the RRC measurement trigger context stored. */
	RB_FOREACH(rrc_meas_ctxt, rrc_meas_trigg_tree, &rrc_meas_t_head) {
		if (rrc_meas_ctxt->rnti == rnti) {
			/* Remove all RRC measurement triggers for this UE. */
			rrc_meas_rem_trigg(rrc_meas_ctxt);
		}
	}

	return 0;
}

int rrc_meas_add_trigg (struct rrc_meas_trigg* ctxt) {

	if (ctxt == NULL)
		return -1;

	RB_INSERT(rrc_meas_trigg_tree, &rrc_meas_t_head, ctxt);

	return 0;
}

int emoai_trig_rrc_measurements (struct rrc_meas_params * p) {

	int i, m, k;

/****** LOCK ******************************************************************/
	pthread_spin_lock(&rrc_meas_t_lock);

	struct rrc_meas_trigg *ctxt;
	ctxt = rrc_meas_get_trigg(p->rnti, p->meas->measId);

	pthread_spin_unlock(&rrc_meas_t_lock);
/****** UNLOCK ****************************************************************/

	if (ctxt == NULL) {

		emoai_RRC_meas_reconf(p->rnti, -1, p->meas->measId, NULL, NULL);
		/* Free the measurement report received from UE. */
		ASN_STRUCT_FREE(asn_DEF_MeasResults, p->meas);
		/* Free the params. */
		free(p);
		return 0;
	}

	uint32_t b_id = emoai_get_b_id();

	/* Check here whether trigger is registered in agent and then proceed.
	*/
	if (em_has_trigger(b_id, ctxt->t_id, EM_RRC_MEAS_TRIGGER) == 0) {

		emoai_RRC_meas_reconf(p->rnti, -1, p->meas->measId, NULL, NULL);
		/* Trigger does not exist in agent so remove from wrapper as well. */
		if (rrc_meas_rem_trigg(ctxt) < 0) {
			goto error;
		}
	}

	if (p->meas == NULL)
		goto error;
	/* RRC Measurements received from UE. */
	MeasResults_t *meas = p->meas;

	/* Initialize and form the reply message. */
	EmageMsg *reply;
	reply = (EmageMsg *) malloc(sizeof(EmageMsg));
	emage_msg__init(reply);

	Header *header;
	/* Initialize header message.
	 * seq: is currently set to zero but will be updated by the agent.
	*/
	if (emoai_create_header(
			b_id,
			0,
			ctxt->t_id,
			&header) != 0)
		goto error;

	reply->head = header;
	reply->event_types_case = EMAGE_MSG__EVENT_TYPES_TE;

	/* RRC measurement for now supports only triggered event based replies. */
	TriggerEvent *te = malloc(sizeof(TriggerEvent));
	trigger_event__init(te);

	/* Fill the trigger event message. */
	te->events_case = TRIGGER_EVENT__EVENTS_M_RRC_MEAS;
	/* Form the RRC measurements message. */
	RrcMeas *mrrc_meas = malloc(sizeof(RrcMeas));
	rrc_meas__init(mrrc_meas);

	/* Set the type of RRC measurements as reply. */
	mrrc_meas->rrc_meas_m_case = RRC_MEAS__RRC_MEAS_M_REPL;

	/* Initialize RRC measurements reply message. */
	RrcMeasRepl *repl = malloc(sizeof(RrcMeasRepl));
	rrc_meas_repl__init(repl);

	/* Set the RNTI of the UE. */
	repl->rnti = p->rnti;
	/* Set the request status. */
	if (p->reconfig_success == 0) {
		repl->status = STATS_REQ_STATUS__SREQS_FAILURE;
		goto error;
	}
	/* Successful outcome. */
	repl->status = STATS_REQ_STATUS__SREQS_SUCCESS;
	/* Set the measurement ID of measurement. */
	repl->has_measid = 1;
	repl->measid = meas->measId;
	/* Fill the Primary Cell RSRP and RSRQ. */
	repl->has_pcell_rsrp = 1;
	repl->has_pcell_rsrq = 1;
	#ifdef Rel10
		repl->has_pcell_rsrp = 1;
		repl->has_pcell_rsrq = 1;
		repl->pcell_rsrp = RSRP_meas_mapping[meas->
													measResultPCell.rsrpResult];
		repl->pcell_rsrq = RSRQ_meas_mapping[meas->
													measResultPCell.rsrqResult];
	#else
		repl->has_pcell_rsrp = 1;
		repl->has_pcell_rsrq = 1;
		repl->pcell_rsrp = RSRP_meas_mapping[meas->
												measResultServCell.rsrpResult];
		repl->pcell_rsrq = RSRQ_meas_mapping[meas->
												measResultServCell.rsrqResult];
	#endif

	repl->neigh_meas = NULL;

	if (meas->measResultNeighCells != NULL) {
		/*
		* Neighboring cells measurements performed by UE.
		*/
		NeighCellsMeasurements *neigh_meas;
		neigh_meas = malloc(sizeof(NeighCellsMeasurements));
		if (neigh_meas == NULL)
			goto error;
		neigh_cells_measurements__init(neigh_meas);

		/* EUTRAN RRC Measurements. */
		if (meas->measResultNeighCells->present ==
					MeasResults__measResultNeighCells_PR_measResultListEUTRA) {

			MeasResultListEUTRA_t meas_list = meas->measResultNeighCells->
													choice.measResultListEUTRA;
			/* Set the number of EUTRAN measurements present in report. */
			neigh_meas->n_eutra_meas = meas_list.list.count;
			if (neigh_meas->n_eutra_meas > 0) {
				/* Initialize EUTRAN measurements. */
				EUTRAMeasurements **eutra_meas;
				eutra_meas = malloc(sizeof(EUTRAMeasurements *) *
													neigh_meas->n_eutra_meas);
				for (i = 0; i < neigh_meas->n_eutra_meas; i++) {
					eutra_meas[i] = malloc(sizeof(EUTRAMeasurements));
					eutra_measurements__init(eutra_meas[i]);
					/* Fill in the physical cell identifier. */
					eutra_meas[i]->has_phys_cell_id = 1;
					eutra_meas[i]->phys_cell_id = meas_list.list.array[i]->
																	physCellId;
					// EMLOG("PCI of Target %d", eutra_meas[i]->phys_cell_id);
					/* Check for Reference signal measurements. */
					if (&(meas_list.list.array[i]->measResult)) {
						/* Initialize Ref. signal measurements. */
						EUTRARefSignalMeas *meas_result;
						meas_result = malloc(sizeof(EUTRARefSignalMeas));
						eutra_ref_signal_meas__init(meas_result);

						if (meas_list.list.array[i]->measResult.rsrpResult) {
							meas_result->has_rsrp = 1;
							meas_result->rsrp = RSRP_meas_mapping[*(meas_list.
										list.array[i]->measResult.rsrpResult)];
							// EMLOG("RSRP of Target %d", meas_result->rsrp);
						}

						if (meas_list.list.array[i]->measResult.rsrqResult) {
							meas_result->has_rsrq = 1;
							meas_result->rsrq = RSRQ_meas_mapping[*(meas_list.
										list.array[i]->measResult.rsrqResult)];
							// EMLOG("RSRQ of Target %d", meas_result->rsrq);
						}
						eutra_meas[i]->meas_result = meas_result;
					}
					/* Check for CGI measurements. */
					if (meas_list.list.array[i]->cgi_Info) {
						/* Initialize CGI measurements. */
						EUTRACgiMeasurements *cgi_meas;
						cgi_meas = malloc(sizeof(EUTRACgiMeasurements));
						eutra_cgi_measurements__init(cgi_meas);

						/* EUTRA Cell Global Identity (CGI). */
						CellGlobalIdEUTRA *cgi;
						cgi = malloc(sizeof(CellGlobalIdEUTRA));
						cell_global_id__eutra__init(cgi);

						cgi->has_cell_id = 1;
						CellIdentity_t cId = meas_list.list.array[i]->
											cgi_Info->cellGlobalId.cellIdentity;
						cgi->cell_id = (cId.buf[0] << 20) + (cId.buf[1] << 12) +
										(cId.buf[2] << 4) + (cId.buf[3] >> 4);

						/* Public land mobile network identifier of neighbor
						 * cell.
						 */
						PlmnIdentity *plmn_id;
						plmn_id = malloc(sizeof(PlmnIdentity));
						plmn_identity__init(plmn_id);

						MNC_t mnc = meas_list.list.array[i]->
									cgi_Info->cellGlobalId.plmn_Identity.mnc;

						plmn_id->has_mnc = 1;
						plmn_id->mnc = 0;
						for (m = 0; m < mnc.list.count; m++) {
							plmn_id->mnc += *mnc.list.array[m] *
								((uint32_t) pow(10, mnc.list.count - m - 1));
						}

						MCC_t *mcc = meas_list.list.array[i]->
									cgi_Info->cellGlobalId.plmn_Identity.mcc;

						plmn_id->has_mcc = 1;
						plmn_id->mcc = 0;
						for (m = 0; m < mcc->list.count; m++) {
							plmn_id->mcc += *mcc->list.array[m] *
								((uint32_t) pow(10, mcc->list.count - m - 1));
						}

						TrackingAreaCode_t tac = meas_list.list.array[i]->
													cgi_Info->trackingAreaCode;

						cgi_meas->has_tracking_area_code = 1;
						cgi_meas->tracking_area_code = (tac.buf[0] << 8) +
																(tac.buf[1]);

						PLMN_IdentityList2_t *plmn_l = meas_list.list.array[i]->
													cgi_Info->plmn_IdentityList;

						cgi_meas->n_plmn_id = plmn_l->list.count;
						/* Set the PLMN ID list in CGI measurements. */
						PlmnIdentity **plmn_id_l;
						plmn_id_l = malloc(sizeof(PlmnIdentity *) *
														cgi_meas->n_plmn_id);

						MNC_t mnc2;
						MCC_t *mcc2;
						for (m = 0; m < cgi_meas->n_plmn_id; m++) {
							plmn_id_l[m] = malloc(sizeof(PlmnIdentity));
							plmn_identity__init(plmn_id_l[m]);

							mnc2 = plmn_l->list.array[m]->mnc;
							plmn_id_l[m]->has_mnc = 1;
							plmn_id_l[m]->mnc = 0;
							for (k = 0; k < mnc2.list.count; k++) {
								plmn_id_l[m]->mnc += *mnc2.list.array[k] *
								((uint32_t) pow(10, mnc2.list.count - k - 1));
							}

							mcc2 = plmn_l->list.array[m]->mcc;
							plmn_id_l[m]->has_mcc = 1;
							plmn_id_l[m]->mcc = 0;
							for (k = 0; k < mcc2->list.count; k++) {
								plmn_id_l[m]->mcc += *mcc2->list.array[k] *
								((uint32_t) pow(10, mcc2->list.count - k - 1));
							}
						}
						cgi_meas->plmn_id = plmn_id_l;
						eutra_meas[i]->cgi_meas = cgi_meas;
					}
				}
				neigh_meas->eutra_meas = eutra_meas;
			}
		}
		repl->neigh_meas = neigh_meas;
	}
	/* Attach the RRC measurement reply message to RRC measurements message. */
	mrrc_meas->repl = repl;
	/* Attach RRC measurement message to triggered event message. */
	te->mrrc_meas = mrrc_meas;
	te->has_action = 0;
	/* Attach the triggered event message to main message. */
	reply->te = te;

	/* Send the report to controller. */
	if (em_send(b_id, reply) < 0) {
		goto error;
	}

	/* Free the measurement report received from UE. */
	ASN_STRUCT_FREE(asn_DEF_MeasResults, p->meas);
	/* Free the params. */
	free(p);

	return 0;

	error:
		EMLOG("Error triggering RRC measurements message!");
		/* Free the measurement report received from UE. */
		ASN_STRUCT_FREE(asn_DEF_MeasResults, p->meas);
		/* Free the params. */
		free(p);
		return -1;
}

float emoai_get_fdd_band_dl_freq (int band_array_index, uint32_t earfcn) {
	/* Return DL freq. */
	float freq = fdd_bands_dl[band_array_index].f_DLl +
					(0.1 * (earfcn - fdd_bands_dl[band_array_index].cn_DLl));
	freq = roundf(freq * 10.0)/10.0;
	return freq;
}

float emoai_get_tdd_band_freq (int band_array_index, uint32_t earfcn) {
	/* Return TDD band freq. */
	float freq = tdd_bands[band_array_index].f_l +
						(0.1 * (earfcn - tdd_bands[band_array_index].cn_l));
	freq = roundf(freq * 10.0)/10.0;
	return freq;
}

int emoai_get_fdd_dl_band_array_index (uint32_t earfcn) {
	/* Iterate through FDD bands. */
	for (int i = 0; i < 38; i++) {
		if ((earfcn > fdd_bands_dl[i].cn_DLl) &&
				(earfcn < fdd_bands_dl[i].cn_DLh)) {
			return i;
		}
	}
	return -1;
}

int emoai_get_tdd_band_array_index (uint32_t earfcn) {
	/* Iterate through TDD bands. */
	for (int i = 0; i < 14; i++) {
		if ((earfcn > tdd_bands[i].cn_l) && (earfcn < tdd_bands[i].cn_h)) {
			return i;
		}
	}
	return -1;
}

int emoai_get_fdd_band_bw (int band_array_index) {
	/* Return total bandwidth of particular EUTRA FDD band. */
	return f_bands_bw[band_array_index].bw;
}

int emoai_get_tdd_band_bw (int band_array_index) {
	/* Return total bandwidth of particular EUTRA TDD band. */
	return t_bands_bw[band_array_index].bw;
}

int emoai_comp_req_freq (uint32_t earfcnR, uint32_t earfcnC) {

	float freqR = 0.0, freqC = 0.0;
	int tdd_bands_index1, tdd_bands_index2;
	int fdd_bands_indexD1, fdd_bands_indexD2;

	/* Get the frequency corresponding to EARFCN of requests message. */
	tdd_bands_index1 = emoai_get_tdd_band_array_index(earfcnR);
	if (tdd_bands_index1 != -1) {
		freqR = emoai_get_tdd_band_freq(tdd_bands_index1, earfcnR);
	}
	/* Check for FDD downlink frequency. */
	fdd_bands_indexD1 = emoai_get_fdd_dl_band_array_index(earfcnR);
	if (fdd_bands_indexD1 != -1) {
		freqR = emoai_get_fdd_band_dl_freq(fdd_bands_indexD1, earfcnR);
	}

	/* Get the frequency corresponding to EARFCN of trig context message. */
	tdd_bands_index2 = emoai_get_tdd_band_array_index(earfcnC);
	if (tdd_bands_index2 != -1) {
		freqC = emoai_get_tdd_band_freq(tdd_bands_index2, earfcnC);
	}
	/* Check for FDD downlink frequency. */
	fdd_bands_indexD2 = emoai_get_fdd_dl_band_array_index(earfcnC);
	if (fdd_bands_indexD2 != -1) {
		freqC = emoai_get_fdd_band_dl_freq(fdd_bands_indexD2, earfcnC);
	}

	/* Return value meaning:
	 * 0 -> frequencies are not equal
	 * -1 -> invalid request
	 * 1 -> frequencies are equal
	 */

	if ((freqR == freqC) && (earfcnR == earfcnC)) {
		return 1;
	} else if ((freqR == freqC) && (earfcnR != earfcnC)) {
		/* Two MeasObj cannot exist at same physical frequency. */
		return -1;
	}
	return 0;
}

int emoai_comp_EUTRA_measObj (MeasObjEUTRA * req_mo, MeasObjEUTRA * ctxt_mo) {

	/* Return value meaning:
	 * 0 -> measOBjs are not equal
	 * -1 -> invalid measOBj request
	 * 1 -> measOBj exists
	 */
	int ret = 1;

	if ((req_mo == NULL) || (ctxt_mo == NULL)) {
		return -1;
	}

	/* Validate the request frequency parameters since two MeasObj
	 * cannot exist at same physical frequency.
	*/
	int compared_value = emoai_comp_req_freq(req_mo->carrier_freq,
											ctxt_mo->carrier_freq);
	if (compared_value == -1) {
		/* Invalid request parameter. */
		return -1;
	} else {
		ret = compared_value;
	}

	/* Check the array size of cells for which measurements have to be
	 * fetched. Size of the array cannot exceed 32 (3GPP). No modification
	 * features support for now, only replacing with new cells list.
	*/
	// if ((ret == 1) && ((req_mo->n_cells + ctxt_mo->n_cells) > 32)) {
	// 	return -1;
	// }
	/* Check the array size of cells to be blacklisted from measurements.
	 * Size of the array cannot exceed 32 (3GPP). No modification
	 * features support for now, only replacing with new cells list.
	*/
	// if ((ret == 1) && ((req_mo->n_bkl_cells + ctxt_mo->n_bkl_cells) > 32)) {
	// 	return -1;
	// }

	return ret;
}

int rrc_meas_val_trigg_measObj (ueid_t ue_id, MeasObject * m_obj) {

	/* return value meaning:
	 * 0 -> not equal
	 * -1 -> error in request
	 * number > 0 -> measOBj exists and the number represents its ID
	 */

	if (m_obj == NULL)
		return -1;

	struct rrc_meas_trigg *ctxt;
	/* Only EUTRA measurements are supported now. */
	if (m_obj->meas_obj_case == MEAS_OBJECT__MEAS_OBJ_MEAS_OBJ__EUTRA) {
		/* Assign the requested measurement object to a variable. */
		MeasObjEUTRA *measobj_eutra = m_obj->measobj_eutra;
		float max_band_freq = 0.0, min_band_freq = 0.0, freqR = 0.0;
		int band_bw = 0;
		/* RBs to bandwidth frequency (MHz) lookup. */
		float meas_bw_f[6] = {1.4, 3, 5, 10, 15, 20};

		/* Check for the existence of bandwidth to measure in request.
		*/
		if (!measobj_eutra->has_meas_bw) {
			/* MeasObj must have allowed measurement bandwidth parameter. */
			return -1;
		}
		if (!measobj_eutra->has_carrier_freq) {
			/* MeasObj must have carrier frequency. */
			return -1;
		}
		/* Check whether UE supports requested frequency band. */
		int ue_supp_bands_n = emoai_get_num_bands(ue_id);
		uint32_t *ue_supp_bands = emoai_get_bands(ue_id);
		int band_flag = 0;
		int tdd_bands_index, fdd_bands_indexD;
		/* Get the frequency band corresponding to EARFCN of request message. */
		tdd_bands_index = emoai_get_tdd_band_array_index(measobj_eutra->
																carrier_freq);
		fdd_bands_indexD = emoai_get_fdd_dl_band_array_index(measobj_eutra->
																carrier_freq);
		if (tdd_bands_index != -1) {
			freqR = emoai_get_tdd_band_freq(tdd_bands_index, measobj_eutra->
																carrier_freq);
			band_bw = emoai_get_tdd_band_bw(tdd_bands_index);
			max_band_freq = tdd_bands[tdd_bands_index].f_l + band_bw;
			min_band_freq = tdd_bands[tdd_bands_index].f_l;
		} else if (fdd_bands_indexD != -1) {
			freqR = emoai_get_fdd_band_dl_freq(fdd_bands_indexD, measobj_eutra->
																carrier_freq);
			band_bw = emoai_get_fdd_band_bw(fdd_bands_indexD);
			max_band_freq = fdd_bands_dl[fdd_bands_indexD].f_DLl + band_bw;
			min_band_freq = fdd_bands_dl[fdd_bands_indexD].f_DLl;
		} else {
			/* Frequency does not belong to EUTRA bands. */
			return -1;
		}
		for (int i = 0; i < ue_supp_bands_n; i++) {
			if ((tdd_bands_index != -1) &&
					(ue_supp_bands[i] == eutra_tdd_bands[tdd_bands_index])) {
				band_flag = 1;
				break;
			} else if ((fdd_bands_indexD != -1) &&
					(ue_supp_bands[i] == eutra_fdd_bands[fdd_bands_indexD])) {
				band_flag = 1;
				break;
			}
		}
		free(ue_supp_bands);
		if (band_flag == 0) {
			/* Measurement in unsupported band requested. */
			return -1;
		}
		if (meas_bw_f[measobj_eutra->meas_bw] > band_bw) {
			/* Requested measurement bandwidth cannot exceed maximum bandwidth
			 * of the band.
			 */
			return -1;
		} else if (
			((freqR + (meas_bw_f[measobj_eutra->meas_bw] / 2)) > max_band_freq)
										||
			((freqR - (meas_bw_f[measobj_eutra->meas_bw] / 2)) < min_band_freq)
																			) {
			/* Requested measurement frequency range cannot go out of band.
			 */
			return -1;
		}
		/* Check the array size of cells for which measurements have to be
		 * fetched. Size of the array cannot exceed 32 (3GPP).
		*/
		if (measobj_eutra->n_cells > 32) {
			return -1;
		}
		/* Check the array size of cells to be blacklisted from measurements.
		 * Size of the array cannot exceed 32 (3GPP).
		*/
		if (measobj_eutra->n_bkl_cells > 32) {
			return -1;
		}
		/* Compare with each of the measurement object stored. */
		RB_FOREACH(ctxt, rrc_meas_trigg_tree, &rrc_meas_t_head) {
			int measObj_id = 0;
			if (ctxt->m_obj->meas_obj_case ==
										MEAS_OBJECT__MEAS_OBJ_MEAS_OBJ__EUTRA) {
				/* Validate and check whether measObj already exists. */
				measObj_id = emoai_comp_EUTRA_measObj(measobj_eutra,
													ctxt->m_obj->measobj_eutra);
			}

			if (measObj_id > 0) {
				/* Measurement Objects are equal. So return MeasObj ID. */
				return ctxt->m_obj->measobjid;
			} else if (measObj_id == -1) {
				/* Invalid parameters given in request. */
				return -1;
			}
		}
	}
	/* Measurement Objects are not equal. */
	return 0;
}

int emoai_comp_EUTRA_repConf (RepConfEUTRA * req_rc, RepConfEUTRA * ctxt_rc) {

	/* Return value meaning:
	 * 0 -> reportConfig objects are not equal
	 * -1 -> invalid reportConfig request
	 * 1 -> reportConfig objects exists
	 */

	if ((req_rc == NULL) || (ctxt_rc == NULL)) {
		return -1;
	}

	/* Two report configurations are not equal under following conditions. */
	if ((req_rc->has_hysteresis == 1) &&
								(req_rc->hysteresis != ctxt_rc->hysteresis)) {
		return 0;
	}
	if ((req_rc->has_trigg_time == 1) &&
								(req_rc->trigg_time != ctxt_rc->trigg_time)) {
		return 0;
	}
	if ((req_rc->has_trigg_quant == 1) &&
								(req_rc->trigg_quant != ctxt_rc->trigg_quant)) {
		return 0;
	}
	if ((req_rc->has_report_quant == 1) &&
							(req_rc->report_quant != ctxt_rc->report_quant)) {
		return 0;
	}
	if ((req_rc->has_max_rep_cells == 1) &&
							(req_rc->max_rep_cells != ctxt_rc->max_rep_cells)) {
		return 0;
	}
	if ((req_rc->has_rep_interval == 1) &&
							(req_rc->rep_interval != ctxt_rc->rep_interval)) {
		return 0;
	}
	if ((req_rc->has_rep_amount == 1) &&
								(req_rc->rep_amount != ctxt_rc->rep_amount)) {
		return 0;
	}
	if (req_rc->conf__eutra_case != ctxt_rc->conf__eutra_case) {
		return 0;
	}
	if ((req_rc->conf__eutra_case == REP_CONF__EUTRA__CONF__EUTRA_PERIODICAL)
										&&
		(ctxt_rc->conf__eutra_case == REP_CONF__EUTRA__CONF__EUTRA_PERIODICAL)
										&&
		(req_rc->periodical->purpose != ctxt_rc->periodical->purpose)) {
		return 0;
	} else if ((req_rc->conf__eutra_case == REP_CONF__EUTRA__CONF__EUTRA_A1)
										&&
			(ctxt_rc->conf__eutra_case == REP_CONF__EUTRA__CONF__EUTRA_A1)) {

		if ((req_rc->a1->a1_threshold->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRP)
										&&
			(ctxt_rc->a1->a1_threshold->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRP)
										&&
			(req_rc->a1->a1_threshold->rsrp !=
											ctxt_rc->a1->a1_threshold->rsrp)) {
			return 0;
		}
		if ((req_rc->a1->a1_threshold->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRQ)
										&&
			(ctxt_rc->a1->a1_threshold->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRQ)
										&&
			(req_rc->a1->a1_threshold->rsrq !=
											ctxt_rc->a1->a1_threshold->rsrq)) {
			return 0;
		}
	} else if ((req_rc->conf__eutra_case == REP_CONF__EUTRA__CONF__EUTRA_A2)
										&&
			(ctxt_rc->conf__eutra_case == REP_CONF__EUTRA__CONF__EUTRA_A2)) {

		if ((req_rc->a2->a2_threshold->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRP)
										&&
			(ctxt_rc->a2->a2_threshold->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRP)
										&&
			(req_rc->a2->a2_threshold->rsrp !=
											ctxt_rc->a2->a2_threshold->rsrp)) {
			return 0;
		}
		if ((req_rc->a2->a2_threshold->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRQ)
										&&
			(ctxt_rc->a2->a2_threshold->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRQ)
										&&
			(req_rc->a2->a2_threshold->rsrq !=
											ctxt_rc->a2->a2_threshold->rsrq)) {
			return 0;
		}
	} else if ((req_rc->conf__eutra_case == REP_CONF__EUTRA__CONF__EUTRA_A3)
										&&
			(ctxt_rc->conf__eutra_case == REP_CONF__EUTRA__CONF__EUTRA_A3)) {

		if (req_rc->a3->a3_offset != ctxt_rc->a3->a3_offset) {
			return 0;
		}
		if (req_rc->a3->report_on_leave != ctxt_rc->a3->report_on_leave) {
			return 0;
		}
	} else if ((req_rc->conf__eutra_case == REP_CONF__EUTRA__CONF__EUTRA_A4)
										&&
			(ctxt_rc->conf__eutra_case == REP_CONF__EUTRA__CONF__EUTRA_A4)) {

		if ((req_rc->a4->a4_threshold->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRP)
										&&
			(ctxt_rc->a4->a4_threshold->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRP)
										&&
			(req_rc->a4->a4_threshold->rsrp !=
											ctxt_rc->a4->a4_threshold->rsrp)) {
			return 0;
		}
		if ((req_rc->a4->a4_threshold->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRQ)
										&&
			(ctxt_rc->a4->a4_threshold->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRQ)
										&&
			(req_rc->a4->a4_threshold->rsrq !=
											ctxt_rc->a4->a4_threshold->rsrq)) {
			return 0;
		}
	} else if ((req_rc->conf__eutra_case == REP_CONF__EUTRA__CONF__EUTRA_A5)
										&&
			(ctxt_rc->conf__eutra_case == REP_CONF__EUTRA__CONF__EUTRA_A5)) {

		if ((req_rc->a5->a5_threshold1->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRP)
										&&
			(ctxt_rc->a5->a5_threshold1->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRP)
										&&
			(req_rc->a5->a5_threshold1->rsrp !=
											ctxt_rc->a5->a5_threshold1->rsrp)) {
			return 0;
		}
		if ((req_rc->a5->a5_threshold1->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRQ)
										&&
			(ctxt_rc->a5->a5_threshold1->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRQ)
										&&
			(req_rc->a5->a5_threshold1->rsrq !=
											ctxt_rc->a5->a5_threshold1->rsrq)) {
			return 0;
		}
		if ((req_rc->a5->a5_threshold2->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRP)
										&&
			(ctxt_rc->a5->a5_threshold2->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRP)
										&&
			(req_rc->a5->a5_threshold2->rsrp !=
											ctxt_rc->a5->a5_threshold2->rsrp)) {
			return 0;
		}
		if ((req_rc->a5->a5_threshold2->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRQ)
										&&
			(ctxt_rc->a5->a5_threshold2->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRQ)
										&&
			(req_rc->a5->a5_threshold2->rsrq !=
											ctxt_rc->a5->a5_threshold2->rsrq)) {
			return 0;
		}
	}

	return 1;
}

int rrc_meas_val_trigg_repConf (
	ueid_t ue_id,
	uint32_t mo_freq,
	ReportConfig * r_conf) {

	/* return value meaning:
	 * 0 -> not equal
	 * -1 -> error in request
	 * number > 0 -> reportConfig exists and the number represents its ID
	 */

	if (r_conf == NULL)
		return -1;

	struct rrc_meas_trigg *ctxt;
	/* Only EUTRA measurements are supported now. */
	if (r_conf->rep_conf_case == REPORT_CONFIG__REP_CONF_RC__EUTRA) {
		/* Assign the requested report configuration to a variable. */
		RepConfEUTRA *rc_eutra = r_conf->rc_eutra;
		/* CC Id 0 is used for now, need to change this code in case of
		 * multiple CCs.
		 */
		float operating_dl_f = emoai_get_operating_dl_freq(0);
		EMLOG("printing value of operating freq %f", operating_dl_f);
		float req_dl_f = 0.0;
		int tdd_bands_index, fdd_bands_indexD, band_num = 0;
		/* If the request event based measurements are A1 and A2, then request
		 * is valid only in serving cell frequency.
		 */
		if ((rc_eutra->conf__eutra_case == REP_CONF__EUTRA__CONF__EUTRA_A1) ||
			(rc_eutra->conf__eutra_case == REP_CONF__EUTRA__CONF__EUTRA_A2)) {
			/* Get the frequency corresponding to EARFCN of requests message. */
			tdd_bands_index = emoai_get_tdd_band_array_index(mo_freq);
			if (tdd_bands_index != -1) {
				req_dl_f = emoai_get_tdd_band_freq(tdd_bands_index, mo_freq);
				band_num = eutra_tdd_bands[tdd_bands_index];
			}
			/* Check for FDD downlink frequency. */
			fdd_bands_indexD = emoai_get_fdd_dl_band_array_index(mo_freq);
			if (fdd_bands_indexD != -1) {
				req_dl_f = emoai_get_fdd_band_dl_freq(fdd_bands_indexD,
																	mo_freq);
				band_num = eutra_fdd_bands[fdd_bands_indexD];
			}
			/* Given frequency is not DL frequency EARFCN or the req. frequency
			 * is not equal to serving cell DL frequency.
			 */
			if ((req_dl_f == 0.0) || (req_dl_f != operating_dl_f)) {
				return -1;
			}
		}
		/* Check the validity of hysteresis value if it exists. */
		if ((rc_eutra->has_hysteresis == 1) &&
				((rc_eutra->hysteresis > 30) || (rc_eutra->hysteresis < 0))) {
			/* Value of Hysteresis must be in range (0 - 30). */
			return -1;
		}
		/* Check the validity of maximum number of cells to be measured. */
		if ((rc_eutra->has_max_rep_cells == 1) &&
			((rc_eutra->max_rep_cells > 8) || (rc_eutra->max_rep_cells < 0))) {
			/* Value msut not exceed 8. Must be in range (0 - 8). */
			return -1;
		}
		if (((rc_eutra->conf__eutra_case == REP_CONF__EUTRA__CONF__EUTRA_A4) ||
			(rc_eutra->conf__eutra_case == REP_CONF__EUTRA__CONF__EUTRA_A5)) &&
			(emoai_is_A5A4_supp(ue_id) == 0)) {
			/* A4 and A5 event based reporting not supported by UE. */
			return -1;
		}
		/* Total number CGI measurements at any time = 1 only. */
		int num_CGI_meas = 0;
		/* Check UE support for periodic ref. signal and CGI measurements. */
		if ((rc_eutra->conf__eutra_case ==
									REP_CONF__EUTRA__CONF__EUTRA_PERIODICAL)) {
			/* CC Id 0 is used for now, need to change this code in case of
			 * multiple CCs.
			 */
			int op_band = emoai_get_operating_band(0);
			/* Intra-frequency periodic measurements. */
			if ((op_band == band_num) &&
						(emoai_is_intraF_refs_per_meas_supp(ue_id) == 0)) {
				/* UE does not support it. */
				return -1;
			} else if ((op_band != band_num) &&
						(emoai_is_interF_refs_per_meas_supp(ue_id) == 0)) {
				/* Inter-frequency periodic measurements. */
				/* UE does not support it. */
				return -1;
			}
			if (rc_eutra->periodical->purpose ==
								PERIODIC_REP_PURPOSE__PERRP_REPORT_CGI_INFO) {
				num_CGI_meas = 1;
			}
		}
		/* Compare with each of the report configuration object stored. */
		RB_FOREACH(ctxt, rrc_meas_trigg_tree, &rrc_meas_t_head) {
			int repConf_id = 0;
			/* Only EUTRA measurements are supported now. */
			if (ctxt->r_conf->rep_conf_case ==
											REPORT_CONFIG__REP_CONF_RC__EUTRA) {
				/* Validate and check whether reportConfig already exists. */
				repConf_id = emoai_comp_EUTRA_repConf(rc_eutra,
														ctxt->r_conf->rc_eutra);
				/* Check for more than one CGI measurements. */
				if ((rc_eutra->conf__eutra_case ==
										REP_CONF__EUTRA__CONF__EUTRA_PERIODICAL)
											&&
					(rc_eutra->periodical->purpose ==
								PERIODIC_REP_PURPOSE__PERRP_REPORT_CGI_INFO)
											&&
					(num_CGI_meas > 0)) {
					/* Total number CGI measurements at any time = 1 only. */
					return -1;
				}
			}

			if (repConf_id > 0) {
				/* Report Configuration Objects are equal. So return its
				 * reportConfig ID.
				 */
				return ctxt->r_conf->reportconfid;
			} else if (repConf_id == -1) {
				/* Invalid parameters given in request. */
				return -1;
			}
		}
	}

	/* Report Configuration Objects are not equal. */
	return 0;
}

int rrc_meas_trigg_update_measObj (int mo_id, MeasObject * m_obj) {
	/* Get the measurement object stored. */
	struct rrc_meas_trigg *ctxt;
	RB_FOREACH(ctxt, rrc_meas_trigg_tree, &rrc_meas_t_head) {
		if (ctxt->m_obj->measobjid == mo_id) {
			break;
		}
	}
	if (ctxt == NULL) {
		goto error;
	}

	if (m_obj->meas_obj_case == MEAS_OBJECT__MEAS_OBJ_MEAS_OBJ__EUTRA) {
		MeasObjEUTRA *req_mo = m_obj->measobj_eutra;
		MeasObjEUTRA *ctxt_mo = ctxt->m_obj->measobj_eutra;
		ctxt_mo->meas_bw = req_mo->meas_bw;
		/* Remove old version of cells. */
		for (int i = 0; i < ctxt_mo->n_cells; i++) {
			cells_to_measure__free_unpacked(ctxt_mo->cells[i], 0);
		}
		free(ctxt_mo->cells);
		/* Remove old version of blacklist cells. */
		for (int i = 0; i < ctxt_mo->n_bkl_cells; i++) {
			blacklist_cells__free_unpacked(ctxt_mo->bkl_cells[i], 0);
		}
		free(ctxt_mo->bkl_cells);
		/* Replace cells and blacklist cells list with that of request. */
		ctxt_mo->n_cells = req_mo->n_cells;
		if (ctxt_mo->n_cells > 0) {
			ctxt_mo->cells = malloc(sizeof(*req_mo->cells));
			memcpy(ctxt_mo->cells, req_mo->cells, sizeof(*req_mo->cells));
		}
		ctxt_mo->n_bkl_cells = req_mo->n_bkl_cells;
		if (ctxt_mo->n_bkl_cells > 0) {
			ctxt_mo->bkl_cells = malloc(sizeof(*req_mo->bkl_cells));
			memcpy(
				ctxt_mo->bkl_cells,
				req_mo->bkl_cells,
				sizeof(*req_mo->bkl_cells));
		}
	} else {
		goto error;
	}

	return 0;

	error:
		EMLOG("Error in updating measurement object of the trigger context");
		return -1;
}

int emoai_RRC_measurements (
	EmageMsg * request,
	EmageMsg ** reply,
	unsigned int trigger_id) {

	uint32_t ue_id;
	struct rrc_meas_trigg *t_ctxt;
	StatsReqStatus req_status = STATS_REQ_STATUS__SREQS_SUCCESS;
	int repConf_id = 0, measObj_id = 0, meas_id = 0;

	RrcMeasReq *req;
	/* RRC measurement for now supports only triggered event based replies. */
	if (request->event_types_case == EMAGE_MSG__EVENT_TYPES_TE) {
		/* Its a triggered event request. */
		req = request->te->mrrc_meas->req;
	} else {
		return -1;
	}

	/* Form the RRC measurements message. */
	RrcMeas *mrrc_meas = malloc(sizeof(RrcMeas));
	rrc_meas__init(mrrc_meas);
	/* Set the type of RRC measurements as reply. */
	mrrc_meas->rrc_meas_m_case = RRC_MEAS__RRC_MEAS_M_REPL;
	/* Initialize RRC measurements reply message. */
	RrcMeasRepl *repl = malloc(sizeof(RrcMeasRepl));
	rrc_meas_repl__init(repl);
	/* Set the RNTI of the UE. */
	repl->rnti = req->rnti;

	/* Check if UE is still active in the system. */
	ue_id = find_UE_id(DEFAULT_ENB_ID, req->rnti);
	if (ue_id < 0) {
		req_status = STATS_REQ_STATUS__SREQS_FAILURE;
		goto req_error;
	}

	/* Make all the validation of the request message. */
	measObj_id = rrc_meas_val_trigg_measObj(ue_id, req->m_obj);
	if (measObj_id == -1) {
		req_status = STATS_REQ_STATUS__SREQS_FAILURE;
		goto req_error;
	}
	if (req->m_obj->meas_obj_case == MEAS_OBJECT__MEAS_OBJ_MEAS_OBJ__EUTRA) {
		repConf_id = rrc_meas_val_trigg_repConf(
										ue_id,
										req->m_obj->measobj_eutra->carrier_freq,
										req->r_conf);
		if (repConf_id == -1) {
			req_status = STATS_REQ_STATUS__SREQS_FAILURE;
			goto req_error;
		}
	} else {
		/* Not supporting measurements other than EUTRA measurements. */
		req_status = STATS_REQ_STATUS__SREQS_FAILURE;
		goto req_error;
	}

	/* Get the UE context which holds all the measurement configuration info. */
	struct rrc_eNB_ue_context_s* ue = emoai_get_ue_context(ue_id);
	/* Assign the measurement identifiers. */
	if (ue->ue_context.MeasId != NULL) {
		for (int i = 0; i < MAX_MEAS_ID; i++) {
			if (ue->ue_context.MeasId[i] == NULL) {
				meas_id = i + 1;
				break;
			}
		}
	}
	/* At this point no measurement slots are not free. */
	if (meas_id == 0) {
		req_status = STATS_REQ_STATUS__SREQS_FAILURE;
		goto req_error;
	}
	/* Get measurement object id. */
	if (measObj_id > 0) {
		req->m_obj->measobjid = measObj_id;
		/* Update the already existing trigger context.
		 * The update procedure is only for measObj and not for reportConfig
		 * becoz, multiple similar reportConfig can exist, but two measObj with
		 * same EARFCN cannot exist (measObj limitation).
		 */
		if (rrc_meas_trigg_update_measObj(measObj_id, req->m_obj) < 0) {
			req_status = STATS_REQ_STATUS__SREQS_FAILURE;
			goto req_error;
		}
	} else if (measObj_id == 0) {
		if (ue->ue_context.MeasObj != NULL) {
			for (int i = 0; i < MAX_MEAS_OBJ; i++) {
				if (ue->ue_context.MeasObj[i] == NULL) {
					measObj_id = i + 1;
					req->m_obj->measobjid = measObj_id;
					break;
				}
			}
		}
	}
	/* At this point no measurement object slots are free. */
	if (measObj_id == 0) {
		req_status = STATS_REQ_STATUS__SREQS_FAILURE;
		goto req_error;
	}
	/* Get report configuration object id. */
	if (repConf_id > 0) {
		req->r_conf->reportconfid = repConf_id;
	} else if (repConf_id == 0) {
		if (ue->ue_context.ReportConfig != NULL) {
			for (int i = 0; i < MAX_MEAS_CONFIG; i++) {
				if (ue->ue_context.ReportConfig[i] == NULL) {
					repConf_id = i + 1;
					req->r_conf->reportconfid = repConf_id;
					break;
				}
			}
		}
	}
	/* At this point no report configuration object slots are free. */
	if (repConf_id == 0) {
		req_status = STATS_REQ_STATUS__SREQS_FAILURE;
		goto req_error;
	}

	/* Load default values for certain fields in report config. */
	/* Only EUTRA measurements are supported now. */
	if (req->r_conf->rep_conf_case == REPORT_CONFIG__REP_CONF_RC__EUTRA) {
		if (!req->r_conf->rc_eutra->has_hysteresis) {
			/* By default set hysteresis value to 0. */
			req->r_conf->rc_eutra->hysteresis = 0;
		}
		if (!req->r_conf->rc_eutra->has_trigg_quant) {
			/* By default trigger based on RSRP. */
			req->r_conf->rc_eutra->trigg_quant = TRIGGER_QUANTITY__TRIGQ_RSRP;
		}
		if (!req->r_conf->rc_eutra->has_report_quant) {
			/* By default report both RSRP and RSRQ. */
			req->r_conf->rc_eutra->report_quant = REPORT_QUANTITY__REPQ_BOTH;
		}
		if (!req->r_conf->rc_eutra->has_max_rep_cells) {
			/* By default report for only two cells. */
			req->r_conf->rc_eutra->max_rep_cells = 2;
		}
		if (!req->r_conf->rc_eutra->has_rep_interval) {
			/* By default report every 5 secs. */
			req->r_conf->rc_eutra->rep_interval =
												REPORT_INTERVAL__REPINT_ms5120;
		}
		if (!req->r_conf->rc_eutra->has_rep_amount) {
			/* By default report infinite number of measurements. */
			req->r_conf->rc_eutra->rep_amount = REPORT_AMOUNT__REPAMT_infinity;
		}
	} else {
		req_status = STATS_REQ_STATUS__SREQS_FAILURE;
		goto req_error;
	}

	/* CALL RRC Measurement reconfiguration procedure. */
	if (emoai_RRC_meas_reconf(
							req->rnti,
							meas_id,
							-1,
							req->m_obj,
							req->r_conf) < 0) {
		req_status = STATS_REQ_STATUS__SREQS_FAILURE;
		goto req_error;
	}

	/* Add the context to rrc measurement trigger context tree. */
	t_ctxt = malloc(sizeof(struct rrc_meas_trigg));
	t_ctxt->t_id = request->head->t_id;
	t_ctxt->rnti = req->rnti;
	t_ctxt->measId = meas_id;
	t_ctxt->m_obj = malloc(sizeof(*req->m_obj));
	memcpy(t_ctxt->m_obj, req->m_obj, sizeof(*req->m_obj));
	t_ctxt->r_conf = malloc(sizeof(*req->r_conf));
	memcpy(t_ctxt->r_conf, req->r_conf, sizeof(*req->r_conf));
	rrc_meas_add_trigg(t_ctxt);

req_error:
	/* Set the request status. Success or failure. */
	repl->status = req_status;
	repl->has_measid = 1;
	repl->measid = meas_id;
	repl->has_pcell_rsrp = 0;
	repl->has_pcell_rsrq = 0;
	repl->neigh_meas = NULL;

	/* Attach the RRC measurement reply message to RRC measurements message. */
	mrrc_meas->repl = repl;
	/* Form message header. */
	Header *header;
	/* Initialize header message. */
	/* Assign the same transaction id as the request message. */
	if (emoai_create_header(
			request->head->b_id,
			request->head->seq,
			request->head->t_id,
			&header) != 0) {
		return -1;
	}
	/* Form the main Emage message here. */
	EmageMsg *reply_msg = (EmageMsg *) malloc(sizeof(EmageMsg));
	emage_msg__init(reply_msg);
	reply_msg->head = header;
	/* Assign event type same as request message. */
	reply_msg->event_types_case = request->event_types_case;
	/* RRC measurement for now supports only triggered event based replies. */
	if (request->event_types_case == EMAGE_MSG__EVENT_TYPES_TE) {
		/* Its a triggered event reply. */
		TriggerEvent *te = malloc(sizeof(TriggerEvent));
		trigger_event__init(te);

		/* Fill the trigger event message. */
		te->has_action = 0;
		te->events_case = TRIGGER_EVENT__EVENTS_M_RRC_MEAS;
		/* Attach RRC measurement message to triggered event message. */
		te->mrrc_meas = mrrc_meas;
		reply_msg->te = te;
	} else {
		return -1;
	}

	/* Send the response to controller. */
	em_send(emoai_get_b_id(), reply_msg);

	return 0;
}

int emoai_store_rrc_eNB_mui (mui_t * mui) {
	emoai_rrc_eNB_mui = mui;
	return 0;
}

int emoai_RRC_meas_reconf (
	uint32_t rnti,
	int measId_add,
	int measId_rem,
	MeasObject * m_obj,
	ReportConfig * r_conf) {

	int m;
	uint8_t buffer[RRC_BUF_SIZE];
	uint16_t size;
	uint32_t ue_id;

	MeasIdToRemoveList_t *mi_rem_l = NULL;
	MeasObjectToRemoveList_t *mo_rem_l = NULL;
	ReportConfigToRemoveList_t *rc_rem_l = NULL;
	MeasId_t *mi_rem = NULL;
	MeasObjectId_t *mo_rem = NULL;
	ReportConfigId_t *rc_rem = NULL;

	MeasIdToAddModList_t *mi_add_l = NULL;
	MeasObjectToAddModList_t *mo_add_l = NULL;
	ReportConfigToAddModList_t *rc_add_l = NULL;
	MeasIdToAddMod_t *mi_add = NULL;
	MeasObjectToAddMod_t *mo_add = NULL;
	ReportConfigToAddMod_t *rc_add = NULL;

	QuantityConfig_t *quantityConfig = NULL;
	MeasGapConfig_t *measGapConfig = NULL;

	/* Check if UE is still active in the system. */
	ue_id = find_UE_id(DEFAULT_ENB_ID, rnti);
	if (ue_id < 0) {
		goto error;
	}

	/* Get the UE context which holds all the measurement configuration info. */
	struct rrc_eNB_ue_context_s *ue = emoai_get_ue_context(ue_id);

	if (ue == NULL) {
		goto error;
	}

	/* No need to modify the DRB which is already established.
	 * Do not modify DRB0.
	*/
	ue->ue_context.DRB_config_action[0] = CONFIG_ACTION_NULL;

	if (measId_rem > 0) {
		/* A measurement needs to be removed. */
		/* In UE context, all the measurement ids are stored sequentially. */
		if (ue->ue_context.MeasId[measId_rem - 1] == NULL) {
			goto error;
		}
		int measObj_id_rem = ue->ue_context.MeasId[measId_rem - 1]->
																measObjectId;
		int repConf_id_rem = ue->ue_context.MeasId[measId_rem - 1]->
																reportConfigId;
		/* Add this measurement id to the "to be removed" list. */
		mi_rem_l = calloc(1, sizeof(*mi_rem_l));
		mi_rem = calloc(1, sizeof(*mi_rem));
		*mi_rem = measId_rem;
		ASN_SEQUENCE_ADD(&mi_rem_l->list, mi_rem);
		/* Free the measId object in UE context. */
		ASN_STRUCT_FREE(asn_DEF_MeasIdToAddMod,
										ue->ue_context.MeasId[measId_rem - 1]);
		ue->ue_context.MeasId[measId_rem - 1] = NULL;
		/* Check if measObj and repConf corresponding to removed measId is
		 * used by other measurements.
		 */
		int mo_present = 0, rc_present = 0;
		for (int i = 0; i < MAX_MEAS_ID; i++) {
			if (ue->ue_context.MeasId[i] != NULL) {
				if (ue->ue_context.MeasId[i]->measObjectId == measObj_id_rem) {
					++mo_present;
				}
				if (ue->ue_context.MeasId[i]->reportConfigId == repConf_id_rem)
				{
					++rc_present;
				}
			}
		}
		if (mo_present == 0) {
			/* Add this measurement object to be removed list. */
			mo_rem_l = calloc(1, sizeof(*mo_rem_l));
			mo_rem = calloc(1, sizeof(*mo_rem));
			*mo_rem = measObj_id_rem;
			ASN_SEQUENCE_ADD(&mo_rem_l->list, mo_rem);
			/* Free MeasObj in UE context. */
			ASN_STRUCT_FREE(asn_DEF_MeasObjectToAddMod,
									ue->ue_context.MeasObj[measObj_id_rem - 1]);
			ue->ue_context.MeasObj[measObj_id_rem - 1] = NULL;
		}
		if (rc_present == 0) {
			/* Add this report configuration to be removed list. */
			rc_rem_l = calloc(1, sizeof(*rc_rem_l));
			rc_rem = calloc(1, sizeof(*rc_rem));
			*rc_rem = repConf_id_rem;
			ASN_SEQUENCE_ADD(&rc_rem_l->list, rc_rem);
			/* Free reportConfig in UE context. */
			ASN_STRUCT_FREE(asn_DEF_ReportConfigToAddMod,
							ue->ue_context.ReportConfig[repConf_id_rem - 1]);
			ue->ue_context.ReportConfig[repConf_id_rem - 1] = NULL;
		}
	}
	else if (measId_add > 0) {
		/* A measurement needs to be added. */
		if ((m_obj == NULL) || (r_conf == NULL)) {
			goto error;
		}
		/* Add this measurement id to the "to be added" list. */
		mi_add_l = calloc(1, sizeof(*mi_add_l));
		mi_add = calloc(1, sizeof(*mi_add));
		mi_add->measId = measId_add;
		mi_add->measObjectId = m_obj->measobjid;
		mi_add->reportConfigId = r_conf->reportconfid;
		ASN_SEQUENCE_ADD(&mi_add_l->list, mi_add);
		/* Update the UE context with this measurement id objecct. */
		ue->ue_context.MeasId[measId_add - 1] = mi_add;
		/* Measurement Object to add. */
		mo_add_l = calloc(1, sizeof(*mo_add_l));
		mo_add = calloc(1, sizeof(*mo_add));
		mo_add->measObjectId = m_obj->measobjid;
		/* EUTRA measurement Object. */
		if (m_obj->meas_obj_case == MEAS_OBJECT__MEAS_OBJ_MEAS_OBJ__EUTRA) {
			MeasObjectEUTRA_t *measObjectEUTRA = &mo_add->measObject.choice.
															measObjectEUTRA;
			mo_add->measObject.present =
						MeasObjectToAddMod__measObject_PR_measObjectEUTRA;

			measObjectEUTRA->carrierFreq = m_obj->measobj_eutra->carrier_freq;
			measObjectEUTRA->allowedMeasBandwidth = m_obj->measobj_eutra->
																		meas_bw;
			measObjectEUTRA->presenceAntennaPort1 = 1;
			measObjectEUTRA->neighCellConfig.buf = calloc(1, sizeof(uint8_t));
			measObjectEUTRA->neighCellConfig.buf[0] = 0;
			measObjectEUTRA->neighCellConfig.size = 1;
			measObjectEUTRA->neighCellConfig.bits_unused = 6;
			measObjectEUTRA->offsetFreq = NULL;
			if (m_obj->measobj_eutra->n_cells > 0) {
				/* Setting cells to be measured and cells not to be measured. */
				if ((r_conf->rc_eutra->conf__eutra_case ==
									REP_CONF__EUTRA__CONF__EUTRA_PERIODICAL) &&
					(r_conf->rc_eutra->periodical->purpose ==
								PERIODIC_REP_PURPOSE__PERRP_REPORT_CGI_INFO)) {
					/* For CGI measurements only. */
					measObjectEUTRA->cellForWhichToReportCGI =
														calloc(1, sizeof(long));
					*measObjectEUTRA->cellForWhichToReportCGI =
									m_obj->measobj_eutra->cells[0]->phy_cell_id;
				}
				/* Remove exisiting set of cells if they exist. */
				measObjectEUTRA->cellsToRemoveList =
											calloc(1, sizeof(CellIndexList_t));
				CellIndexList_t *cells_rem_l =
											measObjectEUTRA->cellsToRemoveList;
				CellIndex_t *cell_r = NULL;
				for (m = 0; m < 32; m++) {
					cell_r = calloc(1, sizeof(*cell_r));
					*cell_r = m + 1;
					ASN_SEQUENCE_ADD(&cells_rem_l->list, cell_r);
				}
				/* Add the cells whose measurements are needed. */
				measObjectEUTRA->cellsToAddModList =
										calloc(1, sizeof(CellsToAddModList_t));
				CellsToAddModList_t *cells = measObjectEUTRA->cellsToAddModList;
				CellsToAddMod_t *cell = NULL;
				for (m = 0; m < m_obj->measobj_eutra->n_cells; m++) {
					cell = calloc(1, sizeof(*cell));
					cell->cellIndex = m + 1;
					cell->physCellId = m_obj->measobj_eutra->
														cells[m]->phy_cell_id;
					if (m_obj->measobj_eutra->cells[m]->has_offset_range) {
						cell->cellIndividualOffset = m_obj->measobj_eutra->
														cells[m]->offset_range;
					} else {
						cell->cellIndividualOffset = Q_OFFSET_RANGE__QOR_dB0;
					}
					ASN_SEQUENCE_ADD(&cells->list, cell);
				}
			}
			if (m_obj->measobj_eutra->n_bkl_cells > 0) {
				/* Remove exisiting set of blacklist cells if they exist. */
				measObjectEUTRA->blackCellsToRemoveList =
											calloc(1, sizeof(CellIndexList_t));
				CellIndexList_t *bcells_rem_l =
										measObjectEUTRA->blackCellsToRemoveList;
				CellIndex_t *bcell_r = NULL;
				for (m = 0; m < 32; m++) {
					bcell_r = calloc(1, sizeof(*bcell_r));
					*bcell_r = m + 1;
					ASN_SEQUENCE_ADD(&bcells_rem_l->list, bcell_r);
				}
				/* Add the cells whose measurements are not needed. */
				measObjectEUTRA->blackCellsToAddModList =
									calloc(1, sizeof(BlackCellsToAddModList_t));
				BlackCellsToAddModList_t *bcells = measObjectEUTRA->
														blackCellsToAddModList;
				BlackCellsToAddMod_t *bcell = NULL;
				for (m = 0; m < m_obj->measobj_eutra->n_bkl_cells; m++) {
					bcell = calloc(1, sizeof(*bcell));
					bcell->cellIndex = m + 1;
					bcell->physCellIdRange.start = m_obj->measobj_eutra->
														bkl_cells[m]->start_pci;
					bcell->physCellIdRange.range = calloc(1, sizeof(long));
					if (m_obj->measobj_eutra->bkl_cells[m]->has_range) {
						*bcell->physCellIdRange.range = m_obj->measobj_eutra->
															bkl_cells[m]->range;
					} else {
						*bcell->physCellIdRange.range = PCI_RANGE__PCIR_n4;
					}
					ASN_SEQUENCE_ADD(&bcells->list, bcell);
				}
			}

			ASN_SEQUENCE_ADD(&mo_add_l->list, mo_add);
		} else {
			goto error;
		}
		/* Check if measObj already exists, if not add it. */
		if (ue->ue_context.MeasObj[m_obj->measobjid - 1] != NULL) {
			/* Free MeasObj in UE context. */
			ASN_STRUCT_FREE(asn_DEF_MeasObjectToAddMod,
								ue->ue_context.MeasObj[m_obj->measobjid - 1]);
		}
		/* Update the contents of MeasObj in UE context. */
		ue->ue_context.MeasObj[m_obj->measobjid - 1] = mo_add;

		/* Report Configuration Object to add. */
		rc_add_l = calloc(1, sizeof(*rc_add_l));
		rc_add = calloc(1, sizeof(*rc_add));
		rc_add->reportConfigId = r_conf->reportconfid;
		/* EUTRA report configuration Object. */
		if (r_conf->rep_conf_case == REPORT_CONFIG__REP_CONF_RC__EUTRA) {
			ReportConfigEUTRA_t *reportConfigEUTRA = &rc_add->reportConfig.
													choice.reportConfigEUTRA;
			rc_add->reportConfig.present =
						ReportConfigToAddMod__reportConfig_PR_reportConfigEUTRA;
			switch (r_conf->rc_eutra->conf__eutra_case) {
	case REP_CONF__EUTRA__CONF__EUTRA_PERIODICAL:
		reportConfigEUTRA->triggerType.present =
								ReportConfigEUTRA__triggerType_PR_periodical;
		reportConfigEUTRA->triggerType.choice.periodical.purpose =
										r_conf->rc_eutra->periodical->purpose;
		/* Configure to use autonomous gaps if supported by UE. */
		if ((r_conf->rc_eutra->periodical->purpose ==
								PERIODIC_REP_PURPOSE__PERRP_REPORT_CGI_INFO) &&
					emoai_is_intraF_neighCellSIacq_supp(ue_id) &&
					emoai_is_interF_neighCellSIacq_supp(ue_id)) {
			reportConfigEUTRA->ext1 = calloc(1,
											sizeof(*(reportConfigEUTRA->ext1)));
			reportConfigEUTRA->ext1->si_RequestForHO_r9 =
														calloc(1, sizeof(long));
			*reportConfigEUTRA->ext1->si_RequestForHO_r9 =
							ReportConfigEUTRA__ext1__si_RequestForHO_r9_setup;
		}
		break;
	case REP_CONF__EUTRA__CONF__EUTRA_A1:
		reportConfigEUTRA->triggerType.present =
										ReportConfigEUTRA__triggerType_PR_event;
		reportConfigEUTRA->triggerType.choice.event.eventId.present =
					ReportConfigEUTRA__triggerType__event__eventId_PR_eventA1;

		struct ReportConfigEUTRA__triggerType__event__eventId__eventA1 *eA1;
		eA1 = &reportConfigEUTRA->triggerType.choice.event.eventId.choice.
																		eventA1;
		if (r_conf->rc_eutra->a1->a1_threshold->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRP) {
			eA1->a1_Threshold.present = ThresholdEUTRA_PR_threshold_RSRP;
			eA1->a1_Threshold.choice.threshold_RSRP = r_conf->rc_eutra->a1->
															a1_threshold->rsrp;
		} else {
			eA1->a1_Threshold.present = ThresholdEUTRA_PR_threshold_RSRQ;
			eA1->a1_Threshold.choice.threshold_RSRQ = r_conf->rc_eutra->a1->
															a1_threshold->rsrq;
		}
		break;
	case REP_CONF__EUTRA__CONF__EUTRA_A2:
		reportConfigEUTRA->triggerType.present =
										ReportConfigEUTRA__triggerType_PR_event;
		reportConfigEUTRA->triggerType.choice.event.eventId.present =
					ReportConfigEUTRA__triggerType__event__eventId_PR_eventA2;

		struct ReportConfigEUTRA__triggerType__event__eventId__eventA2 *eA2;
		eA2 = &reportConfigEUTRA->triggerType.choice.event.eventId.choice.
																		eventA2;
		if (r_conf->rc_eutra->a2->a2_threshold->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRP) {
			eA2->a2_Threshold.present = ThresholdEUTRA_PR_threshold_RSRP;
			eA2->a2_Threshold.choice.threshold_RSRP = r_conf->rc_eutra->a2->
															a2_threshold->rsrp;
		} else {
			eA2->a2_Threshold.present = ThresholdEUTRA_PR_threshold_RSRQ;
			eA2->a2_Threshold.choice.threshold_RSRQ = r_conf->rc_eutra->a2->
															a2_threshold->rsrq;
		}
		break;
	case REP_CONF__EUTRA__CONF__EUTRA_A3:
		reportConfigEUTRA->triggerType.present =
										ReportConfigEUTRA__triggerType_PR_event;
		reportConfigEUTRA->triggerType.choice.event.eventId.present =
					ReportConfigEUTRA__triggerType__event__eventId_PR_eventA3;

		struct ReportConfigEUTRA__triggerType__event__eventId__eventA3 *eA3;
		eA3 = &reportConfigEUTRA->triggerType.choice.event.eventId.choice.
																		eventA3;
		eA3->a3_Offset = r_conf->rc_eutra->a3->a3_offset;
		if (r_conf->rc_eutra->a3->has_report_on_leave) {
			eA3->reportOnLeave = 1;
		} else {
			eA3->reportOnLeave = 0;
		}
		break;
	case REP_CONF__EUTRA__CONF__EUTRA_A4:
		reportConfigEUTRA->triggerType.present =
										ReportConfigEUTRA__triggerType_PR_event;
		reportConfigEUTRA->triggerType.choice.event.eventId.present =
					ReportConfigEUTRA__triggerType__event__eventId_PR_eventA4;

		struct ReportConfigEUTRA__triggerType__event__eventId__eventA4 *eA4;
		eA4 = &reportConfigEUTRA->triggerType.choice.event.eventId.choice.
																		eventA4;
		if (r_conf->rc_eutra->a4->a4_threshold->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRP) {
			eA4->a4_Threshold.present = ThresholdEUTRA_PR_threshold_RSRP;
			eA4->a4_Threshold.choice.threshold_RSRP = r_conf->rc_eutra->a4->
															a4_threshold->rsrp;
		} else {
			eA4->a4_Threshold.present = ThresholdEUTRA_PR_threshold_RSRQ;
			eA4->a4_Threshold.choice.threshold_RSRQ = r_conf->rc_eutra->a4->
															a4_threshold->rsrq;
		}
		break;
	case REP_CONF__EUTRA__CONF__EUTRA_A5:
		reportConfigEUTRA->triggerType.present =
										ReportConfigEUTRA__triggerType_PR_event;
		reportConfigEUTRA->triggerType.choice.event.eventId.present =
					ReportConfigEUTRA__triggerType__event__eventId_PR_eventA5;

		struct ReportConfigEUTRA__triggerType__event__eventId__eventA5 *eA5;
		eA5 = &reportConfigEUTRA->triggerType.choice.event.eventId.choice.
																		eventA5;
		if (r_conf->rc_eutra->a5->a5_threshold1->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRP) {
			eA5->a5_Threshold1.present = ThresholdEUTRA_PR_threshold_RSRP;
			eA5->a5_Threshold1.choice.threshold_RSRP = r_conf->rc_eutra->a5->
															a5_threshold1->rsrp;
		} else {
			eA5->a5_Threshold1.present = ThresholdEUTRA_PR_threshold_RSRQ;
			eA5->a5_Threshold1.choice.threshold_RSRQ = r_conf->rc_eutra->a5->
															a5_threshold1->rsrq;
		}
		if (r_conf->rc_eutra->a5->a5_threshold2->threshold_case ==
											THRESHOLD__EUTRA__THRESHOLD_RSRP) {
			eA5->a5_Threshold2.present = ThresholdEUTRA_PR_threshold_RSRP;
			eA5->a5_Threshold2.choice.threshold_RSRP = r_conf->rc_eutra->a5->
															a5_threshold2->rsrp;
		} else {
			eA5->a5_Threshold2.present = ThresholdEUTRA_PR_threshold_RSRQ;
			eA5->a5_Threshold2.choice.threshold_RSRQ = r_conf->rc_eutra->a5->
															a5_threshold2->rsrq;
		}
		break;
	default:
		goto error;
		break;
			}

			/* Fill remaining report cofiguration parameters. */
			if (r_conf->rc_eutra->has_hysteresis &&
				(r_conf->rc_eutra->conf__eutra_case !=
									REP_CONF__EUTRA__CONF__EUTRA_PERIODICAL)) {
				reportConfigEUTRA->triggerType.choice.event.hysteresis =
												r_conf->rc_eutra->hysteresis;
			}
			if (r_conf->rc_eutra->has_trigg_time &&
				(r_conf->rc_eutra->conf__eutra_case !=
									REP_CONF__EUTRA__CONF__EUTRA_PERIODICAL)) {
				reportConfigEUTRA->triggerType.choice.event.timeToTrigger =
												r_conf->rc_eutra->trigg_time;
			}
			if (r_conf->rc_eutra->has_trigg_quant) {
				reportConfigEUTRA->triggerQuantity =
												r_conf->rc_eutra->trigg_quant;
			}
			if (r_conf->rc_eutra->has_report_quant) {
				reportConfigEUTRA->reportQuantity =
												r_conf->rc_eutra->report_quant;
			}
			if (r_conf->rc_eutra->has_max_rep_cells) {
				reportConfigEUTRA->maxReportCells =
												r_conf->rc_eutra->max_rep_cells;
			}
			if (r_conf->rc_eutra->has_rep_interval) {
				reportConfigEUTRA->reportInterval =
												r_conf->rc_eutra->rep_interval;
			}
			if (r_conf->rc_eutra->has_rep_amount) {
				reportConfigEUTRA->reportAmount = r_conf->rc_eutra->rep_amount;
			}

			ASN_SEQUENCE_ADD(&rc_add_l->list, rc_add);
			/* Check if reportConfig already exists, if not add it. */
			if (ue->ue_context.ReportConfig[r_conf->reportconfid - 1] != NULL) {
				/* Free ReportConfig in UE context. */
				ASN_STRUCT_FREE(asn_DEF_ReportConfigToAddMod,
						ue->ue_context.ReportConfig[r_conf->reportconfid - 1]);
			}
			/* Update the contents of Report Config in UE context. */
			ue->ue_context.ReportConfig[r_conf->reportconfid - 1] = rc_add;
		} else {
			goto error;
		}
	} else {
		goto error;
	}
	/* Load the measurement gap configuration stored in UE context. */
	measGapConfig = ue->ue_context.measGapConfig;
	/* Load the quantity configuration stored in UE context. */
	quantityConfig = ue->ue_context.QuantityConfig;

	memset(buffer, 0, RRC_BUF_SIZE);

	/* Get current frame and subframe number. */
	eNB_MAC_INST *eNB = &eNB_mac_inst[DEFAULT_ENB_ID];

	struct protocol_ctxt_s emage_eNB_RRC_UE_ctxt;
	memset(&emage_eNB_RRC_UE_ctxt, 0, sizeof(emage_eNB_RRC_UE_ctxt));

	PROTOCOL_CTXT_SET_BY_MODULE_ID(&emage_eNB_RRC_UE_ctxt,
		DEFAULT_ENB_ID,
		ENB_FLAG_YES,
		rnti,
		eNB->frame,
		eNB->subframe,
		DEFAULT_ENB_ID);

	size = do_RRCConnectionReconfiguration(
					&emage_eNB_RRC_UE_ctxt,
					buffer,
					rrc_eNB_get_next_transaction_identifier(
											emage_eNB_RRC_UE_ctxt.module_id),
					(SRB_ToAddModList_t*)NULL,
					(DRB_ToAddModList_t*)NULL,
					(DRB_ToReleaseList_t*)NULL,
					(struct SPS_Config*)NULL,
#ifdef EXMIMO_IOT
					NULL, NULL, NULL, NULL,NULL,
#else
					NULL,
					mo_add_l,
					mo_rem_l,
					(ReportConfigToAddModList_t*)rc_add_l,
					rc_rem_l,
					quantityConfig,
					mi_add_l,
					mi_rem_l,
#endif
					NULL,
					measGapConfig,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL
#ifdef Rel10
					,(SCellToAddMod_r10_t*)NULL
#endif
					);

	MSC_LOG_TX_MESSAGE(
		MSC_RRC_ENB,
		MSC_RRC_UE,
		buffer,
		size,
		MSC_AS_TIME_FMT" rrcConnectionReconfiguration UE %x MUI %d size %u",
		MSC_AS_TIME_ARGS(&emage_eNB_RRC_UE_ctxt),
		ue_context_pP->ue_context.rnti,
		*emoai_rrc_eNB_mui,
		size);

	rrc_data_req(
		&emage_eNB_RRC_UE_ctxt,
		DCCH,
		*emoai_rrc_eNB_mui++,
		SDU_CONFIRM_NO,
		size,
		buffer,
		PDCP_TRANSMISSION_MODE_CONTROL);

	return 0;

error:
	EMLOG("Error in reconfiguration of UE RRC measurements! ");
	return -1;
}




int rrc_meas_req (uint32_t * rnti) {
	/* Initialize the request message. */
	EmageMsg * request = (EmageMsg *) malloc(sizeof(EmageMsg));
	emage_msg__init(request);

	Header *header;
	/* Initialize header message. */
	/* seq field of header is updated in agent. */
	if (emoai_create_header(
			emoai_get_b_id(),
			0,
			0,
			&header) != 0)
		return -1;

	request->head = header;
	request->event_types_case = EMAGE_MSG__EVENT_TYPES_TE;

	TriggerEvent *te = malloc(sizeof(TriggerEvent));
	trigger_event__init(te);

	/* Fill the trigger event message. */
	te->events_case = TRIGGER_EVENT__EVENTS_M_RRC_MEAS;
	te->action = EVENT_ACTION__EA_ADD;
	/* Form the UE RRC measurement message. */
	RrcMeas *mrrc_meas = malloc(sizeof(RrcMeas));
	rrc_meas__init(mrrc_meas);

	mrrc_meas->rrc_meas_m_case = RRC_MEAS__RRC_MEAS_M_REQ;

	/* Form the UE RRC measurement request message. */
	RrcMeasReq *req = malloc(sizeof(RrcMeasReq));
	rrc_meas_req__init(req);
	/* Set the RNTI of the UE. */
	req->rnti = *rnti;
	req->rat = RAT_TYPE__RAT_EUTRA;

	MeasObject *m_obj = malloc(sizeof(*m_obj));
	meas_object__init(m_obj);
	m_obj->meas_obj_case = MEAS_OBJECT__MEAS_OBJ_MEAS_OBJ__EUTRA;
	MeasObjEUTRA *measobj_eutra = malloc(sizeof(*measobj_eutra));
	meas_obj__eutra__init(measobj_eutra);
	measobj_eutra->has_carrier_freq = 1;
	measobj_eutra->carrier_freq = 1850;

	measobj_eutra->has_meas_bw = 1;
	measobj_eutra->meas_bw = ALLOWED_MEAS_BW__AMBW_25;

	measobj_eutra->n_cells = 3;
	CellsToMeasure **cells = NULL;
	if (measobj_eutra->n_cells > 0) {
		cells = malloc(measobj_eutra->n_cells * sizeof(*cells));
		for (int i = 0; i < measobj_eutra->n_cells; i++) {
			cells[i] = malloc(sizeof(**cells));
			cells_to_measure__init(cells[i]);
		}
		cells[0]->has_phy_cell_id = 1;
		cells[0]->phy_cell_id = 60;
		cells[0]->has_offset_range = 1;
		cells[0]->offset_range = Q_OFFSET_RANGE__QOR_dB0;
		cells[1]->has_phy_cell_id = 1;
		cells[1]->phy_cell_id = 198;
		cells[1]->has_offset_range = 1;
		cells[1]->offset_range = Q_OFFSET_RANGE__QOR_dB0;
		cells[2]->has_phy_cell_id = 1;
		cells[2]->phy_cell_id = 15;
		cells[2]->has_offset_range = 1;
		cells[2]->offset_range = Q_OFFSET_RANGE__QOR_dB0;
		measobj_eutra->cells = cells;
	}

	measobj_eutra->n_bkl_cells = 0;
	BlacklistCells **bkl_cells = NULL;
	if (measobj_eutra->n_bkl_cells > 0) {
		bkl_cells = malloc(measobj_eutra->n_bkl_cells * sizeof(*bkl_cells));
		for (int i = 0; i < measobj_eutra->n_bkl_cells; i++) {
			bkl_cells[i] = malloc(sizeof(**bkl_cells));
			blacklist_cells__init(bkl_cells[i]);
		}
		bkl_cells[0]->has_start_pci = 1;
		bkl_cells[0]->start_pci = 206;
		bkl_cells[0]->has_range = 1;
		bkl_cells[0]->range = PCI_RANGE__PCIR_n4;
		measobj_eutra->bkl_cells = bkl_cells;
	}

	m_obj->measobj_eutra = measobj_eutra;
	req->m_obj = m_obj;

	ReportConfig *r_conf = malloc(sizeof(*r_conf));
	report_config__init(r_conf);
	r_conf->rep_conf_case = REPORT_CONFIG__REP_CONF_RC__EUTRA;
	RepConfEUTRA *rc_eutra = malloc(sizeof(*rc_eutra));
	rep_conf__eutra__init(rc_eutra);
	rc_eutra->has_hysteresis = 1;
	rc_eutra->hysteresis = 0;
	rc_eutra->has_trigg_time = 1;
	rc_eutra->trigg_time = TIME_TO_TRIGGER__TTRIG_ms0;
	rc_eutra->has_trigg_quant = 1;
	rc_eutra->trigg_quant = TRIGGER_QUANTITY__TRIGQ_RSRP;
	rc_eutra->has_report_quant = 1;
	rc_eutra->report_quant = REPORT_QUANTITY__REPQ_BOTH;
	rc_eutra->has_max_rep_cells = 1;
	rc_eutra->max_rep_cells = 2;
	rc_eutra->has_rep_interval = 1;
	rc_eutra->rep_interval = REPORT_INTERVAL__REPINT_ms10240;
	rc_eutra->has_rep_amount = 1;
	rc_eutra->rep_amount = REPORT_AMOUNT__REPAMT_infinity;
	rc_eutra->has_ue_rxtx_time_diff = 0;

	rc_eutra->conf__eutra_case = REP_CONF__EUTRA__CONF__EUTRA_PERIODICAL;
	RepConfPer *periodical = malloc(sizeof(*periodical));
	rep_conf_per__init(periodical);
	periodical->has_purpose = 1;
	periodical->purpose = PERIODIC_REP_PURPOSE__PERRP_REPORT_STRONGEST_CELLS;
	// periodical->purpose = PERIODIC_REP_PURPOSE__PERRP_REPORT_CGI_INFO;
	rc_eutra->periodical = periodical;

	// rc_eutra->conf__eutra_case = REP_CONF__EUTRA__CONF__EUTRA_A1;
	// RepConfA1 *a1 = malloc(sizeof(*a1));
	// rep_conf_a1__init(a1);
	// a1->a1_threshold = malloc(sizeof(*a1->a1_threshold));
	// threshold__eutra__init(a1->a1_threshold);
	// a1->a1_threshold->threshold_case = THRESHOLD__EUTRA__THRESHOLD_RSRP;
	// a1->a1_threshold->rsrp = 40;
	// // a1->a1_threshold->threshold_case = THRESHOLD__EUTRA__THRESHOLD_RSRQ;
	// // a1->a1_threshold->rsrq = 0;
	// rc_eutra->a1 = a1;

	// rc_eutra->conf__eutra_case = REP_CONF__EUTRA__CONF__EUTRA_A2;
	// RepConfA2 *a2 = malloc(sizeof(*a2));
	// rep_conf_a2__init(a2);
	// a2->a2_threshold = malloc(sizeof(*a2->a2_threshold));
	// threshold__eutra__init(a2->a2_threshold);
	// a2->a2_threshold->threshold_case = THRESHOLD__EUTRA__THRESHOLD_RSRP;
	// a2->a2_threshold->rsrp = 50;
	// // a2->a2_threshold->threshold_case = THRESHOLD__EUTRA__THRESHOLD_RSRQ;
	// // a2->a2_threshold->rsrq = 0;
	// rc_eutra->a2 = a2;

	// rc_eutra->conf__eutra_case = REP_CONF__EUTRA__CONF__EUTRA_A3;
	// RepConfA3 *a3 = malloc(sizeof(*a3));
	// rep_conf_a3__init(a3);
	// a3->has_a3_offset = 1;
	// a3->a3_offset = 5;
	// a3->has_report_on_leave = 1;
	// a3->report_on_leave = 1;
	// rc_eutra->a3 = a3;

	// rc_eutra->conf__eutra_case = REP_CONF__EUTRA__CONF__EUTRA_A4;
	// RepConfA4 *a4 = malloc(sizeof(*a4));
	// rep_conf_a4__init(a4);
	// a4->a4_threshold = malloc(sizeof(*a4->a4_threshold));
	// threshold__eutra__init(a4->a4_threshold);
	// a4->a4_threshold->threshold_case = THRESHOLD__EUTRA__THRESHOLD_RSRP;
	// a4->a4_threshold->rsrp = 80;
	// // a4->a4_threshold->threshold_case = THRESHOLD__EUTRA__THRESHOLD_RSRQ;
	// // a4->a4_threshold->rsrq = 0;
	// rc_eutra->a4 = a4;

	// rc_eutra->conf__eutra_case = REP_CONF__EUTRA__CONF__EUTRA_A5;
	// RepConfA5 *a5 = malloc(sizeof(*a5));
	// rep_conf_a5__init(a5);
	// a5->a5_threshold1 = malloc(sizeof(*a5->a5_threshold1));
	// a5->a5_threshold2 = malloc(sizeof(*a5->a5_threshold2));
	// threshold__eutra__init(a5->a5_threshold1);
	// threshold__eutra__init(a5->a5_threshold2);
	// a5->a5_threshold1->threshold_case = THRESHOLD__EUTRA__THRESHOLD_RSRP;
	// a5->a5_threshold1->rsrp = 80;
	// // a5->a5_threshold1->threshold_case = THRESHOLD__EUTRA__THRESHOLD_RSRQ;
	// // a5->a5_threshold1->rsrq = 0;
	// a5->a5_threshold2->threshold_case = THRESHOLD__EUTRA__THRESHOLD_RSRP;
	// a5->a5_threshold2->rsrp = 80;
	// // a5->a5_threshold2->threshold_case = THRESHOLD__EUTRA__THRESHOLD_RSRQ;
	// // a5->a5_threshold2->rsrq = 0;
	// rc_eutra->a5 = a5;

	r_conf->rc_eutra = rc_eutra;
	req->r_conf = r_conf;

	mrrc_meas->req = req;
	te->mrrc_meas = mrrc_meas;
	request->te = te;

	EmageMsg * reply = NULL;

	if (emoai_RRC_measurements (request, &reply, 0) < 0) {
		EMLOG("Error in creating supreeth RRC measurements! ");
	}
	return 0;
}