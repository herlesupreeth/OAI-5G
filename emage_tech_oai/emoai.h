/* Copyright (c) 2016 Supreeth Herle <s.herle@create-net.org>
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

/* OAI technology abstractions for emage.
 *
 * This element defines the operations that can be performed on OAI by agent
 * upon receiving of controller messages or based on triggers at OAI.
 *
 */

#ifndef __EMOAI_H
#define __EMOAI_H

#include <pthread.h>

#include <emage/emage.h>

/* OAI supports number of cards = 1 in current implementation.
 * If more than one CC is supported, multithreaded approach will
 * be taken.
 */
#define DEFAULT_ENB_ID 0

/* Operations offered by this technology abstraction module. */
extern struct em_agent_ops sim_ops;

/* Lock for handling the rrc measurements triggers list. */
pthread_spinlock_t rrc_meas_t_lock;

/* Lock for handling the rrc measurements configuration triggers list. */
pthread_spinlock_t rrc_m_conf_t_lock;

/* Defines the custom initialization for the OAI abstraction layer, which are to be performed
 * initally when OAI attaches to controller through the agent.
 */
int emoai_init (void);

#endif
