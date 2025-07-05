#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "Errors.h"
#include "Argument.h"


static int setValue( Argument_t *self, const char *value )
{
   if( self-> value != NULL )
   {
      free( self-> value );
      return CLI_ERROR_ALREADY_EXISTS;
   }

   if( ( self-> value = strdup( value ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   return CLI_SUCCESS;
}


static void delete( Argument_t *self )
{
   if( self == NULL )
   {
      return;
   }

   free( self-> name );
   free( self-> description );
   free( self-> value );
   free( self );
}


Argument_t * newArgument( const char *name, const char *description, bool required )
{
Argument_t *self;

   if( ( self = calloc( 1, sizeof( Argument_t ) ) ) == NULL )
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
   self-> setValue = setValue;
   self-> delete = delete;

   return self;
}
