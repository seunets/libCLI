#ifndef LIBCLI_ARGUMENT_H
#define LIBCLI_ARGUMENT_H


#include <stdbool.h>


typedef struct Argument
{
   const char * ( *getName )( const struct Argument * );
   const char * ( *getDescription )( const struct Argument * );
   const char * ( *getValue )( const struct Argument * );
   bool ( *isRequired )( const struct Argument * );
   void ( *setValue )( const struct Argument *, const char * );
   void ( *delete )( struct Argument * );
   void *private;
} Argument_t;

Argument_t * newArgument( const char *, const char *, bool );

#endif
