#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "Argument.h"


typedef struct
{
   Argument_t interface;
   char *name;
   char *description;
   char *value;
   bool required;
} Implementation;


static const char * getName( const Argument_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> name;
}


static const char * getDescription( const Argument_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> description;
}


static const char * getValue( const Argument_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> value;
}


static bool isRequired( const Argument_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return false;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> required;
}


static void setValue( const Argument_t *self, const char *value )
{
Implementation *impl;

   if( self == NULL )
   {
      return;
   }

   if( ( impl = __containerof( self, Implementation, interface ) ) != NULL )
   {
      free( impl-> value );
   }
   if( value != NULL )
   {
      impl-> value = strdup( value );
   }
   else
   {
      impl-> value = NULL;
   }
}


static void delete( Argument_t **selfPtr )
{
Implementation *impl;

   if( selfPtr == NULL || *selfPtr == NULL )
   {
      return;
   }

   if( ( impl = __containerof( *selfPtr, Implementation, interface ) ) != NULL )
   {
      free( impl-> name );
      free( impl-> description );
      free( impl-> value );
      free( impl );
   }
   *selfPtr = NULL;
}


Argument_t * newArgument( const char *name, const char *description, bool required )
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
   self-> required = required;
   self-> interface.getName = getName;
   self-> interface.getDescription = getDescription;
   self-> interface.getValue = getValue;
   self-> interface.isRequired = isRequired;
   self-> interface.setValue = setValue;
   self-> interface.delete = delete;

   return &self-> interface;
}
