#ifndef LIBCLI_COMMANDCONTEXT_H
#define LIBCLI_COMMANDCONTEXT_H


#include <stdbool.h>
#include "Argument.h"
#include "Flag.h"
#include "Option.h"


struct Command;


typedef struct CommandContext
{
   const char * ( *getArgument )( const struct CommandContext *, const char *name );
   bool ( *getFlag )( const struct CommandContext *, const char *name );
   const char * ( *getOption )( const struct CommandContext *, const char *name );
   struct Command * ( *getCommand )( const struct CommandContext * );
   void ( *delete )( struct CommandContext ** );
} CommandContext_t;


CommandContext_t * newCommandContext( struct Command *cmd, Argument_t **arguments, int argumentCount, Flag_t **flags, int flagCount, Option_t **options, int optionCount );

#endif
