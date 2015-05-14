// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <stdio.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define BUFFER_SIZE 100
#define INC_BUFSIZE 10

/*      ----------- Structure for Queue --------------  */

typedef struct GraphNode{
  command_t command;
  struct GraphNode** before;       //Is an array of GraphNode pointers
  int before_size;                 //The size of the array 'before'
  pid_t pid;                       //Initialized to -1
  int buffer;
  bool done;
}GraphNode;

void initGraphNode(GraphNode* gn, command_t command)
{
	gn->command = command;
	gn->pid = -1;
	gn->buffer = 10;
	gn->before_size = 0;
	gn->before = NULL;
	return;
}

void addDependency(DependencyGraph_t dg, GraphNode_t gn)
{
	return;
}

void addNoDependency(DependencyGraph_t dg, GraphNode_t gn)
{
	return;
}

typedef struct DependencyGraph{
  GraphNode_t* no_dependencies;
  GraphNode_t* dependencies;
  int nodep_size;
  int dep_size;
  int nodep_buffer;
  int dep_buffer;
}DependencyGraph;


void insertGraphNode(GraphNode_t gn, GraphNode_t new_node)
{
  if(gn->before == NULL)
    {
      gn->before = (GraphNode_t *) malloc(sizeof(struct GraphNode) * gn->buffer);
      if (gn->before == NULL)
	{
	  error(1,0,"malloc error");
	  exit(-1);
	}
    }
  else if (gn->before_size == gn->buffer)
    {
      gn->buffer = gn->buffer * 2;
      gn->before = (GraphNode_t *) realloc(gn->before, sizeof(struct GraphNode) * gn->buffer);
      if (gn->before == NULL)
	{
	  error(1,0,"Realloc error");
	  exit(-1);
	}
    }
  gn->before[gn->before_size++] = new_node;
  return;
}

typedef struct ListNode{
  ListNode_t next;
  GraphNode_t g_node;
  char **read_list;
  char **write_list;
  int read_buffer;
  int write_buffer;
  int rl_size;
  int wl_size;
}ListNode;

void addToReadList(ListNode_t lnode, char *s)
{
  if (lnode->read_list == NULL)
    {
      lnode->read_list = (char **) malloc(sizeof(char *) * lnode->read_buffer);
      if (lnode->read_list == NULL)
	{
	  error(1,0,"malloc error");
	  exit(-1);
	}
    }
  else if (lnode->rl_size == lnode->read_buffer)
    {
      lnode->read_buffer = lnode->read_buffer * 2;
      lnode->read_list = (char **) realloc(lnode->read_list,sizeof(char *) * lnode->read_buffer);
      if (lnode->read_list == NULL)
	{
	  error(1,0,"malloc error");
	  exit(-1);
	}
    }
  lnode->read_list[lnode->rl_size++] = s;
  return;
}

void addToWriteList(ListNode_t lnode, char *s)
{
  if (lnode->write_list == NULL)
    {
      lnode->write_list =(char **) malloc(sizeof(char *) * lnode->write_buffer);
      if (lnode->write_list == NULL)
	{
	  error(1,0,"malloc error");
	  exit(-1);
	}
    }
  else if (lnode->wl_size == lnode->write_buffer)
    {
      lnode->write_buffer = lnode->write_buffer *2;
      lnode->write_list = (char **) realloc(lnode->write_list, sizeof(char *) * lnode->write_buffer);
      if (lnode->write_list == NULL)
	{
	  error(1,0,"malloc error");
	  exit(-1);
	}
    }
  lnode->write_list[lnode->wl_size++] = s;
  return;
}

void processCommand(command_t command, ListNode_t lnode)
{
  if (command->type == SIMPLE_COMMAND)
    {
      int i;
      for (i = 1; command->u.word[i] != NULL; i++)
	{
	  if (command->u.word[i][0] != '-')
	    {
	      addToReadList(lnode, command->u.word[i]);
	    }
	}
      if (command->output != NULL)
	addToWriteList(lnode, command->output);
      if (command->input != NULL)
	addToReadList(lnode, command->input);
    }
  else if (command->type == SUBSHELL_COMMAND)
    {
      if (command->output != NULL)
	addToWriteList(lnode, command->output);
      if (command->input != NULL)
	addToReadList(lnode, command->input);
    }
  else
    {
      processCommand(command->u.command[0], lnode);
      processCommand(command->u.command[1], lnode);
    }
  return;
}

bool isDependent(char **list_a, int list_a_size, char **list_b, int list_b_size)
{
  int i;
  int j;
  for (i = 0; i < list_a_size; i++)
    {
      for (j = 0; j < list_b_size; j++)
	{
	  if (strcmp(list_a[i], list_b[j]) == 0)
	    return true;
	}
    }
  return false;
}

DependencyGraph_t createGraph(command_stream_t c_stream)
{
  DependencyGraph_t d_graph = (DependencyGraph_t) malloc(sizeof(struct DependencyGraph));
  d_graph->no_dependencies = (GraphNode_t*)malloc(sizeof(GraphNode));
  d_graph->dependencies = (GraphNode_t*)malloc(sizeof(struct GraphNode));
  d_graph->nodep_size = 0;
  d_graph->dep_size = 0;
  d_graph->nodep_buffer = BUFFER_SIZE;
  d_graph->dep_buffer = BUFFER_SIZE;

  command_t command;
  ListNode_t list = NULL;
  while ((command = read_command_stream(c_stream)))
    {
      GraphNode_t g_node = (GraphNode_t) malloc(sizeof(struct GraphNode));
      g_node->command = command;
      g_node->before = NULL;     
      g_node->before_size = 0;
      g_node->pid = -1;               
      g_node->buffer = BUFFER_SIZE;
      
      ListNode_t l_node = (ListNode_t) malloc(sizeof(struct ListNode));
      l_node->next = NULL;
      l_node->g_node = g_node;
      l_node->read_list = NULL;
      l_node->write_list = NULL;
      l_node->read_buffer = BUFFER_SIZE;
      l_node->write_buffer = BUFFER_SIZE;
      l_node->rl_size = 0;
      l_node->wl_size = 0;

      processCommand(command, l_node);

      if (list != NULL)
	l_node->next = list;

      ListNode_t temp_list = l_node->next;
      while (temp_list != NULL)
	{
	  if (l_node->read_list != NULL && temp_list->write_list != NULL && isDependent(l_node->read_list, l_node->rl_size, temp_list->write_list, temp_list->wl_size))
	    insertGraphNode(l_node->g_node, temp_list->g_node);
	  else if (l_node->write_list != NULL && temp_list->read_list != NULL && isDependent(l_node->write_list, l_node->wl_size, temp_list->read_list, temp_list->rl_size))
	    insertGraphNode(l_node->g_node, temp_list->g_node);
	  else if (l_node->write_list != NULL && temp_list->write_list != NULL && isDependent(l_node->write_list, l_node->wl_size, temp_list->write_list, temp_list->wl_size))
	    insertGraphNode(l_node->g_node, temp_list->g_node);
	  temp_list = temp_list->next;
	}
      if (l_node->g_node->before == NULL)
	addNoDependency(d_graph, l_node->g_node);
      else
	addDependency(d_graph, l_node->g_node);
      list = l_node;
    }
  return d_graph;
}

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
	//Should execute the command and then set the process's status so that it can return
	//the correct value.
    pid_t child;
    int child_status; 
	int fd[2];
	const int READ = 0;
	const int WRITE = 1;
	
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
				  error(1, 0, "File does not exit\n");
					//Is trying to read from a file that doesn't exist
					return;
				}

				if(dup2(fd, 0) < 0 ) //Changes the standard input to read from this file.
				{
				  error(1, 0, "Error changing standard input for reading from file\n");
					return;
				}

			}
			//If there is an output redirection
			if( c->output != 0 )
			{
				int fd = open( c->output, O_CREAT|O_TRUNC|O_WRONLY, 0644 );
				if ( fd < 0 ) 
					return;
				if(dup2(fd, 1) < 0) //redirects the output to the file specified
				{
				  error(1, 0, "Error redirecting output to file\n");
					return;
				}
				
			}
			
			execvp( c->u.word[0], c->u.word );
			printf("Executing %s", c->u.word[0]);
			error(1, 0, "Error executing c->u.word[0]\n");
		}
		else if (child > 0) // parent process
		{
		  waitpid(child, &child_status, 0);
		  c->status = child_status;
		  return;
			//PARENT PROCESS STUFF
		}
		else
		{
		  error(1, 0, "Error creating child through fork()\n");
			//NO CHILD PROCESS CREATED
		}
		break;

	case AND_COMMAND: //return status of cmd[0] if false, else cmd[1] status if cmd[0] is true
	        execute_command(c->u.command[0], time_travel);
		c->status = c->u.command[0]->status;
		if ( c->status == 0 )
	        {
		  execute_command(c->u.command[1], time_travel);
		  c->status = c->u.command[1]->status;
		}
		break;
	case OR_COMMAND: //return status of cmd[0] if true, else return cmd[1] status
	        execute_command(c->u.command[0], time_travel);
		c->status = c->u.command[0]->status;
		if ( c->status != 0 )
		{
		  execute_command(c->u.command[1], time_travel);
		  c->status = c->u.command[1]->status;
		}
		break;
	case SEQUENCE_COMMAND:
		execute_command(c->u.command[0], time_travel);
		c->status = c->u.command[0]->status;
		if (c->u.command[1] != 0)
	        {
		    execute_command(c->u.command[1], time_travel);
		    c->status = c->u.command[1]->status;
        	}
              	break;
	case SUBSHELL_COMMAND:
		execute_command(c->u.subshell_command, time_travel);
		c->status = c->u.subshell_command->status;
		break;
	case PIPE_COMMAND:
		if(pipe(fd) < 0)
			error(1, 0, "Error creating pipe\n");
		pid_t firstpid = fork();
	        
		if (firstpid == 0)
		{
			//Child process will execute right command
			close(fd[WRITE]);
			if(dup2(fd[READ], READ) < 0)
				error(1, 0, "Error redirecting read end\n");
			execute_command( c->u.command[1], time_travel );
			c->status = c->u.command[1]->status;
			close(fd[READ]);
			exit(c->status);
		}

		else if (firstpid > 0)
		{
			int secondpid = fork();
			if(secondpid == 0)
			{
				//Second child process will execute left command.
				close(fd[0]);
				if(dup2(fd[WRITE], WRITE) < 0)
					error(1, 0, "Error redirecting write end\n");
				execute_command( c->u.command[0], time_travel );
				c->status = c->u.command[0]->status;
				close(fd[WRITE]);
			}
			else if( secondpid > 0)
			{
				close(fd[READ]);
				close(fd[WRITE]);
				int status;
				int returnedpid = waitpid(-1, &status, 0);
				//first process finished first
				if(returnedpid == secondpid)
					waitpid(firstpid, &status, 0);

				//second process finished first
				if(returnedpid == firstpid)
					waitpid(secondpid, &status, 0);
			}

		}
		else
		{
			error(1, 0, "Error creating first child process\n");
		}
		
		break;

	default:

		//NOT A VALID COMMAND TYPE
		  error (1, 0, "Command type invalid\n");
		  break;
	}

}


void executeNoDependencies(DependencyGraph_t dg){
	int i = 0;
	
	for(i = 0; i < dg->nodep_size; i++)
	{
		pid_t pid = fork();

		if(pid == 0)
		{
			execute_command(dg->no_dependencies[i]->command, true);
			_exit(0);
		}
		else if(pid > 0)
			dg->no_dependencies[i]->pid = pid;
		else
			error(1, 0, "could not fork");
	}

}

void executeDependencies(DependencyGraph_t dg)

	int i;

	for(i = 0; i < dg->dep_size; i++){
		int j;
		GraphNode_t iter = dg->dependencies[i];

		/* Wait for processes in graph node's before list to start */
		
		for(j = 0; j < iter->before_size; j++ ){
			while (iter->before[j]->pid == -1)
				continue;
		}
		

		/* Wait for process in current graph node's before list to finish */
		int status;

		for(j = 0; j < iter->before_size; j++){
			if(!iter->before[j]->done){
				waitpid(iter->before[j]->pid, &status, 0);
				iter->before[j]->done = true;
			}
		}


		/* After processes in before list finish, can run yourself */
		pid_t pid = fork();
		if(pid == 0){
			execute_command(iter->command, true);
			_exit(0);
		}
		else if (pid > 0)
			iter->pid = pid;
		else
			error(1, 0, "could not fork");
	}
}

int executeGraph(DependencyGraph_t graph){
	if(graph == NULL)
		printf("null");
	executeNoDependencies(graph);
	executeDependencies(graph);
	return 1;
}
