/* Copyright (c) 2016 Kewin Rausch <kewin.rausch@create-net.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/*
 * Empower Agent internal network logic.
 */

#ifndef __EMAGE_NET_H
#define __EMAGE_NET_H

#include <pthread.h>

/* Not connected to the controller. */
#define EM_STATUS_NOT_CONNECTED		0
/* Connected to the controller. */
#define EM_STATUS_CONNECTED		1

/* Private context of a network listener. */
struct net_context {
	/* Address to listen. */
	char addr[16];
	/* Port to listen. */
	unsigned short port;
	/* Socket fd used for communication. */
	int sockfd;

	/* A value different than 0 stop this listener. */
	int stop;
	/* Status of the listener. */
	int status;
	/* Sequence number. */
	int seq;

	/* Thread in charge of this listening. */
	pthread_t thread;
	/* Lock for elements of this context. */
	pthread_spinlock_t lock;
	/* Time to wait at the end of each loop, in ms. */
	unsigned int interval;
};

/* Get the next valid sequence number to emit with this context. */
int net_next_seq(struct net_context * net);

/* Adjust the context due a network error. */
int net_not_connected(struct net_context * net);

/* Send a generic message using the network listener logic.
 */
int net_send(struct net_context * net, char * buf, unsigned int size);

/* Start a new listener in a different threading context.
 *
 * Returns 0 on success, otherwise a negative error number.
 */
int net_start(struct net_context * net);

/* Order the network listener to stop it's operations. */
void net_stop(struct net_context * net);

#endif /* __EMAGE_NET_H */
