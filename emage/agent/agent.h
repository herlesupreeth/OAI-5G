/* Copyright (c) 2016 Kewin Rausch <kewin.rausch@create-net.org>
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
