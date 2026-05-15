#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "Command.h"
#include "CommandContext.h"
#include "Argument.h"
#include "Flag.h"
#include "CLI.h"


typedef struct
{
   Command_t interface;
   char *name;
   char *description;
   struct Command **subCommands;
   Argument_t **arguments;
   Flag_t **flags;
   struct Command *parent;
   int ( *handler )( const CommandContext_t * );
   int subCommandCount;
   int argumentCount;
   int flagCount;
} Implementation;


static const char * getName( const Command_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> name;
}


static const char * getDescription( const Command_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> description;
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

   if( self == NULL )
   {
      return NULL;
   }

   arguments = self-> getArguments( self );
   count = self-> getArgumentCount( self );

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
Implementation *impl = __containerof( self, Implementation, interface );
size_t len;
char *buf;
char *parentPath;

   if( impl-> parent == NULL )
   {
   char *result;

      if( ( result = strdup( impl-> name ) ) == NULL )
      {
         return NULL;
      }

      return result;
   }
   else
   {
      if( ( parentPath = buildCommandPath( impl-> parent ) ) == NULL )
      {
         return NULL;
      }

      len = strlen( parentPath ) + strlen( impl-> name ) + 2;
      if( ( buf = calloc( 1, len ) ) == NULL )
      {
         free( parentPath );
         return NULL;
      }

      snprintf( buf, len, "%s %s", parentPath, impl-> name );
      free( parentPath );

      return buf;
   }
}


static void printHelp( const Command_t *self )
{
Implementation *impl = __containerof( self, Implementation, interface );
char *fullPath = buildCommandPath( self );
Argument_t **args;
Flag_t **flags;
int i, argCount, flagCount;

   if( self == NULL )
   {
      free( fullPath );
      return;
   }

   if( impl-> description != NULL )
   {
      fprintf( stderr, "%s\n\n", impl-> description );
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

   if( impl-> subCommandCount > 0 )
   {
      fprintf( stderr, " COMMAND" );
   }

   fputs( "\n\n", stderr );

   // Subcommands section
   if( impl-> subCommandCount > 0 )
   {
      fputs( "Commands:\n", stderr );

      // Sort
      for( int pass = 0; pass < impl-> subCommandCount - 1; pass++ )
      {
         for( i = 0; i < impl-> subCommandCount - 1; i++ )
         {
         Command_t *a = impl-> subCommands[ i ];
         Command_t *b = impl-> subCommands[ i + 1 ];

            if( strcmp( a-> getName( a ), b-> getName( b ) ) > 0 )
            {
            Command_t *tmp = impl-> subCommands[ i ];

               impl-> subCommands[ i ] = b;
               impl-> subCommands[ i + 1 ] = tmp;
            }
         }
      }

      for( i = 0; i < impl-> subCommandCount; i++ )
      {
      Command_t *sub = impl-> subCommands[ i ];
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
Implementation *impl = __containerof( self, Implementation, interface );
Command_t **tmp;

   if( ( tmp = realloc( impl-> subCommands, sizeof( Command_t * ) * ( size_t ) ( impl-> subCommandCount + 1 ) ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   ( ( Implementation * )( subCommand ) )-> parent = self;
   impl-> subCommands = tmp;
   impl-> subCommands[ impl-> subCommandCount ] = subCommand;
   impl-> subCommandCount++;

   return CLI_SUCCESS;
}


static int addArgument( const Command_t *self, Argument_t *argument )
{
Implementation *impl = __containerof( self, Implementation, interface );
Argument_t **tmp;

   if( ( tmp = realloc( impl-> arguments, sizeof( Argument_t * ) * ( size_t ) ( impl-> argumentCount + 1 ) ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   impl-> arguments = tmp;
   impl-> arguments[ impl-> argumentCount ] = argument;
   impl-> argumentCount++;

   return CLI_SUCCESS;
}


static int addFlag( const Command_t *self, Flag_t *flag )
{
Implementation *impl = __containerof( self, Implementation, interface );
Flag_t **tmp;

   if( ( tmp = realloc( impl-> flags, sizeof( Flag_t * ) * ( size_t ) ( impl-> flagCount + 1 ) ) ) == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   impl-> flags = tmp;
   impl-> flags[ impl-> flagCount ] = flag;
   impl-> flagCount++;

   return CLI_SUCCESS;
}


static void delete( Command_t **selfPtr )
{
Implementation *impl;
Command_t *self;

   if( selfPtr == NULL || *selfPtr == NULL )
   {
      return;
   }

   self = *selfPtr;

   if( ( impl = __containerof( self, Implementation, interface ) ) != NULL )
   {
      if( impl-> subCommands != NULL )
      {
         for( int i = 0; i < impl-> subCommandCount; i++ )
         {
            impl-> subCommands[ i ]-> delete( &impl-> subCommands[ i ] );
         }
         free( impl-> subCommands );
      }

      if( impl-> arguments != NULL )
      {
         for( int i = 0; i < impl-> argumentCount; i++ )
         {
            impl-> arguments[ i ]-> delete( &impl-> arguments[ i ] );
         }
         free( impl-> arguments );
      }

      if( impl-> flags != NULL )
      {
         for( int i = 0; i < impl-> flagCount; i++ )
         {
            impl-> flags[ i ]-> delete( &impl-> flags[ i ] );
         }
         free( impl-> flags );
      }

      free( impl-> name );
      free( impl-> description );
      free( impl );
   }
   *selfPtr = NULL;
}


static bool parseFlag( const Command_t *self, const char *flagStr )
{
Implementation *impl;
Flag_t *flag;

   if( self == NULL || flagStr == NULL || flagStr[ 0 ] != '-' )
   {
      return false;
   }

   impl = __containerof( self, Implementation, interface );
   if( impl-> flags == NULL )
   {
      return false;
   }

   // Long flag: --flag
   if( flagStr[ 1 ] == '-' && flagStr[ 2 ] != '\0' )
   {
      for( int i = 0; i < impl-> flagCount; i++ )
      {
         flag = impl-> flags[ i ];
         if( flag != NULL )
         {
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
      for( int i = 0; i < impl-> flagCount; i++ )
      {
         flag = impl-> flags[ i ];
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


static Command_t *findSubCommand( Command_t *self, const char *name )
{
Implementation *impl = __containerof( self, Implementation, interface );
int i;

   for( i = 0; i < impl-> subCommandCount; i++ )
   {
   Command_t *sub = impl-> subCommands[ i ];

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
Implementation *impl;
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
   impl = ( Implementation * ) current;
   if( i < argc && argv[ i ][ 0 ] != '-' && impl-> handler == NULL )
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
   if( impl-> handler != NULL )
   {
   CommandContext_t *ctx;

      if( ( ctx = newCommandContext( current, current-> getArguments( current ), current-> getArgumentCount( current ), current-> getFlags( current ), current-> getFlagCount( current ) ) ) == NULL )
      {
         fputs( "Error: Failed to create command context\n", stderr );
         return CLI_ERROR_CONTEXT_FAILED;
      }
      result = impl-> handler( ctx );
      ctx-> delete( &ctx );
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
Implementation *impl;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> subCommands;
}


static int getSubCommandCount( const Command_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return 0;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> subCommandCount;
}


static Argument_t **getArguments( const Command_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> arguments;
}


static int getArgumentCount( const Command_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return 0;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> argumentCount;
}


static Flag_t **getFlags( const Command_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> flags;
}


static int getFlagCount( const Command_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return 0;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> flagCount;
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
Implementation *self;

   if( ( self = calloc( 1, sizeof( Implementation ) ) ) == NULL )
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

   if( description != NULL && ( self-> description = strdup( description ) ) == NULL )
   {
      fputs( "Error: Failed to allocate memory for command description.\n", stderr );
      free( self-> name );
      free( self );
      return NULL;
   }

   self-> handler = handler;
   self-> interface.addSubCommand = addSubCommand;
   self-> interface.addArgument = addArgument;
   self-> interface.addFlag = addFlag;
   self-> interface.parse = parse;
   self-> interface.delete = delete;
   self-> interface.getName = getName;
   self-> interface.getDescription = getDescription;
   self-> interface.getArgumentValue = getArgumentValue;
   self-> interface.getArguments = getArguments;
   self-> interface.getArgumentCount = getArgumentCount;
   self-> interface.getFlags = getFlags;
   self-> interface.getFlagCount = getFlagCount;
   self-> interface.getSubCommands = getSubCommands;
   self-> interface.getSubCommandCount = getSubCommandCount;
   self-> interface.printHelp = printHelp;
   self-> interface.forEachSubCommand = forEachSubCommand;

   return &self-> interface;
}

