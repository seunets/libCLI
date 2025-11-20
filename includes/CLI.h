#ifndef LIBCLI_CLI_H
#define LIBCLI_CLI_H


#include "Command.h"


#define CLI_SUCCESS                   0
#define CLI_ERROR_MEMORY             -1
#define CLI_ERROR_INVALID_ARGUMENT   -2
#define CLI_ERROR_NOT_FOUND          -3
#define CLI_ERROR_ALREADY_EXISTS     -4
#define CLI_ERROR_PARSE_FAILED       -5
#define CLI_ERROR_CONTEXT_FAILED     -6


typedef struct CLI
{
   int ( *addCommand )( const struct CLI *, const char *, const char *, int ( * )( const struct CommandContext * ) );
   int ( *addSubCommand )( const struct CLI *, const char *, const char *, const char *, int ( * )( const struct CommandContext * ) );
   int ( *addArgument )( const struct CLI *, const char *, const char *, const char *, bool );
   int ( *addFlag )( const struct CLI *, const char *, const char *, char, const char * );
   int ( *parse )( const struct CLI *, int, char *[] );
   void ( *delete )( struct CLI * );
   void *private;
} CLI_t;

CLI_t * newCLI( const char * );


#endif
