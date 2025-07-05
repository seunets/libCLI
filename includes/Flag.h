#ifndef FLAG_H
#define FLAG_H


#include <stdbool.h>


typedef struct Flag
{
   char *name;
   char *description;
   char shortName;
   bool isSet;
   char pad[ 6 ];
   void ( *set )( struct Flag * );
   void ( *delete )( struct Flag * );
} Flag_t;


Flag_t * newFlag( const char *, char, const char * );

#endif 
