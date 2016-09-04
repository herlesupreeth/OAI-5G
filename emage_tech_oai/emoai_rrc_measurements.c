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

int emoai_send_rrc_measurements (struct rrc_meas_params * p) {

	int i, m, k;

	/* Initialize and form the reply message. */
	EmageMsg *reply;
	reply = (EmageMsg *) malloc(sizeof(EmageMsg));
	if(reply == NULL)
		goto error;
	emage_msg__init(reply);

	Header *header;
	/* Initialize header message.
	 * t_id: is set to measId since its a triggered message from OAI.
	 * seq: is currently set to zero but will be updated by the agent.
	*/
	if (emoai_create_header(
			p->b_id,
			p->m_id,
			0,
			p->t_id,
			MSG_TYPE__STATS_REP,
			&header) != 0)
		goto error;

	reply->head = header;
	reply->message_case = EMAGE_MSG__MESSAGE_M_STATS;

	/* Initialize the statistics message. */
	Statistics *stats = (Statistics *) malloc(sizeof(Statistics));
	if (stats == NULL)
		goto error;
	statistics__init(stats);
	/* Filling the statistics message. */
	stats->type = STATS_MSG_TYPE__RRC_MEASUREMENTS_REPLY;
	stats->has_type = 1;
	stats->stats_msg_case = STATISTICS__STATS_MSG_RRC_MEAS_REPL;

	if (p->meas == NULL)
		goto error;
	/* RRC Measurements received from UE. */
	MeasResults_t meas = p->meas;

	/*
	 * Reply message with RRC measurements.
	*/
	RrcMeasurementsReply *rrc_meas_repl;
	rrc_meas_repl = malloc(sizeof(RrcMeasurementsReply));
	if (rrc_meas_repl == NULL)
		goto error;
	rrc_measurements_reply__init(rrc_meas_repl);

	rrc_meas_repl->has_rnti = 1;
	rrc_meas_repl->rnti = p->rnti;
	if (p->reconfig_success == 0) {
		rrc_meas_repl->has_reconfig_flag = 1;
		rrc_meas_repl->reconfig_flag = RECONF_STATUS__RECOS_FAILURE;
	} else {
		rrc_meas_repl->has_reconfig_flag = 1;
		rrc_meas_repl->reconfig_flag = RECONF_STATUS__RECOS_SUCCESS;
	}
	rrc_meas_repl->has_measid = 1;
	rrc_meas_repl->measid = meas->measId;
	rrc_meas_repl->has_pcell_rsrp = 1;
	rrc_meas_repl->has_pcell_rsrq = 1;
	#ifdef Rel10
		rrc_meas_repl->pcell_rsrp = RSRP_meas_mapping[meas->
													measResultPCell.rsrpResult];
		rrc_meas_repl->pcell_rsrq = RSRQ_meas_mapping[meas->
													measResultPCell.rsrqResult];
	#else
		rrc_meas_repl->pcell_rsrp = RSRP_meas_mapping[meas->
												measResultServCell.rsrpResult];
		rrc_meas_repl->pcell_rsrq = RSRQ_meas_mapping[meas->
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
					if (eutra_meas[i] == NULL)
						goto error;
					eutra_measurements__init(eutra_meas[i]);
					/* Fill in the physical cell identifier. */
					eutra_meas[i]->has_phys_cell_id = 1;
					eutra_meas[i]->phys_cell_id = meas_list.list.array[i]->
																	physCellId;
					/* Check for Reference signal measurements. */
					if (&(meas_list.list.array[i]->measResult)) {
						/* Initialize Ref. signal measurements. */
						EUTRARefSignalMeas *meas_result;
						meas_result = malloc(sizeof(EUTRARefSignalMeas));
						if (meas_result == NULL)
							goto error;
						eutra_ref_signal_meas__init(meas_result);

						meas_result->has_rsrp = 1;
						meas_result->rsrp = RSRP_meas_mapping[*(meas_list.
										list.array[i]->measResult.rsrpResult)];
						EMLOG("RSRP of Target %d\n", meas_result->rsrp);

						meas_result->has_rsrq = 1;
						meas_result->rsrq = RSRP_meas_mapping[*(meas_list.
										list.array[i]->measResult.rsrqResult)];
						EMLOG("RSRQ of Target %d\n", meas_result->rsrp);

						eutra_meas[i]->meas_result = meas_result;
					}
					/* Check for CGI measurements. */
					if (meas_list.list.array[i]->cgi_Info) {
						/* Initialize CGI measurements. */
						EUTRACgiMeasurements *cgi_meas;
						cgi_meas = malloc(sizeof(EUTRACgiMeasurements));
						if (cgi_meas == NULL)
							goto error;
						eutra_cgi_measurements__init(cgi_meas);

						/* EUTRA Cell Global Identity (CGI). */
						CellGlobalIdEUTRA *cgi;
						cgi = malloc(sizeof(CellGlobalIdEUTRA));
						if (cgi == NULL)
							goto error;
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
							plmn_id->mnc += mnc.list.array[m] *
								((uint32_t) pow(10, mnc.list.count - m - 1));
						}

						MCC_t mcc = meas_list.list.array[i]->
									cgi_Info->cellGlobalId.plmn_Identity.mcc;

						plmn_id->has_mcc = 1;
						plmn_id->mcc = 0;
						for (m = 0; m < mcc.list.count; m++) {
							plmn_id->mcc += mcc.list.array[m] *
								((uint32_t) pow(10, mcc.list.count - m - 1));
						}

						TrackingAreaCode_t tac = meas_list.list.array[i]->
													cgi_Info->trackingAreaCode;

						cgi_meas->has_tracking_area_code = 1;
						cgi_meas->tracking_area_code = (tac.buf[0] << 8) +
																(tac.buf[1]);

						PLMN_IdentityList2_t plmn_l = meas_list.list.array[i]->
													cgi_Info->plmn_IdentityList;

						cgi_meas->n_plmn_id = plmn_l->list.count;
						/* Set the PLMN ID list in CGI measurements. */
						PlmnIdentity **plmn_id_l;
						plmn_id_l = malloc(sizeof(PlmnIdentity *) *
														cgi_meas->n_plmn_id);

						MNC_t mnc2;
						MCC_t mcc2;
						for (m = 0; m < cgi_meas->n_plmn_id; m++) {
							plmn_id_l[m] = malloc(sizeof(PlmnIdentity));
							plmn_identity__init(plmn_id_l[m]);

							mnc2 = plmn_l->list.array[m]->mnc;
							plmn_id_l[m]->has_mnc = 1;
							plmn_id_l[m]->mnc = 0;
							for (k = 0; k < mnc2.list.count; k++) {
								plmn_id_l[m]->mnc += mnc2.list.array[k] *
								((uint32_t) pow(10, mnc2.list.count - k - 1));
							}

							mcc2 = plmn_l->list.array[m]->mcc;
							plmn_id_l[m]->has_mcc = 1;
							plmn_id_l[m]->mcc = 0;
							for (k = 0; k < mcc2.list.count; k++) {
								plmn_id_l[m]->mcc += mcc2.list.array[k] *
								((uint32_t) pow(10, mcc2.list.count - k - 1));
							}
						}
						cgi_meas->plmn_id = plmn_id_l;
						eutra_meas[i]->cgi_meas = cgi_meas;
					}
				}
				neigh_meas->eutra_meas = eutra_meas;
			}
		}
		rrc_meas_repl->neigh_meas = neigh_meas;
	}
	stats->rrc_meas_repl = rrc_meas_repl;
	reply->mstats = stats;

	if (em_send(p->b_id, reply) < 0)
		goto error;

	return 0;

	error:
		return -1;
}