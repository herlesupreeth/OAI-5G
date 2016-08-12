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
