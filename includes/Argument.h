#ifndef ARGUMENT_H
#define ARGUMENT_H


#include <stdbool.h>


typedef struct Argument
{
   char *name;
   char *description;
   char *value;
   bool required;
   char pad[ 7 ];
   int ( *setValue )( struct Argument *, const char * );
   void ( *delete )( struct Argument * );
} Argument_t;


Argument_t * newArgument( const char *, const char *, bool );

#endif
