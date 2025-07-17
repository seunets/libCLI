#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"


#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "CommandContext.h"
#include "Command.h"
#include "Argument.h"
#include "Flag.h"


struct privateData
{
   struct Command *command;
   Argument_t **arguments;
   int argumentCount;
   Flag_t **flags;
   int flagCount;
};


static const char * getArgument( const CommandContext_t *self, const char *name )
{
struct privateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return NULL;
   }

   private = self-> private;
   for( int i = 0; i < private-> argumentCount; i++ )
   {
      if( strcmp( private-> arguments[ i ]-> getName( private-> arguments[ i ] ), name ) == 0 )
      {
         return private-> arguments[ i ]-> getValue( private-> arguments[ i ] );
      }
   }

   return NULL;
}


static bool getFlag( const CommandContext_t *self, const char *name )
{
struct privateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return false;
   }

   private = self-> private;
   for( int i = 0; i < private-> flagCount; i++ )
   {
      if( strcmp( private-> flags[ i ]-> getName( private-> flags[ i ] ), name ) == 0 )
      {
         return private-> flags[ i ]-> isSet( private-> flags[ i ] );
      }
   }

   return false;
}


static void delete( CommandContext_t *self )
{
   if( self == NULL )
   {
      return;
   }

   if( self-> private != NULL )
   {
      free( self-> private );
   }
   free( self );
}


CommandContext_t * newCommandContext( struct Command *cmd, Argument_t **arguments, int argumentCount, Flag_t **flags, int flagCount )
{
CommandContext_t *self;
struct privateData *private;

   if( cmd == NULL )
   {
      return NULL;
   }

   if( ( self = calloc( 1, sizeof( CommandContext_t ) ) ) == NULL )
   {
      return NULL;
   }

   if( ( private = calloc( 1, sizeof( struct privateData ) ) ) == NULL )
   {
      free( self );
      return NULL;
   }

   private-> command = cmd;
   private-> arguments = arguments;
   private-> argumentCount = argumentCount;
   private-> flags = flags;
   private-> flagCount = flagCount;
   self-> private = private;
   self-> getArgument = getArgument;
   self-> getFlag = getFlag;
   self-> delete = delete;

   return self;
} 
