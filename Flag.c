#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Flag.h"


typedef struct privateData
{
   char *name;
   char *description;
   char shortName;
   bool isSet;
} PrivateData;


static const char * getName( const Flag_t *self )
{
const PrivateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return NULL;
   }

   private = self-> private;
   return private-> name;
}


static const char * getDescription( const Flag_t *self )
{
const PrivateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return NULL;
   }

   private = self-> private;
   return private-> description;
}


static char getShortName( const Flag_t *self )
{
const PrivateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return '\0';
   }

   private = self-> private;
   return private-> shortName;
}


static bool isSet( const Flag_t *self )
{
const PrivateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return false;
   }

   private = self-> private;
   return private-> isSet;
}


static void set( const Flag_t *self )
{
PrivateData *private = self-> private;

   private-> isSet = true;
}


static void delete( Flag_t *self )
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
      free( private );
   }
   free( self );
}


Flag_t * newFlag( const char *name, char shortName, const char *description )
{
Flag_t *self;
PrivateData *private;

   if( ( self = calloc( 1, sizeof( Flag_t ) ) ) == NULL )
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
   private-> shortName = shortName;
   self-> private = private;
   self-> getName = getName;
   self-> getDescription = getDescription;
   self-> getShortName = getShortName;
   self-> isSet = isSet;
   self-> set = set;
   self-> delete = delete;

   return self;
}
