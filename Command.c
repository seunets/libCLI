#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "Command.h"
#include "CommandContext.h"
#include "Argument.h"
#include "Flag.h"
#include "Option.h"
#include "CLI.h"


typedef struct
{
   Command_t interface;
   char *name;
   char *description;
   struct Command **subCommands;
   Argument_t **arguments;
   Flag_t **flags;
   Option_t **options;
   struct Command *parent;
   int ( *handler )( const CommandContext_t * );
   int subCommandCount;
   int argumentCount;
   int flagCount;
   int optionCount;
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


static struct Command * getParent( const Command_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> parent;
}


static void setParent( Command_t *self, Command_t *parent )
{
Implementation *impl;

   if( self == NULL )
   {
      return;
   }

   impl = __containerof( self, Implementation, interface );
   impl-> parent = parent;
}


static int findArgumentByName( Argument_t **arguments, int count, const char *name )
{
   if( name == NULL )
   {
      return -1;
   }

   for( int i = 0; i < count; i++ )
   {
      if( arguments[ i ] != NULL && arguments[ i ]-> getName( arguments[ i ] ) != NULL && strcmp( arguments[ i ]-> getName( arguments[ i ] ), name ) == 0 )
      {
         return i;
      }
   }

   return -1;
}


static Argument_t * getCommandArgument( const Command_t *self, const char *name )
{
Argument_t **arguments;
int count, i;

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

   return( arg != NULL ) ? arg-> getValue( arg ) : NULL;
}


static char * buildCommandPath( const Command_t *self )
{
Implementation *impl;
char *buf;
char *parentPath;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   if( impl-> parent == NULL )
   {
      return strdup( impl-> name );
   }

   parentPath = buildCommandPath( impl-> parent );
   if( parentPath == NULL )
   {
      return NULL;
   }

   if( asprintf( &buf, "%s %s", parentPath, impl-> name ) == -1 )
   {
      free( parentPath );
      return NULL;
   }

   free( parentPath );
   return buf;
}


static void printHelp( const Command_t *self, FILE *stream )
{
Implementation *impl;
char *fullPath;
Argument_t **args;
Flag_t **flags;
Option_t **options;
int i, argCount, flagCount, optionCount;

   if( self == NULL || stream == NULL )
   {
      return;
   }

   impl = __containerof( self, Implementation, interface );
   if( impl-> description != NULL )
   {
      fprintf( stream, "%s\n\n", impl-> description );
   }

   fullPath = buildCommandPath( self );
   fprintf( stream, "Usage: %s", fullPath ? fullPath : ( impl-> name ? impl-> name : "" ) );

   args = self-> getArguments( self );
   argCount = self-> getArgumentCount( self );

   for( i = 0; i < argCount; i++ )
   {
      if( args[ i ]-> isRequired( args[ i ] ) )
      {
         fprintf( stream, " <%s>", args[ i ]-> getName( args[ i ] ) );
      }
      else
      {
         fprintf( stream, " [%s]", args[ i ]-> getName( args[ i ] ) );
      }
   }

   flags = self-> getFlags( self );
   flagCount = self-> getFlagCount( self );
   options = self-> getOptions( self );
   optionCount = self-> getOptionCount( self );

   if( flagCount > 0 || optionCount > 0 )
   {
      fputs( " [OPTIONS]\n", stream );
   }

   if( impl-> subCommandCount > 0 )
   {
      fputs( " COMMAND\n", stream );
   }

   fputs( "", stream );

   // Subcommands
   if( impl-> subCommandCount > 0 )
   {
      fputs( "Commands:\n", stream );

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

         fprintf( stream, "   %-12s %s\n", sub-> getName( sub ), desc ? desc : "" );
      }
      fprintf( stream, "\nRun '%s COMMAND --help' for more information on a command.\n\n", fullPath ? fullPath : "" );
   }

   // Options + Flags
   if( flagCount > 0 || optionCount > 0 )
   {
      fputs( "Options:\n", stream );

      for( i = 0; i < flagCount; i++ )
      {
      Flag_t *f = flags[ i ];
      char shortBuf[ 8 ] = { 0 };

         if( f-> getShortName( f ) )
         {
            snprintf( shortBuf, sizeof( shortBuf ), "-%c, ", f-> getShortName( f ) );
         }
         fprintf( stream, "   %s--%-18s %s\n", f-> getShortName( f ) ? shortBuf : "    ", f-> getName( f ), f-> getDescription( f ) ? f-> getDescription( f ) : "" );
      }

      for( i = 0; i < optionCount; i++ )
      {
      Option_t *o = options[ i ];
      char shortBuf[ 8 ] = { 0 };

         if( o-> getShortName( o ) )
         {
            snprintf( shortBuf, sizeof( shortBuf ), "-%c, ", o-> getShortName( o ) );
         }
         fprintf( stream, "   %s--%-18s <value> %s%s\n", o-> getShortName( o ) ? shortBuf : "    ", o-> getName( o ), o-> getDescription( o ) ? o-> getDescription( o ) : "", o-> isRequired( o ) ? " (required)" : "" );
      }
      fputs( "", stream );
   }

   free( fullPath );
}


static Command_t * findSubCommand( Command_t *self, const char *name )
{
Implementation *impl;

   if( self == NULL || name == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   for( int i = 0; i < impl-> subCommandCount; i++ )
   {
   Command_t *sub = impl-> subCommands[ i ];

      if( strcmp( sub-> getName( sub ), name ) == 0 )
      {
         return sub;
      }
   }

   return NULL;
}


static int addSubCommand( Command_t *self, Command_t *subCommand )
{
Implementation *impl;
Command_t **tmp;

   if( self == NULL || subCommand == NULL )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }

   if( findSubCommand( self, subCommand-> getName( subCommand ) ) != NULL )
   {
      return CLI_ERROR_ALREADY_EXISTS;
   }

   impl = __containerof( self, Implementation, interface );
   tmp = realloc( impl-> subCommands, sizeof( Command_t * ) * ( size_t )( impl-> subCommandCount + 1 ) );
   if( tmp == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   if( subCommand-> setParent )
   {
      subCommand-> setParent( subCommand, self );
   }

   impl-> subCommands = tmp;
   impl-> subCommands[ impl-> subCommandCount++ ] = subCommand;

   return CLI_SUCCESS;
}


static int addArgument( const Command_t *self, Argument_t *argument )
{
Implementation *impl;
Argument_t **tmp;

   if( self == NULL || argument == NULL )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }

   impl = __containerof( self, Implementation, interface );
   if( findArgumentByName( impl-> arguments, impl-> argumentCount, argument-> getName( argument ) ) >= 0 )
   {
      return CLI_ERROR_ALREADY_EXISTS;
   }

   tmp = realloc( impl-> arguments, sizeof( Argument_t * ) * ( size_t )( impl-> argumentCount + 1 ) );
   if( tmp == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   impl-> arguments = tmp;
   impl-> arguments[ impl-> argumentCount++ ] = argument;

   return CLI_SUCCESS;
}


static int addFlag( const Command_t *self, Flag_t *flag )
{
Implementation *impl;
Flag_t **tmp;

   if( self == NULL || flag == NULL )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }

   impl = __containerof( self, Implementation, interface );
   for( int i = 0; i < impl-> flagCount; i++ )
   {
   Flag_t *existing = impl-> flags[ i ];

      if( strcmp( existing-> getName( existing ), flag-> getName( flag ) ) == 0 || ( flag-> getShortName( flag ) != '\0' && existing-> getShortName( existing ) == flag-> getShortName( flag ) ) )
      {
         return CLI_ERROR_ALREADY_EXISTS;
      }
   }

   tmp = realloc( impl-> flags, sizeof( Flag_t * ) * ( size_t )( impl-> flagCount + 1 ) );
   if( tmp == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   impl-> flags = tmp;
   impl-> flags[ impl-> flagCount++ ] = flag;
   return CLI_SUCCESS;
}


static int addOption( const Command_t *self, Option_t *option )
{
Implementation *impl;
Option_t **tmp;

   if( self == NULL || option == NULL )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }

   impl = __containerof( self, Implementation, interface );

   for( int i = 0; i < impl-> optionCount; i++ )
   {
   Option_t *existing = impl-> options[ i ];

      if( strcmp( existing-> getName( existing ), option-> getName( option ) ) == 0 || ( option-> getShortName( option ) != '\0' && existing-> getShortName( existing ) == option-> getShortName( option ) ) )
      {
         return CLI_ERROR_ALREADY_EXISTS;
      }
   }

   for( int i = 0; i < impl-> flagCount; i++ )
   {
   Flag_t *f = impl-> flags[ i ];
 
      if( strcmp( f-> getName( f ), option-> getName( option ) ) == 0 || ( option-> getShortName( option ) != '\0' && f-> getShortName( f ) == option-> getShortName( option ) ) )
      {
         return CLI_ERROR_ALREADY_EXISTS;
      }
   }

   tmp = realloc( impl-> options, sizeof( Option_t * ) * ( size_t )( impl-> optionCount + 1 ) );
   if( tmp == NULL )
   {
      return CLI_ERROR_MEMORY;
   }

   impl-> options = tmp;
   impl-> options[ impl-> optionCount++ ] = option;
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
   impl = __containerof( self, Implementation, interface );
   if( impl-> subCommands )
   {
      for( int i = 0; i < impl-> subCommandCount; i++ )
      {
         impl-> subCommands[ i ]-> delete( &impl-> subCommands[ i ] );
      }
      free( impl-> subCommands );
   }

   if( impl-> arguments )
   {
      for( int i = 0; i < impl-> argumentCount; i++ )
      {
         impl-> arguments[ i ]-> delete( &impl-> arguments[ i ] );
      }
      free( impl-> arguments );
   }

   if( impl->flags )
   {
      for( int i = 0; i < impl-> flagCount; i++ )
      {
         impl-> flags[ i ]-> delete( &impl-> flags[ i ] );
      }
      free( impl-> flags );
   }

   if( impl-> options )
   {
      for( int i = 0; i < impl-> optionCount; i++ )
      {
        impl-> options[ i ]-> delete( &impl-> options[ i ] );
      }
      free( impl-> options );
   }

   free( impl-> name );
   free( impl-> description );
   free( impl );
   *selfPtr = NULL;
}


static bool parseFlagOrOption( const Command_t *self, const char *token, const char *next, int *consumedExtra )
{
Implementation *impl;
bool isLong;
char *eq;
char nameBuf[128];
const char *valueFromEq;
const char *p;
const char *nameToMatch;
const char *val;
size_t len;
int i;
Flag_t *f;
Option_t *o;
char ch;
bool found;

   if( consumedExtra != NULL )
   {
      *consumedExtra = 0;
   }

   if( self == NULL || token == NULL || token[ 0 ] != '-' || token[ 1 ] == '\0' )
   {
      return false;
   }

   impl = __containerof(self, Implementation, interface);
   isLong = ( token[ 1 ] == '-' && token[ 2 ] != '\0' );
   eq = NULL;
   valueFromEq = NULL;

   if( isLong )
   {
      eq = strchr( token + 2, '=' );
      if( eq != NULL )
      {
         len = ( size_t )( eq - ( token + 2 ) );
         if( len == 0 || len >= sizeof( nameBuf ) )
         {
            return false;
         }
         memcpy( nameBuf, token + 2, len );
         nameBuf[ len ] = '\0';
         valueFromEq = eq + 1;
      }

      for( i = 0; i < impl-> flagCount; i++ )
      {
         f = impl-> flags[ i ];
         if( f == NULL )
         {
            continue;
         }

         nameToMatch = ( eq != NULL ) ? nameBuf : ( token + 2 );
         if( f-> getName( f ) != NULL && strcmp( nameToMatch, f-> getName( f ) ) == 0 )
         {
            if( eq != NULL )
            {
               return false;
            }

            if( f-> set )
            {
               f-> set(f);
            }
            return true;
         }
      }

      for( i = 0; i < impl-> optionCount; i++ )
      {
         o = impl-> options[ i ];
         if( o == NULL )
         {
            continue;
         }

         nameToMatch = ( eq != NULL ) ? nameBuf : ( token + 2 );
         if( o-> getName( o ) != NULL && strcmp( nameToMatch, o-> getName( o ) ) == 0 )
         {
            val = NULL;

            if( eq != NULL )
            {
               val = valueFromEq;
            }
            else
            {
               if( next != NULL )
               {
                  val = next;
                  if( consumedExtra != NULL )
                  {
                     *consumedExtra = 1;
                  }
               }
               else
               {
                  return false;
               }
            }

            if( o-> setValue != NULL )
            {
               o-> setValue( o, val );
            }
            return true;
         }
      }

      return false;
   }

   p = token + 1;

   while( *p != '\0' )
   {
      ch = *p;
      found = false;

      for( i = 0; i < impl-> flagCount; i++ )
      {
         f = impl-> flags[ i ];
         if( f == NULL )
         {
            continue;
         }

         if( f-> getShortName( f ) == ch )
         {
            if( f-> set != NULL )
            {
               f-> set( f );
            }
            found = true;
            p++;
            break;
         }
      }
      if( found )
      {
         continue;
      }

      for( i = 0; i < impl-> optionCount; i++ )
      {
         o = impl-> options[ i ];
         if( o == NULL )
         {
            continue;
         }

         if( o-> getShortName( o ) == ch )
         {
            p++;
            if( *p != '\0' )
            {
               val = p;
               p += strlen( p );
            }
            else
            {
               if( next != NULL )
               {
                  val = next;
                  if( consumedExtra )
                  {
                     *consumedExtra = 1;
                  }
               }
               else
               {
                  return false;
               }
            }

            if( o-> setValue != NULL )
            {
               o-> setValue( o, val );
            }

            found = true;
            break;
         }
      }

      if( !found )
      {
         return false;
      }
   }

   return true;
}


static int parse( Command_t *self, int argc, char *argv[] )
{
Command_t *current = self;
Implementation *impl;
Argument_t **arguments;
int argCount, i = 1, pos = 0, j, result;

   if( self == NULL || argv == NULL || argc < 0 )
   {
      return CLI_ERROR_INVALID_ARGUMENT;
   }

   if( argc == 1 )
   {
      self-> printHelp( self, stderr );
      return CLI_SUCCESS;
   }

   for( i = 1; i < argc; i++ )
   {
      if( strcmp( argv[ i ], "help" ) == 0 || strcmp( argv[ i ], "--help" ) == 0 || strcmp( argv[ i ], "-h" ) == 0 )
      {
         current = self;
         for( j = 1; j < i; j++ )
         {
         Command_t *sub = findSubCommand( current, argv[ j ] );

            if( sub == NULL )
            {
               break;
            }
            current = sub;
         }
         current-> printHelp( current, stderr );
         return CLI_SUCCESS;
      }
   }

   i = 1;
   while( i < argc && argv[ i ][ 0 ] != '-' )
   {
   Command_t *sub = findSubCommand( current, argv[ i ] );

      if( sub == NULL )
      {
         break;
      }
      current = sub;
      i++;
   }

   if( current == self && argc > 1 && argv[ 1 ][ 0 ] != '-' && i == 1 )
   {
      fprintf( stderr, "Error: Unknown command '%s'\n", argv[ 1 ] );
      self-> printHelp( self, stderr );
      return CLI_ERROR_PARSE_FAILED;
   }

   impl = __containerof(current, Implementation, interface);
   if (i < argc && argv[ i ][ 0 ] != '-' && impl-> handler == NULL )
   {
      fprintf( stderr, "Error: Unknown subcommand '%s'\n", argv[ i ] );
      current-> printHelp( current, stderr );
      return CLI_ERROR_PARSE_FAILED;
   }

   arguments = current-> getArguments( current );
   argCount  = current-> getArgumentCount( current );

   for( ; i < argc; i++ )
   {
      if( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] != '\0' )
      {
      int consumed = 0;
      const char *next = ( i + 1 < argc ) ? argv[ i + 1 ] : NULL;

         if( !parseFlagOrOption( current, argv[ i ], next, &consumed ) )
         {
            fprintf( stderr, "Error: Unknown flag/option '%s'\n", argv[ i ] );
            current-> printHelp( current, stderr );
            return CLI_ERROR_PARSE_FAILED;
         }
         if( consumed )
         {
            i++;
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
            fprintf( stderr, "Error: Unexpected argument '%s'\n", argv[ i ] );
         }
         else
         {
            fputs( "Error: Too many arguments", stderr );
         }
         current-> printHelp( current, stderr );
         return CLI_ERROR_INVALID_ARGUMENT;
      }
   }

   for( j = 0; j < argCount; j++ )
   {
   Argument_t *a = arguments[ j ];

      if( a-> isRequired( a ) && !a-> isSet( a ) )
      {
         fprintf( stderr, "Error: Required argument '%s' is missing\n", a-> getName( a ) );
         current-> printHelp( current, stderr );
         return CLI_ERROR_INVALID_ARGUMENT;
      }
   }

   for( j = 0; j < impl-> optionCount; j++ )
   {
   Option_t *o = impl-> options[ j ];

      if( o-> isRequired( o ) && !o-> isSet( o ) )
      {
         fprintf( stderr, "Error: Required option '--%s' is missing\n", o-> getName( o ) );
         current-> printHelp( current, stderr );
         return CLI_ERROR_INVALID_ARGUMENT;
      }
   }

   if( impl-> handler != NULL )
   {
   CommandContext_t *ctx = newCommandContext( current, current-> getArguments( current ), current-> getArgumentCount( current ), current-> getFlags( current ), current-> getFlagCount( current ), current-> getOptions( current ), current-> getOptionCount( current ) );

      if( ctx == NULL )
      {
         fputs( "Error: Failed to create command context", stderr );
         return CLI_ERROR_CONTEXT_FAILED;
      }

      result = impl-> handler( ctx );
      ctx-> delete( &ctx );

      if( result != CLI_SUCCESS && strcmp( current-> getName( current ), "help" ) != 0 )
      {
         fputs( "Error: Command execution failed", stderr );
         current-> printHelp( current, stderr );
      }
      return result;
   }

   current-> printHelp( current, stderr );
   return CLI_SUCCESS;
}


static Command_t ** getSubCommands( const Command_t *self )
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


static Argument_t ** getArguments( const Command_t *self )
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


static Flag_t ** getFlags( const Command_t *self )
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


static Option_t ** getOptions( const Command_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return NULL;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> options;
}


static int getOptionCount( const Command_t *self )
{
Implementation *impl;

   if( self == NULL )
   {
      return 0;
   }

   impl = __containerof( self, Implementation, interface );
   return impl-> optionCount;
}



Command_t * newCommand( const char *name, const char *description, int ( *handler )( const CommandContext_t * ) )
{
Implementation *self;

   if( name == NULL || ( self = calloc( 1, sizeof( Implementation ) ) ) == NULL )
   {
      fputs( "Error: Failed to allocate memory for Command_t.", stderr );
      return NULL;
   }

   if( ( self-> name = strdup( name ) ) == NULL )
   {
      fputs( "Error: Failed to allocate memory for command name.", stderr );
      free( self );
      return NULL;
   }

   if( description != NULL && ( self-> description = strdup( description ) ) == NULL )
   {
      fputs( "Error: Failed to allocate memory for command description.", stderr );
      free( self-> name );
      free( self );
      return NULL;
   }

   self-> handler = handler;
   self-> parent = NULL;

   self-> interface.addSubCommand = addSubCommand;
   self-> interface.addArgument = addArgument;
   self-> interface.addFlag = addFlag;
   self-> interface.addOption = addOption;
   self-> interface.parse = parse;
   self-> interface.delete = delete;
   self-> interface.getName = getName;
   self-> interface.getDescription = getDescription;
   self-> interface.getArgumentValue = getArgumentValue;
   self-> interface.getArguments = getArguments;
   self-> interface.getArgumentCount = getArgumentCount;
   self-> interface.getFlags = getFlags;
   self-> interface.getFlagCount = getFlagCount;
   self-> interface.getOptions = getOptions;
   self-> interface.getOptionCount = getOptionCount;
   self-> interface.getSubCommands = getSubCommands;
   self-> interface.getSubCommandCount = getSubCommandCount;
   self-> interface.printHelp = printHelp;
   self-> interface.getParent = getParent;
   self-> interface.setParent = setParent;

   return &self-> interface;
}
