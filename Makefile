CC = gcc
LD = gcc

CFLAGS = -Wall -I include/ -D_POSIX_C_SOURCE
LDFLAGS = -Wall -I include/ -D_POSIX_C_SOURCE

SRCS = clients.c init.c ioloop.c logfile.c main.c protocol.c sharedmem.c
OBJECTS = $(SRCS:.c=.o)
EXEC = server 

OBJS = $(addprefix ./obj/, $(OBJECTS))

$(EXEC): $(OBJS)
	$(LD) $(OBJS) -o $@ $(LDFLAGS)

./obj/%.o: ./src/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJS) $(EXEC)
