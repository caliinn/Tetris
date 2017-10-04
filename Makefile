CC := gcc

CFLAGS := `sdl2-config --cflags --libs` -ggdb3 -O0 --std=c11 -Wall -lSDL2_ttf

HDRS :=

SRCS := main.c

OBJS := $(SRCS:.c=.o)

EXEC := tetris

all: $(EXEC)

$(EXEC): $(OBJS) $(HDRS) Makefile
	$(CC) -o $@ $(OBJS) $(CFLAGS)
