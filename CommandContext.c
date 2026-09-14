#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "CommandContext.h"
#include "Command.h"
#include "Argument.h"
#include "Flag.h"
#include "Option.h"


typedef struct
{
   CommandContext_t interface;
   struct Command *command;
   Argument_t **arguments;
   Flag_t **flags;
   Option_t **options;
   int argumentCount;
   int flagCount;
   int optionCount;
} Implementation;


static const char * getArgument( const CommandContext_t *self, const char *name )
{
Implementation *impl;

   if( self == NULL || name == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );

   for( int i = 0; i < impl-> argumentCount; i++ )
   {
      if( strcmp( impl-> arguments[ i ]-> getName( impl-> arguments[ i ] ), name ) == 0 )
      {
         return impl-> arguments[ i ]-> getValue( impl-> arguments[ i ] );
      }
   }

   return NULL;
}


static bool getFlag( const CommandContext_t *self, const char *name )
{
Implementation *impl;

   if( self == NULL || name == NULL )
   {
      return false;
   }

   impl = __containerof( self, Implementation, interface );

   for( int i = 0; i < impl-> flagCount; i++ )
   {
      if( strcmp( impl-> flags[ i ]-> getName( impl-> flags[ i ]), name ) == 0 )
      {
         return impl-> flags[ i ]-> isSet( impl-> flags[ i ] );
      }
   }

   return false;
}


static const char * getOption( const CommandContext_t *self, const char *name )
{
Implementation *impl;

   if( self == NULL || name == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );

   for( int i = 0; i < impl-> optionCount; i++ )
   {
      if( strcmp( impl-> options[ i ]-> getName( impl-> options[ i ] ), name ) == 0 )
      {
         return impl-> options[ i ]-> getValue( impl-> options[ i ] );
      }
   }

   return NULL;
}


static struct Command * getCommand( const CommandContext_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> command;
}


static void delete( CommandContext_t **selfPtr )
{
Implementation *impl;

   if( selfPtr == NULL || *selfPtr == NULL )
   {
      return;
   }

   impl = __containerof( *selfPtr, Implementation, interface );
   free( impl );
   *selfPtr = NULL;
}


CommandContext_t * newCommandContext( struct Command *cmd, Argument_t **arguments, int argumentCount, Flag_t **flags, int flagCount, Option_t **options, int optionCount )
{
Implementation *self;

   if( cmd == NULL )
   {
      return NULL;
   }

   if( ( self = calloc( 1, sizeof( Implementation ) ) ) == NULL )
   {
      return NULL;
   }

   self-> command = cmd;
   self-> arguments = arguments;
   self-> argumentCount = argumentCount;
   self-> flags = flags;
   self-> flagCount = flagCount;
   self-> options = options;
   self-> optionCount = optionCount;

   self-> interface.getArgument = getArgument;
   self-> interface.getFlag = getFlag;
   self-> interface.getOption = getOption;
   self-> interface.getCommand = getCommand;
   self-> interface.delete = delete;

   return &self-> interface;
}
