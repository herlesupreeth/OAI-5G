	/* Copyright (c) 2016 Kewin Rausch <kewin.rausch@create-net.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/*
 *  Empower Agent messages handling procedures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>

#include <emlog.h>

#include "msg.h"

/* Reserved area for every message to define the message length. */
#define EM_MSG_LENGTH_HEADROOM		4

int msg_parse(char ** buf, int * size, EmageMsg * msg) {
	char * rb = 0;

	int hr = 0;
	int ms = emage_msg__get_packed_size(msg);

	hr = htonl(ms);

	/*
	 * Buffer creation and fill up.
	 */

	rb = malloc(sizeof(char) * (ms + EM_MSG_LENGTH_HEADROOM));

	if(!rb) {
		EMLOG("No more memory!");
		return -1;
	}

	/* Data to the buffer. */
	memcpy(rb, &hr, EM_MSG_LENGTH_HEADROOM);
	emage_msg__pack(msg, rb + EM_MSG_LENGTH_HEADROOM);

	*buf = rb;
	*size = ms + EM_MSG_LENGTH_HEADROOM;

	return 0;
}

int msg_parse_hello(int seq, int bid, char ** buf, int * size) {
	char * rb = 0;
	int ms = 0;
	int hr = 0;

	EmageMsg msg = EMAGE_MSG__INIT;
	Header hdr = HEADER__INIT;
	Hello hello = HELLO__INIT;

	/*
	 * Filling the header.
	 */

	hdr.has_type = 1;
	hdr.type = MSG_TYPE__HELLO_REP;

	hdr.has_vers = 1;
	hdr.vers = 1;

	hdr.has_b_id = 1;
	hdr.b_id = bid;

	hdr.has_seq = 1;
	hdr.seq = seq;

	hdr.has_t_id = 1;
	hdr.t_id = 0;

	/*
	 * Filling Hello.
	 */

	hello.has_period = 1;
	hello.period = 1000;

	/*
	 * Link everything together.
	 */

	msg.head = &hdr;
	msg.mhello = &hello;
	msg.message_case = EMAGE_MSG__MESSAGE_M_HELLO;

	/*
	 * Buffer creation and fill up.
	 */

	return msg_parse(buf, size, &msg);
}
