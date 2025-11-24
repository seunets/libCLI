# libCLI

A lightweight C library for building command-line interfaces with support for commands, subcommands, arguments, and flags.

## Features

- **Hierarchical Commands**: Support for commands and subcommands
- **Arguments**: Required and optional arguments with descriptions
- **Flags**: Long and short flags (e.g., `--verbose` and `-v`) with proper validation
- **Help System**: Automatic help generation for commands and subcommands
- **Error Handling**: Standardized error codes and descriptive error messages
- **Memory Safety**: No memory leaks, validated with valgrind
- **Object-Oriented Design**

## API Documentation

### Core CLI Functions

#### `CLI_t * newCLI( const char *description )`
Creates a new CLI instance. Returns `NULL` on memory allocation failure.

#### `int addCommand( const CLI_t *cli, const char *name, const char *description, int ( *handler )( const CommandContext_t *context ) )`
Adds a command to the root level. Returns `CLI_SUCCESS` on success, or a negative error code on failure.

#### `int addSubCommand( const CLI_t *cli, const char *parentPath, const char *name, const char *description, int ( *handler )(const CommandContext_t *context ) )`
Adds a subcommand to a parent command. `parentPath` specifies the path to the parent (e.g., "parent subparent"). Returns `CLI_SUCCESS` on success, or a negative error code on failure.

#### `int addArgument( const CLI_t *cli, const char *path, const char *name, const char *description, bool required )`
Adds an argument to a command. `path` specifies the command path. Returns `CLI_SUCCESS` on success, or a negative error code on failure.

#### `int addFlag( const CLI_t *cli, const char *path, const char *name, char shortName, const char *description )`
Adds a flag to a command. `path` specifies the command path. Returns `CLI_SUCCESS` on success, or a negative error code on failure.

#### `int parse( const CLI_t *cli, int argc, char *argv[] )`
Parses the command line arguments. Returns `CLI_SUCCESS` on success, or a negative error code on failure.

#### `void delete( CLI_t *cli )`
Deletes the CLI instance and all associated memory.


### Command Context Methods

The `CommandContext_t` provides access to parsed arguments and flags within command handlers:

#### `const char * getArgument( const CommandContext_t *context, const char *name )`
Gets the value of an argument from the context. Returns the argument value or `NULL` if not found.

#### `bool getFlag( const CommandContext_t *context, const char *name )`
Gets the value of a flag from the context. Returns `true` if the flag is set, `false` otherwise.


## Error Codes

All major functions in libCLI return standardized error codes. These codes are defined in `includes/CLI.h`:

| Error Code                | Value | Meaning                                      |
|---------------------------|-------|----------------------------------------------|
| `CLI_SUCCESS`             | 0     | Operation succeeded                          |
| `CLI_ERROR_MEMORY`        | -1    | Memory allocation failed                     |
| `CLI_ERROR_INVALID_ARGUMENT` | -2 | Invalid or missing argument                  |
| `CLI_ERROR_NOT_FOUND`     | -3    | Item not found (e.g., subcommand, argument)  |
| `CLI_ERROR_ALREADY_EXISTS`| -4    | Item already exists                          |
| `CLI_ERROR_PARSE_FAILED`  | -5    | Parsing failed (e.g., unknown flag)          |
| `CLI_ERROR_CONTEXT_FAILED`| -6    | Failed to create command context             |

Always check the return value of all API calls and handle errors accordingly.


## Usage Example

```c
#include <stdio.h>
#include "CLI.h"

static int initHandler( const CommandContext_t *context )
{
   const char *resource = context-> getArgument( context, "resource" );
   bool verbose = context-> getFlag( context, "verbose" );

   if( verbose )
   {
      printf( "Verbose mode enabled\n" );
   }

   if( resource != NULL )
   {
      printf( "Initializing resource %s!\n", resource );
   }

   return CLI_SUCCESS;
}

int main( int argc, char *argv[] )
{
CLI_t *cli = newCLI( "Example" );
int err;

   if( cli == NULL )
   {
      fprintf( stderr, "Failed to create CLI instance.\n" );
      return 1;
   }

   err = cli-> addCommand( cli, "init", "Initialize a resource", initHandler );
   if( err != CLI_SUCCESS )
   {
      fprintf( stderr, "Failed to add command: %d\n", err );
      cli-> delete( cli );
      return 1;
   }

   cli-> addArgument( cli, "init", "resource", "Resource to initialize", false );
   cli-> addFlag( cli, "init", "verbose", 'v', "Verbose output" );

   err = cli-> parse( cli, argc, argv );
   if( err != CLI_SUCCESS )
   {
      fprintf( stderr, "Error parsing command line: %d\n", err );
      cli-> delete( cli );
      return 1;
   }

   cli-> delete( cli );

   return 0;
}
```
