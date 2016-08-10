/* Copyright (c) 2016 Supreeth Herle <s.herle@create-net.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/* OAI technology abstractions for emage.
 *
 * This element defines the operations that can be performed on OAI by agent
 * upon receiving of controller messages or based on triggers at OAI.
 *
 */

#ifndef __EMOAI_H
#define __EMOAI_H

#include <emage.h>
#include "emoai_layer2.h"
#include "emoai_config.h"

/* Operations offered by this technology abstraction module. */
extern struct em_agent_ops sim_ops;

#endif
