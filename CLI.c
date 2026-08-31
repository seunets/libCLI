#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "CLI.h"
#include "Command.h"
#include "Argument.h"
#include "Flag.h"


typedef struct
{
   CLI_t interface;
   Command_t *rootCommand;
} Implementation;


typedef struct
{
   const char *token;
   Command_t *found;
} SearchContext_t;


static bool findSubCmd( Command_t *sub, void *context )
{
SearchContext_t *ctx = ( SearchContext_t * ) context;

   if( strcmp( sub-> getName( sub ), ctx-> token ) == 0 )
   {
      ctx-> found = sub;
      return false;
   }

   return true;
}


static Command_t * resolveCommandPath( Command_t *root, const char *path )
{
char *pathCopy;
const char *token;
char *savePtr;
Command_t *current;
SearchContext_t searchCtx;

   if( root == NULL || path == NULL || *path == '\0' )
   {
      return root;
   }

   if( ( pathCopy = strdup( path ) ) == NULL )
   {
      return NULL;
   }

   token = strtok_r( pathCopy, " ", &savePtr );
   current = root;
   while( token != NULL && current != NULL )
   {
      searchCtx.token = token;
      searchCtx.found = NULL;
      current-> forEachSubCommand( current, findSubCmd, &searchCtx );
      current = searchCtx.found;
      if( current == NULL )
      {
         free( pathCopy );
         return NULL;
      }
      token = strtok_r( NULL, " ", &savePtr );
   }
   free( pathCopy );

   return current;
}


static int addCommand( const CLI_t *self, const char *name, const char *description, int ( *handler )( const CommandContext_t * ) )
{
Implementation *impl;
Command_t *cmd;

   if( self == NULL || name == NULL )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }

   if( ( cmd = newCommand( name, description, handler ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   impl = __containerof( self, Implementation, interface );
   if( impl-> rootCommand-> addSubCommand( impl-> rootCommand, cmd ) != CLI_SUCCESS )
   {
      cmd-> delete( &cmd );
      return CLI_ERROR_MEMORY;
   }

   return CLI_SUCCESS;
}


static int addSubCommand( const CLI_t *self, const char *parentPath, const char *name, const char *description, int ( *handler )( const CommandContext_t * ) )
{
Command_t *parent, *sub;

   if( self == NULL || parentPath == NULL || *parentPath == '\0' || name == NULL || description == NULL || handler == NULL )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }
   else
   {
   Implementation *impl = __containerof( self, Implementation, interface );

      if( ( parent = resolveCommandPath( impl-> rootCommand, parentPath ) ) == NULL )
      {
         return CLI_ERROR_NOT_FOUND;
      }
   }

   if( ( sub = newCommand( name, description, handler ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   if( parent-> addSubCommand( parent, sub ) != CLI_SUCCESS )
   {
      sub-> delete( &sub );
      return CLI_ERROR_MEMORY;
   }

   return CLI_SUCCESS;
}


static int addArgument( const CLI_t *self, const char *path, const char *name, const char *description, bool required )
{
Implementation *impl;
Command_t *cmd;
Argument_t *arg;

   if( self == NULL || name == NULL )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }

   impl = __containerof( self, Implementation, interface );
   if( path != NULL && *path != '\0' )
   {
      cmd = resolveCommandPath( impl-> rootCommand, path );
   }
   else
   {
      cmd = impl-> rootCommand;
   }

   if( cmd == NULL )
   {
      return CLI_ERROR_NOT_FOUND;
   }

   if( ( arg = newArgument( name, description, required ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   if( cmd-> addArgument( cmd, arg ) != CLI_SUCCESS )
   {
      arg-> delete( &arg );
      return CLI_ERROR_MEMORY;
   }

   return CLI_SUCCESS;
}


static int addFlag( const CLI_t *self, const char *path, const char *name, char shortName, const char *description )
{
Implementation *impl = __containerof( self, Implementation, interface );
Command_t *cmd;
Flag_t *flag;

   if( self == NULL || path == NULL || name == NULL )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }

   impl = __containerof( self, Implementation, interface );
   if( path != NULL && *path != '\0' )
   {
      cmd = resolveCommandPath( impl-> rootCommand, path );
   }
   else
   {
      cmd = impl-> rootCommand;
   }

   if( cmd == NULL )
   {
      return CLI_ERROR_NOT_FOUND;
   }

   if( ( flag = newFlag( name, shortName, description ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   if( cmd-> addFlag( cmd, flag ) != CLI_SUCCESS )
   {
      flag-> delete( &flag );
      return CLI_ERROR_MEMORY;
   }

   return CLI_SUCCESS;
}


static int parse( const CLI_t *self, int argc, char *argv[] )
{
Implementation *impl;

   if( self == NULL )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }

   impl = __containerof( self, Implementation, interface );
   if( impl-> rootCommand != NULL && impl-> rootCommand-> parse != NULL )
   {
      return impl-> rootCommand-> parse( impl-> rootCommand, argc, argv );
   }

   return CLI_ERROR_INVALID_ARGUMENT;
}


static void delete( CLI_t **selfPtr )
{
Implementation *impl;
CLI_t *self;

   if( selfPtr == NULL || *selfPtr == NULL )
   {
      return;
   }

   self = *selfPtr;

   if( ( impl = __containerof( self, Implementation, interface ) ) != NULL )
   {
      if( impl-> rootCommand != NULL )
      {
         impl-> rootCommand-> delete( &impl-> rootCommand );
      }
      free( impl );
   }
   *selfPtr = NULL;
}


__attribute__( ( visibility( "default" ) ) ) CLI_t * newCLI( const char *description )
{
Implementation *self;

   if( ( self = calloc( 1, sizeof( Implementation ) ) ) == NULL )
   {
      return NULL;
   }

   self-> interface.addCommand = addCommand;
   self-> interface.addSubCommand = addSubCommand;
   self-> interface.addArgument = addArgument;
   self-> interface.addFlag = addFlag;
   self-> interface.parse = parse;
   self-> interface.delete = delete;
   self-> rootCommand = newCommand( getprogname(), description, NULL );

   return &self-> interface;
}
