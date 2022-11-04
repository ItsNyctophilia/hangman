.DEFAULT_GOAL := hangman
CFLAGS += -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal

.PHONY: debug
debug: CFLAGS += -g
debug: hangman

.PHONY: clean
clean:
	${RM} hangman
