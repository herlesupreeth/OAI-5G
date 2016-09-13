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
 * OAI technology abstractions implementation for emage.
 */

#include "emoai.h"
#include "emoai_config.h"
#include "emoai_rrc_measurements.h"

/* Agent operations for OAI application. */
struct em_agent_ops sim_ops = {
	.init = emoai_init,
	.UEs_ID_report = emoai_UEs_ID_report,
	.RRC_meas_conf = emoai_RRC_meas_conf_report,
	.RRC_measurements = emoai_RRC_measurements,
};

int emoai_init (void) {

	/* Initializing lock for rrc measurements triggers list. */
	if (pthread_spin_init(&rrc_meas_t_lock, PTHREAD_PROCESS_SHARED) != 0) {
		goto error;
	}
	/* Initializing lock for rrc measurements configuration triggers list. */
	if (pthread_spin_init(&rrc_m_conf_t_lock, PTHREAD_PROCESS_SHARED) != 0) {
		goto error;
	}

	return 0;

	error:
		return -1;
}
