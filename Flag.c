#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Flag.h"


typedef struct
{
   Flag_t interface;
   char *name;
   char *description;
   char shortName;
   bool isSet;
} Implementation;


static const char * getName( const Flag_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> name;
}


static const char * getDescription( const Flag_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> description;
}


static char getShortName( const Flag_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return '\0';
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> shortName;
}


static bool isSet( const Flag_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return false;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> isSet;
}


static void set( const Flag_t *self )
{
Implementation *impl = __containerof( self, Implementation, interface );

   impl-> isSet = true;
}


static void delete( Flag_t **selfPtr )
{
Implementation *impl;

   if( selfPtr == NULL || *selfPtr == NULL )
   {
      return;
   }

   impl = __containerof( *selfPtr, Implementation, interface );
   free( impl-> name );
   free( impl-> description );
   free( impl );
   *selfPtr = NULL;
}


Flag_t * newFlag( const char *name, char shortName, const char *description )
{
Implementation *self;

   if( ( self = calloc( 1, sizeof( Implementation ) ) ) == NULL )
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
   self-> interface.getName = getName;
   self-> interface.getDescription = getDescription;
   self-> interface.getShortName = getShortName;
   self-> interface.isSet = isSet;
   self-> interface.set = set;
   self-> interface.delete = delete;

   return &self-> interface;
}
