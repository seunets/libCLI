#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "Command.h"
#include "CommandContext.h"
#include "Argument.h"
#include "Flag.h"
#include "CLI.h"


typedef struct privateData
{
   char *name;
   char *description;
   struct Command **subCommands;
   Argument_t **arguments;
   Flag_t **flags;
   struct Command *parent;
   int ( *handler )( const CommandContext_t * );
   int argumentCount;
   int flagCount;
   int subCommandCount;
   char pad[ 4 ];
} PrivateData;


static const char * getName( const Command_t *self )
{
const PrivateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return NULL;
   }

   private = self-> private;
   return private-> name;
}


static const char * getDescription( const Command_t *self )
{
const PrivateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return NULL;
   }

   private = self-> private;
   return private-> description;
}


static int findArgumentByName( Argument_t **arguments, int count, const char *name )
{
   for( int i = 0; i < count; i++ )
   {
      if( arguments[ i ] && arguments[ i ]-> getName( arguments[ i ] ) && strcmp( arguments[ i ]-> getName( arguments[ i ] ), name ) == 0 )
      {
         return i;
      }
   }

   return -1;
}


static Argument_t * getCommandArgument( const Command_t *self, const char *name )
{
Argument_t **arguments;
int count;
int i;
const struct privateData *private;

   if( self == NULL || name == NULL )
   {
      return NULL;
   }

   if( ( private = self-> private ) != NULL )
   {
      arguments = self-> getArguments( self );
      count = self-> getArgumentCount( self );
   }
   else
   {
      arguments = NULL;
      count = 0;
   }

   if( arguments != NULL && ( i = findArgumentByName( arguments, count, name ) ) >= 0 )
   {
      return arguments[ i ];
   }

   return NULL;
}


static const char * getArgumentValue( const Command_t *cmd, const char *name )
{
Argument_t *arg = getCommandArgument( cmd, name );

   if( arg != NULL )
   {
      return arg-> getValue( arg );
   }

   return NULL;
}


static char * buildCommandPath( const Command_t *self )
{
const struct privateData *private = self-> private;
size_t len;
char *buf;
char *parentPath;

   if( private-> parent == NULL )
   {
   char *result;

      if( ( result = strdup( private-> name ) ) == NULL )
      {
         return NULL;
      }

      return result;
   }
   else
   {
      if( ( parentPath = buildCommandPath( private-> parent ) ) == NULL )
      {
         return NULL;
      }

      len = strlen( parentPath ) + strlen( private-> name ) + 2;
      if( ( buf = calloc( 1, len ) ) == NULL )
      {
         free( parentPath );
         return NULL;
      }

      snprintf( buf, len, "%s %s", parentPath, private-> name );
      free( parentPath );

      return buf;
   }
}


static void printHelp( const Command_t *self )
{
PrivateData *private = self-> private;
char *fullPath = buildCommandPath( self );
Argument_t **args;
Flag_t **flags;
int i, argCount, flagCount;

   if( self == NULL || private == NULL )
   {
      free( fullPath );
      return;
   }

   if( private-> description != NULL )
   {
      fprintf( stderr, "%s\n\n", private-> description );
   }

   fprintf( stderr, "Usage: %s", fullPath );

   args = self-> getArguments( self );
   argCount = self-> getArgumentCount( self );

   // Positional arguments
   for( i = 0; i < argCount; i++ )
   {
      if( args[ i ]-> isRequired( args[ i ] ) )
      {
         fprintf( stderr, " <%s>", args[ i ]-> getName( args[ i ] ) );
      }
      else
      {
         fprintf( stderr, " [%s]", args[ i ]-> getName( args[ i ] ) );
      }
   }

   flags = self-> getFlags( self );
   flagCount = self-> getFlagCount( self );

   if( flagCount > 0 )
   {
      fprintf( stderr, " [OPTIONS]" );
   }

   if( private-> subCommandCount > 0 )
   {
      fprintf( stderr, " COMMAND" );
   }

   fputs( "\n\n", stderr );

   // Subcommands section
   if( private-> subCommandCount > 0 )
   {
      fputs( "Commands:\n", stderr );

      // Sort
      for( int pass = 0; pass < private-> subCommandCount - 1; pass++ )
      {
         for( i = 0; i < private-> subCommandCount - 1; i++ )
         {
         Command_t *a = private-> subCommands[ i ];
         Command_t *b = private-> subCommands[ i + 1 ];

            if( strcmp( a-> getName( a ), b-> getName( b ) ) > 0 )
            {
            Command_t *tmp = private-> subCommands[ i ];

               private-> subCommands[ i ] = b;
               private-> subCommands[ i + 1 ] = tmp;
            }
         }
      }

      for( i = 0; i < private-> subCommandCount; i++ )
      {
      Command_t *sub = private-> subCommands[ i ];
      const char *desc = sub-> getDescription( sub );

         fprintf( stderr, "   %-12s %s\n", sub-> getName( sub ), desc != NULL ? desc : "" );
      }
      fprintf( stderr, "\nRun '%s COMMAND --help' for more information on a command.\n\n", fullPath );
   }

   // Flags section
   if( flagCount > 0 )
   {
      fputs( "Options:\n", stderr );
      for( i = 0; i < flagCount; i++ )
      {
      Flag_t *f = flags[ i ];
      char shortBuf[ 8 ] = { 0 };

         if( f-> getShortName( f ) )
         {
            snprintf( shortBuf, sizeof( shortBuf ), "-%c, ", f-> getShortName( f ) );
         }

         fprintf( stderr, "   %s--%-18s %s\n", f-> getShortName( f ) ? shortBuf : "    ", f-> getName( f ), f-> getDescription( f ) ? f-> getDescription( f ) : "" );
      }
   }

   free( fullPath );
}


static int addSubCommand( Command_t *self, Command_t *subCommand )
{
struct privateData *private = self-> private;
Command_t **tmp;

   if( ( tmp = realloc( private-> subCommands, sizeof( Command_t * ) * ( size_t ) ( private-> subCommandCount + 1 ) ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   ( ( struct privateData * )subCommand-> private )-> parent = self;
   private-> subCommands = tmp;
   private-> subCommands[ private-> subCommandCount ] = subCommand;
   private-> subCommandCount++;

   return CLI_SUCCESS;
}


static int addArgument( const Command_t *self, Argument_t *argument )
{
struct privateData *private = (struct privateData *)self-> private;
Argument_t **tmp;

   if( ( tmp = realloc( private-> arguments, sizeof( Argument_t * ) * ( size_t ) ( private-> argumentCount + 1 ) ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   private-> arguments = tmp;
   private-> arguments[ private-> argumentCount ] = argument;
   private-> argumentCount++;

   return CLI_SUCCESS;
}


static int addFlag( const Command_t *self, Flag_t *flag )
{
struct privateData *private = (struct privateData *)self-> private;
Flag_t **tmp;

   if( ( tmp = realloc( private-> flags, sizeof( Flag_t * ) * ( size_t ) ( private-> flagCount + 1 ) ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   private-> flags = tmp;
   private-> flags[ private-> flagCount ] = flag;
   private-> flagCount++;

   return CLI_SUCCESS;
}


static void delete( Command_t *self )
{
struct privateData *private;

   if( self == NULL )
   {
      return;
   }

   private = self-> private;
   if( private != NULL )
   {
      if( private-> subCommands != NULL )
      {
         for( int i = 0; i < private-> subCommandCount; i++ )
         {
            private-> subCommands[ i ]-> delete( private-> subCommands[ i ] );
         }
         free( private-> subCommands );
      }

      if( private-> arguments != NULL )
      {
         for( int i = 0; i < private-> argumentCount; i++ )
         {
            private-> arguments[ i ]-> delete( private-> arguments[ i ] );
         }
         free( private-> arguments );
      }

      if( private-> flags != NULL )
      {
         for( int i = 0; i < private-> flagCount; i++ )
         {
            private-> flags[ i ]-> delete( private-> flags[ i ] );
         }
         free( private-> flags );
      }

      free( private-> name );
      free( private-> description );
      free( private );
   }
   free( self );
}


static bool parseFlag( const Command_t *self, const char *flagStr )
{
const struct privateData *private;
Flag_t *flag;

   if( self == NULL || self-> private == NULL || flagStr == NULL || flagStr[ 0 ] != '-' )
   {
      return false;
   }

   private = self-> private;
   if( private-> flags == NULL )
   {
      return false;
   }

   // Long flag: --flag
   if( flagStr[ 1 ] == '-' && flagStr[ 2 ] != '\0' )
   {
      for( int i = 0; i < private-> flagCount; i++ )
      {
         flag = private-> flags[ i ];
         if( flag != NULL )
         {
            // Use accessor functions instead of direct private data access
            if( flag-> getName( flag ) != NULL && strcmp( flag-> getName( flag ), flagStr + 2 ) == 0 )
            {
               if( flag-> set != NULL )
               {
                  flag-> set( flag );
               }
               return true;
            }
         }
      }
   }
   // Short flag: -f
   else
   {
      for( int i = 0; i < private-> flagCount; i++ )
      {
         flag = private-> flags[ i ];
         if( flag != NULL )
         {
            if( flag-> getShortName( flag ) == flagStr[ 1 ] )
            {
               if( flag-> set != NULL )
               {
                  flag-> set( flag );
               }
               return true;
            }
         }
      }
   }

   return false;
}


static Command_t *findSubCommand( Command_t *cmd, const char *name )
{
PrivateData *private = cmd->private;
int i;

   for( i = 0; i < private-> subCommandCount; i++ )
   {
   Command_t *sub = private-> subCommands[ i ];

      if( strcmp( sub-> getName( sub ), name ) == 0 )
      {
         return sub;
      }
   }
   return NULL;
}


static int parse( Command_t *self, int argc, char *argv[] )
{
Command_t *current = self;
PrivateData *private;
Argument_t **arguments;
int argCount;
int i = 1;
int pos = 0;
int j;
int result;

   if( argc == 1 )
   {
      self-> printHelp( self );
      return CLI_SUCCESS;
   }
   if( self == NULL || argv == NULL || argc < 0 )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }

   // Early help detection anywhere in the command chain
   for( i = 1; i < argc; i++ )
   {
      if( strcmp( argv[ i ], "help" ) == 0 || strcmp( argv[ i ], "--help" ) == 0 || strcmp( argv[ i ], "-h" ) == 0 )
      {
         current = self;
         for( j = 1; j < i; j++ )
         {
         Command_t *sub;

            if( ( sub = findSubCommand( current, argv[ j ] ) ) == NULL )
            {
               break;
            }
            current = sub;
         }
         current-> printHelp( current );
         return CLI_SUCCESS;
      }
   }

   // Resolve subcommand chain
   i = 1;
   while( i < argc && argv[ i ][ 0 ] != '-' )
   {
   Command_t *sub;

      if( ( sub = findSubCommand( current, argv[ i ] ) ) == NULL )
      {
         break;
      }
      current = sub;
      i++;
   }

   // Unknown root command?
   if( current == self && argc > 1 && argv[ 1 ][ 0 ] != '-' && i == 1 )
   {
      fprintf( stderr, "Error: Unknown command '%s'\n", argv[ 1 ] );
      self-> printHelp( self );
      return CLI_ERROR_PARSE_FAILED;
   }

   // Unknown subcommand in a group?
   private = current-> private;
   if( i < argc && argv[ i ][ 0 ] != '-' && private-> handler == NULL )
   {
      fprintf( stderr, "Error: Unknown subcommand '%s'\n", argv[ i ] );
      current-> printHelp( current );
      return CLI_ERROR_PARSE_FAILED;
   }

   arguments = current-> getArguments( current );
   argCount  = current-> getArgumentCount( current );

   // Parse flags + positional arguments
   for( ; i < argc; i++ )
   {
      if( argv[ i ][ 0 ] == '-' )
      {
         if( !parseFlag( current, argv[ i ] ) )
         {
            fprintf( stderr, "Error: Unknown flag '%s'\n", argv[ i ] );
            current-> printHelp( current );
            return CLI_ERROR_PARSE_FAILED;
         }
         continue;
      }

      if( pos < argCount )
      {
         arguments[ pos ]-> setValue( arguments[ pos ], argv[ i ] );
         pos++;
      }
      else
      {
         if( argCount == 0 )
         {
            fprintf( stderr, "Error: Unexpected argument '%s' (command takes no arguments)\n", argv[ i ] );
         }
         else
         {
            fputs( "Error: Too many arguments\n", stderr );
         }
         current-> printHelp( current );
         return CLI_ERROR_INVALID_ARGUMENT;
      }
   }

   // Check required arguments
   for( j = 0; j < argCount; j++ )
   {
   Argument_t *a = arguments[ j ];

      if( a-> isRequired( a ) && a-> getValue( a ) == NULL )
      {
         fprintf( stderr, "Error: Required argument '%s' is missing\n", a-> getName( a ) );
         current-> printHelp( current );
         return CLI_ERROR_INVALID_ARGUMENT;
      }
   }

   // Execute handler if exists
   if( private-> handler != NULL )
   {
   CommandContext_t *ctx;

      if( ( ctx = newCommandContext( current, current-> getArguments( current ), current-> getArgumentCount( current ), current-> getFlags( current ), current-> getFlagCount( current ) ) ) == NULL )
      {
         fputs( "Error: Failed to create command context\n", stderr );
         return CLI_ERROR_CONTEXT_FAILED;
      }
      result = private-> handler( ctx );
      ctx-> delete( ctx );
      if( result != CLI_SUCCESS && strcmp( current-> getName( current ), "help" ) != 0 )
      {
         fputs( "Error: Command execution failed\n", stderr );
         current-> printHelp( current );
      }
      return result;
   }

   current-> printHelp( current );
   return CLI_SUCCESS;
}


static Command_t **getSubCommands( const Command_t *self )
{
const struct privateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return NULL;
   }

   private = self-> private;
   return private-> subCommands;
}


static int getSubCommandCount( const Command_t *self )
{
const struct privateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return 0;
   }

   private = self-> private;
   return private-> subCommandCount;
}


static Argument_t **getArguments( const Command_t *self )
{
const struct privateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return NULL;
   }

   private = self-> private;
   return private-> arguments;
}


static int getArgumentCount( const Command_t *self )
{
const struct privateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return 0;
   }

   private = self-> private;
   return private-> argumentCount;
}


static Flag_t **getFlags( const Command_t *self )
{
const struct privateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return NULL;
   }

   private = self-> private;
   return private-> flags;
}


static int getFlagCount( const Command_t *self )
{
const struct privateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return 0;
   }

   private = self-> private;
   return private-> flagCount;
}


static void forEachSubCommand( const Command_t *self, bool ( *cb )( Command_t * ) )
{
int count;
Command_t **subs;

   if( self == NULL || cb == NULL )
   {
      return;
   }

   count = self-> getSubCommandCount( self );
   subs = self-> getSubCommands( self );
   for( int i = 0; i < count; i++ )
   {
      if( !cb( subs[ i ] ) )
      {
         break;
      }
   }
}


Command_t * newCommand( const char *name, const char *description, int ( *handler )( const CommandContext_t * ) )
{
Command_t *self;
struct privateData *private;

   if( ( self = calloc( 1, sizeof( Command_t ) ) ) == NULL )
   {
      fputs( "Error: Failed to allocate memory for Command_t.\n", stderr );
      return NULL;
   }

   if( ( private = calloc( 1, sizeof( struct privateData ) ) ) == NULL )
   {
      fputs( "Error: Failed to allocate memory for CommandPrivate_t.\n", stderr );
      free( self );
      return NULL;
   }

   if( ( private-> name = strdup( name ) ) == NULL )
   {
      fputs( "Error: Failed to allocate memory for command name.\n", stderr );
      free( private );
      free( self );
      return NULL;
   }

   if( description != NULL && ( private-> description = strdup( description ) ) == NULL )
   {
      fputs( "Error: Failed to allocate memory for command description.\n", stderr );
      free( private-> name );
      free( private );
      free( self );
      return NULL;
   }

   private-> handler = handler;
   self-> private = private;
   self-> addSubCommand = addSubCommand;
   self-> addArgument = addArgument;
   self-> addFlag = addFlag;
   self-> parse = parse;
   self-> delete = delete;
   self-> getName = getName;
   self-> getDescription = getDescription;
   self-> getArgumentValue = getArgumentValue;
   self-> getArguments = getArguments;
   self-> getArgumentCount = getArgumentCount;
   self-> getFlags = getFlags;
   self-> getFlagCount = getFlagCount;
   self-> getSubCommands = getSubCommands;
   self-> getSubCommandCount = getSubCommandCount;
   self-> printHelp = printHelp;
   self-> forEachSubCommand = forEachSubCommand;

   return self;
}

