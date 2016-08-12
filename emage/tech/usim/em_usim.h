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
