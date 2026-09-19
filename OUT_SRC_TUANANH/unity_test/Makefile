.PHONY: all clean

CC ?= gcc
CFLAGS = -Wall -Wextra -O2 -I.
LDFLAGS = -lm

TARGET = run_test_bin
SRCS = unity.c mock_hal.c test_tuananh_firmware.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c unity.h mock_hal.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
