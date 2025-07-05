#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "Command.h"
#include "Errors.h"


typedef struct CommandContextData
{
   int argumentCount;
   int flagCount;
   Argument_t **arguments;
   Flag_t **flags;
   Command_t *command;
} CommandContextData_t;


static int findArgument( Argument_t *const *restrict arguments, int count, const char *restrict name )
{
   for( int i = 0; i < count; i++ )
   {
      if( strcmp( arguments[ i ]-> name, name ) == 0 )
      {
         return i;
      }
   }

   return CLI_ERROR_NOT_FOUND;
}


static Argument_t * getCommandArgument( Command_t *cmd, const char *name )
{
int index;

   if( cmd == NULL || name == NULL )
   {
      return NULL;
   }

   if( ( index = findArgument( cmd-> arguments, cmd-> argumentCount, name ) ) >= 0 )
   {
      return cmd-> arguments[ index ];
   }

   return NULL;
}


static const char * getArgumentValue( Command_t *cmd, const char *name )
{
Argument_t *arg = getCommandArgument( cmd, name );

   return arg != NULL ? arg-> value : NULL;
}


static bool hasArgument( Command_t *cmd, const char *name )
{
   return getCommandArgument( cmd, name ) != NULL;
}


static Command_t * contextGetCommand( CommandContext_t *ctx )
{
CommandContextData_t *data;

   if( ctx == NULL || ctx-> privateData == NULL )
   {
      return NULL;
   }

   data = ( CommandContextData_t * ) ctx-> privateData;

   return data-> command;
}


static char * buildCommandPath( Command_t *cmd )
{
size_t len;
char *buf;
char *parentPath;

   if( cmd-> parent == NULL )
   {
   char *result;

      if( ( result = strdup( cmd-> name ) ) == NULL )
      {
         return NULL;
      }

      return result;
   }
   else
   {
      if( ( parentPath = buildCommandPath( cmd-> parent ) ) == NULL )
      {
         return NULL;
      }

      len = strlen( parentPath ) + 1 + strlen( cmd-> name ) + 1;
      if( ( buf = calloc( 1, len ) ) == NULL )
      {
         free( parentPath );
         return NULL;
      }

      snprintf( buf, len, "%s %s", parentPath, cmd-> name );
      free( parentPath );

      return buf;
   }
}


static void printHelp( Command_t *cmd )
{
char *path = buildCommandPath( cmd );
char *progname = NULL;
char *trimmedPath = path;
char *space;

   if( ( progname = strdup( cmd-> name ) ) == NULL )
   {
      return;
   }

   if( cmd-> parent == NULL )
   {
      fprintf( stderr, "Usage: %s [command] [options] [arguments]\n\n", progname );
      if( cmd-> description && cmd-> description[ 0 ] )
      {
         fprintf( stderr, "%s\n\n", cmd-> description );
      }

      fputs( "Commands:\n", stderr );
      for( int i = 0; i < cmd-> subCommandCount; i++ )
      {
      const Command_t *sub = cmd-> subCommands[ i ];

         fprintf( stderr, "  %-18s %s\n", sub-> name, sub-> description ? sub-> description : "" );
      }

      fputs( "\n", stderr );

      free( path );
      free( progname );

      return;
   }

   // At this point, cmd->parent is always non-NULL due to earlier check
   space = strchr( path, ' ' );
   if( space != NULL && *( space + 1 ) != '\0' )
   {
      trimmedPath = space + 1;
   }

   fprintf( stderr, "Help for: %s\n\n", trimmedPath );
   if( cmd-> description != NULL )
   {
      fprintf( stderr, "%s\n\n", cmd-> description );
   }

   fprintf( stderr, "Usage: %s", trimmedPath );
   for( int i = 0; i < cmd-> argumentCount; i++ )
   {
   const Argument_t *arg = cmd-> arguments[ i ];

      if( arg-> required )
      {
         fprintf( stderr, " <%s>", arg-> name );
      }
   }

   for( int i = 0; i < cmd-> argumentCount; i++ )
   {
   const Argument_t *arg = cmd-> arguments[ i ];

      if( !arg-> required )
      {
         fprintf( stderr, " [%s]", arg-> name );
      }
   }
   fputs( "\n\n", stderr );

   if( cmd-> argumentCount > 0 )
   {
      fputs( "Arguments:\n", stderr );
      for( int i = 0; i < cmd-> argumentCount; i++ )
      {
      const Argument_t *arg = cmd-> arguments[ i ];

         fprintf( stderr, "  %-12s %s%s", arg-> name, arg-> description, arg-> required ? " (required)" : "" );
         if( !arg-> required && arg-> value != NULL )
         {
            fprintf( stderr, " [default: %s]", arg-> value );
         }
         fputs( "\n", stderr );
      }
      fputs( "\n", stderr );
   }

   if( cmd-> flagCount > 0 )
   {
      fputs( "Flags:\n", stderr );
      for( int i = 0; i < cmd-> flagCount; i++ )
      {
      const Flag_t *flag = cmd-> flags[ i ];

         if( flag-> shortName != '\0' )
         {
            fprintf( stderr, "  -%c, --%-8s %s\n", flag-> shortName, flag-> name, flag-> description );
         }
         else
         {
            fprintf( stderr, "      --%-8s %s\n", flag-> name, flag-> description );
         }
      }
      fputs( "\n", stderr );
   }

   if( cmd-> subCommandCount > 0 )
   {
      fputs( "Subcommands:\n", stderr );
      for( int i = 0; i < cmd-> subCommandCount; i++ )
      {
      const Command_t *sub = cmd-> subCommands[ i ];

         fprintf( stderr, "  %-12s %s\n", sub-> name, sub-> description );
      }
      fputs( "\n", stderr );
   }

   free( progname );
   free( path );
}


static const char * contextGetArgument( CommandContext_t *ctx, const char *name )
{
CommandContextData_t *data;

   if( ctx == NULL || ctx-> privateData == NULL )
   {
      return NULL;
   }

   data = ( CommandContextData_t * ) ctx-> privateData;
   for( int i = 0; i < data-> argumentCount; i++ )
   {
      if( strcmp( data-> arguments[ i ]-> name, name ) == 0 )
      {
         return data-> arguments[ i ]-> value;
      }
   }

   return NULL;
}


static bool contextGetFlag( CommandContext_t *ctx, const char *name )
{
CommandContextData_t *data;

   if( ctx == NULL || ctx-> privateData == NULL )
   {
      return false;
   }

   data = ( CommandContextData_t * ) ctx-> privateData;
   for( int i = 0; i < data-> flagCount; i++ )
   {
      if( strcmp( data-> flags[ i ]-> name, name ) == 0 )
      {
         return data-> flags[ i ]-> isSet;
      }
   }

   return false;
}


static void contextDelete( CommandContext_t *ctx )
{
   if( ctx == NULL )
   {
      return;
   }

   if( ctx-> privateData != NULL )
   {
      free( ctx-> privateData );
   }
   free( ctx );
}


// Function to create a CommandContext
static CommandContext_t * createCommandContext( Command_t *cmd )
{
CommandContext_t *ctx;
CommandContextData_t *data;

   if( cmd == NULL )
   {
      return NULL;
   }

   if( ( ctx = calloc( 1, sizeof( CommandContext_t ) ) ) == NULL )
   {
      return NULL;
   }

   if( ( data = calloc( 1, sizeof( CommandContextData_t ) ) ) == NULL )
   {
      free( ctx );
      return NULL;
   }

   data-> command = cmd;
   data-> arguments = cmd-> arguments;
   data-> argumentCount = cmd-> argumentCount;
   data-> flags = cmd-> flags;
   data-> flagCount = cmd-> flagCount;
   ctx-> privateData = data;
   ctx-> getArgument = contextGetArgument;
   ctx-> getFlag = contextGetFlag;
   ctx-> getCommand = contextGetCommand;
   ctx-> delete = contextDelete;

   return ctx;
}


static int addSubCommand( Command_t *self, Command_t *subCommand )
{
Command_t **tmp;

   if( ( tmp = realloc( self-> subCommands, sizeof( Command_t * ) * ( size_t ) ( self-> subCommandCount + 1 ) ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   subCommand-> parent = self;
   self-> subCommands = tmp;
   self-> subCommands[ self-> subCommandCount ] = subCommand;
   self-> subCommandCount++;

   return CLI_SUCCESS;
}


static int addArgument( Command_t *self, Argument_t *argument )
{
Argument_t **tmp;

   if( ( tmp = realloc( self-> arguments, sizeof( Argument_t * ) * ( size_t ) ( self-> argumentCount + 1 ) ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   self-> arguments = tmp;
   self-> arguments[ self-> argumentCount ] = argument;
   self-> argumentCount++;

   return CLI_SUCCESS;
}


static int addFlag( Command_t *self, Flag_t *flag )
{
Flag_t **tmp;

   if( ( tmp = realloc( self-> flags, sizeof( Flag_t * ) * ( size_t ) ( self-> flagCount + 1 ) ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   self-> flags = tmp;
   self-> flags[ self-> flagCount ] = flag;
   self-> flagCount++;

   return CLI_SUCCESS;
}


static void delete( Command_t *self )
{
   if( self == NULL )
   {
      return;
   }

   for( int i = 0; i < self-> subCommandCount; i++ )
   {
      self-> subCommands[ i ]-> delete( self-> subCommands[ i ] );
   }
   free( self-> subCommands );

   for( int i = 0; i < self-> argumentCount; i++ )
   {
      self-> arguments[ i ]-> delete( self-> arguments[ i ] );
   }
   free( self-> arguments );

   for( int i = 0; i < self-> flagCount; i++ )
   {
      self-> flags[ i ]-> delete( self-> flags[ i ] );
   }
   free( self-> flags );

   free( self-> name );
   free( self-> description );
   free( self );
}


static bool parseFlag( Command_t *cmd, const char *flagStr )
{
Flag_t *flag;

   if( flagStr[ 0 ] != '-' )
   {
      return false;
   }

   // Long flag: --flag
   if( flagStr[ 1 ] == '-' )
   {
      for( int i = 0; i < cmd-> flagCount; i++ )
      {
         flag = cmd-> flags[ i ];
         if( strcmp( flag-> name, flagStr + 2 ) == 0 )
         {
            flag-> set( flag );
            return true;
         }
      }
   }
   // Short flag: -f
   else
   {
      for( int i = 0; i < cmd-> flagCount; i++ )
      {
         flag = cmd-> flags[ i ];
         if( flag-> shortName == flagStr[ 1 ] )
         {
            flag-> set( flag );
            return true;
         }
      }
   }

   return false;
}


static int parse( Command_t *self, int argc, char *argv[] )
{
Command_t *current = self;
int i = 1;
int argIndex = 0;
Argument_t *arg;

   if( argc < 1 )
   {
      return CLI_SUCCESS;
   }

   // First, process flags and subcommands
   while( i < argc )
   {
   bool matchedSub;

      if( argv[ i ][ 0 ] == '-' )
      {
         if( !parseFlag( current, argv[ i ] ) )
         {
            fprintf( stderr, "Error: Unknown flag '%s'\n", argv[ i ] );
            printHelp( current );
            return CLI_ERROR_PARSE_FAILED;
         }
         i++;
         continue;
      }

      // Try to find a subcommand
      matchedSub = false;
      for( int j = 0; j < current-> subCommandCount; j++ )
      {
         if( strcmp( current-> subCommands[ j ]-> name, argv[ i ] ) == 0 )
         {
            current = current-> subCommands[ j ];
            matchedSub = true;
            i++;
            break;
         }
      }
      if( !matchedSub )
      {
         // No subcommand matched, but we have a non-flag argument
         // Check if it's "help" - if so, show help for current command
         if( strcmp( argv[ i ], "help" ) == 0 )
         {
            printHelp( current );
            return CLI_SUCCESS;
         }
         
         // If the current command has arguments, this might be an argument
         if( current-> argumentCount > 0 )
         {
            // This is an argument for the root command, not an invalid subcommand
            break;
         }
         
         // Otherwise, this means we have an invalid subcommand
         fprintf( stderr, "Error: Unknown subcommand '%s'\n", argv[ i ] );
         printHelp( current );
         return CLI_ERROR_NOT_FOUND;
      }
   }

   // Now, process remaining argv[ i ] as arguments for the current command
   for( ; i < argc; i++ )
   {
      if( argv[ i ][ 0 ] == '-' )
      {
         if( !parseFlag( current, argv[ i ] ) )
         {
            fprintf( stderr, "Error: Unknown flag '%s'\n", argv[ i ] );
            printHelp( current );
            return CLI_ERROR_PARSE_FAILED;
         }
         continue;
      }
      if( argIndex < current-> argumentCount )
      {
         arg = current-> arguments[ argIndex ];
         arg-> setValue( arg, argv[ i ] );
         argIndex++;
      }
      else
      {
         fputs( "Error: Too many arguments\n", stderr );
         printHelp( current );
         return CLI_ERROR_INVALID_ARGUMENT;
      }
   }

   // Check if all required arguments were provided
   for( int j = 0; j < current-> argumentCount; j++ )
   {
      arg = current-> arguments[ j ];
      if( arg-> required && arg-> value == NULL )
      {
         fprintf( stderr, "Error: Required argument '%s' is missing\n", arg-> name );
         printHelp( current );
         return CLI_ERROR_INVALID_ARGUMENT;
      }
   }

   // Execute the command handler
   if( current-> handler != NULL )
   {
   CommandContext_t *ctx = createCommandContext( current );
   int result;

      if( ctx == NULL )
      {
         fputs( "Error: Failed to create command context\n", stderr );
         return CLI_ERROR_CONTEXT_FAILED;
      }

      result = current-> handler( ctx );
      ctx-> delete( ctx );

      if( result != CLI_SUCCESS )
      {
         if( strcmp( current-> name, "help" ) != 0 )
         {
            fputs( "Error: Command execution failed\n", stderr );
            printHelp( current );
         }

         return result;
      }
   }
   else
   {
      printHelp( current );
   }

   return CLI_SUCCESS;
}


Command_t * newCommand( const char *name, const char *description, int ( *handler )( CommandContext_t * ) )
{
Command_t *self;

   if( ( self = calloc( 1, sizeof( Command_t ) ) ) == NULL )
   {
      fputs( "Error: Failed to allocate memory for Command_t.\n", stderr );
      return NULL;
   }

   if( ( self-> name = strdup( name ) ) == NULL )
   {
      fputs( "Error: Failed to allocate memory for command name.\n", stderr );
      free( self );
      return NULL;
   }

   if( description != NULL )
   {
      if( ( self-> description = strdup( description ) ) == NULL )
      {
         fputs( "Error: Failed to allocate memory for command description.\n", stderr );
         free( self-> name );
         free( self );
         return NULL;
      }
   }

   self-> handler = handler;
   self-> addSubCommand = addSubCommand;
   self-> addArgument = addArgument;
   self-> addFlag = addFlag;
   self-> parse = parse;
   self-> delete = delete;
   self-> getArgumentValue = getArgumentValue;
   self-> hasArgument = hasArgument;

   return self;
}
