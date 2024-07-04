CC = g++
CC_FLAGS = -Wall -Wextra

NVCC = nvcc
NVCC_FLAGS = 
NV_LIB = -lcuda -lcudart

INCLUDES = include/ include/network/
SRC = MinecraftServer.cpp network/VarIntLong.cpp Position.cpp
OBJS = $(SRCS:.cpp=.o)

LFLAGS = -lm -lboost_system
FILES = server.properties
MAIN = McCPP

.PHONY: clean depend

all: $(MAIN)
	@echo Compiling the Server!

$(MAIN): $(OBJS)
	$(CC) $(OBJS) -o $(MAIN) $(LFLAGS) $(CC_FLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@


clean: 
	$(RM) *.o *~ $(MAIN) $(FILES) 

depend: $(SRCS)
    makedepend $(INCLUDES) $^
