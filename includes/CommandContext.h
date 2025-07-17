#ifndef LIBCLI_COMMANDCONTEXT_H
#define LIBCLI_COMMANDCONTEXT_H


#include <stdbool.h>
#include "Argument.h"
#include "Flag.h"

struct Command;


typedef struct CommandContext
{
   const char * ( *getArgument )( const struct CommandContext *, const char * );
   bool ( *getFlag )( const struct CommandContext *, const char * );
   void ( *delete )( struct CommandContext * );
   void *private;
} CommandContext_t;

CommandContext_t * newCommandContext( struct Command *, Argument_t **, int, Flag_t **, int );

#endif 
