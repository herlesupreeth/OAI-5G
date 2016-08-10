CC=gcc

LIBS=-lpthread -lprotobuf-c

PROTOP=../../proto

all:
	$(CC) -I$(EMPATH) -I$(EMTECH) -I$(PROTOP)/pb -o uctrl 		\
		$(PROTOP)/pb/main.pb-c.c				\
		$(PROTOP)/pb/hello.pb-c.c				\
		$(PROTOP)/pb/statistics.pb-c.c				\
		uctrl.c							\
		$(LIBS) 