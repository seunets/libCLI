#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "Errors.h"
#include "CLI.h"
#include "Argument.h"
#include "Flag.h"


static int findCommand( Command_t *const *restrict commands, int count, const char *restrict name )
{
   for( int i = 0; i < count; i++ )
   {
      if( strcmp( commands[ i ]-> name, name ) == 0 )
      {
         return i;
      }
   }

   return CLI_ERROR_NOT_FOUND;
}


static Command_t * resolveCommandPath( Command_t *root, const char *path )
{
char *pathCopy;
const char *token;
Command_t *current;

   if( root == NULL || path == NULL || *path == '\0' )
   {
      return NULL;
   }

   if( ( pathCopy = strdup( path ) ) == NULL )
   {
      fputs( "Error: Failed to allocate memory for path copy.\n", stderr );
      return NULL;
   }

   token = strtok( pathCopy, " " );
   current = root;

   while( token != NULL && current != NULL )
   {
   int i = findCommand( current-> subCommands, current-> subCommandCount, token );

      if( i < 0 )
      {
         free( pathCopy );
         return NULL;
      }
      current = current-> subCommands[ i ];
      token = strtok( NULL, " " );
   }
   free( pathCopy );

   return current;
}


static int addCommand( CLI_t *cli, const char *name, const char *description, int ( *handler )( CommandContext_t * ) )
{
Command_t *cmd;

   if( ( cmd = newCommand( name, description, handler ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   if( cli-> rootCommand-> addSubCommand( cli-> rootCommand, cmd ) != CLI_SUCCESS )
   {
      cmd-> delete( cmd );
      return CLI_ERROR_MEMORY;
   }

   return CLI_SUCCESS;
}


static int addSubCommand( CLI_t *cli, const char *parentPath, const char *name, const char *description, int ( *handler )( CommandContext_t * ) )
{
Command_t *parent, *sub;

   if( parentPath == NULL || *parentPath == '\0' )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }
   else
   {
      if( ( parent = resolveCommandPath( cli-> rootCommand, parentPath ) ) == NULL )
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


static int addArgument( CLI_t *cli, const char *path, const char *name, const char *description, bool required )
{
Command_t *cmd;
Argument_t *arg;

   cmd = ( path != NULL && *path != '\0' ) ? resolveCommandPath( cli-> rootCommand, path ) : cli-> rootCommand;
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


static int addFlag( CLI_t *cli, const char *path, const char *name, char shortName, const char *description )
{
Command_t *cmd;
Flag_t *flag;

   cmd = ( path != NULL && *path != '\0' ) ? resolveCommandPath( cli-> rootCommand, path ) : cli-> rootCommand;
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


static int parse( CLI_t *self, int argc, char *argv[] )
{
   if( self-> rootCommand != NULL && self-> rootCommand-> parse != NULL )
   {
      return self-> rootCommand-> parse( self-> rootCommand, argc, argv );
   }

   return CLI_ERROR_INVALID_ARGUMENT;
}


static void delete( CLI_t *self )
{
   if( self == NULL )
   {
      return;
   }

   if( self-> rootCommand != NULL )
   {
      self-> rootCommand-> delete( self-> rootCommand );
   }
   free( self );
}

CLI_t * newCLI( void )
{
CLI_t *self;

   if( ( self = calloc( 1, sizeof( CLI_t ) ) ) == NULL )
   {
      return NULL;
   }

   self-> addCommand = addCommand;
   self-> addSubCommand = addSubCommand;
   self-> addArgument = addArgument;
   self-> addFlag = addFlag;
   self-> parse = parse;
   self-> delete = delete;
   self-> rootCommand = newCommand( getprogname(), NULL, NULL );

   return self;
}
