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

#include "MeasResults.h"
#include <math.h>

/* Defines the parameters used to send RRC measurements reply message to
 * controller whenever the UE sends RRC measurements to OAI.
 */
struct rrc_meas_params {
	/* Module identifier. */
	mid_t m_id;
	/* Base station identifier. */
	uint32_t b_id;
	/* Transaction identifier. */
	tid_t t_id,
	/* RNTI of the UE. */
	uint32_t rnti;
	/* Flag to indicate whether measurements reconfig was success or failure. */
	int reconfig_success;
	/* RRC Measurements received from UE. */
	MeasResults_t meas;
};

#endif