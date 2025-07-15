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


struct privateData
{
   char *name;
   char *description;
   Command_t **subCommands;
   Argument_t **arguments;
   Flag_t **flags;
   Command_t *parent;
   int ( *handler )( const CommandContext_t * );
   Command_t ** ( *getSubCommands )( const Command_t * );
   int ( *getSubCommandCount )( const Command_t * );
   Argument_t ** ( *getArguments )( const Command_t * );
   int ( *getArgumentCount )( const Command_t * );
   Flag_t ** ( *getFlags )( const Command_t * );
   int ( *getFlagCount )( const Command_t * );
   int subCommandCount;
   int argumentCount;
   int flagCount;
   char pad[ 4 ];
};


static const char * getName( const Command_t *self )
{
const struct privateData *private;

   if( self == NULL || self-> private == NULL )
   {
      return NULL;
   }

   private = self-> private;
   return private-> name;
}


static const char * getDescription( const Command_t *self )
{
const struct privateData *private;

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
      arguments = private-> getArguments( self );
      count = private-> getArgumentCount( self );
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


static bool hasArgument( const Command_t *cmd, const char *name )
{
   return getCommandArgument( cmd, name ) != NULL;
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

      len = strlen( parentPath ) + 1 + strlen( private-> name ) + 1;
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


static void printHelp( Command_t *self )
{
const struct privateData *private;
char *path, *progname, *trimmedPath, *space;
Argument_t **arguments;
int argCount;
Flag_t **flags;
int flagCount;

   if( self == NULL || self-> private == NULL )
   {
      return;
   }

   private = self-> private;
   path = buildCommandPath( self );
   trimmedPath = path;

   if( ( progname = strdup( self-> getName( self ) ) ) == NULL )
   {
      if( path != NULL )
      {
         free( path );
      }
      return;
   }

   if( private-> parent == NULL )
   {
      int subCount;
      Command_t **subs;
      const char *description = self-> getDescription( self );

      if( description != NULL && *description != '\0' )
      {
         fprintf( stderr, "\n%s - %s\n\n", progname, description );
      }
      else
      {
         fprintf( stderr, "\n%s\n\n", progname );
      }
      fprintf( stderr, "Usage: %s <command> [options]\n\n", progname );
      subCount = self-> getSubCommandCount( self );
      subs = self-> getSubCommands( self );
      if( subCount > 0 && subs != NULL )
      {
         fprintf( stderr, "Available commands:\n" );
         for( int i = 0; i < subCount; i++ )
         {
            fprintf( stderr, "  %-12s %s\n", subs[i]-> getName( subs[i] ), subs[i]-> getDescription( subs[i] ) ? subs[i]-> getDescription( subs[i] ) : "" );
         }
         fprintf( stderr, "\n" );
      }
      free( path );
      free( progname );
      return;
   }

   space = strchr( path, ' ' );
   if( space != NULL && *( space + 1 ) != '\0' )
   {
      trimmedPath = space + 1;
   }

   fprintf( stderr, "Help for: %s\n\n", trimmedPath );
   if( self-> getDescription( self ) )
   {
      fprintf( stderr, "%s\n\n", self-> getDescription( self ) );
   }

   fprintf( stderr, "Usage: %s", trimmedPath );

   arguments = self-> getArguments( self );
   argCount = self-> getArgumentCount( self );
   for( int i = 0; i < argCount; i++ )
   {
   const Argument_t *arg = arguments[ i ];

      if( arg-> isRequired( arg ) )
      {
         fprintf( stderr, " <%s>", arg-> getName( arg ) );
      }
   }
   for( int i = 0; i < argCount; i++ )
   {
   const Argument_t *arg = arguments[ i ];

      if( !arg-> isRequired( arg ) )
      {
         fprintf( stderr, " [%s]", arg-> getName( arg ) );
      }
   }
   fputs( "\n\n", stderr );
   if( argCount > 0 )
   {
      fputs( "Arguments:\n", stderr );
      for( int i = 0; i < argCount; i++ )
      {
      const Argument_t *arg = arguments[ i ];

         fprintf( stderr, "  %-12s %s%s", arg-> getName( arg ), arg-> getDescription( arg ), arg-> isRequired( arg ) ? " (required)" : "" );
         if( !arg-> isRequired( arg ) && arg-> getValue( arg ) )
         {
            fprintf( stderr, " [default: %s]", arg-> getValue( arg ) );
         }
         fputs( "\n", stderr );
      }
      fputs( "\n", stderr );
   }

   flags = self-> getFlags( self );
   flagCount = self-> getFlagCount( self );
   if( flagCount > 0 )
   {
      fputs( "Flags:\n", stderr );
      for( int i = 0; i < flagCount; i++ )
      {
      const Flag_t *flag = flags[ i ];

         fprintf( stderr, "  -%c, --%-12s %s\n", flag-> getShortName( flag ), flag-> getName( flag ), flag-> getDescription( flag ) );
      }
      fputs( "\n", stderr );
   }
   free( path );
   free( progname );
}


// Function to create a CommandContext
static CommandContext_t * createCommandContext( Command_t *self )
{
const struct privateData *private;

   if( self == NULL )
   {
      return NULL;
   }

   private = self-> private;

   return newCommandContext( self, private-> getArguments( self ), private-> getArgumentCount( self ), private-> getFlags( self ), private-> getFlagCount( self ) );
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


static int parse( Command_t *self, int argc, char *argv[] )
{
Command_t *current = NULL;
int i = 1;
int argIndex = 0;
Argument_t *arg = NULL, **arguments;
int argCount;
const struct privateData *private;

   if( argc == 1 )
   {
      printHelp( self );
      return CLI_SUCCESS;
   }

   if( i >= argc )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }
   if( self == NULL )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }
   if( argv == NULL )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }
   if( argc < 0 )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }
   if( argc < 1 )
   {
      return CLI_SUCCESS;
   }

   // First, process flags and subcommands
   current = self;
   while( i < argc && i >= 0 )
   {
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
      if( strcmp( argv[ i ], "help" ) == 0 )
      {
         printHelp( current );
         return CLI_SUCCESS;
      }
      
      // Try to find a subcommand
      findSubCmdToken = argv[ i ];
      findSubCmdFound = NULL;
      current-> forEachSubCommand( current, findSubCmd );
      
      if( findSubCmdFound != NULL )
      {
         // Found a subcommand, navigate to it
         current = findSubCmdFound;
         i++;
         continue;
      }
      
      // No subcommand found, treat as argument
      break;
   }

   // Now, process remaining argv[ i ] as arguments for the current command
   for( ; i < argc; i++ )
   {
      if( current == NULL )
      {
         return CLI_ERROR_INVALID_ARGUMENT;
      }
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
      if( argIndex < current-> getArgumentCount( current ) )
      {
         arguments = current-> getArguments( current );

         if( arguments != NULL )
         {
            arg = arguments[ argIndex ];
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
      else
      {
         fputs( "Error: Too many arguments\n", stderr );
         printHelp( current );
         return CLI_ERROR_INVALID_ARGUMENT;
      }
   }

   // Check if all required arguments were provided
   arguments = current-> getArguments( current );
   argCount = current-> getArgumentCount( current );
   if( arguments != NULL )
   {
      for( int j = 0; j < argCount; j++ )
      {
         arg = arguments[ j ];
         if( arg-> isRequired( arg ) && arg-> getValue( arg ) == NULL )
         {
            fprintf( stderr, "Error: Required argument '%s' is missing\n", arg-> getName( arg ) );
            printHelp( current );
            return CLI_ERROR_INVALID_ARGUMENT;
         }
      }
   }
   // Execute the command handler
   private = current-> private;
   if( private != NULL && private-> handler != NULL )
   {
   CommandContext_t *ctx = createCommandContext( current );
   int result;

      if( ctx == NULL )
      {
         fputs( "Error: Failed to create command context\n", stderr );
         return CLI_ERROR_CONTEXT_FAILED;
      }
      result = private-> handler( ctx );
      ctx-> delete( ctx );
      if( result != CLI_SUCCESS )
      {
         if( strcmp( current-> getName( current ), "help" ) != 0 )
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


static void forEachSubCommand( Command_t *self, bool ( *cb )( Command_t * ) )
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
   private-> getSubCommands = getSubCommands;
   private-> getSubCommandCount = getSubCommandCount;
   private-> getArguments = getArguments;
   private-> getArgumentCount = getArgumentCount;
   private-> getFlags = getFlags;
   private-> getFlagCount = getFlagCount;
   self-> private = private;
   self-> addSubCommand = addSubCommand;
   self-> addArgument = addArgument;
   self-> addFlag = addFlag;
   self-> parse = parse;
   self-> delete = delete;
   self-> getName = getName;
   self-> getDescription = getDescription;
   self-> hasArgument = hasArgument;
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

