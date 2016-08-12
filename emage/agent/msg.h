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
