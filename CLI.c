#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "CLI.h"
#include "Command.h"
#include "Argument.h"
#include "Flag.h"
#include "Option.h"


typedef struct
{
   CLI_t interface;
   Command_t *rootCommand;
} Implementation;


static Command_t *resolveCommandPath( Command_t *root, const char *path )
{
char *copy;
const char *token;
char *savePtr;
Command_t *current;
Command_t **subs;
int count, i;

   if( root == NULL || path == NULL || *path == '\0' )
   {
      return root;
   }

   if( ( copy = strdup( path ) ) == NULL )
   {
      return NULL;
   }

   token = strtok_r( copy, " ", &savePtr );
   current = root;

   while( token != NULL && current != NULL )
   {
   Command_t *found = NULL;

      subs = current-> getSubCommands( current );
      count = current-> getSubCommandCount( current );

      for( i = 0; i < count; i++ )
      {
         if( strcmp( subs[ i ]-> getName( subs[ i ] ), token ) == 0 )
         {
            found = subs[ i ];
            break;
         }
      }

      current = found;
      if( current == NULL )
      {
         free( copy );
         return NULL;
      }

      token = strtok_r( NULL, " ", &savePtr );
   }

   free( copy );
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
Implementation *impl;

   if( self == NULL || parentPath == NULL || *parentPath == '\0' || name == NULL || description == NULL || handler == NULL )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }

   impl = __containerof( self, Implementation, interface );
   if( ( parent = resolveCommandPath( impl-> rootCommand, parentPath ) ) == NULL )
   {
      return CLI_ERROR_NOT_FOUND;
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
Implementation *impl;
Command_t *cmd;
Flag_t *flag;

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


static int addOption( const CLI_t *self, const char *path, const char *name, char shortName, const char *description, bool required )
{
Implementation *impl;
Command_t *cmd;
Option_t *option;

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

   if( ( option = newOption( name, shortName, description, required ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   if( cmd-> addOption( cmd, option ) != CLI_SUCCESS )
   {
      option-> delete( &option );
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
   impl = __containerof( self, Implementation, interface );

   if( impl-> rootCommand != NULL )
   {
      impl-> rootCommand-> delete( &impl-> rootCommand );
   }

   free( impl );
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
   self-> interface.addOption = addOption;
   self-> interface.parse = parse;
   self-> interface.delete = delete;

   self-> rootCommand = newCommand( getprogname(), description, NULL );
   if( self-> rootCommand == NULL )
   {
      free( self) ;
      return NULL;
   }

   return &self-> interface;
}
