/* Copyright (c) 2016 Kewin Rausch <kewin.rausch@create-net.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/*
 * Definition for an agent.
 */

#ifndef __EMAGE_AGENT_H
#define __EMAGE_AGENT_H

#include "net.h"
#include "sched.h"

struct em_agent_ops;

/* This is ultimately the agent. */
struct agent {
	/* Member of a list. */
	struct list_head listh;

	/* Base station id which the agent is serving. */
	int b_id;

	/* Registered, technology dependant, operations. */
	struct em_agent_ops * ops;

	/* Network operation context for this agent. */
	struct net_context net;
	/* Scheduler context for this agent jobs. */
	struct sched_context sched;
};

#endif /* __EMAGE_AGENT_H */
