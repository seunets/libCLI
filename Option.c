#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "Option.h"


typedef struct
{
   Option_t interface;
   char *name;
   char *description;
   char shortName;
   char *value;
   bool isSet;
   bool required;
} Implementation;


static const char * getName( const Option_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> name;
}


static const char * getDescription( const Option_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> description;
}


static char getShortName( const Option_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return '\0';
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> shortName;
}


static bool isSet( const Option_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return false;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> isSet;
}


static bool isRequired( const Option_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return false;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> required;
}


static const char * getValue( const Option_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> value;
}


static void setValue( const Option_t *self, const char *value )
{
Implementation *impl;

   if( self == NULL )
   {
      return;
   }

   impl = __containerof( self, Implementation, interface );

   free( impl-> value );
   if( value != NULL )
   {
      impl-> value = strdup( value );
      if( impl-> value == NULL )
      {
         return;
      }
      impl-> isSet = true;
   }
   else
   {
      impl-> value = NULL;
      impl-> isSet = false;
   }
}


static void delete( Option_t **selfPtr )
{
Implementation *impl;

   if( selfPtr == NULL || *selfPtr == NULL )
   {
      return;
   }

   impl = __containerof( *selfPtr, Implementation, interface );
   free( impl-> name );
   free( impl-> description );
   free( impl-> value );
   free( impl );
   *selfPtr = NULL;
}


Option_t * newOption( const char *name, char shortName, const char *description, bool required )
{
Implementation *self;

   if( name == NULL || ( self = calloc( 1, sizeof( Implementation ) ) ) == NULL )
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
   self-> required = required;

   self-> interface.getName = getName;
   self-> interface.getDescription = getDescription;
   self-> interface.getShortName = getShortName;
   self-> interface.isSet = isSet;
   self-> interface.isRequired = isRequired;
   self-> interface.getValue = getValue;
   self-> interface.setValue = setValue;
   self-> interface.delete = delete;

   return &self-> interface;
}
