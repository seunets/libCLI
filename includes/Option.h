#ifndef LIBCLI_OPTION_H
#define LIBCLI_OPTION_H


#include <stdbool.h>


typedef struct Option
{
   const char * ( *getName )( const struct Option * );
   const char * ( *getDescription )( const struct Option * );
   char ( *getShortName )( const struct Option * );
   bool ( *isSet )( const struct Option * );
   bool ( *isRequired )( const struct Option * );
   const char * ( *getValue )( const struct Option * );
   void ( *setValue )( const struct Option *, const char *value );
   void ( *delete )( struct Option ** );
} Option_t;


Option_t *newOption( const char *name, char shortName, const char *description, bool required );

#endif
