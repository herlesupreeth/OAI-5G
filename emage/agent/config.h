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
