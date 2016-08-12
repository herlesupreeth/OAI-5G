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

/* Userspace Agent simulator main application.
 */


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <emage.h>
#include <em_usim.h>
#include <emlog.h>

/* When to stop the sending loop? */
volatile sig_atomic_t stop;

void sign_handler(int signal) {
	if(signal == SIGINT) {
		printf("SIGNINT detected!/n");
		/* Stop the agent processing. */
		em_stop();
	}
}

int main(int argc, char ** argv) {
	em_start(&sim_ops, 0xba5e);

	printf("Press Ctrl-c to exit...\n");

	/* Keep the application alive waiting for signal. */
	while(1) {
		sleep(1);
	}

	return 0;
}
