CC = gcc
LD = gcc

CFLAGS = -Wall -I include/ 
LDFLAGS = -Wall -I include/

SRCS = main.c init.c 
OBJECTS = $(SRCS:.c=.o)
EXEC = server 

OBJS = $(addprefix ./obj/, $(OBJECTS))

$(EXEC): $(OBJS)
	$(LD) $(OBJS) -o $@ $(LDFLAGS)

./obj/%.o: ./src/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJS) $(EXEC)
