CC = g++

CC_FLAGS = -Wall -Wextra


OBJS = MinecraftServer.o

LFLAGS = -lm -lboost_system


.PHONY: all

all: McServer

McServer: $(OBJS)
	$(CC) $(OBJS) -o McCPP $(LFLAGS) $(CC_FLAGS)
MinecraftServer.o: 
	$(CC) -c MinecraftServer.cpp -o MinecraftServer.o $(CC_FLAGS) $(LFLAGS)