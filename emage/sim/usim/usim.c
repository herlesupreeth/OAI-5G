/* Copyright (c) 2016 Kewin Rausch <kewin.rausch@create-net.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/* Userspace Agent simulator main application.
 */


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "emlog.h"
#include <emage.h>
#include <em_usim.h>

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
