#ifndef LIBCLI_FLAG_H
#define LIBCLI_FLAG_H


#include <stdbool.h>


typedef struct Flag
{
   const char * ( *getName )( const struct Flag * );
   const char * ( *getDescription )( const struct Flag * );
   char ( *getShortName )( const struct Flag * );
   bool ( *isSet )( const struct Flag * );
   void ( *set )( const struct Flag * );
   void ( *delete )( struct Flag * );
   void *private;
} Flag_t;

Flag_t * newFlag( const char *, char, const char * );

#endif 
