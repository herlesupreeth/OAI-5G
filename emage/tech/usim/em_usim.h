/* Copyright (c) 2016 Kewin Rausch <kewin.rausch@create-net.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/* Userspace Agent simulator technology.
 *
 * This element simulates an empower agent which is present on an eNB, and just
 * provides dummy data and operations which are arranged following it's personal
 * logic.
 */

#ifndef __EM_USIM_H
#define __EM_USIM_H

/* Operations offered by this technology abstraction module. */
extern struct em_agent_ops sim_ops;

#endif
