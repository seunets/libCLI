#ifndef LIBCLI_ARGUMENT_H
#define LIBCLI_ARGUMENT_H


#include <stdbool.h>


typedef struct Argument
{
   const char * ( *getName )( const struct Argument * );
   const char * ( *getDescription )( const struct Argument * );
   const char * ( *getValue )( const struct Argument * );
   bool ( *isRequired )( const struct Argument * );
   bool ( *isSet )( const struct Argument * );
   void ( *setValue )( const struct Argument *, const char * );
   void ( *delete )( struct Argument ** );
} Argument_t;


Argument_t * newArgument( const char *name, const char *description, bool required );

#endif
