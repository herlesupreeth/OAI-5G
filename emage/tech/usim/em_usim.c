/* Copyright (c) 2016 Kewin Rausch <kewin.rausch@create-net.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/* Userspace Agent simulator technology.
 */

#include <stdio.h>

#include <emage.h>

#include "em_usim.h"

int emsim_L2_stat_reply(EmageMsg * request, EmageMsg ** reply) {
	printf("---> Custom technology reply!\n");
	return 0;
}

/* Agent operations for this simulator application. */
struct em_agent_ops sim_ops = {
	.L2_stat_reply = emsim_L2_stat_reply,
};
