/* Copyright (c) 2016 Kewin Rausch <kewin.rausch@create-net.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/*
 * Profile of the configuration read from the config file.
 */

#ifndef __EMAGE_CONFIG_H
#define __EMAGE_CONFIG_H

struct net_listener;

/* This is ultimately the agent. */
struct config_profile {
	/* IPv4 address for the controller. */
	char ctrl_ipv4_addr[16];
	/* Port of the controller. */
	unsigned short ctrl_port;
};

#endif /* __EMAGE_CONFIG_H */
