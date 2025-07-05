#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Flag.h"


static void set( Flag_t *self )
{
   self-> isSet = true;
}


static void delete( Flag_t *self )
{
   if( self == NULL )
   {
      return;
   }

   free( self-> name );
   free( self-> description );
   free( self );
}


Flag_t * newFlag( const char *name, char shortName, const char *description )
{
Flag_t *self;

   if( ( self = calloc( 1, sizeof( Flag_t ) ) ) == NULL )
   {
      return NULL;
   }

   if( ( self-> name = strdup( name ) ) == NULL )
   {
      free( self );
      return NULL;
   }

   if( description != NULL )
   {
      if( ( self-> description = strdup( description ) ) == NULL )
      {
         free( self-> name );
         free( self );
         return NULL;
      }
   }
   self-> shortName = shortName;
   self-> set = set;
   self-> delete = delete;

   return self;
}
