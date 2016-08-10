/* Copyright (c) 2016 Kewin Rausch <kewin.rausch@create-net.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/* Userspace Agent controller simulator.
 */


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include <main.pb-c.h>

/* Define to enable receiver side raw-data debugging. */
/* #define UCTRL_DEBUG_RECV */

#define UCTRL_BUF_SIZE				4096
#define UCTRL_PENDING_CONNECTION		5
#define EM_MSG_LENGTH_HEADROOM			4

/* Provides the info for the serving thread. */
struct uctrl_connection {
	/* The client address. */
	struct sockaddr_in caddr;
	/* file descriptor for this specific connection. */
	int fd;
	/* Thread that will serve the client connection. */
	pthread_t thread;
};

/*
 * Early fault and exiting:
 */

/* Ctrl-c handler. */
volatile sig_atomic_t uctrl_ctrlc = 0;
/* Counts how many 'ctrl-c' have been invoked on this program. */
unsigned int uctrl_ctrl_i = 0;

/*
 * Globals used for networking:
 */

/* The manager listening file descriptor. */
int uctrl_lfd = 0;
/* Listening port. */
unsigned short uctrl_port = 0;
/* Sequence number. */
int uctrl_seq = 0;
/* The agent to communicate with. */
struct uctrl_connection * uctrl_agent = 0;

/******************************************************************************
 * Early fail.                                                                *
 ******************************************************************************/

/* Handle a break-execution signal from the user. */
void handle_ctrlc(int signal) {
	uctrl_ctrlc = 1;
	uctrl_ctrl_i++;

	/* Force stop on triple ctrl-c. */
	if(signal == SIGINT) {
		close(uctrl_lfd);
		exit(-1);
	}
}

/******************************************************************************
 * Connection logic.                                                          *
 * Every incoming connection will have it's own thread which will do the      *
 * simulation job depending on the command sent.                              *
 ******************************************************************************/

/* Prepares a L@ statistic request message which can be personalized. This is
 * not a complete and valid message, and it's here only for debugging purposes.
 *
 * Types: 0=all, 1=cell or 2=ue.
 * Modes: 0=once, 1=periodical or 3=off.
 */
int parse_l2_stats_request(
	char ** buf, int * size, int type, int mode, int interval) {

	char * rb = 0;
	int ms = 0;
	int hr = 0;

	EmageMsg msg = EMAGE_MSG__INIT;
	Header hdr = HEADER__INIT;
	Statistics stats = STATISTICS__INIT;
	L2StatisticsRequest l2req = L2_STATISTICS_REQUEST__INIT;

	/*
	 * Filling the header.
	 */

	/* Length provided later. */
	hdr.has_len = 1;

	hdr.has_type = 1;
	hdr.type = MSG_TYPE__STATS_REQ;

	hdr.has_vers = 1;
	hdr.vers = 1;

	hdr.has_b_id = 1;
	hdr.b_id = 0;

	hdr.has_seq = 1;
	hdr.seq = uctrl_seq++;

	/*
	 * Filling L2 stat request.
	 */

	l2req.has_t_id = 1;
	l2req.t_id = rand();

	l2req.has_type = 1;
	l2req.type = type;

	l2req.has_report_freq = 1;
	l2req.report_freq = mode;

	l2req.has_subframe = 1;
	l2req.subframe = interval;

	/*
	 * Filling L2 stat request.
	 */

	stats.has_type = 1;
	stats.type = STATS_MSG_TYPE__L2_STATISTICS_REQUEST;
	stats.stats_msg_case = STATISTICS__STATS_MSG_L2_STATS_REQ;

	stats.l2_stats_req = &l2req;

	/*
	 * Link everything together.
	 */

	msg.head = &hdr;
	msg.mstats = &stats;
	msg.message_case = EMAGE_MSG__MESSAGE_M_STATS;

	/*
	 * Size evaluation.
	 */

	ms = emage_msg__get_packed_size(&msg);
	hr = htonl(ms);

	/*
	 * Buffer creation and fill up.
	 */

	rb = malloc(sizeof(char) * (ms + EM_MSG_LENGTH_HEADROOM));

	if(!rb) {
		printf("No more memory!\n");
		return -1;
	}

	memcpy(rb, &hr, EM_MSG_LENGTH_HEADROOM);
	emage_msg__pack(&msg, rb + EM_MSG_LENGTH_HEADROOM);

	*buf = rb;
	*size = ms + EM_MSG_LENGTH_HEADROOM;

	return 0;
}

int parse_msg(char * buf, int size) {
	int mlen = 0;
	EmageMsg * msg = 0;

#ifdef UCTRL_DEBUG_RECV
	int i = 0;

	printf("Message is %d bytes long...\n", mlen);

	for(i = 0; i < size; i++) {
		if(i % 16 == 0) {
			printf("\n");
		}

		printf("%02x ", (unsigned char)buf[i]);
	}

	printf("\n");
#endif

	memcpy(&mlen, buf, 4);
	mlen = ntohl(mlen);

	msg = emage_msg__unpack(0, mlen, buf + 4);

	if(!msg) {
		printf("ERROR: Cannot decode the message!\n");
		return -1;
	}

	switch(msg->head->type) {
	case MSG_TYPE__HELLO_REP:
		/*
		printf("HELLO reply\n    Period:%d\n",
			msg->mhello->period);
		*/
		break;
	case MSG_TYPE__INVALID:
		printf("Invalid message!\n");
		break;
	default:
		printf("Unknown message!\n");
		break;
	}

	emage_msg__free_unpacked(msg, 0);

	return 0;
}

void * serve_connection(void * args) {
	struct uctrl_connection * c = (struct uctrl_connection *)args;

	char * hostaddr = 0;

	char buf[UCTRL_BUF_SIZE];
	int bread = 0;

	hostaddr = inet_ntoa(c->caddr.sin_addr);

	if(!hostaddr) {
		printf("Could not parse the host address!\n");
		goto out;
	}

	printf("Connection with %s established...\n", hostaddr);
	uctrl_agent = c;

	/* Again, continue until out is issued! */
	while(!uctrl_ctrl_i) {
		bread = read(c->fd, buf, UCTRL_BUF_SIZE);

		/* Something has been read. */
		if(bread > 0) {
			parse_msg(buf, bread);
		}
		/* Connection closed. */
		else if (bread == 0) {
			printf("Shutting down connection...\n");
			close(c->fd);
			break;
		}
	}

out:
	uctrl_agent = 0;
	/* Free the informations at the end. */
	free(c);
}

/******************************************************************************
 * Listening loop.                                                            *
 ******************************************************************************/

/* This procedure does not return until the server close. Will listen and serve
 * incoming connections.
 *
 * A negative return value will identify an error.
 */
int listen_loop() {
	struct sockaddr_in saddr = {0};
	struct uctrl_connection * mc = 0;

	socklen_t clen = 0;
	int status = 0;

	if(uctrl_port <= 0 || uctrl_port > 65535) {
		printf("Port not valid!\n");
		return -1;
	}

	uctrl_lfd = socket(AF_INET, SOCK_STREAM, 0);

	if(uctrl_lfd < 0) {
		printf("Could not open a socket, error=%d\n", uctrl_lfd);
		return -1;
	}

	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(uctrl_port);

	status = bind(
		uctrl_lfd,
		(struct sockaddr*)&saddr,
		sizeof(struct sockaddr_in));

	/* Bind the address with the socket. */
	if(status) {
		printf("Could not BIND the socket, error=%d\n");
		perror("bind");
		goto err;
	}

	/* Prepare to accept incoming connections. */
	status = listen(uctrl_lfd, UCTRL_PENDING_CONNECTION);

	if(status < 0) {
		printf("Could not LISTEN for socket, error=%d\n");
		perror("listen");
		goto err;
	}

	/* Continue until the first ctrl-c. */
	while(!uctrl_ctrl_i) {
		clen = sizeof(struct sockaddr);
		mc = (struct uctrl_connection *) malloc(
			sizeof(struct uctrl_connection));

		if(!mc) {
			printf("Not enough memory for connection!\n");
			goto err;
		}

		/* This will make everything wait until new client connects. */
		mc->fd = accept(
			uctrl_lfd, (struct sockaddr *)&mc->caddr, &clen);

		if(mc->fd < 0) {
			printf("Accepting failure, error=%d\n", mc->fd);
			perror("accept");
			goto err;
		}

		/* Create the thread which will take care of it. */
		if(pthread_create(&mc->thread, NULL, serve_connection, mc)) {
			printf("Pthread failure!\n");
			free(mc);
			goto err;
		}
	}

	return 0;

err:
	close(uctrl_lfd);
	return -1;
}

/******************************************************************************
 * Entry point.                                                               *
 ******************************************************************************/

void * input_loop(void * args) {
	char * line = 0;
	size_t len = 0;
	size_t llen = 0;

	char * buf = 0;
	int mlen = 0;

	char * token = 0;
	int opt1 = 0;
	int opt2 = 0;
	int opt3 = 0;

	/* Continue until the first ctrl-c. */
	while(!uctrl_ctrl_i) {
		printf("> ");
		llen = getline(&line, &len, stdin);

		if(llen <= 1) {
			printf("Command not recognized!\n");
			continue;
		}

		line[llen - 1] = 0;
		token = strtok(line, " ");

		/* We required a L2 stats request. */
		if(strcmp(token, "l2_stats_req") == 0) {
			token = strtok(0, " ");

			if(!token) {
				printf(
"Syntax: l2_stats_req <type> <mode> <interval>\n");

				continue;
			}

			opt1=atoi(token);

			token = strtok(0, " ");
			opt2=atoi(token);

			token = strtok(0, " ");
			opt3=atoi(token);


			parse_l2_stats_request(&buf, &mlen, opt1, opt2, opt3);

			if(uctrl_agent) {
				printf("Sending L2 stats request...\n");

				send(
					uctrl_agent->fd,
					buf,
					mlen,
					MSG_DONTWAIT | MSG_NOSIGNAL);
			} else {
				printf("Nobody is connected. :(\n");
			}
		}
	}
}

void help() {
	printf(
"Emage Controller Simulator\n"
"Copyright (c) 2016 Kewin Rausch <kewin.rausch@create-net.org>\n"
"\n"
"Usage: mgrsim <port>\n"
"\n");
}

int main(int argc, char ** argv) {
	pthread_t tid;

	/* User want to terminate this. */
	signal(SIGINT, handle_ctrlc);

	if(argc != 2) {
		help();
		return 0;
	}

	uctrl_port = atoi(argv[1]);

	printf("Starting the EMAGE controller on port %d...\n", uctrl_port);

	/* Create the thread which will take care of it. */
	if(pthread_create(&tid, NULL, input_loop, NULL)) {
		printf("Pthread failure!\n");
		return -1;
	}

	/* This returns only on error or if the program should exit. */
	if(listen_loop() < 1) {
		return -1;
	}

	printf("Terminating the controller...\n");

	return 0;
}
