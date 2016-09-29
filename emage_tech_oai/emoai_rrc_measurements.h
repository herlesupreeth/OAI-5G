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

/* OAI RRC measurements related technology abstractions for emage.
 *
 * This element forms the rrc measurements reply, which are sent
 * to the controller.
 *
 */

#ifndef __EMOAI_RRC_MEASUREMENTS_H
#define __EMOAI_RRC_MEASUREMENTS_H

#include <math.h>
#include "emoai_common.h"

#include "MeasResults.h"
#include "MeasObjectEUTRA.h"
#include "ReportConfigEUTRA.h"

/* Defines the parameters used to send RRC measurements reply message to
 * controller whenever the UE sends RRC measurements to OAI.
 */
struct rrc_meas_params {
	/* RNTI of the UE. */
	uint32_t rnti;
	/* Flag to indicate whether measurements reconfig was success or failure. */
	int reconfig_success;
	/* RRC Measurements received from UE. */
	MeasResults_t *meas;
};

/* Trigger to send RRC measurements to controller. Triggered when a UE sends
 * measurement report to the network.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_trig_rrc_measurements (struct rrc_meas_params * p);

/* Request parameters related to RRC measurements trigger.
 */
struct rrc_meas_trigg {
	/* Tree related data */
	RB_ENTRY(rrc_meas_trigg) rrc_m_t;
	/* Transaction identifier. */
	tid_t t_id;
	/* RNTI of the UE. */
	uint32_t rnti;
	/* Measurement Identifier. */
	int measId;
	/* Measurement object configuration of RRC Measurement request message. */
	MeasObject *m_obj;
	/* Report configuration of RRC Measurement request message. */
	ReportConfig *r_conf;
	/* Store the UE context here, just in case reconfig fails. */
};

/* Compares two triggers based on UE rnti and MeasId.
 */
int rrc_meas_comp_trigg (
	struct rrc_meas_trigg * t1,
	struct rrc_meas_trigg * t2);

/* Fetches RRC measurement trigger context based on UE rnti and MeasId.
 */
struct rrc_meas_trigg* rrc_meas_get_trigg (uint32_t rnti, int measId);

/* Removes RRC measurement trigger context from tree.
 */
int rrc_meas_rem_trigg (struct rrc_meas_trigg* ctxt);

/* Removes all the RRC measurement trigger contexts from tree for a particular
 * UE.
 */
int rrc_meas_rem_ue_all_trigg (uint32_t rnti);

/* Insert RRC measurement triggger context into tree.
 */
int rrc_meas_add_trigg (struct rrc_meas_trigg* ctxt);

/* Holds all the information about Bandwidth of each of EUTRA TDD bands.
 */
struct tdd_bands_bw {
	/* Band number of the EUTRA band. */
	int n;
	/* Total avaiable bandwidth in this band (MHz). */
	int bw;
};

/* Holds all the information about each of EUTRA TDD bands.
 */
struct tdd_bands_i {
	/* Band number of the EUTRA band. */
	int n;
	/* EARFCN lowest value for this band. */
	uint32_t cn_l;
	/* EARFCN highest value for this band. */
	uint32_t cn_h;
	/* Lowest frequency for this band (MHz). */
	float f_l;
};

/* Holds all the information about Bandwidth of each of EUTRA FDD bands.
 */
struct fdd_bands_bw {
	/* Band number of the EUTRA band. */
	int n;
	/* Total avaiable bandwidth in this band (MHz). */
	int bw;
};

/* Holds all the information about DL of each of EUTRA FDD bands.
 */
struct fdd_bands_dl_i {
	/* Band number of the EUTRA band. */
	int n;
	/* EARFCN lowest value in DL for this band. */
	uint32_t cn_DLl;
	/* EARFCN highest value in DL for this band. */
	uint32_t cn_DLh;
	/* Lowest frequency in DL for this band (MHz). */
	float f_DLl;
};

/* Get DL frequency from EARFCN number for FDD bands.
 */
float emoai_get_fdd_band_dl_freq (int band_array_index, uint32_t earfcn);

/* Get UL frequency from EARFCN number for FDD bands.
 */
float emoai_get_fdd_band_ul_freq (int band_array_index, uint32_t earfcn);

/* Get frequency from EARFCN number for TDD bands.
 */
float emoai_get_tdd_band_freq (int band_array_index, uint32_t earfcn);

/* Get the array index in fdd_dl_bands corresponding to EARFCN.
 */
int emoai_get_fdd_dl_band_array_index (uint32_t earfcn);

/* Get the array index in fdd_ul_bands corresponding to EARFCN.
 */
int emoai_get_fdd_ul_band_array_index (uint32_t earfcn);

/* Get the array index in tdd_bands corresponding to EARFCN.
 */
int emoai_get_tdd_band_array_index (uint32_t earfcn);

/* Get total bandwidth of a particular EUTRA FDD band corresponding to array
 * index in fdd_bands_dl.
 */
int emoai_get_fdd_band_bw (int band_array_index);

/* Get total bandwidth of a particular EUTRA TDD band corresponding to array
 * index in tdd_bands.
 */
int emoai_get_tdd_band_bw (int band_array_index);

/* Compares physical carrier frequency of the requested measurement object
 * with that of already existing measurement objects.
 * Returns:
 * 		0 -> frequencies are not equal
 * 		-1 -> requested frequency leads to erroneous condition
 * 		1 -> frequencies are equal
 */
int emoai_comp_req_freq (uint32_t earfcnR, uint32_t earfcnC);

/* Compares requested EUTRA measurement object with that of already existing
 * EUTRA measurement objects.
 * Returns:
 * 		0 -> EUTRA measurement objects are not equal
 * 		-1 -> requested EUTRA measurement object leads to erroneous condition
 * 		1 -> EUTRA measurement objects are equal
 */
int emoai_comp_EUTRA_measObj (MeasObjEUTRA * req_mo, MeasObjEUTRA * ctxt_mo);

/* Validates requested measurement object parameters and also checks whether
 * similar measurement object already exists.
 * Returns:
 * 		0 -> measurement object does not exist and is valid
 * 		-1 -> requested measurement object leads to erroneous condition
 * 		(> 0) -> measurement object already exist and is valid. Return value
 *				 is measurement object identifier of existing object
 */
int rrc_meas_val_trigg_measObj (ueid_t ue_id, MeasObject * m_obj);

/* Compares requested EUTRA report configuration with that of already existing
 * EUTRA report configurations.
 * Returns:
 * 		0 -> EUTRA report configurations are not equal
 * 		-1 -> requested EUTRA report configuration leads to erroneous condition
 * 		1 -> EUTRA report configurations are equal
 */
int emoai_comp_EUTRA_repConf (RepConfEUTRA * req_rc, RepConfEUTRA * ctxt_rc);

/* Validates requested report configuration parameters and also checks whether
 * similar report configuration already exists.
 * Returns:
 * 		0 -> report configuration does not exist and is valid
 * 		-1 -> requested report configuration leads to erroneous condition
 * 		(> 0) -> report configuration already exist and is valid. Return value
 *				 is report configuration identifier of existing object
 */
int rrc_meas_val_trigg_repConf (
	ueid_t ue_id,
	uint32_t mo_freq,
	ReportConfig * r_conf);

/* Update the contents of existing measurement object if the requested
 * measurement object has the same carrier frequency as the existing object.
 */
int rrc_meas_trigg_update_measObj (int mo_id, MeasObject * m_obj);

/* Receives RRC measurements request from controller and process it.
 * Sends back a success reply upon successfully processing the request, else
 * sends backs a failure reply to the controller.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_RRC_measurements (
	EmageMsg * request,
	EmageMsg ** reply,
	unsigned int trigger_id);

/* Stores the pointer to mui variable stored in RRC eNB of OAI.
 * This is used to in lower layer signalling. (OAI specific)
 */
int emoai_store_rrc_eNB_mui (mui_t * mui);

/* Standard RRC reconfiguration stack operation for a UE in order to reconfigure
 * its RRC measurements.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_RRC_meas_reconf (
	uint32_t rnti,
	int measId_add,
	int measId_rem,
	MeasObject * m_obj,
	ReportConfig * r_conf);

int rrc_meas_req (uint32_t * rnti);

/* RB Tree holding all request parameters related to RRC measurements trigger.
 */
struct rrc_meas_trigg_tree;

#endif