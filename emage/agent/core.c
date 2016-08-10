/* Copyright (c) 2016 Kewin Rausch <kewin.rausch@create-net.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/* Empower Agent.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <pthread.h>

#include <emage.h>
#include <emlog.h>

#include "config.h"
#include "agent.h"
#include "emlist.h"
#include "net.h"
#include "sched.h"

/* Static location of the configuration file. */
#define EM_CONFIG_FILE			"/etc/empower/agent.conf"

/* For the first run only. */
int initialized = 0;

/* Configuration of the agent. */
struct config_profile em_conf = {0};

/* Agent which are actually actives. */
LIST_HEAD(em_agents);
/* Lock for handling the agents list. */
pthread_spinlock_t em_agents_lock;

/******************************************************************************
 * Misc.                                                                      *
 ******************************************************************************/

int em_init(void) {
	if(!initialized) {
		/* Initialize locking. */
		pthread_spin_init(&em_agents_lock, 0);

		if(em_load_config(EM_CONFIG_FILE, &em_conf)) {
			EMLOG("No configuration file...");
			return -1;
		}

		EMDBG("Configuration file loaded successfully...");

		/* Don't perform initialization again. */
		initialized = 1;
	}

	return 0;
}

/* Read the configuration file and set the globals properly. Returns a negative
 * error number on error.
 */
int em_load_config(char * config_path, struct config_profile * config) {
	char buf[4096];
	int br = 0;

	char * token = 0;

	int fd = open(EM_CONFIG_FILE, O_RDONLY);

	if(fd < 0) {
		EMLOG("Failed to load config file %s.", config_path);
		return -1;
	}

	br = read(fd, buf, 4096);

	if(br <= 0) {
		EMLOG("Nothing to be read from the %s config file!",
			config_path);

		close(fd);
		return -1;
	}

	token = strtok(buf, " ");

	if(!token) {
		EMLOG("Configuration file address is malformed.");
		close(fd);
		return -1;
	}

	strncpy(config->ctrl_ipv4_addr, token, 16);
	token = strtok(NULL, " ");

	if(!token) {
		EMLOG("Configuration file port is malformed.");
		close(fd);
		return -1;
	}

	config->ctrl_port = atoi(token);

	close(fd);
	return 0;
}

int em_release_agent(struct agent * a) {
	free(a);
}

int em_terminate_agent(int b_id) {
	struct agent * a = 0;

	int found = 0;
	int status = 0;

/****** LOCK ******************************************************************/
	pthread_spin_lock(&em_agents_lock);
	list_for_each_entry(a, &em_agents, listh) {
		if(a->b_id == b_id) {
			list_del(&a->listh);
			found = 1;
		}
	}
	pthread_spin_unlock(&em_agents_lock);
/****** UNLOCK ****************************************************************/

	if(found) {
		if(a->ops->release) {
			status = a->ops->release();
		}

		EMDBG("Releasing agent for base station %d", a->b_id);
		em_release_agent(a);
	}

	return status;
}

/******************************************************************************
 * Entry/exit points for the Agent.                                           *
 ******************************************************************************/

int em_start(struct em_agent_ops * ops, int b_id) {
	struct agent * a = 0;

	int status = 0;
	int running = 0;	/* Already running? */
	int nm = 0;		/* No mem? */

	if(em_init()) {
		return -1;
	}

	/* Any check for necessary callbacks here. For the moment you can also
	 * implement no callbacks: your agent will simply do nothing.
	 */
	if(!ops) {
		EMLOG("Invalid set of operations...");
		return -1;
	}

/****** LOCK ******************************************************************/
	pthread_spin_lock(&em_agents_lock);
	/* Find for an already present agent. */
	list_for_each_entry(a, &em_agents, listh) {
		if(a->b_id == b_id) {
			running = 1;
			break;
		}
	}

	/* New agent? */
	if(!running) {
		a = malloc(sizeof(struct agent));

		if(a) {
			memset(a, 0, sizeof(struct agent));
			INIT_LIST_HEAD(&a->listh);
			a->b_id = b_id;

			/* Quickly add it to the list. */
			list_add(&a->listh, &em_agents);
		} else {
			nm = 1;
		}

	}
	pthread_spin_unlock(&em_agents_lock);
/****** UNLOCK ****************************************************************/

	/* Duplicate? */
	if(running) {
		EMLOG("Agent for base station %d is already running...",
			b_id);

		return -1;
	}

	/* No memory check. */
	if(nm) {
		EMLOG("Not enough memory!");
		return -1;
	}


	EMDBG("New agent for %d created", b_id);

	/*
	 * Finish to fill up important fields.
	 */
	memcpy(a->net.addr, em_conf.ctrl_ipv4_addr, 16);
	a->net.port = em_conf.ctrl_port;
	a->ops = ops;

	if (a->ops->init) {
		status = a->ops->init();

		/* On error, do not launch the agent. */
		if (status < 0) {
			EMLOG("Custom initialization failed with error %d",
				status);

/****** LOCK ******************************************************************/
			pthread_spin_lock(&em_agents_lock);
			list_del(&a->listh);
			pthread_spin_unlock(&em_agents_lock);
/****** UNLOCK ****************************************************************/

			em_release_agent(a);

			return status;
		}
	}

	/*
	 * Start this agent scheduler.
	 */

	if(sched_start(&a->sched)) {
/****** LOCK ******************************************************************/
		pthread_spin_lock(&em_agents_lock);
		list_del(&a->listh);
		pthread_spin_unlock(&em_agents_lock);
/****** UNLOCK ****************************************************************/

		EMLOG("Failed to create the agent scheduler thread.");
		em_release_agent(a);

		return -1;
	}

	/*
	 * Start this agent networking operations.
	 */

	if(net_start(&a->net)) {
/****** LOCK ******************************************************************/
		pthread_spin_lock(&em_agents_lock);
		list_del(&a->listh);
		pthread_spin_unlock(&em_agents_lock);
/****** UNLOCK ****************************************************************/

		EMLOG("Failed to create the listener agent thread.");
		sched_stop(&a->sched);
		em_release_agent(a);

		return -1;
	}

	return 0;
}

int em_stop(void) {
	/* Wait for it to finish.
	pthread_join(em_listener.thread, NULL);
	/*pthread_join(em_listen_thread, NULL);*/

	EMLOG("Agent has been terminated!");

	return 0;
}
