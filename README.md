# libCLI

A lightweight C library for building command-line interfaces with support for commands, subcommands, arguments, flags and options.

## Features

- **Hierarchical Commands**: Support for commands and subcommands
- **Arguments**: Required and optional positional arguments
- **Flags**: Boolean flags (long and short forms)
- **Options**: Key-value options that accept a value
- **Help System**: Automatic help generation
- **Error Handling**: Standardized error codes
- **Memory Safety**: Designed to be leak-free
- **Object-Oriented Design** using embedded interfaces

## API Overview

### Creating a CLI

```c
CLI_t *cli = newCLI( "My application description" );
```

### Adding Commands

```c
cli-> addCommand( cli, "init", "Initialize a resource", initHandler );
cli-> addSubCommand( cli, "parent", "child", "Child command", childHandler );
```

### Arguments (positional)

```c
cli-> addArgument( cli, "init", "resource", "Resource to initialize", false ); // optional
cli-> addArgument( cli, "init", "name", "Required name", true );               // required
```

### Flags (boolean)

```c
cli-> addFlag( cli, "init", "verbose", 'v', "Enable verbose output" );
```

### Options (with value)

```c
cli-> addOption( cli, "init", "output", 'o', "Output file", false );  // optional
cli-> addOption( cli, "init", "config", 'c', "Config file", true );   // required
```

### Parsing

```c
int err = cli-> parse( cli, argc, argv );

   if( err != CLI_SUCCESS )
   {
      // handle error
   }
```

### Cleaning up

```c
cli-> delete( &cli );
```

## Command Context

Inside a handler you can access parsed values:

```c
static int initHandler( const CommandContext_t *ctx )
{
const char *resource = ctx-> getArgument( ctx, "resource" );
bool verbose         = ctx-> getFlag( ctx, "verbose" );
const char *output   = ctx-> getOption( ctx, "output" );
Command_t *cmd       = ctx-> getCommand( ctx );

   // ...

   return CLI_SUCCESS;
}
```

### Advanced: accessing the current command

For more advanced use cases you can also retrieve the `Command_t` that is being executed:

```c
Command_t *cmd = ctx-> getCommand( ctx );
const char *name = cmd-> getName( cmd );
```

This is useful when:

- The same handler is reused by multiple commands
- You need the command name or description for logging/error messages
- You want to inspect the command hierarchy (parent, subcommands, etc.)

In most simple handlers you will not need it.

## Supported Syntax

```sh
# Flags
-v
--verbose

# Options
-o file.txt
--output file.txt
--output=file.txt

# Combined short flags
-vo file.txt
-vofile.txt
```

## Error Codes

 Error Code                  | Value | Meaning
-----------------------------|-------|----------------------------------
 `CLI_SUCCESS`               |  0    | Operation succeeded
 `CLI_ERROR_MEMORY`          | -1    | Memory allocation failed
 `CLI_ERROR_INVALID_ARGUMENT`| -2    | Invalid or missing argument
 `CLI_ERROR_NOT_FOUND`       | -3    | Item not found
 `CLI_ERROR_ALREADY_EXISTS`  | -4    | Item already exists
 `CLI_ERROR_PARSE_FAILED`    | -5    | Parsing failed
 `CLI_ERROR_CONTEXT_FAILED`  | -6    | Failed to create command context

## Complete Example

```c
#include <stdio.h>
#include "CLI.h"


static int initHandler( const CommandContext_t *ctx )
{
const char *resource = ctx-> getArgument( ctx, "resource" );
bool verbose         = ctx-> getFlag( ctx, "verbose" );
const char *output   = ctx-> getOption( ctx, "output" );

   if( verbose )
   {
      puts( "Verbose mode enabled" );
   }

   if( resource != NULL )
   {
      printf( "Initializing resource: %s\n", resource );
   }

   if( output != NULL )
   {
      printf( "Output file: %s\n", output );
   }

   return CLI_SUCCESS;
}


int main( int argc, char *argv[] )
{
CLI_t *cli = newCLI( "Example CLI application" );
int err;

   if( cli == NULL )
   {
      fputs( "Failed to create CLI", stderr );
      return 1;
   }

   cli-> addCommand( cli, "init", "Initialize a resource", initHandler );
   cli-> addArgument( cli, "init", "resource", "Resource to initialize", false );
   cli-> addFlag( cli, "init", "verbose", 'v', "Verbose output" );
   cli-> addOption( cli, "init", "output", 'o', "Output file", false );

   err = cli-> parse( cli, argc, argv );
   if( err != CLI_SUCCESS )
   {
      fprintf( stderr, "Parse error: %d\n", err );
      cli-> delete( &cli );
      return 1;
   }

   cli-> delete( &cli );
   return 0;
}
```

## Building

```sh
make
```
