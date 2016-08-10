# Copyright (c) 2016 Kewin Rausch <kewin.rausch@create-net.org>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# Makefile to compile the Empower Agent protocol buffers.
#

GCC=gcc
PCC=protoc-c
PROTO-OUT=./pb

all:
	make proto
	make protolib
	
protolib:
	$(GCC) -I./pb -c ./pb/*.pb-c.c
	ar -rcs libemproto.a *.pb-c.o  

proto:
	mkdir -p $(PROTO-OUT)
	$(PCC) ./*.proto --c_out=$(PROTO-OUT)

clean:
	rm -rf $(PROTO-OUT)
	rm -f ./*.o
	rm -f ./*.a
	
