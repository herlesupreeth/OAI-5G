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
