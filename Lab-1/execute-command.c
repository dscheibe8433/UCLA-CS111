// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

#include <unistd.h>

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, bool time_travel)
{
	//Need to write implentation for simple command execution first
	//We can then use a recursive call to execute_command to execute other commands.


	switch( c->type ){
	case SIMPLE_COMMAND:
		
		//If there is an input redirection
		if( input != 0 )
		{
			int fd = open( input, O_RDONLY, 0444 );
			if ( fd < 0 )
			{
				//Is trying to read from a file that doesn't exist
				return;
			}

			dup2(fd, 0); //Changes the standard input to read from this file.
		}
		//If there is an output redirection
		if( output != 0 )
		{
			int fd = open( output, O_CREAT|O_TRUNC|O_WRONLY, 0644 );
			if ( fd < 0 ) 
				return;
			dup2(fd, 1); //redirects the output to the file specified
		}

		execvp( word[0], word );

		
	default:

	}

  error (1, 0, "command execution not yet implemented");
}
