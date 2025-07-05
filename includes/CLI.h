#ifndef CLI_H
#define CLI_H


#include "Errors.h"
#include "Command.h"


typedef struct CLI
{
   Command_t *rootCommand;
   int ( *addCommand )( struct CLI *, const char *, const char *, int ( *handler )( CommandContext_t * ) );
   int ( *addSubCommand )( struct CLI *, const char *, const char *, const char *, int ( *handler )( CommandContext_t * ) );
   int ( *addArgument )( struct CLI *, const char *, const char *, const char *, bool );
   Flag_t * ( *addFlag )( Command_t *, const char *, char, const char * );
   int ( *parse )( struct CLI *, int, char *[] );
   void ( *delete )( struct CLI * );
} CLI_t;


CLI_t * newCLI( void );

#endif
