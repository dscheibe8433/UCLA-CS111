// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

#include <unistd.h>

#define DEBUG 1

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, bool time_travel)
{
  if ( DEBUG == 1)
    printf("Beginning of execute_command");
  
	//Need to write implentation for simple command execution first
	//We can then use a recursive call to execute_command to execute other commands.
<<<<<<< HEAD
  int status_to_return;
=======
	//Should execute the command and then set the process's status so that it can return
	//the correct value.
>>>>>>> dd3c6a639c807761153ccbfc50375e53fb5d2d2b

	switch( c->type ){
	case SIMPLE_COMMAND:
		
		// create child to execute command
		child = fork();
		
		if (child == 0) // child process
		{

			//If there is an input redirection
			if( c->input != 0 )
			{
				int fd = open( c->input, O_RDONLY, 0444 );
				if ( fd < 0 )
				{
					//Is trying to read from a file that doesn't exist
					return;
				}

				if(dup2(fd, 0) < 0 ) //Changes the standard input to read from this file.
				{
					return;
				}

				close(fd);
			}
			//If there is an output redirection
			if( c->output != 0 )
			{
				int fd = open( c->output, O_CREAT|O_TRUNC|O_WRONLY, 0644 );
				if ( fd < 0 ) 
					return;
				if(dup2(fd, 1) < 0) //redirects the output to the file specified
				{
					return;
				}
				close(fd);
			}

			execvp( c->u.word[0], c->u.word );
		}
		else if (child > 0) // parent process
		{
		  waitpid(child, &status, 0);
		  c->status = status;//PARENT PROCESS STUFF
		}
		else
		{
		  //NO CHILD PROCESS CREATED
		}
		break;

<<<<<<< HEAD
	case AND_COMMAND: // AND COMMAND
	        execute_command(c->u.command[0], time_travel);
		c->status = c->u.command[0]->status;
		if (c->status == 0) // set status of AND command. Equals second command if 1st is true
		{
		    execute_command(c->u.command[1], time_travel);
		    c->status = c->command[1]->status;
		}
		break;
	case OR_COMMAND: // OR COMMAND
	        execute_command(c->u.command[0], time_travel);
		c->status = c->u.command[0]->status;
		if (c->status != 0) // set status of AND command. Equals 1st command if its true, second if not
		{
		  execute_command(c->u.command[1], time_travel);
		  c->status = c->u.command[1]->status;
		}
	        break;
	case SEQUENCE_COMMAND:  // SEQUENCE COMMAND
=======
	case AND_COMMAND:
		break;
	case OR_COMMAND:
		break;
	case SEQUENCE_COMMAND:
		execute_command(c->u.command[0], time_travel);
		execute_command(c->u.command[1], time_travel);
		c->status = c->u.command[1]->status;
		break;
	case SUBSHELL_COMMAND:
		execute_command(c->u.subshell_command, false);
		c->status = c->u.subshell_command->status;
>>>>>>> dd3c6a639c807761153ccbfc50375e53fb5d2d2b
		break;
	case SUBSHELL_COMMAND:  //SUBSHELL COMMAND. execute subshell command[0] and update status
	        execute_command(c->u.subshell_command, 0);
		c->status = c->u.subshell_command->status);
	        break;
        case PIPE_COMMAND:  //PIPE COMMAND
		break;
	default:
		//NOT A VALID COMMAND TYPE
	}

  error (1, 0, "command execution not yet implemented");
}
