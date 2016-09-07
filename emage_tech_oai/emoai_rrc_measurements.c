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

/* Lookup for UL of each of EUTRA FDD bands.
 */
struct fdd_bands_ul_i fdd_bands_ul[38] = {
	{.n = 1, .cn_ULl = 18000, .cn_ULh = 18599, .f_ULl = 1920},
	{.n = 2, .cn_ULl = 18600, .cn_ULh = 19199, .f_ULl = 1850},
	{.n = 3, .cn_ULl = 19200, .cn_ULh = 19949, .f_ULl = 1710},
	{.n = 4, .cn_ULl = 19950, .cn_ULh = 20399, .f_ULl = 1710},
	{.n = 5, .cn_ULl = 20400, .cn_ULh = 20649, .f_ULl = 824},
	{.n = 6, .cn_ULl = 20650, .cn_ULh = 20749, .f_ULl = 830},
	{.n = 7, .cn_ULl = 20750, .cn_ULh = 21449, .f_ULl = 2500},
	{.n = 8, .cn_ULl = 21450, .cn_ULh = 21799, .f_ULl = 880},
	{.n = 9, .cn_ULl = 21800, .cn_ULh = 22149, .f_ULl = 1749.9},
	{.n = 10, .cn_ULl = 22150, .cn_ULh = 22749, .f_ULl = 1710},
	{.n = 11, .cn_ULl = 22750, .cn_ULh = 22949, .f_ULl = 1427.9},
	{.n = 12, .cn_ULl = 23010, .cn_ULh = 23179, .f_ULl = 699},
	{.n = 13, .cn_ULl = 23180, .cn_ULh = 23279, .f_ULl = 777},
	{.n = 14, .cn_ULl = 23280, .cn_ULh = 23379, .f_ULl = 788},
	{.n = 17, .cn_ULl = 23730, .cn_ULh = 23849, .f_ULl = 704},
	{.n = 18, .cn_ULl = 23850, .cn_ULh = 23999, .f_ULl = 815},
	{.n = 19, .cn_ULl = 24000, .cn_ULh = 24149, .f_ULl = 830},
	{.n = 20, .cn_ULl = 24150, .cn_ULh = 24449, .f_ULl = 832},
	{.n = 21, .cn_ULl = 24450, .cn_ULh = 24599, .f_ULl = 1447.9},
	{.n = 22, .cn_ULl = 24600, .cn_ULh = 25399, .f_ULl = 3410},
	{.n = 23, .cn_ULl = 25500, .cn_ULh = 25699, .f_ULl = 2000},
	{.n = 24, .cn_ULl = 25700, .cn_ULh = 26039, .f_ULl = 1626.5},
	{.n = 25, .cn_ULl = 26040, .cn_ULh = 26689, .f_ULl = 1850},
	{.n = 26, .cn_ULl = 26690, .cn_ULh = 27039, .f_ULl = 814},
	{.n = 27, .cn_ULl = 27040, .cn_ULh = 27209, .f_ULl = 807},
	{.n = 28, .cn_ULl = 27210, .cn_ULh = 27659, .f_ULl = 703},
	{.n = 29, .cn_ULl = 0, .cn_ULh = 0, .f_ULl = 0},
	{.n = 30, .cn_ULl = 27660, .cn_ULh = 27759, .f_ULl = 2305},
	{.n = 31, .cn_ULl = 27760, .cn_ULh = 27809, .f_ULl = 452.5},
	{.n = 32, .cn_ULl = 0, .cn_ULh = 0, .f_ULl = 0},
	{.n = 65, .cn_ULl = 131072, .cn_ULh = 131971, .f_ULl = 1920},
	{.n = 66, .cn_ULl = 131972, .cn_ULh = 132671, .f_ULl = 1710},
	{.n = 67, .cn_ULl = 0, .cn_ULh = 0, .f_ULl = 0},
	{.n = 68, .cn_ULl = 132672, .cn_ULh = 132971, .f_ULl = 698},
	{.n = 69, .cn_ULl = 0, .cn_ULh = 0, .f_ULl = 0},
	{.n = 70, .cn_ULl = 132972, .cn_ULh = 133121, .f_ULl = 1695},
	{.n = 252, .cn_ULl = 0, .cn_ULh = 0, .f_ULl = 0},
	{.n = 255, .cn_ULl = 0, .cn_ULh = 0, .f_ULl = 0}
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
	struct rrc_meas_trigg* t1,
	struct rrc_meas_trigg* t2) {

	if (t1->t_id > t2->t_id) {
		return 1;
	}
	if (t1->t_id < t2->t_id) {
		return -1;
	}
	return 0;
}

struct rrc_meas_trigg* rrc_meas_get_trigg (uint32_t rnti, int measId) {

	struct rrc_meas_trigg ctxt;
	memset(&ctxt, 0, sizeof(struct rrc_meas_trigg));
	ctxt.rnti = rnti;
	ctxt.measId = measId;
	return RB_FIND(rrc_meas_trigg_tree, &rrc_meas_t_head, &ctxt);
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
		/* Free the measurement report received from UE. */
		ASN_STRUCT_FREE(asn_DEF_MeasResults, p->meas);
		/* Free the params. */
		free(p);
		return 0;
	}

	/* Check here whether trigger is registered in agent and then proceed.
	 * If the trigger is not enabled remove the trigger context.
	*/

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
			emoai_get_b_id(),
			0,
			ctxt->t_id,
			&header) != 0)
		goto error;

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
		repl->pcell_rsrp = RSRP_meas_mapping[meas->
													measResultPCell.rsrpResult];
		repl->pcell_rsrq = RSRQ_meas_mapping[meas->
													measResultPCell.rsrqResult];
	#else
		repl->pcell_rsrp = RSRP_meas_mapping[meas->
												measResultServCell.rsrpResult];
		repl->pcell_rsrq = RSRQ_meas_mapping[meas->
												measResultServCell.rsrqResult];
	#endif

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
					EMLOG("PCI of Target %d", eutra_meas[i]->phys_cell_id);
					/* Check for Reference signal measurements. */
					if (&(meas_list.list.array[i]->measResult)) {
						/* Initialize Ref. signal measurements. */
						EUTRARefSignalMeas *meas_result;
						meas_result = malloc(sizeof(EUTRARefSignalMeas));
						eutra_ref_signal_meas__init(meas_result);

						meas_result->has_rsrp = 1;
						meas_result->rsrp = RSRP_meas_mapping[*(meas_list.
										list.array[i]->measResult.rsrpResult)];
						EMLOG("RSRP of Target %d", meas_result->rsrp);

						meas_result->has_rsrq = 1;
						meas_result->rsrq = RSRQ_meas_mapping[*(meas_list.
										list.array[i]->measResult.rsrqResult)];
						EMLOG("RSRQ of Target %d", meas_result->rsrq);

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
	/* Attach the triggered event message to main message. */
	reply->te = te;

	/* Send the report to controller. */
	em_send(emoai_get_b_id(), reply);

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
	return (fdd_bands_dl[band_array_index].f_DLl +
				(0.1 * (earfcn - fdd_bands_dl[band_array_index].cn_DLl)));
}

float emoai_get_fdd_band_ul_freq (int band_array_index, uint32_t earfcn) {
	/* Return UL freq. */
	return (fdd_bands_ul[band_array_index].f_ULl +
				(0.1 * (earfcn - fdd_bands_ul[band_array_index].cn_ULl)));
}

float emoai_get_tdd_band_freq (int band_array_index, uint32_t earfcn) {
	/* Return TDD band freq. */
	return (tdd_bands[band_array_index].f_l +
				(0.1 * (earfcn - tdd_bands[band_array_index].cn_l)));
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

int emoai_get_fdd_ul_band_array_index (uint32_t earfcn) {
	/* Iterate through FDD bands. */
	for (int i = 0; i < 38; i++) {
		if ((earfcn > fdd_bands_ul[i].cn_ULl) &&
				(earfcn < fdd_bands_ul[i].cn_ULh)) {
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

// int emoai_comp_EUTRA_measObj (MeasObjEUTRA * req_mo, MeasObjEUTRA * ctxt_mo) {

// 	if (req_mo == NULL || ctxt_mo == NULL) {
// 		return 0;
// 	}

// 	if (req_mo->has_carrier_freq && ctxt_mo->has_carrier_freq) {



// 	}

// 	(*m)->has_meas_bw = 1;
// 	(*m)->meas_bw = m_obj.allowedMeasBandwidth;
// 	/* Fill the cells for which measurements must be fetched. */
// 	if (m_obj.cellsToAddModList != NULL) {
// 		size_t n_cells = m_obj.cellsToAddModList->list.count;
// 		CellsToMeasure **cells = malloc(n_cells * sizeof(CellsToMeasure *));

// 		for (i = 0; i < n_cells; i++) {
// 			cells[i] = malloc(sizeof(CellsToMeasure));
// 			cells_to_measure__init(cells[i]);
// 			/* Fill the physical cell id. */
// 			cells[i]->has_phy_cell_id = 1;
// 			cells[i]->phy_cell_id = m_obj.cellsToAddModList->
// 													list.array[i]->physCellId;
// 			/* Fill the cell offset range. */
// 			cells[i]->has_offset_range = 1;
// 			cells[i]->offset_range =  m_obj.cellsToAddModList->
// 											list.array[i]->cellIndividualOffset;

// 		}
// 		(*m)->n_cells = n_cells;
// 		(*m)->cells = cells;
// 	}
// 	/* Fill the blacklisted cells for which measurements must not be fetched. */
// 	if (m_obj.blackCellsToAddModList != NULL) {
// 		size_t n_bkl_cells = m_obj.blackCellsToAddModList->list.count;
// 		BlacklistCells **bkl_cells =
// 								malloc(n_bkl_cells * sizeof(BlacklistCells *));

// 		for (i = 0; i < n_bkl_cells; i++) {
// 			bkl_cells[i] = malloc(sizeof(BlacklistCells));
// 			blacklist_cells__init(bkl_cells[i]);
// 			/* Fill the starting PCI value. */
// 			bkl_cells[i]->has_start_pci = 1;
// 			bkl_cells[i]->start_pci = m_obj.blackCellsToAddModList->
// 										list.array[i]->physCellIdRange.start;
// 			/* Fill in the range of PCI to blacklist. */
// 			bkl_cells[i]->has_range = 1;
// 			bkl_cells[i]->range = *m_obj.blackCellsToAddModList->
// 										list.array[i]->physCellIdRange.range;
// 		}
// 		(*m)->n_bkl_cells = n_bkl_cells;
// 		(*m)->bkl_cells = bkl_cells;
// 	}

// }

// int rrc_meas_comp_trigg_measObj (MeasObject *m_obj) {

// 	if (m_obj == NULL)
// 		return -1;

// 	struct rrc_meas_trigg ctxt;
// 	memset(&ctxt, 0, sizeof(struct rrc_meas_trigg));
// 	/* Only EUTRA measurements are supported now. */
// 	if (m_obj->meas_obj_case == MEAS_OBJECT__MEAS_OBJ_MEAS_OBJ__EUTRA) {
// 		/* Compare with each of the measurement object stored. */
// 		RB_FOREACH(ctxt, rrc_meas_trigg_tree, &rrc_meas_t_head) {
// 			int flag = 1;

// 			if (ctxt->m_obj->meas_obj_case ==
// 										MEAS_OBJECT__MEAS_OBJ_MEAS_OBJ__EUTRA) {
// 				flag = emoai_comp_EUTRA_measObj(m_obj->measobj_eutra,
// 												ctxt->m_obj->measobj_eutra);
// 			}
// 			if (flag == 1) {
// 				/* Measurement Objects are equal. */
// 				return 1;
// 			}
// 		}
// 	}
// 	/* Measurement Objects are not equal. */
// 	return 0;
// }

// int emoai_RRC_measurements (EmageMsg * request, EmageMsg ** reply) {

// 	int i;
// 	uint32_t ue_id;
// 	struct rrc_meas_trigg *ctxt;
// 	StatsReqStatus req_status = STATS_REQ_STATUS__SREQS_SUCCESS;

// 	RrcMeasReq *req;

// 	/* RRC measurement for now supports only triggered event based replies. */
// 	if (request->event_types_case == EMAGE_MSG__EVENT_TYPES_TE) {
// 		/* Its a triggered event request. */
// 		req = request->te->mrrc_meas->req;
// 	} else {
// 		goto error;
// 	}

// 	/* Check if UE is still active in the system. */
// 	ue_id = find_UE_id(DEFAULT_ENB_ID, req->rnti);
// 	if (ue_id < 0) {
// 		goto req_error;
// 	}

// 	/* Make all the validation here before forming the header. */

// 	Header *header;
// 	/* Initialize header message. */
// 	/* Assign the same transaction id as the request message. */
// 	if (emoai_create_header(
// 			request->head->b_id,
// 			request->head->seq,
// 			request->head->t_id,
// 			&header) != 0)
// 		goto error;

// 	/* Form the main Emage message here. */
// 	*reply = (EmageMsg *) malloc(sizeof(EmageMsg));
// 	emage_msg__init(*reply);
// 	(*reply)->head = header;
// 	/* Assign event type same as request message. */
// 	(*reply)->event_types_case = request->event_types_case;


// 	/* Form the UE RRC measurement configuration message. */
// 	UeRrcMeasConf *mue_rrc_meas_conf = malloc(sizeof(UeRrcMeasConf));
// 	ue_rrc_meas_conf__init(mue_rrc_meas_conf);

// 	mue_rrc_meas_conf->ue_rrc_meas_conf_m_case =
// 									UE_RRC_MEAS_CONF__UE_RRC_MEAS_CONF_M_REPL;

// 	/* Form the UE RRC measurement configuration reply message. */
// 	UeRrcMeasConfRepl *repl = malloc(sizeof(UeRrcMeasConfRepl));
// 	ue_rrc_meas_conf_repl__init(repl);


// 	/* Fill the RNTI. */
// 	repl->rnti = req->rnti;

// 	/* Add the context at the last. */
// 	ctxt = malloc(sizeof(struct rrc_meas_trigg));
// 	ctxt->t_id = request->head->t_id;
// 	ctxt->rnti = req->rnti;
// 	rrc_m_conf_add_trigg(ctxt);

// 	req_error:
// 		/* Failed outcome of request. */
// 		req_status = CONF_REQ_STATUS__CREQS_FAILURE;
// 		return 0;

// }