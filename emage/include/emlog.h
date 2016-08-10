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

#ifndef __EMAGE_LOG_H
#define __EMAGE_LOG_H

/* Comment this to remove debugging logs. */
#define EM_DEBUG
/* Comment this to remove send message RAW logs. */
/* #define EM_DEBUG_SEND */

/* Log routine for every feedback. */
#define EMLOG(x, ...)							\
	printf("emage: "x"\n", ##__VA_ARGS__)

#ifdef EM_DEBUG
/* Debugging routine. */
#define EMDBG(x, ...)							\
	printf("emage-debug:"x"\n", ##__VA_ARGS__)
#else
/* Debugging routine. */
#define EMDBG(x, ...)
#endif

#endif
