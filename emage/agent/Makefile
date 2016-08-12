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
	ar -rcs libemagent.a *.o  
		
clean:
	rm -r ./*.o
	rm -r ./*.a