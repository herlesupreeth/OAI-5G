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

/* Agent operations for OAI application. */
struct em_agent_ops sim_ops = {
	.L2_stat_reply = emoai_L2_stats_reply,
	.UE_config_reply = emoai_ue_config_reply,
	.eNB_config_reply = emoai_eNB_config_reply,
};
