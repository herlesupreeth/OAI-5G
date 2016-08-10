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

#ifndef __EMAGE_MSG_H
#define __EMAGE_MSG_H

#include <main.pb-c.h>

/* Parse in a buffer a generic message. This procedure will fill the message
 * header length field.
 */
int msg_parse(char ** buf, int * size, EmageMsg * msg);

/* Parse an hello message in a generic buffer. You are in charge of freeing the
 * returned buffer once you are done with that data.
 *
 * Returns 0 on success, a negative integer number on error
 */
int msg_parse_hello(
	/* The sequence number for this message. */
	int seq,
	/* The base station identifier. */
	int bid,
	/* Buffer containing the message and its length. */
	char ** buf, int * size);

/* Parse an incoming buffer into a message. */
//EmageMsg * em_parse_message(char * buf, int size);

#endif
