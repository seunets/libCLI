LIB = CLI

SHLIB_MAJOR = 1

SRCS = CLI.c Command.c CommandContext.c Flag.c Option.c Argument.c

MAN=

CFLAGS += -Iincludes -Wall -pedantic -Weverything -Wno-gnu-statement-expression-from-macro-expansion -Wno-unsafe-buffer-usage -fvisibility=hidden

.include <bsd.lib.mk>
