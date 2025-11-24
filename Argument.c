#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "Argument.h"


typedef struct privateData
{
   char *name;
   char *description;
   char *value;
   bool required;
} PrivateData;


static const char * getName( const Argument_t *self )
{
const PrivateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return NULL;
   }

   private = self-> private;
   return private-> name;
}


static const char * getDescription( const Argument_t *self )
{
const PrivateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return NULL;
   }

   private = self-> private;
   return private-> description;
}


static const char * getValue( const Argument_t *self )
{
const PrivateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return NULL;
   }

   private = self-> private;
   return private-> value;
}


static bool isRequired( const Argument_t *self )
{
const PrivateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return false;
   }

   private = self-> private;
   return private-> required;
}


static void setValue( const Argument_t *self, const char *value )
{
PrivateData *private = NULL;

   if( self == NULL || self-> private == NULL )
   {
      return;
   }

   if( ( private = self-> private ) != NULL )
   {
      free( private-> value );
   }
   if( value != NULL )
   {
      private-> value = strdup( value );
   }
   else
   {
      private-> value = NULL;
   }
}


static void delete( Argument_t *self )
{
PrivateData *private;

   if( self == NULL )
   {
      return;
   }

   if( ( private = self-> private ) != NULL )
   {
      free( private-> name );
      free( private-> description );
      free( private-> value );
      free( private );
   }
   free( self );
}


Argument_t * newArgument( const char *name, const char *description, bool required )
{
Argument_t *self;
PrivateData *private;

   if( ( self = calloc( 1, sizeof( Argument_t ) ) ) == NULL )
   {
      return NULL;
   }

   if( ( private = calloc( 1, sizeof( PrivateData ) ) ) == NULL )
   {
      free( self );
      return NULL;
   }

   if( ( private-> name = strdup( name ) ) == NULL )
   {
      free( private );
      free( self );
      return NULL;
   }

   if( description != NULL )
   {
      if( ( private-> description = strdup( description ) ) == NULL )
      {
         free( private-> name );
         free( private );
         free( self );
         return NULL;
      }
   }
   private-> required = required;
   self-> private = private;
   self-> getName = getName;
   self-> getDescription = getDescription;
   self-> getValue = getValue;
   self-> isRequired = isRequired;
   self-> setValue = setValue;
   self-> delete = delete;

   return self;
}
