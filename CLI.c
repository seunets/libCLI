#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "CLI.h"
#include "Command.h"
#include "Argument.h"
#include "Flag.h"


typedef struct privateData
{
   Command_t *rootCommand;
} PrivateData;


static const char *findSubCmdToken = NULL;
static Command_t *findSubCmdFound = NULL;


static bool findSubCmd( Command_t *sub )
{
   if( strcmp( sub-> getName( sub ), findSubCmdToken ) == 0 )
   {
      findSubCmdFound = sub;
      return false;
   }

   return true;
}


static Command_t * resolveCommandPath( Command_t *root, const char *path )
{
char *pathCopy;
const char *token;
Command_t *current;

   if( root == NULL || path == NULL || *path == '\0' )
   {
      return root;
   }

   if( ( pathCopy = strdup( path ) ) == NULL )
   {
      return NULL;
   }

   token = strtok( pathCopy, " " );
   current = root;
   while( token != NULL && current != NULL )
   {
      findSubCmdToken = token;
      findSubCmdFound = NULL;
      current-> forEachSubCommand( current, findSubCmd );
      current = findSubCmdFound;
      if( current == NULL )
      {
         free( pathCopy );
         return NULL;
      }
      token = strtok( NULL, " " );
   }
   free( pathCopy );

   return current;
}


static int addCommand( const CLI_t *self, const char *name, const char *description, int ( *handler )( const CommandContext_t * ) )
{
PrivateData *private = self-> private;
Command_t *cmd;

   if( ( cmd = newCommand( name, description, handler ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   if( private-> rootCommand-> addSubCommand( private-> rootCommand, cmd ) != CLI_SUCCESS )
   {
      cmd-> delete( cmd );
      return CLI_ERROR_MEMORY;
   }

   return CLI_SUCCESS;
}


static int addSubCommand( const CLI_t *self, const char *parentPath, const char *name, const char *description, int ( *handler )( const CommandContext_t * ) )
{
PrivateData *private = self-> private;
Command_t *parent, *sub;

   if( parentPath == NULL || *parentPath == '\0' )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }
   else
   {
      if( ( parent = resolveCommandPath( private-> rootCommand, parentPath ) ) == NULL )
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
      sub-> delete( sub );
      return CLI_ERROR_MEMORY;
   }

   return CLI_SUCCESS;
}


static int addArgument( const CLI_t *self, const char *path, const char *name, const char *description, bool required )
{
PrivateData *private = self-> private;
Command_t *cmd;
Argument_t *arg;

   if( path != NULL && *path != '\0' )
   {
      cmd = resolveCommandPath( private-> rootCommand, path );
   }
   else
   {
      cmd = private-> rootCommand;
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
      arg-> delete( arg );
      return CLI_ERROR_MEMORY;
   }

   return CLI_SUCCESS;
}


static int addFlag( const CLI_t *self, const char *path, const char *name, char shortName, const char *description )
{
PrivateData *private = self-> private;
Command_t *cmd;
Flag_t *flag;

   if( path != NULL && *path != '\0' )
   {
      cmd = resolveCommandPath( private-> rootCommand, path );
   }
   else
   {
      cmd = private-> rootCommand;
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
      flag-> delete( flag );
      return CLI_ERROR_MEMORY;
   }

   return CLI_SUCCESS;
}


static int parse( const CLI_t *self, int argc, char *argv[] )
{
PrivateData *private = self-> private;

   if( private-> rootCommand != NULL && private-> rootCommand-> parse != NULL )
   {
      return private-> rootCommand-> parse( private-> rootCommand, argc, argv );
   }

   return CLI_ERROR_INVALID_ARGUMENT;
}


static void delete( CLI_t *self )
{
PrivateData *private;

   if( self == NULL )
   {
      return;
   }

   if( ( private = self-> private ) != NULL )
   {
      if( private-> rootCommand != NULL )
      {
         private-> rootCommand-> delete( private-> rootCommand );
      }
      free( private );
   }
   free( self );
}


CLI_t * newCLI( const char *description )
{
CLI_t *self;
PrivateData *private;

   if( ( self = calloc( 1, sizeof( CLI_t ) ) ) == NULL )
   {
      return NULL;
   }

   if( ( private = calloc( 1, sizeof( PrivateData ) ) ) == NULL )
   {
      free( self );
      return NULL;
   }

   self-> private = private;
   self-> addCommand = addCommand;
   self-> addSubCommand = addSubCommand;
   self-> addArgument = addArgument;
   self-> addFlag = addFlag;
   self-> parse = parse;
   self-> delete = delete;
   private-> rootCommand = newCommand( getprogname(), description, NULL );

   return self;
}
