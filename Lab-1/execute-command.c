// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER_SIZE 100

/*      ----------- Structure for Queue --------------  */

typedef struct GraphNode{
  command_t command;
  struct GraphNode** before;       //Is an array of GraphNode pointers
  int before_size;                 //The size of the array 'before'
  pid_t pid;                       //Initialized to -1
  int buffer;
  struct GraphNode* next;
}GraphNode;


typedef struct Queue{
  struct GraphNode* front;
  struct GraphNode* end;
} Queue;

bool QueueIsEmpty(Queue* q)
{
  return (q->front == NULL);
}

void Queue_Insert(Queue* q, command_t command)
{
  //Make a new command_list node first
  struct GraphNode* toAdd = (GraphNode*)malloc(sizeof(GraphNode));
  toAdd->command = command;
  toAdd->pid = -1;
  toAdd->next = NULL;

  if(QueueIsEmpty(q))
    {
      q->front = toAdd;
      q->end = toAdd;
    }
  q->end->next = toAdd;
  q->end = toAdd;
}

command_t Queue_Next(Queue* q) {
  command_t next;
  GraphNode* toRemove;
  if(QueueIsEmpty(q))
    return 0;
  else if(q->front == q->end) //Last node left
    {
      toRemove = q->front;
      q->front = 0;
      q->end = 0;
    }
  else
    {
      toRemove = q->front;
      q->front = q->front->next;
    }

  next = toRemove->command;
  free(toRemove);

  return next;
}


typedef struct{
  Queue* no_dependencies;
  Queue* dependencies;
  int nodep_size;
  int dep_size;
  int nodep_buffer;
  int dep_buffer;
}Dependency_Graph;


void insertGraphNode(GraphNode_t gn, GraphNode_t new_node)
{
  if(gn->before == NULL)
    {
      gn->before = (GraphNode *) malloc(sizeof(GraphNode) * gn->buffer);
      if (gn->before == NULL)
	{
	  error(1,0,"malloc error");
	  exit(-1);
	}
    }
  else if (gn->before_size == gn->buffer)
    {
      gn->buffer = gn->buffer * 2;
      gn->before = (GraphNode *) realloc(gn->before, sizeof(GraphNode) * gn->buffer);
      if (gn->before == NULL)
	{
	  error(1,0,"Realloc error");
	  exit(-1)
	}
    }
  gn->before[gn->before_size++] = new_node;
  return;
}

typedef struct{
  ListNode next;
  GraphNode g_node;
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
      ln->read_list = (char **) realloc(sizeof(char *) * lnode->read_buffer);
      if (lnode->read_list == NULL)
	{
	  error(1,0,"malloc error");
	  exit(-1);
	}
    }
  lnode->read_list[ln->rl_size++] = s;
  return;
}

void addToWriteList(ListNode lnode, char *s)
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
      ln->write_list = (char **) realloc(sizeof(char *) * lnode->write_buffer);
      if (lnode->write_list == NULL)
	{
	  error(1,0,"malloc error");
	  exit(-1);
	}
    }
  lnode->write_list[ln->wl_size++] = s;
  return;
}

void processCommand(command_t command, ListNode lnode)
{
  if (command->type == SIMPLE_COMMAND)
    {
      for (int i = 1; command->u.word[i] != NULL; i++)
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
      processCommand(c->u.command[0], lnode);
      processCommand(c->u.command[1], lnode);
    }
  return;
}

bool isDependent(char **list_a, int list_a_size, char **list_b, int list_b_size)
{
  for (int i = 0; i < list_a_size; i++)
    {
      for (int j = 0; j < list_b_size; j++)
	{
	  if (strcmp(list_a[i], list_b[j]) == 0)
	    return true;
	}
    }
  return false;
}

Dependency_Graph createGraph(command_stream_t c_stream)
{
  Dependency_Graph d_graph = (Dependency_Graph) malloc(sizeof(Dependency_Graph));
  d_graph->no_dependencies = NULL;
  d_graph->dependencies = NULL;
  d_graph->nodep_size = 0;
  d_graph->dep_size = 0;
  d_graph->nodep_buffer = BUFFER_SIZE;
  d_graph->dep_buffer = BUFFER_SIZE;

  command_t command;
  ListNode list = NULL;
  while ((c = read_command_stream(c_stream)))
    {
      GraphNode g_node = (GraphNode) malloc(sizeof(GraphNode));
      g_node->command = command;
      g_node->before = NULL;     
      g_node->before_size = 0;
      g_node->pid = -1;               
      g_node->buffer = BUFFER_SIZE;
      
      ListNode l_node = (ListNode) malloc(sizeof(ListNode));
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

      ListNode temp_list = l_node->next;
      while (temp_list != NULL)
	{
	  if (l_node->read_list != NULL && temp_list->write_list != NULL && isDependent(l_node->read_list, l_node->rl_size, temp->write_list, temp->wl_size))
	    insertGraphNode(l_node->g_node, temp_list->g_node);
	  else if (l_node->write_list != NULL && temp_list->read_list != NULL && isDependent(l_node->write_list, l_node->wl_size, temp_list->read_list, temp_list->rl_size))
	    insertGraphNode(l_node->g_node, temp_list->g_node);
	  else if (l_node->write_list != NULL && temp_list->write_list != NULL && isDependent(l_node->write_list, l_node->wl_size, temp_list->write_list, temp_list->wl_size))
	    insertGraphNode(l_node->g_node, temp_list->g_node);
	  temp_list = temp_list->next;
	}
      if (l_node->g_node->before == NULL)
	Queue_Insert(d_graph->no_dependencies, command);
      else
	Queue_Insert(d_graph->dependencies, command);
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
				  error(1, 0, "Error redirecting output to file\n");
					return;
				}
				close(fd);
			}
			
			execvp( c->u.word[0], c->u.word );
			error(1, 0, "Error executing c->u.word[0]\n");
		}
		else if (child > 0) // parent process
		{
		  waitpid(child, &child_status, 0);
		  c->status = child_status;
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
		execute_command(c->u.subshell_command, false);
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

void executeGraph(Dependency_Graph* graph){
	executeNoDependencies(graph->no_dependencies);
	executeDependencies(graph->dependencies);
}

void executeNoDependencies(Queue* no_dependencies){
	GraphNode* iter = no_dependencies->front;
	while(iter != no_dependencies->end)
	{
		pid_t pid = fork();

		if(pid == 0)
			execute_command(iter->command, true);
		else if(pid > 0)
			iter->pid = pid;
		else
			error(1, errno, "could not fork");
		
		iter = iter->next;
	}
}

void executeDependencies(Queue* dependencies)
{
	GraphNode* iter = dependencies->front;
	while(iter != dependencies->end)
	{
		/* Wait for processes in graph node's before list to start */
		GraphNode* i = iter->before;
		
		for( int i = 0; i < iter->before_size; i++ ){
			while (iter->before[i]->pid == -1)
				continue;
		}

		/* Wait for process in current graph node's before list to finish */
		int status;

		for(int i = 0; i < iter->before_size; i++){
			waitpid(iter->before[i]->pid, &status, 0);
		}


		/* After processes in before list finish, can run yourself */
		pid_t pid = fork();
		if(pid == 0){
			execute_command(iter->command);
			_exit(0);
		}
		else if (pid > 0)
			temp->pid = pid;
		else
			error(1, errno, "could not fork");


		iter = iter->next;
	}
}
