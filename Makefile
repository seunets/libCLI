LIB = CLI

SRCS = CLI.c Command.c Flag.c Argument.c

MAN=

CFLAGS += -Iincludes -Wall -pedantic -Weverything

.include <bsd.lib.mk>
