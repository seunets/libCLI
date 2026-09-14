#ifndef LIBCLI_COMMAND_H
#define LIBCLI_COMMAND_H


#include <stdbool.h>
#include <stdio.h>
#include "CommandContext.h"
#include "Argument.h"
#include "Flag.h"
#include "Option.h"


typedef struct Command
{
   int ( *addSubCommand )( struct Command *, struct Command * );
   int ( *addArgument )( const struct Command *, Argument_t * );
   int ( *addFlag )( const struct Command *, Flag_t * );
   int ( *addOption )( const struct Command *, Option_t * );
   int ( *parse )( struct Command *, int, char *[] );
   void ( *delete )( struct Command ** );

   const char * ( *getName )( const struct Command * );
   const char * ( *getDescription )( const struct Command * );
   const char * ( *getArgumentValue )( const struct Command *, const char * );

   Argument_t ** ( *getArguments )( const struct Command * );
   int ( *getArgumentCount )( const struct Command * );
   Flag_t ** ( *getFlags )( const struct Command * );
   int ( *getFlagCount )( const struct Command * );
   Option_t ** ( *getOptions )( const struct Command * );
   int ( *getOptionCount )( const struct Command * );
   struct Command ** ( *getSubCommands )( const struct Command * );
   int ( *getSubCommandCount )( const struct Command * );

   void ( *printHelp )( const struct Command *, FILE * );

   struct Command * ( *getParent )( const struct Command * );
   void ( *setParent )( struct Command *, struct Command * );
} Command_t;


Command_t * newCommand( const char *name, const char *description, int ( *handler )( const CommandContext_t * ) );

#endif
