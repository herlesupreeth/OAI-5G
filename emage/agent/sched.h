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
 * Empower Agent internal scheduler logic.
 */

#ifndef __EMAGE_SCHEDULER_H
#define __EMAGE_SCHEDULER_H

#define JOB_TYPE_INVALID			0
#define JOB_TYPE_HELLO				1
#define JOB_TYPE_L2_STAT_REQ			2
#define JOB_TYPE_ENB_CONFIG_REQ			3
#define JOB_TYPE_UE_CONFIG_REQ			4

#include <time.h>
#include <pthread.h>

#include <main.pb-c.h>

#include "emlist.h"

/* Job for agent scheduler. */
struct sched_job {
	/* Member of a list. */
	struct list_head next;

	/* Id of this job. */
	unsigned int id;
	/* Type of job scheduled. */
	int type;

	/* Data arguments for this job. */
	void * args;
	/* Setting this to 1 cause the job to be automatically re-scheduled. */
	int reschedule;

	/* Time when the job has been enqueued. */
	struct timespec issued;
	/* time in 'ms' after that the job will be run. */
	int elapse;
};

struct sched_context {
	/* A value different than 0 stop this listener. */
	int stop;

	/* Jobs actually active in the scheduler. */
	struct list_head jobs;
	/* Jobs to do but not scheduled for this run. */
	struct list_head todo;

	/* Thread in charge of this listening. */
	pthread_t thread;
	/* Lock for elements of this context. */
	pthread_spinlock_t lock;
	/* Time to wait at the end of each loop, in ms. */
	unsigned int interval;
};

/* Adds a job to a scheduler context. */
int sched_add_job(struct sched_job * job, struct sched_context * sched);

/* Release a job which is currently scheduled by using the associated id. */
int sched_remove_job(unsigned int id, struct sched_context * sched);

/* Correctly start a new scheduler in it's own context. */
int sched_start(struct sched_context * sched);

/* Stop a scheduler. */
int sched_stop(struct sched_context * sched);

#endif /* __EMAGE_SCHEDULER_H */
