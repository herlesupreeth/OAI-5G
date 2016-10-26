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

/* Holds the transaction id of UEs ID report trigger request.
 * If negative, trigger is not enabled, else holds transaction id of request.
 * At any point in time, only one trigger of type UEs ID report can exist.
*/
int ues_id_trigg_tid = -1;

/* RB Tree holding all request parameters related to RRC measurements
 * configuration trigger.
 */
RB_HEAD(rrc_m_conf_trigg_tree, rrc_m_conf_trigg)
					rrc_m_conf_t_head = RB_INITIALIZER(&rrc_m_conf_t_head);

RB_PROTOTYPE(
	rrc_m_conf_trigg_tree,
	rrc_m_conf_trigg,
	rrc_m_c_t,
	rrc_m_conf_comp_trigg);

/* Generate the tree. */
RB_GENERATE(
	rrc_m_conf_trigg_tree,
	rrc_m_conf_trigg,
	rrc_m_c_t,
	rrc_m_conf_comp_trigg);


int emoai_trig_UEs_ID_report (void) {

	if (ues_id_trigg_tid < 0) {
		/* Trigger does not exist so drop the report. */
		return 0;
	}

	uint32_t b_id = emoai_get_b_id();

	/* Check here whether trigger is registered in agent and then proceed.
	 */
	if (em_has_trigger(b_id, ues_id_trigg_tid, EM_UEs_ID_REPORT_TRIGGER) == 0) {
		/* Trigger does not exist in agent so remove from wrapper as well. */
		ues_id_trigg_tid = -1;
	}

	/* Reply message. */
	EmageMsg *reply;

	/* Initialize the request message. */
	EmageMsg *request = (EmageMsg *) malloc(sizeof(EmageMsg));
	emage_msg__init(request);

	Header *header;
	/* Initialize header message. */
	/* seq field of header is updated in agent. */
	if (emoai_create_header(
			b_id,
			0,
			ues_id_trigg_tid,
			&header) != 0)
		goto error;

	request->head = header;
	request->event_types_case = EMAGE_MSG__EVENT_TYPES_TE;

	TriggerEvent *te = malloc(sizeof(TriggerEvent));
	trigger_event__init(te);

	/* Fill the trigger event message. */
	te->events_case = TRIGGER_EVENT__EVENTS_M_UES_ID;
	/* Form the UEs id message. */
	UesId *mues_id = malloc(sizeof(UesId));
	ues_id__init(mues_id);

	mues_id->ues_id_m_case = UES_ID__UES_ID_M_REQ;
	/* Form the UEs id request message. */
	UesIdReq *req = malloc(sizeof(UesIdReq));
	ues_id_req__init(req);
	req->has_dummy = 1;
	req->dummy = 0;

	mues_id->req = req;
	te->mues_id = mues_id;
	request->te = te;

	if (emoai_UEs_ID_report (request, &reply, 0) < 0) {
		goto error;
	}

	emage_msg__free_unpacked(request, 0);

	/* Send the triggered event reply. */
	if (em_send(b_id, reply) < 0) {
		goto error;
	}

	return 0;

	error:
		EMLOG("Error triggering UEs Id report message!");
		return -1;
}

int emoai_UEs_ID_report (
	EmageMsg * request,
	EmageMsg ** reply,
	unsigned int trigger_id) {

	int i;

	/* Form the UEs id message. */
	UesId *mues_id = (UesId *) malloc(sizeof(UesId));
	ues_id__init(mues_id);

	mues_id->ues_id_m_case = UES_ID__UES_ID_M_REPL;

	/* Form the UEs id reply message. */
	UesIdRepl *repl = (UesIdRepl *) malloc(sizeof(UesIdRepl));
	ues_id_repl__init(repl);

	size_t n_active_ue_id = 0;
	size_t n_inactive_ue_id = 0;
	ActiveUe **active_ue_id = NULL;
	InactiveUe **inactive_ue_id = NULL;

	for (i = 0; i < NUMBER_OF_UE_MAX; i++) {
		if (emoai_get_ue_crnti(i) != NOT_A_RNTI) {
			if (emoai_get_ue_state(i) > RRC_STATE__RS_RRC_INACTIVE) {
				++n_active_ue_id;
				active_ue_id = realloc(active_ue_id, n_active_ue_id *
															sizeof(ActiveUe *));
				active_ue_id[n_active_ue_id - 1] = malloc(sizeof(ActiveUe));
				active_ue__init(active_ue_id[n_active_ue_id - 1]);
				active_ue_id[n_active_ue_id - 1]->rnti = emoai_get_ue_crnti(i);
				active_ue_id[n_active_ue_id - 1]->imsi = emoai_get_ue_imsi(i);
				active_ue_id[n_active_ue_id - 1]->plmn_id =
												emoai_get_selected_plmn_id(i);
			} else {
				++n_inactive_ue_id;
				inactive_ue_id = realloc(inactive_ue_id,
									n_inactive_ue_id * sizeof(InactiveUe *));
				inactive_ue_id[n_inactive_ue_id - 1] =
													malloc(sizeof(InactiveUe));
				inactive_ue__init(inactive_ue_id[n_inactive_ue_id - 1]);
				inactive_ue_id[n_inactive_ue_id - 1]->rnti =
														emoai_get_ue_crnti(i);
				inactive_ue_id[n_inactive_ue_id - 1]->imsi =
														emoai_get_ue_imsi(i);
				inactive_ue_id[n_inactive_ue_id - 1]->plmn_id =
												emoai_get_selected_plmn_id(i);
			}
		}
	}

	/* Successful outcome of request. */
	repl->status = CONF_REQ_STATUS__CREQS_SUCCESS;
	repl->n_active_ue_id = n_active_ue_id;
	repl->active_ue_id = active_ue_id;
	repl->n_inactive_ue_id = n_inactive_ue_id;
	repl->inactive_ue_id = inactive_ue_id;

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
	*reply = (EmageMsg *) malloc(sizeof(EmageMsg));
	emage_msg__init(*reply);
	(*reply)->head = header;
	/* Assign event type same as request message. */
	(*reply)->event_types_case = request->event_types_case;

	/* Attach the UEs id reply message to the main UEs Id message. */
	mues_id->repl = repl;

	if (request->event_types_case == EMAGE_MSG__EVENT_TYPES_TE) {
		/* Its a triggered event reply. */
		TriggerEvent *te = malloc(sizeof(TriggerEvent));
		trigger_event__init(te);

		/* Update the transaction id which tells us whether trigger is active
		 * or not.
		*/
		ues_id_trigg_tid = request->head->t_id;

		/* Fill the trigger event message. */
		te->events_case = TRIGGER_EVENT__EVENTS_M_UES_ID;
		te->mues_id = mues_id;
		(*reply)->te = te;
	} else if (request->event_types_case == EMAGE_MSG__EVENT_TYPES_SCHE) {
		/* Its a scheduled event reply. */
		ScheduleEvent *sche = malloc(sizeof(ScheduleEvent));
		schedule_event__init(sche);

		/* Fill the schedule event message. */
		sche->events_case = SCHEDULE_EVENT__EVENTS_M_UES_ID;
		sche->mues_id = mues_id;
		(*reply)->sche = sche;
	} else if (request->event_types_case == EMAGE_MSG__EVENT_TYPES_SE) {
		/* Its a single event reply. */
		SingleEvent *se = malloc(sizeof(SingleEvent));
		single_event__init(se);

		/* Fill the single event message. */
		se->events_case = SINGLE_EVENT__EVENTS_M_UES_ID;
		se->mues_id = mues_id;
		(*reply)->se = se;
	} else {
		return -1;
	}

	return 0;
}

int emoai_trig_RRC_meas_conf_report (rnti_t * rnti) {

/****** LOCK ******************************************************************/
	pthread_spin_lock(&rrc_m_conf_t_lock);

	struct rrc_m_conf_trigg *ctxt;
	ctxt = rrc_m_conf_get_trigg(*rnti);

	pthread_spin_unlock(&rrc_m_conf_t_lock);
/****** UNLOCK ****************************************************************/

	if (ctxt == NULL) {
		/* Trigger is not enabled for this UE. */
		return 0;
	}

	uint32_t b_id = emoai_get_b_id();

	/* Check here whether trigger is registered in agent and then proceed.
	 */
	if (em_has_trigger(b_id, ctxt->t_id, EM_RRC_MEAS_CONF_TRIGGER) == 0) {
		/* Trigger does not exist in agent so remove from wrapper as well. */
		if (rrc_m_conf_rem_trigg(ctxt) < 0) {
			goto error;
		}
	}

	/* Reply message. */
	EmageMsg *reply;

	/* Initialize the request message. */
	EmageMsg * request = (EmageMsg *) malloc(sizeof(EmageMsg));
	emage_msg__init(request);

	Header *header;
	/* Initialize header message. */
	/* seq field of header is updated in agent. */
	if (emoai_create_header(
			b_id,
			0,
			ctxt->t_id,
			&header) != 0)
		goto error;

	request->head = header;
	request->event_types_case = EMAGE_MSG__EVENT_TYPES_TE;

	TriggerEvent *te = malloc(sizeof(TriggerEvent));
	trigger_event__init(te);

	/* Fill the trigger event message. */
	te->events_case = TRIGGER_EVENT__EVENTS_M_UE_RRC_MEAS_CONF;
	/* Form the UE RRC measurement configuration message. */
	UeRrcMeasConf *mue_rrc_meas_conf = malloc(sizeof(UeRrcMeasConf));
	ue_rrc_meas_conf__init(mue_rrc_meas_conf);

	mue_rrc_meas_conf->ue_rrc_meas_conf_m_case =
									UE_RRC_MEAS_CONF__UE_RRC_MEAS_CONF_M_REQ;

	/* Form the UE RRC measurement configuration request message. */
	UeRrcMeasConfReq *req = malloc(sizeof(UeRrcMeasConfReq));
	ue_rrc_meas_conf_req__init(req);
	/* Set the RNTI of the UE. */
	req->rnti = *rnti;

	mue_rrc_meas_conf->req = req;
	te->mue_rrc_meas_conf = mue_rrc_meas_conf;
	request->te = te;

	if (emoai_RRC_meas_conf_report (request, &reply, 0) < 0) {
		goto error;
	}

	emage_msg__free_unpacked(request, 0);

	/* Send the triggered event reply. */
	if (em_send(b_id, reply) < 0) {
		goto error;
	}

	return 0;

	error:
		EMLOG("Error triggering RRC measurement configuration message!");
		return -1;
}

int emoai_form_EUTRA_meas_obj (MeasObjectEUTRA_t m_obj, MeasObjEUTRA ** m) {
	int i;

	*m = malloc(sizeof(MeasObjEUTRA));
	meas_obj__eutra__init(*m);
	/* Fill the carrier frequency at which measurement to be performed. */
	(*m)->has_carrier_freq = 1;
	(*m)->carrier_freq = m_obj.carrierFreq;
	/* Fill in the allowed bandwidth for the measurement. */
	(*m)->has_meas_bw = 1;
	(*m)->meas_bw = m_obj.allowedMeasBandwidth;
	/* Fill the cells for which measurements must be fetched. */
	if (m_obj.cellsToAddModList != NULL) {
		size_t n_cells = m_obj.cellsToAddModList->list.count;
		CellsToMeasure **cells = malloc(n_cells * sizeof(CellsToMeasure *));

		for (i = 0; i < n_cells; i++) {
			cells[i] = malloc(sizeof(CellsToMeasure));
			cells_to_measure__init(cells[i]);
			/* Fill the physical cell id. */
			cells[i]->has_phy_cell_id = 1;
			cells[i]->phy_cell_id = m_obj.cellsToAddModList->
													list.array[i]->physCellId;
			/* Fill the cell offset range. */
			cells[i]->has_offset_range = 1;
			cells[i]->offset_range =  m_obj.cellsToAddModList->
											list.array[i]->cellIndividualOffset;

		}
		(*m)->n_cells = n_cells;
		(*m)->cells = cells;
	}
	/* Fill the blacklisted cells for which measurements must not be fetched. */
	if (m_obj.blackCellsToAddModList != NULL) {
		size_t n_bkl_cells = m_obj.blackCellsToAddModList->list.count;
		BlacklistCells **bkl_cells =
								malloc(n_bkl_cells * sizeof(BlacklistCells *));

		for (i = 0; i < n_bkl_cells; i++) {
			bkl_cells[i] = malloc(sizeof(BlacklistCells));
			blacklist_cells__init(bkl_cells[i]);
			/* Fill the starting PCI value. */
			bkl_cells[i]->has_start_pci = 1;
			bkl_cells[i]->start_pci = m_obj.blackCellsToAddModList->
										list.array[i]->physCellIdRange.start;
			/* Fill in the range of PCI to blacklist. */
			bkl_cells[i]->has_range = 1;
			bkl_cells[i]->range = *m_obj.blackCellsToAddModList->
										list.array[i]->physCellIdRange.range;
		}
		(*m)->n_bkl_cells = n_bkl_cells;
		(*m)->bkl_cells = bkl_cells;
	}

	return 0;
}

int emoai_form_EUTRA_rep_conf (ReportConfigEUTRA_t r_c, RepConfEUTRA ** r) {

	*r = malloc(sizeof(RepConfEUTRA));
	rep_conf__eutra__init(*r);

	/* Fill the hysteresis value. */
	(*r)->has_hysteresis = 1;
	(*r)->hysteresis = r_c.triggerType.choice.event.hysteresis;
	/* Fill the time to trigger value. */
	(*r)->has_trigg_time = 1;
	(*r)->trigg_time = r_c.triggerType.choice.event.timeToTrigger;
	/* Fill the quantity based on which to trigger. */
	(*r)->has_trigg_quant = 1;
	(*r)->trigg_quant = r_c.triggerQuantity;
	/* Fill the quantity to be reported. */
	(*r)->has_report_quant = 1;
	(*r)->report_quant = r_c.reportQuantity;
	/* Fill the max number of cell's measurements to be reported. */
	(*r)->has_max_rep_cells = 1;
	(*r)->max_rep_cells = r_c.maxReportCells;
	/* Fill the report interval between consecutive reporting. */
	(*r)->has_rep_interval = 1;
	(*r)->rep_interval = r_c.reportInterval;
	/* Fill the amount of reports to send. */
	(*r)->has_rep_amount = 1;
	(*r)->rep_amount = r_c.reportAmount;

	if (r_c.ext1 != NULL && r_c.ext1->ue_RxTxTimeDiffPeriodical_r9) {
		/* Fill whether to report UE Tx Rx time difference. */
		(*r)->has_ue_rxtx_time_diff = 1;
		(*r)->ue_rxtx_time_diff = *r_c.ext1->ue_RxTxTimeDiffPeriodical_r9;
	}

	if (r_c.triggerType.present ==
								ReportConfigEUTRA__triggerType_PR_periodical) {
		/* Periodical measurement event. */
		(*r)->conf__eutra_case = REP_CONF__EUTRA__CONF__EUTRA_PERIODICAL;
		RepConfPer *periodical = malloc(sizeof(RepConfPer));
		rep_conf_per__init(periodical);
		/* Fill the purpose of periodical measurements. */
		periodical->has_purpose = 1;
		periodical->purpose = r_c.triggerType.choice.periodical.purpose;
		(*r)->periodical = periodical;
	} else if (r_c.triggerType.present ==
									ReportConfigEUTRA__triggerType_PR_event) {
		/* List of event type measurements. */
		switch (r_c.triggerType.choice.event.eventId.present) {
		/* A1 event. */
		case ReportConfigEUTRA__triggerType__event__eventId_PR_eventA1:
			/* Set A1 as event type. */
			(*r)->conf__eutra_case = REP_CONF__EUTRA__CONF__EUTRA_A1;
			RepConfA1 *a1 =  malloc(sizeof(RepConfA1));
			rep_conf_a1__init(a1);

			struct ReportConfigEUTRA__triggerType__event__eventId__eventA1 *eA1;
			eA1 = &r_c.triggerType.choice.event.eventId.choice.eventA1;

			/* Fill the threshold parameter for triggering A1 event. */
			ThresholdEUTRA *a1_threshold = malloc(sizeof(ThresholdEUTRA));
			threshold__eutra__init(a1_threshold);
			if (eA1->a1_Threshold.present == ThresholdEUTRA_PR_threshold_RSRP) {
				/* RSRP is the parameter used for triggering. */
				a1_threshold->threshold_case = THRESHOLD__EUTRA__THRESHOLD_RSRP;
				a1_threshold->rsrp = eA1->a1_Threshold.choice.threshold_RSRP;
			} else {
				/* RSRQ is the parameter used for triggering. */
				a1_threshold->threshold_case = THRESHOLD__EUTRA__THRESHOLD_RSRQ;
				a1_threshold->rsrq = eA1->a1_Threshold.choice.threshold_RSRQ;
			}
			a1->a1_threshold = a1_threshold;
			(*r)->a1 = a1;
			break;
		/* A2 event. */
		case ReportConfigEUTRA__triggerType__event__eventId_PR_eventA2:
			/* Set A2 as event type. */
			(*r)->conf__eutra_case = REP_CONF__EUTRA__CONF__EUTRA_A2;
			RepConfA2 *a2 =  malloc(sizeof(RepConfA2));
			rep_conf_a2__init(a2);

			struct ReportConfigEUTRA__triggerType__event__eventId__eventA2 *eA2;
			eA2 = &r_c.triggerType.choice.event.eventId.choice.eventA2;

			/* Fill the threshold parameter for triggering A2 event. */
			ThresholdEUTRA *a2_threshold = malloc(sizeof(ThresholdEUTRA));
			threshold__eutra__init(a2_threshold);
			if (eA2->a2_Threshold.present == ThresholdEUTRA_PR_threshold_RSRP) {
				/* RSRP is the parameter used for triggering. */
				a2_threshold->threshold_case = THRESHOLD__EUTRA__THRESHOLD_RSRP;
				a2_threshold->rsrp = eA2->a2_Threshold.choice.threshold_RSRP;
			} else {
				/* RSRQ is the parameter used for triggering. */
				a2_threshold->threshold_case = THRESHOLD__EUTRA__THRESHOLD_RSRQ;
				a2_threshold->rsrq = eA2->a2_Threshold.choice.threshold_RSRQ;
			}
			a2->a2_threshold = a2_threshold;
			(*r)->a2 = a2;
			break;
		/* A3 event. */
		case ReportConfigEUTRA__triggerType__event__eventId_PR_eventA3:
			/* Set A3 as event type. */
			(*r)->conf__eutra_case = REP_CONF__EUTRA__CONF__EUTRA_A3;
			RepConfA3 *a3 =  malloc(sizeof(RepConfA3));
			rep_conf_a3__init(a3);

			struct ReportConfigEUTRA__triggerType__event__eventId__eventA3 *eA3;
			eA3 = &r_c.triggerType.choice.event.eventId.choice.eventA3;

			/* Fill the offset parameter for triggering A3 event. */
			a3->has_a3_offset = 1;
			a3->a3_offset = eA3->a3_Offset;
			/* Fill flag for reporting when leave condition of event is met. */
			a3->has_report_on_leave = 1;
			a3->report_on_leave = eA3->reportOnLeave;
			(*r)->a3 = a3;
			break;
		/* A4 event. */
		case ReportConfigEUTRA__triggerType__event__eventId_PR_eventA4:
			/* Set A4 as event type. */
			(*r)->conf__eutra_case = REP_CONF__EUTRA__CONF__EUTRA_A4;
			RepConfA4 *a4 =  malloc(sizeof(RepConfA4));
			rep_conf_a4__init(a4);

			struct ReportConfigEUTRA__triggerType__event__eventId__eventA4 *eA4;
			eA4 = &r_c.triggerType.choice.event.eventId.choice.eventA4;

			/* Fill the threshold parameter for triggering A4 event. */
			ThresholdEUTRA *a4_threshold = malloc(sizeof(ThresholdEUTRA));
			threshold__eutra__init(a4_threshold);
			if (eA4->a4_Threshold.present == ThresholdEUTRA_PR_threshold_RSRP) {
				/* RSRP is the parameter used for triggering. */
				a4_threshold->threshold_case = THRESHOLD__EUTRA__THRESHOLD_RSRP;
				a4_threshold->rsrp = eA4->a4_Threshold.choice.threshold_RSRP;
			} else {
				/* RSRQ is the parameter used for triggering. */
				a4_threshold->threshold_case = THRESHOLD__EUTRA__THRESHOLD_RSRQ;
				a4_threshold->rsrq = eA4->a4_Threshold.choice.threshold_RSRQ;
			}
			a4->a4_threshold = a4_threshold;
			(*r)->a4 = a4;
			break;
		/* A5 event. */
		case ReportConfigEUTRA__triggerType__event__eventId_PR_eventA5:
			/* Set A5 as event type. */
			(*r)->conf__eutra_case = REP_CONF__EUTRA__CONF__EUTRA_A5;
			RepConfA5 *a5 =  malloc(sizeof(RepConfA5));
			rep_conf_a5__init(a5);

			struct ReportConfigEUTRA__triggerType__event__eventId__eventA5 *eA5;
			eA5 = &r_c.triggerType.choice.event.eventId.choice.eventA5;

			/* Fill the threshold 1 parameter for triggering A4 event. */
			ThresholdEUTRA *a5_threshold1 = malloc(sizeof(ThresholdEUTRA));
			threshold__eutra__init(a5_threshold1);
			if (eA5->a5_Threshold1.present ==
											ThresholdEUTRA_PR_threshold_RSRP) {
				/* RSRP is the parameter used for triggering. */
				a5_threshold1->threshold_case =
											THRESHOLD__EUTRA__THRESHOLD_RSRP;
				a5_threshold1->rsrp = eA5->a5_Threshold1.choice.threshold_RSRP;
			} else {
				/* RSRQ is the parameter used for triggering. */
				a5_threshold1->threshold_case =
											THRESHOLD__EUTRA__THRESHOLD_RSRQ;
				a5_threshold1->rsrq = eA5->a5_Threshold1.choice.threshold_RSRQ;
			}

			/* Fill the threshold 2 parameter for triggering A4 event. */
			ThresholdEUTRA *a5_threshold2 = malloc(sizeof(ThresholdEUTRA));
			threshold__eutra__init(a5_threshold2);
			if (eA5->a5_Threshold2.present ==
											ThresholdEUTRA_PR_threshold_RSRP) {
				/* RSRP is the parameter used for triggering. */
				a5_threshold2->threshold_case =
											THRESHOLD__EUTRA__THRESHOLD_RSRP;
				a5_threshold2->rsrp = eA5->a5_Threshold2.choice.threshold_RSRP;
			} else {
				/* RSRQ is the parameter used for triggering. */
				a5_threshold2->threshold_case =
											THRESHOLD__EUTRA__THRESHOLD_RSRQ;
				a5_threshold2->rsrq = eA5->a5_Threshold2.choice.threshold_RSRQ;
			}
			a5->a5_threshold1 = a5_threshold1;
			a5->a5_threshold2 = a5_threshold2;
			(*r)->a5 = a5;
			break;
		default:
			goto error;
			break;
		}
	} else {
		goto error;
	}

	return 0;

	error:
		EMLOG("Error forming EUTRA report configuration object message!");
		return -1;
}

int emoai_RRC_meas_conf_report (
	EmageMsg * request,
	EmageMsg ** reply,
	unsigned int trigger_id) {

	int i;
	uint32_t ue_id;
	struct rrc_m_conf_trigg *ctxt = NULL;
	ConfReqStatus req_status = CONF_REQ_STATUS__CREQS_SUCCESS;

	UeRrcMeasConfReq *req;

	if (request->event_types_case == EMAGE_MSG__EVENT_TYPES_TE) {
		/* Its a triggered event request. */
		req = request->te->mue_rrc_meas_conf->req;
		/****** LOCK **********************************************************/
		pthread_spin_lock(&rrc_m_conf_t_lock);

		ctxt = rrc_m_conf_get_trigg(req->rnti);

		pthread_spin_unlock(&rrc_m_conf_t_lock);
		/****** UNLOCK ********************************************************/

	} else if (request->event_types_case == EMAGE_MSG__EVENT_TYPES_SCHE) {
		/* Its a scheduled event request. */
		req = request->sche->mue_rrc_meas_conf->req;
	} else if (request->event_types_case == EMAGE_MSG__EVENT_TYPES_SE) {
		/* Its a single event request. */
		req = request->se->mue_rrc_meas_conf->req;
	} else {
		return -1;
	}

	/* Form the UE RRC measurement configuration message. */
	UeRrcMeasConf *mue_rrc_meas_conf = malloc(sizeof(UeRrcMeasConf));
	ue_rrc_meas_conf__init(mue_rrc_meas_conf);

	mue_rrc_meas_conf->ue_rrc_meas_conf_m_case =
									UE_RRC_MEAS_CONF__UE_RRC_MEAS_CONF_M_REPL;

	/* Form the UE RRC measurement configuration reply message. */
	UeRrcMeasConfRepl *repl = malloc(sizeof(UeRrcMeasConfRepl));
	ue_rrc_meas_conf_repl__init(repl);

	/* Fill the RNTI. */
	repl->rnti = req->rnti;

	/* Check if UE is still active in the system. */
	ue_id = find_UE_id(DEFAULT_ENB_ID, req->rnti);
	if (ue_id < 0) {
		if (ctxt != NULL) {
			/* UE no longer exists so remove its trigger. */
			rrc_m_conf_rem_trigg(ctxt);
		}
		/* Failed outcome of request. */
		req_status = CONF_REQ_STATUS__CREQS_FAILURE;
		goto req_error;
	}

	/* Set the RRC state of the UE. */
	repl->has_ue_rrc_state = 1;
	repl->ue_rrc_state = emoai_get_ue_state(ue_id);

	/* Set the measurement gap configuration pattern. */
	MeasGapPattern mg_p = emoai_get_meas_gap_config(ue_id);
	if (mg_p >= 0 && mg_p < 3){
		repl->meas_gap_patt = mg_p;
		repl->has_meas_gap_patt = 1;
	}
	/* Set the measurement gap offset if applicable. */
	if (repl->has_meas_gap_patt == 1 &&
		repl->meas_gap_patt != MEAS_GAP_PATTERN__MGP_OFF) {
		repl->meas_gap_config_sf_offset =
								emoai_get_meas_gap_config_offset(ue_id);
		repl->has_meas_gap_config_sf_offset = 1;
	}

	/* Set the UE capabilities. */
	UeCapabilities *capabilities;
	capabilities = malloc(sizeof(UeCapabilities));
	ue_capabilities__init(capabilities);
	/* Set the LTE bands supported by UE. */
	uint32_t num_bands = emoai_get_num_bands(ue_id);
	uint32_t* bands = emoai_get_bands(ue_id);
	if (bands != NULL) {
		capabilities->n_band = num_bands;
		capabilities->band = bands;
	}
	int ret_val;
	ret_val = emoai_get_access_release_vers(ue_id);
	if (ret_val != -1){
		capabilities->has_release_3gpp = 1;
		capabilities->release_3gpp = ret_val;
	}
	ret_val = emoai_is_interF_neighCellSIacq_supp(ue_id);
	if (ret_val != -1){
		capabilities->has_interfreq_si_acq = 1;
		capabilities->interfreq_si_acq = ret_val;
	}
	ret_val = emoai_is_intraF_neighCellSIacq_supp(ue_id);
	if (ret_val != -1){
		capabilities->has_intrafreq_si_acq = 1;
		capabilities->intrafreq_si_acq = ret_val;
	}
	ret_val = emoai_is_A5A4_supp(ue_id);
	if (ret_val != -1){
		capabilities->has_a5_a4_events = 1;
		capabilities->a5_a4_events = ret_val;
	}
	ret_val = emoai_is_interF_meas_supp(ue_id);
	if (ret_val != -1){
		capabilities->has_interfreq_meas = 1;
		capabilities->interfreq_meas = ret_val;
	}
	ret_val = emoai_is_intraF_refs_per_meas_supp(ue_id);
	if (ret_val != -1){
		capabilities->has_intrafreq_ref_per_meas = 1;
		capabilities->intrafreq_ref_per_meas = ret_val;
	}
	ret_val = emoai_is_interF_refs_per_meas_supp(ue_id);
	if (ret_val != -1){
		capabilities->has_interfreq_ref_per_meas = 1;
		capabilities->interfreq_ref_per_meas = ret_val;
	}

	repl->capabilities = capabilities;

	/* Get the UE context which holds all the measurement configuration info. */
	struct rrc_eNB_ue_context_s* ue = emoai_get_ue_context(ue_id);

	/* Fill the measurement object configuration. */
	size_t n_m_obj = 0;
	MeasObject **m_obj = NULL;
	if (ue && ue->ue_context.MeasObj != NULL) {
		for (i = 0; i < MAX_MEAS_OBJ; i++) {
			if (ue->ue_context.MeasObj[i] == NULL) {
				continue;
			}
			++n_m_obj;
			m_obj = (MeasObject **) realloc(m_obj, n_m_obj *
														sizeof(MeasObject *));

			m_obj[n_m_obj - 1] = malloc(sizeof(MeasObject));
			meas_object__init(m_obj[n_m_obj - 1]);

			m_obj[n_m_obj - 1]->measobjid = ue->ue_context.MeasObj[i]->
																measObjectId;

			if (ue->ue_context.MeasObj[i]->measObject.present ==
							MeasObjectToAddMod__measObject_PR_measObjectEUTRA) {
				/* Set the type of Measurement object. */
				m_obj[n_m_obj - 1]->meas_obj_case =
										MEAS_OBJECT__MEAS_OBJ_MEAS_OBJ__EUTRA;
				/* Fill in the EUTRA measurement object. */
				if (emoai_form_EUTRA_meas_obj(
						ue->ue_context.MeasObj[i]->measObject.choice.
								measObjectEUTRA,
								&(m_obj[n_m_obj - 1]->measobj_eutra)) < 0) {
					req_status = CONF_REQ_STATUS__CREQS_FAILURE;
					goto req_error;
				}
			} else {
				/* Only EUTRA measurements are supported now. */
				req_status = CONF_REQ_STATUS__CREQS_FAILURE;
				goto req_error;
			}
		}
		repl->n_m_obj = n_m_obj;
		repl->m_obj = m_obj;
	}

	/* Fill the report configuration object. */
	size_t n_r_conf = 0;
	ReportConfig **r_conf = NULL;
	if (ue && ue->ue_context.ReportConfig != NULL) {
		for (i = 0; i < MAX_MEAS_CONFIG; i++) {
			if (ue->ue_context.ReportConfig[i] == NULL) {
				continue;
			}
			++n_r_conf;
			r_conf = (ReportConfig **) realloc(r_conf, n_r_conf *
														sizeof(ReportConfig *));

			r_conf[n_r_conf - 1] = malloc(sizeof(ReportConfig));
			report_config__init(r_conf[n_r_conf - 1]);

			r_conf[n_r_conf - 1]->reportconfid = ue->ue_context.ReportConfig[i]
															->reportConfigId;

			if (ue->ue_context.ReportConfig[i]->reportConfig.present ==
					ReportConfigToAddMod__reportConfig_PR_reportConfigEUTRA) {
				/* Set the type of Report Configuration. */
				r_conf[n_r_conf - 1]->rep_conf_case =
											REPORT_CONFIG__REP_CONF_RC__EUTRA;
				/* Fill in the EUTRA Report Configuration object. */
				if (emoai_form_EUTRA_rep_conf(
						ue->ue_context.ReportConfig[i]->reportConfig.choice.
									reportConfigEUTRA,
									&(r_conf[n_r_conf - 1]->rc_eutra)) < 0) {
					req_status = CONF_REQ_STATUS__CREQS_FAILURE;
					goto req_error;
				}
			} else {
				/* Only EUTRA measurements are supported now. */
				req_status = CONF_REQ_STATUS__CREQS_FAILURE;
				goto req_error;
			}
		}
		repl->n_r_conf = n_r_conf;
		repl->r_conf = r_conf;
	}

	/* Fill the measurement id object. */
	size_t n_meas_id = 0;
	MeasIdentifier **meas_id = NULL;
	if (ue && ue->ue_context.MeasId != NULL) {
		for (i=0; i < MAX_MEAS_ID; i++) {
			if (ue->ue_context.MeasId[i] == NULL) {
				continue;
			}
			++n_meas_id;
			meas_id = (MeasIdentifier **) realloc(meas_id, n_meas_id *
													sizeof(MeasIdentifier *));

			meas_id[n_meas_id - 1] = malloc(sizeof(MeasIdentifier));
			meas_identifier__init(meas_id[n_meas_id - 1]);
			/* Fill the measurement id. */
			meas_id[n_meas_id - 1]->id = ue->ue_context.MeasId[i]->measId;
			/* Fill the measurement object id. */
			meas_id[n_meas_id - 1]->measobj_id = ue->ue_context.MeasId[i]->
																measObjectId;
			/* Fill the report configuration id. */
			meas_id[n_meas_id - 1]->report_conf_id = ue->ue_context.MeasId[i]->
																reportConfigId;
		}
		repl->n_meas_id = n_meas_id;
		repl->meas_id = meas_id;
	}

	repl->has_freq = 1;
	/* Fetching operating DL frequency of eNB on CC ID 0. */
	repl->freq = emoai_get_operating_dl_freq(0);

	repl->has_pcell_dd = 1;
	/* Fetching operating duplexing mode of eNB on CC ID 0. */
	repl->pcell_dd = emoai_get_eNB_dupl_mode(0);

req_error:
	/* Set the status of request message. Success or failure. */
	repl->status = req_status;

	/* Attach the measurement configuration reply message to
	 * the main measurement configuration message.
	 */
	mue_rrc_meas_conf->repl = repl;

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
	*reply = (EmageMsg *) malloc(sizeof(EmageMsg));
	emage_msg__init(*reply);
	(*reply)->head = header;
	/* Assign event type same as request message. */
	(*reply)->event_types_case = request->event_types_case;

	if (request->event_types_case == EMAGE_MSG__EVENT_TYPES_TE) {
		/* Its a triggered event reply. */
		TriggerEvent *te = malloc(sizeof(TriggerEvent));
		trigger_event__init(te);

		/* Fill the trigger event message. */
		te->events_case = TRIGGER_EVENT__EVENTS_M_UE_RRC_MEAS_CONF;
		te->mue_rrc_meas_conf = mue_rrc_meas_conf;
		(*reply)->te = te;

		/* If trigger context does not exist add the context.
		 * Check for triggered reply or request event to add trigger.
		*/
		if (ctxt == NULL) {
			ctxt = malloc(sizeof(struct rrc_m_conf_trigg));
			ctxt->t_id = request->head->t_id;
			ctxt->rnti = req->rnti;
			rrc_m_conf_add_trigg(ctxt);
		}
	} else if (request->event_types_case == EMAGE_MSG__EVENT_TYPES_SCHE) {
		/* Its a scheduled event reply. */
		ScheduleEvent *sche = malloc(sizeof(ScheduleEvent));
		schedule_event__init(sche);

		/* Fill the schedule event message. */
		sche->events_case = SCHEDULE_EVENT__EVENTS_M_UE_RRC_MEAS_CONF;
		sche->mue_rrc_meas_conf = mue_rrc_meas_conf;
		(*reply)->sche = sche;
	} else if (request->event_types_case == EMAGE_MSG__EVENT_TYPES_SE) {
		/* Its a single event reply. */
		SingleEvent *se = malloc(sizeof(SingleEvent));
		single_event__init(se);

		/* Fill the single event message. */
		se->events_case = SINGLE_EVENT__EVENTS_M_UE_RRC_MEAS_CONF;
		se->mue_rrc_meas_conf = mue_rrc_meas_conf;
		(*reply)->se = se;
	} else {
		return -1;
	}

	return 0;
}

int rrc_m_conf_comp_trigg (
	struct rrc_m_conf_trigg* t1,
	struct rrc_m_conf_trigg* t2) {

	if (t1->t_id > t2->t_id) {
		return 1;
	}
	if (t1->t_id < t2->t_id) {
		return -1;
	}
	return 0;
}

struct rrc_m_conf_trigg* rrc_m_conf_get_trigg (uint32_t rnti) {

	struct rrc_m_conf_trigg ctxt;
	memset(&ctxt, 0, sizeof(struct rrc_m_conf_trigg));
	ctxt.rnti = rnti;
	return RB_FIND(rrc_m_conf_trigg_tree, &rrc_m_conf_t_head, &ctxt);
}

int rrc_m_conf_rem_trigg (struct rrc_m_conf_trigg* ctxt) {

	if (ctxt == NULL)
		return -1;

	RB_REMOVE(rrc_m_conf_trigg_tree, &rrc_m_conf_t_head, ctxt);
	/* Free the measurement context. */
	if (ctxt) {
		free(ctxt);
	}

	return 0;
}

int rrc_m_conf_add_trigg (struct rrc_m_conf_trigg* ctxt) {

	if (ctxt == NULL)
		return -1;

	RB_INSERT(rrc_m_conf_trigg_tree, &rrc_m_conf_t_head, ctxt);

	return 0;
}