# libCLI

A FreeBSD C library for building command-line interfaces with support for commands, subcommands, arguments, and flags.

## Features

- **Hierarchical Commands**: Support for commands and subcommands
- **Arguments**: Required and optional arguments with descriptions
- **Flags**: Long and short flags (e.g., `--verbose` and `-v`)
- **Help System**: Automatic help generation
- **Error Handling**: Standardized error codes and messages
- **Object-Oriented Design**

## API Documentation

### CLI_t *newCLI( void );
Creates a new CLI instance. Returns `NULL` on memory allocation failure.

### int addCommand( CLI_t *cli, const char *name, const char *description, int ( *handler )( CommandContext_t *context ) )
Adds a command. Returns `CLI_SUCCESS` on success, or a negative error code on failure.

### int addSubCommand( CLI_t *cli, const char *parentPath, const char *name, const char *description, int ( *handler )( CommandContext_t *context ) )
Adds a subcommand. Returns `CLI_SUCCESS` on success, or a negative error code on failure.

### int addArgument( CLI_t *cli, const char *path, const char *name, const char *description, bool required )
Adds an argument. Returns `CLI_SUCCESS` on success, or a negative error code on failure.

### int addFlag(CLI_t *cli, const char *path, const char *name, char shortName, const char *description)
Adds a flag. Returns `CLI_SUCCESS` on success, or a negative error code on failure.

### int parse( CLI_t *cli, int argc, char *argv[] )
Parses the command line. Returns `CLI_SUCCESS` on success, or a negative error code on failure (see Error Codes).

### void delete( CLI_t *cli )
Deletes the CLI instance and all associated memory.

### Context Methods

#### const char * getArgument( CommandContext_t *context, const char *name )
Gets the value of an argument from the context. Returns the argument value or `NULL` if not found.

#### bool getFlag( CommandContext_t *context, const char *name )
Gets the value of a flag from the context. Returns `true` if the flag is set.

## Error Codes

All major functions in libCLI return standardized error codes. These codes are defined in `includes/Errors.h`:

| Error Code                   | Value | Meaning                                      |
|------------------------------|-------|----------------------------------------------|
| `CLI_SUCCESS`                |     0 | Operation succeeded                          |
| `CLI_ERROR_MEMORY`           |    -1 | Memory allocation failed                     |
| `CLI_ERROR_INVALID_ARGUMENT` |    -2 | Invalid or missing argument                  |
| `CLI_ERROR_NOT_FOUND`        |    -3 | Item not found (e.g., subcommand, argument)  |
| `CLI_ERROR_ALREADY_EXISTS`   |    -4 | Item already exists                          |
| `CLI_ERROR_PARSE_FAILED`     |    -5 | Parsing failed (e.g., unknown flag)          |
| `CLI_ERROR_CONTEXT_FAILED`   |    -6 | Failed to create command context             |

Check the return value of all API calls and handle errors accordingly.
