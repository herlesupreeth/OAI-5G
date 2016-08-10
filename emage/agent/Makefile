# Copyright (c) 2016 Kewin Rausch <kewin.rausch@create-net.org>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# Makefile to compile the Emage agent as a library.
#

CC=gcc

AGENTP=.
PROTOP=../proto

LIBS=-lpthread -lprotobuf-c
INCLUDES=-I../include -I../proto/pb

all:
	$(CC) $(INCLUDES) -c						\
		$(AGENTP)/msg.c 					\
		$(AGENTP)/net.c						\
		$(AGENTP)/sched.c 					\
		$(AGENTP)/core.c
	ar -rcs libagent.a *.o  
		
clean:
	rm -r ./*.o
	rm -r ./*.a