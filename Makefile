LIB = CLI

SRCS = CLI.c Command.c CommandContext.c Flag.c Argument.c

MAN=

CFLAGS += -Iincludes -Wall -pedantic -Weverything -Wno-gnu-statement-expression-from-macro-expansion -Wno-unsafe-buffer-usage

.include <bsd.lib.mk>
