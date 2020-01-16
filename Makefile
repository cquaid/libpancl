src = \
	$(wildcard \
		*.c \
		lexer/*.c \
		parser/*.c \
		types/*.c)
obj = $(patsubst %.c,%.o,$(src))
CFLAGS = -Wall -Werror -ggdb3 -g -oO -I. -Iinclude

.PHONY: all
all: $(obj)
	$(CC) -o pancl $(obj) -Wall -Werror

.PHONY: clean
clean:
	rm -f $(obj) pancl

