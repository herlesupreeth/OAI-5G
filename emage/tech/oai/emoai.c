/* Copyright (c) 2016 Supreeth Herle <s.herle@create-net.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/* OAI technology abstractions implementation for emage.
 */

#include "emoai.h"

/* Agent operations for OAI application. */
struct em_agent_ops sim_ops = {
	.L2_stat_reply = emoai_L2_stats_reply,
	.UE_config_reply = emoai_ue_config_reply,
	.eNB_config_reply = emoai_eNB_config_reply,
};
