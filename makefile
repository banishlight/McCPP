CC = mvcc

CC_FLAGS = -Wall -Wextra


OBJS = 

LFLAGS = -lm


.PHONY: all

all: 

McServer: 

MinecraftServer.o: MinecraftServer.cpp
	$(CC) -c MinecraftServer.cpp -o MinecraftServer.o $(CC_FLAGS)