CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=c11

SRCS = main.c julia_cpu.c julia_opencl.c image.c timer.c

ifeq ($(OS),Windows_NT)
    TARGET = julia.exe
    OBJS = $(SRCS:.c=.o)
    LDFLAGS = -lOpenCL

    RM = del /Q
    CLEANFILES = $(OBJS) $(TARGET)
else
    TARGET = julia
    OBJS = $(SRCS:.c=.o)
    LDFLAGS = -lOpenCL

    RM = rm -f
    CLEANFILES = $(OBJS) $(TARGET)
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-$(RM) $(CLEANFILES)