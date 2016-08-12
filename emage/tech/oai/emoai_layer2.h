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

/* OAI Layer 2 related technology abstractions for emage.
 *
 * This element implements OAI Layer 2 related technologies, and provides
 * service (statistics) to the agent.
 *
 */

#ifndef __EMOAI_LAYER2_H
#define __EMOAI_LAYER2_H

#include <main.pb-c.h>
#include <statistics.pb-c.h>
#include <emlog.h>

/* UE identifier template. */
typedef int ueid_t;
/* Module identifier template. */
typedef uint32_t mid_t;

/* Refers to the layer 2 statistics request and prepares the requested layer 2
 * statistics reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_L2_stats_reply (EmageMsg * request, EmageMsg ** reply);

/* Prepares the buffer status report for layer 2
 * statistics reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_prep_bsr (size_t n_bsr, uint32_t ** bsr, mid_t m_id, ueid_t ue_id);

/* Prepares the rlc buffer status report for layer 2
 * statistics reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_prep_rlc_bsr (size_t n_rlc_bsr,
						RlcBsrReport ***buff_stat_reports,
						mid_t m_id,
						ueid_t ue_id);

/* Prepares the downlink CQI report for layer 2
 * statistics reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_prep_dl_cqi (CsiType type,
					   DlCqiReport **dl_cqi_report,
					   mid_t m_id,
					   ueid_t ue_id);

/* Prepares the paging buffer status report for layer 2
 * statistics reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_prep_pbsr (PagingBufferReport **paging_buff_stats_report,
					 mid_t m_id,
					 ueid_t ue_id);

/* Prepares the uplink CQI report for layer 2
 * statistics reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_prep_ul_cqi (uint32_t type,
					   UlCqiReport **uplink_cqi_report,
					   mid_t m_id,
					   ueid_t ue_id);

/* Prepares the noise interference report for layer 2
 * statistics reply.
 *
 * Returns 0 on success, or a negative error code on failure.
 */
int emoai_prep_noise_interf (NoiseInterferenceReport **noise_interf_report,
							 mid_t m_id,
							 int CC_id);

#endif
