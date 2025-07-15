#ifndef LIBCLI_COMMAND_H
#define LIBCLI_COMMAND_H


#include <stdbool.h>
#include "CommandContext.h"


typedef struct Command
{
   int ( *addSubCommand )( struct Command *, struct Command * );
   int ( *addArgument )( const struct Command *, struct Argument * );
   int ( *addFlag )( const struct Command *, struct Flag * );
   int ( *parse )( struct Command *, int, char *[] );
   void ( *delete )( struct Command * );
   const char * ( *getName )( const struct Command * );
   const char * ( *getDescription )( const struct Command * );
   bool ( *hasArgument )( const struct Command *, const char * );
   const char * ( *getArgumentValue )( const struct Command *, const char * );
   Argument_t ** ( *getArguments )( const struct Command * );
   int ( *getArgumentCount )( const struct Command * );
   Flag_t ** ( *getFlags )( const struct Command * );
   int ( *getFlagCount )( const struct Command * );
   struct Command ** ( *getSubCommands )( const struct Command * );
   int ( *getSubCommandCount )( const struct Command * );
   void ( *printHelp )( struct Command * );
   void ( *forEachSubCommand )( struct Command *, bool( * )( struct Command * ) );
   void *private;
} Command_t;

Command_t * newCommand( const char *, const char *, int ( * )( const CommandContext_t * ) );

#endif
