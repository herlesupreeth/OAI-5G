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
int rrc_meas_comp_trigg (struct rrc_meas_trigg* t1, struct rrc_meas_trigg* t2);

/* Fetches RRC measurement trigger context based on UE rnti and MeasId.
 */
struct rrc_meas_trigg* rrc_meas_get_trigg (uint32_t rnti, int measId);

/* Removes RRC measurement triggger context from tree.
 */
int rrc_meas_rem_trigg (struct rrc_meas_trigg* ctxt);

/* Insert RRC measurement triggger context into tree.
 */
int rrc_meas_add_trigg (struct rrc_meas_trigg* ctxt);

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
 * index in fdd_bands_dl or fdd_bands_ul.
 */
int emoai_get_fdd_band_bw (int band_array_index);

/* Get total bandwidth of a particular EUTRA TDD band corresponding to array
 * index in tdd_bands.
 */
int emoai_get_tdd_band_bw (int band_array_index);

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

/* Holds all the information about UL of each of EUTRA FDD bands.
 */
struct fdd_bands_ul_i {
	/* Band number of the EUTRA band. */
	int n;
	/* EARFCN lowest value in UL for this band. */
	uint32_t cn_ULl;
	/* EARFCN highest value in UL for this band. */
	uint32_t cn_ULh;
	/* Lowest frequency in UL for this band (MHz). */
	float f_ULl;
};

#endif