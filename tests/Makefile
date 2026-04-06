CC = gcc
CFLAGS = -I../main -I./unity

SRC = \
    ../main/state_machine_logic.c \
    ../main/event_flags.c \
    ../main/mock_gpio.c \
    ../main/mock_timer.c

TEST = test_state_machine.c test_runner.c

all:
	$(CC) $(CFLAGS) $(SRC) $(TEST) unity/unity.c -o test_runner
	./test_runner

clean:
	rm -f test_runner
