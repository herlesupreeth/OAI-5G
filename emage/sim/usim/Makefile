# Copyright (c) 2016 Kewin Rausch <kewin.rausch@create-net.org>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# Makefile to compile the Empower Agent protocol buffers.
#

CC=gcc

ROOT=/home/sup/openairinterface5g/emage

TECHSP=$(ROOT)/tech
AGENTP=$(ROOT)/agent
PROTOP=$(ROOT)/proto

LIBS=-L$(AGENTP) -L$(PROTOP) -lpthread -lprotobuf-c -lemproto
INCLUDES=-I$(ROOT)/include -I$(TECHSP)/usim/ -I$(PROTOP)/pb

all:
	$(CC) $(INCLUDES) -o usim					\
		$(AGENTP)/core.c					\
		$(AGENTP)/sched.c					\
		$(AGENTP)/net.c						\
		$(AGENTP)/msg.c						\
		usim.c							\
		$(TECHSP)/usim/em_usim.c				\
		$(LIBS) 
		
		
clean:
	rm -f ./usim
