# Copyright (c) 2016 Kewin Rausch <kewin.rausch@create-net.org>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Makefile to compile the Empower Userspace Controller simulator.
#

CC=gcc

ROOT=../..

TECHSP=$(ROOT)/tech
AGENTP=$(ROOT)/agent
PROTOP=$(ROOT)/proto

LIBS=-L$(AGENTP) -L$(PROTOP) -lemproto -lpthread -lprotobuf-c
INCLUDES=-I$(ROOT)/include -I$(TECHSP)/usim/ -I$(PROTOP)/pb

all:
	$(CC) -I$(EMPATH) -I$(EMTECH) -I$(PROTOP)/pb -o uctrl 		\
		uctrl.c							\
		$(LIBS)

clean:
	rm -f ./uctrl
