/* Copyright (c) 2016 Kewin Rausch <kewin.rausch@create-net.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/*
 * Empower Agent internal network listener logic.
 */

#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <emlog.h>
#include <main.pb-c.h>

#include "agent.h"
#include "msg.h"
#include "net.h"

#include "emlist.h"
#include "sched.h"

/* Default buffer size. */
#define EM_BUF_SIZE			4096

/******************************************************************************
 * Network procedures.                                                        *
 ******************************************************************************/

/* Common operations done when it successfully connects again. */
int net_connected(struct net_context * net) {
	struct agent * a = container_of(net, struct agent, net);
	struct sched_job * h = 0;

	EMDBG("Connected to controller %s:%d", net->addr, net->port);
	net->status = EM_STATUS_CONNECTED;

	h = malloc(sizeof(struct sched_job));

	if(!h) {
		EMLOG("No more memory!");
		return -1;
	}

	INIT_LIST_HEAD(&h->next);
	h->elapse = 1000;
	h->type = JOB_TYPE_HELLO;
	h->reschedule = 1;

	/* Add the Hello message. */
	sched_add_job(h, &a->sched);

	return 0;
}

int net_next_seq(struct net_context * net) {
	int ret = 0;

/****** LOCK ******************************************************************/
	pthread_spin_lock(&net->lock);
	ret = net->seq++;
	pthread_spin_unlock(&net->lock);
/****** UNLOCK ****************************************************************/

	return ret;
}

/* Turn the socket in an non-blocking one. */
int net_noblock_socket(int sockfd) {
	int flags = fcntl(sockfd, F_GETFL, 0);

	if(flags < 0) {
		return -1;
	}

	return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

/* Apply the desired personalization to the socket. */
int net_nodelay_socket(int sockfd) {
	int flag = 1; /* Enable no delay... */
	int result = setsockopt(
		sockfd,
		SOL_TCP,
		TCP_NODELAY,
		(char *) &flag,
		sizeof(int));

	if (result < 0) {
		EMLOG("Could not personalize the socket!");
		perror("setsockopt");

		return -1;
	}

	return 0;
}

int net_not_connected(struct net_context * net) {
	EMDBG("No more connected with controller!");

	close(net->sockfd);
	net->sockfd = 0;

	net->status = EM_STATUS_NOT_CONNECTED;
	net->seq = 0;
}

/* Connect to the controller. Returns the open socket file descriptor if a
 * socket for the controller has been open, otherwise loop until a connection
 * has been established.
 *
 * Returns a negative number on error.
 */
int net_connect_to_controller(struct net_context * net) {
	int status = 0;

	struct sockaddr_in srvaddr = {0};
	struct hostent * ctrli = 0;

	if(net->sockfd == 0) {
		status = socket(AF_INET, SOCK_STREAM, 0);
	}

	if(status < 0) {
		EMLOG("Could not create the socket, error=%d", net->sockfd);
		perror("socket");

		return -1;
	}
	/* Socket has been created now. */
	else if (status > 0) {
		net->sockfd = status;
		net_nodelay_socket(net->sockfd);
	}

	EMDBG("Connecting to %s:%d...", net->addr, net->port);

	ctrli = gethostbyname(net->addr);

	if(!ctrli) {
		EMLOG("Could not resolve controller!");
		perror("gethostbyname");

		return -1;
	}

	srvaddr.sin_family = AF_INET;
	memcpy(
		&srvaddr.sin_addr.s_addr,
		ctrli->h_addr,
		ctrli->h_length);
	srvaddr.sin_port = htons(net->port);

	status = connect(
		net->sockfd,
		(struct sockaddr *)&srvaddr,
		sizeof(struct sockaddr));

	if(status < 0) {
		EMLOG("Error while connecting to %s, error=%d",
			net->addr,
			status);
		perror("connect");


		return -1;
	}

	return 0;
}

/* Receive data. */
int net_recv(struct net_context * context, char * buf, unsigned int size) {
	return recv(context->sockfd, buf, size, MSG_DONTWAIT | MSG_NOSIGNAL);
}

/* Send data. */
int net_send(struct net_context * context, char * buf, unsigned int size) {
#ifdef EM_DEBUG_SEND
	int i = 0;

	for(i = 0; i < size; i++) {
		if(i % 16 == 0) {
			printf("\n");
		}

		printf("%02x ", (unsigned char)buf[i]);
	}

	printf("\n");
#endif

	/* NOTE:
	 * Since sending on a dead socket can cause a single to be issued to the
	 * application (SIGPIPE), we don't want that the host get disturbed by
	 * this, and so we ask not to notify the error.
	 */
	return send(context->sockfd, buf, size, MSG_DONTWAIT | MSG_NOSIGNAL);
}

int net_enbcr(struct net_context * net, EmageMsg * msg) {
	struct agent * a = container_of(net, struct agent, net);
	EnbConfigRequest * cr = msg->mconfs->enb_conf_req;
	struct sched_job * job = malloc(sizeof(struct sched_job));

	if (!job) {
		EMLOG("Not enough memory!");
		goto err_free;
	}

	memset(job, 0, sizeof(struct sched_job));
	INIT_LIST_HEAD(&job->next);
	job->type = JOB_TYPE_ENB_CONFIG_REQ;
	job->args = msg;
	job->id = msg->head->t_id;
	job->elapse = 0;

	sched_add_job(job, &a->sched);

	return 0;

err_free:
	/* Free the memory and return a failure. */
	emage_msg__free_unpacked(msg, 0);
	return -1;
}

int net_uecr(struct net_context * net, EmageMsg * msg) {
	struct agent * a = container_of(net, struct agent, net);
	EnbConfigRequest * cr = msg->mconfs->enb_conf_req;
	struct sched_job * job = malloc(sizeof(struct sched_job));

	if (!job) {
		EMLOG("Not enough memory!");
		goto err_free;
	}

	memset(job, 0, sizeof(struct sched_job));
	INIT_LIST_HEAD(&job->next);
	job->type = JOB_TYPE_UE_CONFIG_REQ;
	job->args = msg;
	job->id = msg->head->t_id;
	job->elapse = 0;

	sched_add_job(job, &a->sched);

	return 0;

err_free:
	/* Free the memory and return a failure. */
	emage_msg__free_unpacked(msg, 0);
	return -1;
}

int net_L2sr(struct net_context * net, EmageMsg * msg) {
	struct agent * a = container_of(net, struct agent, net);
	L2StatisticsRequest * l2r = msg->mstats->l2_stats_req;
	struct sched_job * job = 0;

	/* Remove an already schedued job. */
	if (l2r->report_freq == REPORTING_FREQUENCY__REPF_OFF) {
		sched_remove_job(msg->head->t_id, &a->sched);

		/* Everything ends here; message is freed... */
		emage_msg__free_unpacked(msg, 0);
		return 0;
	}

	job = malloc(sizeof(struct sched_job));

	if(!job) {
		EMLOG("Not enough memory!");
		goto err_free;
	}

	memset(job, 0, sizeof(struct sched_job));
	INIT_LIST_HEAD(&job->next);
	job->type = JOB_TYPE_L2_STAT_REQ;
	job->args = msg;
	job->id = msg->head->t_id;
	job->elapse = l2r->subframe;

	/* Do it more than once? */
	if (l2r->report_freq == REPORTING_FREQUENCY__REPF_PERIODICAL) {
		job->reschedule = 1;
	}

	sched_add_job(job, &a->sched);

	return 0;

err_free:
	/* Free the memory and return a failure. */
	emage_msg__free_unpacked(msg, 0);
	return -1;
}

/* Process an incoming configuration request message. */
int net_process_config_req(struct net_context * net, EmageMsg * msg) {
	Configs * c = msg->mconfs;

	if (c->type == CONFIG_MSG_TYPE__ENB_CONF_REQUEST) {
		EMDBG("eNB configuration request detected");
		return net_enbcr(net, msg);
	}
	else if (c->type == CONFIG_MSG_TYPE__UE_CONF_REQUEST) {
		EMDBG("UE configuration request detected");
		return net_uecr(net, msg);
	}

	return 0;
}

/* Process an incoming statistic request message. */
int net_process_stats_req(struct net_context * net, EmageMsg * msg) {
	Statistics * s = msg->mstats;

	if(s->type == STATS_MSG_TYPE__L2_STATISTICS_REQUEST) {
		EMDBG("L2 statistic request detected");
		return net_L2sr(net, msg);
	}

	return 0;
}

/* Process incoming messages. */
int net_process_message(struct net_context * net, EmageMsg * msg) {
	switch(msg->head->type) {
	case MSG_TYPE__STATS_REQ:
		EMDBG("Statistics request from the controller");
		return net_process_stats_req(net, msg);
	case MSG_TYPE__CONF_REQ:
		EMDBG("Configuration request from the controller");
		return net_process_config_req(net, msg);
	default:
		EMLOG("Message not known, type=%d", msg->head->type);
		break;
	}

	return 0;
}

/******************************************************************************
 * Network listener logic.                                                    *
 ******************************************************************************/

void * net_loop(void * args) {
	struct net_context * net = (struct net_context *)args;

	int bread = 0;
	int mlen = 0;

	char buf[EM_BUF_SIZE] = {0};
	EmageMsg * msg = 0;

	unsigned int wi = net->interval;
	struct timespec wt = {0};	/* Wait time. */
	struct timespec td = {0};

	/* Convert the wait interval in a timespec struct. */
	while(wi >= 1000) {
		wi -= 1000;
		wt.tv_sec += 1;
	}
	wt.tv_nsec = wi * 1000000;

	while(!net->stop) {
		if(net->status == EM_STATUS_NOT_CONNECTED) {
			if(net_connect_to_controller(net) == 0) {
				net_connected(net);
			}
		}

		bread = net_recv(net, buf, 4);

		/* Something has been received.
		 * NOTE: Message must be at least 4 bytes long.
		 */
		if(bread == 4) {
			memcpy(&mlen, buf, 4);
			mlen = ntohl(mlen);

			bread = net_recv(net, buf, mlen);

			if (bread != mlen) {
				EMLOG("Malformed message received, "
					"msg=%d, recv=%d", mlen, bread);
				/* 
				 * Ok, this is serious, since we can loose the
				 * alignment of the 4 firsts bytes which 
				 * contains the message size.
				 */

				/* Reset the connection. */
				close(net->sockfd);
				net->sockfd = 0;
				net->status = EM_STATUS_NOT_CONNECTED;
			}

			msg = emage_msg__unpack(0, mlen, buf);

			if(msg) {
				net_process_message(net, msg);
			} else {
				EMLOG("Failed to decode the message!");
			}
		}

		/* Relax the CPU. */
		nanosleep(&wt, &td);
	}

	EMDBG("Listening loop is terminating...\n");

	/*
	 * If you need to release 'net' specific resources, do it here!
	 */

	return 0;
}

int net_start(struct net_context * net) {
	/* 1 second interval by default. */
	net->interval = 1000;

	pthread_spin_init(&net->lock, 0);

	/* Create the context where the agent scheduler will run on. */
	if(pthread_create(
		(pthread_t *)&net->thread, NULL, net_loop, net)) {

		EMLOG("Failed to create the listener agent thread.");
		return -1;
	}

	return 0;
}

int void_stop(struct net_context * net) {
	/* Stop and wait for it... */
	net->stop = 1;
	pthread_join(net->thread, 0);

	return 0;
}
