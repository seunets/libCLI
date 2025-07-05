#ifndef COMMAND_H
#define COMMAND_H


#include <stdbool.h>
#include "Argument.h"
#include "Flag.h"


typedef struct CommandContext CommandContext_t;


typedef struct Command
{
   char *name;
   char *description;
   int ( *handler )( struct CommandContext * );
   struct Command *parent;
   struct Command **subCommands;
   Argument_t **arguments;
   Flag_t **flags;
   int ( *addSubCommand )( struct Command *, struct Command * );
   int ( *addArgument )( struct Command *, Argument_t * );
   int ( *addFlag )( struct Command *, Flag_t * );
   int ( *parse )( struct Command *, int, char *[] );
   void ( *delete )( struct Command * );
   const char * ( *getArgumentValue )( struct Command *, const char * );
   bool ( *hasArgument )( struct Command *, const char * );
   int subCommandCount;
   int argumentCount;
   int flagCount;
   char pad[ 4 ];
} Command_t;


struct CommandContext
{
   void *privateData;
   const char * ( *getArgument )( struct CommandContext *, const char * );
   bool ( *getFlag )( struct CommandContext *, const char * );
   Command_t * ( *getCommand )( struct CommandContext * );
   void ( *delete )( struct CommandContext * );
};


Command_t * newCommand( const char *, const char *, int ( *handler )( CommandContext_t * ) );

#endif
