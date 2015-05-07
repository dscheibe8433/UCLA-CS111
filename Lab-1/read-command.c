// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

//if DEBUG = 0, no debug
//if DEBUG = 1, debug stack
//if DEBUG = 1, debug tokensToNodes
//if DEBUG = 1, debug make_command_stream

#define DEBUG 0
#define SIZE_BUFFER 10000
#define SIZE_COM_ALLOC 100

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */


/*	----------- Structure for Queue --------------  */

typedef struct{
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
	struct GraphNode* toAdd = (GraphNode*)checked_malloc(sizeof(GraphNode));
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

struct command_t Queue_Next(Queue* q)
{
	command_t next;
	GraphNode* toRemove;
	if(QueueIsEmpty(q))
		return NULL;
	else if(q->front == q->end) //Last node left
	{
		toRemove = q->front;
		q->front = NULL;
		q->end = NULL;
	}
	else
	{
		toRemove = q->front;
		q->front = q->front->next;
	}

	next = toRemove->command;
	free(toRemove);

	return result;
}


/*	----------- Structure for Dependency_Graph --------------  */

typedef struct{
	Queue* no_dependencies;
	Queue* dependencies;
}Dependency_Graph;

typde struct{
	command_t command;
	struct GraphNode** before;		//Is an array of GraphNode pointers
	int before_size;				//The size of the array 'before'
	pid_t pid;						//Initialized to -1
	GraphNode* next;				//Each one is also a node in the Queue or linked list
} GraphNode;

/*	----------- Stuff from 1a --------------  */

//Nodes of command_stream
struct command_list
{
  command_t command;
  struct command_list *next;
};


//Linked list of command trees
struct command_stream
{
  struct command_list *start;
  struct command_list *end;
  struct command_list *cursor;
};

// adds command tree from stack_create_tree to command stream
void addCommand(struct command_stream *c_stream, command_t command)
{
  struct command_list *new_command_list = (struct command_list *) checked_malloc(sizeof(struct command_list));
  new_command_list->command = command;
  if ( c_stream->start != NULL )
  {
    if ( DEBUG == 1 )
      printf("addCommand: command added to non-empty command stream\n");
    c_stream->end->next = new_command_list;
    c_stream->end = new_command_list;
    new_command_list->next = NULL;
  }
  else
  {
    if ( DEBUG == 1 )
      printf("addCommand: command added to empty command stream\n");
    c_stream->start = new_command_list;
    c_stream->end = new_command_list;
    new_command_list->next = NULL;
  }
}

//My stack structure
struct stack
{
  struct command *command;
  struct stack *next;
};

//stack pop function that pops and returns the top of the stack
struct command* stackPop(struct stack **top_of_stack)
{
  if (*top_of_stack == NULL)
  {
    if (  DEBUG == 1 )
	    printf("stackPop: top of stack is NULL\n");
    return NULL;
  }
  struct stack *temporary_top = (*top_of_stack);
  struct command *command = (*top_of_stack)->command;
  *top_of_stack = (*top_of_stack)->next;
  free(temporary_top);
  if ( DEBUG == 1 )
    printf("stackPop: top of stack popped off\n");
  return command; 
}

//stack push function that adds onto the stack
void stackPush(struct stack **top_of_stack, struct command *command)
{
  if (  DEBUG == 1 )
  {
    printf("stackPush: top of stack = %p\n", top_of_stack);
    printf("stackPush: command being pushed = %d\n", command->type);
  }
  struct stack *temporary_stack = (struct stack *) checked_malloc(sizeof(struct stack));
  temporary_stack->command = command;
  temporary_stack->next = *top_of_stack;
  *top_of_stack = temporary_stack;
}

bool stackEmpty(struct stack *top_of_stack)
{
  if (top_of_stack == NULL)
  {
    if ( DEBUG == 1 )
      printf("stackEmpty: Top of stack is NULL/empty\n");
    return true;
  } 
  else 
  {
    if ( DEBUG == 1 )
        printf("stackEmpty: Top of stack is not NULL/has item\n");
    return false;
  }
}

struct command* stackTop(struct stack **top_of_stack)
{
  return ((*top_of_stack)->command);
}

void printError(int line_error)
{
  error(1, 0, "STOP!!! - Syntax error has occoured on line: %d", line_error);
}

struct token
{
  char *word;
  struct token *next;
};

struct token_list
{
  struct token *start;
  struct token *end;
};

void addToken(struct token *t, struct token_list *t_list)
{
  if (t_list->start == NULL)
  { 
    if ( DEBUG == 1 )
      printf("addToken: token added to empty token list\n");
    t_list->start = t;
    t_list->end = t;
  }
  else
  {
    if ( DEBUG == 1 )
      printf("addToken: token added to non-empty token list\n");
    t_list->end->next = t;
    t_list->end = t;
  }
}  
		
enum node_type 
{
  SIMPLE = 1,
  SEMI = 2,
  AND = 3,
  OR = 3,
  PIPE = 4,
  LEFT_PAREN = 5,
  RIGHT_PAREN = 5,
};

struct node 
{
  enum node_type type;
  command_t command_SIMPLE;
  struct node *next;
  bool is_AND;
  bool is_LEFT_PAREN;
};

struct node_list
{
  struct node *start;
  struct node *end;
};
  
void addNode(struct node *n, struct node_list *n_list)
{
  if (n_list->start == NULL)
  {
    if ( DEBUG == 1 )
      printf("addNode: node added to empty node list\n");
    n_list->start = n;
    n_list->end = n;
  }
  else
  {
    if ( DEBUG == 1 )
      printf("addNode: node added to empty node list\n");
    n_list->end->next = n;
    n_list->end = n; 
  }
};

//gets all characters/bytes from the script being run
char* pullBytes(int (*get_next_byte) (void*), void *get_next_byte_argument)
{
  size_t line_size = SIZE_COM_ALLOC;
  char *line = (char *) checked_malloc(sizeof(char *) * line_size);
  int i = (*get_next_byte)(get_next_byte_argument);
  
  if ( i == EOF || i < 0 )
    return NULL;
    
  size_t position = 0;
  while( true )
  {
    if ( DEBUG != 0 )
      printf("Reading char = %c\n", (char)i);
    if ( position == line_size )
    {
      line_size *= 2;
      line = (char *) checked_grow_alloc((void *) line, &line_size);
    }
    if ( i < 0 || ((char) i) == '\n' )
    {
      line[position] = '\0';
      return line;
    }
    line[position] = ((char) i);
    position++;
    i = (*get_next_byte)(get_next_byte_argument);
  } 
  if ( DEBUG == 1 )
      printf("pullBytes: Finished reading input\n");
}

bool specialToken(char c)
{
  return strchr(";|&()", c);
  /*
  char* s;
  s = strchr(";|&()", c);
  if ( s == NULL )
    return false;
  return true;
  */
}

char* getString(char *line, int start, int end)
{
  size_t size = end - start + 1;
  char *my_string = (char *) checked_malloc(size+1);
  strncpy(my_string, line + start, size);
  my_string[size] = '\0';
  return my_string;
}

bool validWord(char c)
{
  if ( strchr("!%+-,./:@^_<>", c) || isalnum(c) )
    return true;
  return false;
}

//check if the token as any characters in it
bool ifEmptyToken(char *c)
{
  int pos = 0;
  bool if_char = false;
  while (c[pos] != '\0')
  {
    if ( c[pos] != ' ' )
      if_char = true;
    pos++;
  }
  return if_char;
}

struct command* create_stack_tree(struct node_list *n_list)
{
  struct node *cursor = n_list->start;
  struct stack *command_stack = NULL;
  struct stack *operator_stack = NULL;
  if ( cursor == NULL )
    return NULL;

  bool b_subshell = false;
  int num_parens = 0;
  unsigned int op_stack_prec = 0;

  if ( DEBUG == 1 )
  printf("create_stack_tree: Entered stack creation and set cursor\n");

  while ( cursor != NULL )
  {
    if ( cursor->type == SIMPLE )
    {
      struct command *temp_command = (struct command *) checked_malloc(sizeof(struct command));
      temp_command->type = SIMPLE_COMMAND;
      temp_command->status = -1;
      temp_command->input = cursor->command_SIMPLE->input;
      temp_command->output = cursor->command_SIMPLE->output;
      temp_command->u.word = cursor->command_SIMPLE->u.word;
      stackPush(&command_stack, temp_command);
      if ( DEBUG == 1 )
      {
        printf("create_stack_tree: found simple command\n");
        printf("create_stack_tree: SIMPLE command type = %d\n", temp_command->type);
      }
      //stackPush(&command_stack, cursor->command_SIMPLE);
      b_subshell = false;
    }
    else 
    {
      if ( cursor->type == RIGHT_PAREN && cursor->is_LEFT_PAREN == false )
        return NULL;
      else if ( cursor->type == LEFT_PAREN && cursor->is_LEFT_PAREN == true )
      {    
        num_parens++;
        if (b_subshell)
          return NULL;
        struct node *iterator_node = cursor;
        struct node *saved_cursor_node = cursor;
        cursor = cursor->next;
        if ( cursor == NULL )
          return NULL;
        while ( cursor != NULL)
        {
          if ( cursor->type == RIGHT_PAREN )
            num_parens--;
          else if ( cursor->type == LEFT_PAREN && cursor->is_LEFT_PAREN )
            num_parens++;
          if ( num_parens == 0 ) // found matching paren
          {
            iterator_node->next = NULL;
            break;
          }
          else
          {
            iterator_node = iterator_node->next;
            cursor = cursor->next;
          }
        }
        if ( num_parens > 0 )
          return NULL;

        if ( DEBUG == 1 )
        printf("create_stack_tree: created subshell command and pushed onto op stack\n");
        struct node_list *subshell_list = (struct node_list *) checked_malloc(sizeof(struct node_list));
        subshell_list->start = saved_cursor_node;
        subshell_list->end = iterator_node;
        struct command *command_subshell = create_stack_tree(subshell_list);
        if ( command_subshell != NULL )
        {
        struct command *root_subshell = (struct command *) checked_malloc(sizeof(struct command));
        root_subshell->type = SUBSHELL_COMMAND;
        root_subshell->u.subshell_command = command_subshell;
        stackPush(&command_stack, root_subshell);
        if ( DEBUG == 1 )
          printf("create_stack_tree: pushed subshell command to stack");
        }
        b_subshell = true;
        iterator_node->next = cursor;
      }
      else if ( cursor->type <= op_stack_prec ) 
      {
        op_stack_prec = cursor->type;
        if ( op_stack_prec == PIPE || op_stack_prec == AND || op_stack_prec == OR || op_stack_prec == SEMI )
        {
          if ( DEBUG == 1 )
          {
            printf("create_stack_tree: op is ||, &&, |, or ;\n");
            printf("create_stack_tree: op popped\n");
          }
          struct command *command = stackPop(&operator_stack);
          if ( command->type == SEQUENCE_COMMAND )
          {
            command->u.command[0] = stackPop(&command_stack);
            if ( command->u.command[0] == NULL )
              return NULL;
            if ( stackEmpty(command_stack) )
              command->u.command[1] = '\0';
            else
            {
              struct command *temp2 = stackPop(&command_stack);
              if ( temp2 == NULL )
                return NULL;
              command->u.command[1] = command->u.command[0];
              command->u.command[0] = temp2;
            }
          }
          else
          {
            struct command *temp3 = stackPop(&command_stack);
            struct command *temp4 = stackPop(&command_stack);
            if ( temp3 == NULL || temp4 == NULL )
              return NULL;
            command->u.command[1] = temp3;
            command->u.command[0] = temp4;
          }
          if ( DEBUG == 1 )
            printf("create_stack_tree: operand command created and pushed\n");
          stackPush(&command_stack, command);

          struct command *operator = (struct command *) checked_malloc(sizeof(struct command)); 
          operator->input = 0;
          operator->output = 0;
          operator->status = -1;
          if ( op_stack_prec == PIPE )
            operator->type = PIPE_COMMAND;
          else if ( op_stack_prec == OR && cursor->is_AND == false)
            operator->type = OR_COMMAND;
          else if ( op_stack_prec == AND && cursor->is_AND )
            operator->type = AND_COMMAND;
          else if ( op_stack_prec == SEMI )
            operator->type = SEQUENCE_COMMAND;
          stackPush(&operator_stack, operator);
          if ( DEBUG == 1 )
            printf("create_stack_tree: operator command pushed onto stack\n");
        }
        b_subshell = false;  
      }
      else
      {
        op_stack_prec = cursor->type;
        struct command * operator = (struct command *) checked_malloc(sizeof(struct command));
        if ( op_stack_prec == PIPE )
          operator->type = PIPE_COMMAND;
        else if ( op_stack_prec == OR && cursor->is_AND == false)
          operator->type = OR_COMMAND;
        else if ( op_stack_prec == AND && cursor->is_AND )
          operator->type = AND_COMMAND;
        else if ( op_stack_prec == SEMI )
          operator->type = SEQUENCE_COMMAND;
        operator->status = -1;
        operator->input = 0;
        operator->output = 0;
        stackPush(&operator_stack, operator);
        if ( DEBUG == 1 )
          printf("create_stack_tree: operator pushed onto stack\n");
        b_subshell = false;
      }
    }
    cursor = cursor->next;
    if ( DEBUG == 1 )
      printf("create_stack_tree: next node in node_list\n");
  }
  if ( DEBUG == 1 )
    printf("create_stack_tree: reached end node of node_list/1st while loop\n");

  while( stackEmpty(operator_stack) == false )
  {
    if ( DEBUG == 1 )
      printf("create_stack_tree: operator stack not empty, popped operator\n");
    struct command *operator = stackPop(&operator_stack);
    if ( operator->type == SEQUENCE_COMMAND )
    {
      struct command *temp5 = stackPop(&command_stack);
      if ( temp5 == NULL )
        return NULL;
      operator->u.command[0] = temp5;
      if ( stackEmpty(command_stack) )
        operator->u.command[1] = '\0';
      else
      {
        struct command *temp6 = stackPop(&command_stack);
        if ( temp6 == NULL )
          return NULL;
        operator->u.command[1] = operator->u.command[0];
        operator->u.command[0] = temp6;
      }
    }
    else
    {
      struct command *temp7 = stackPop(&command_stack);
      struct command *temp8 = stackPop(&command_stack);
      if ( temp7 == NULL || temp8 == NULL )
        return NULL;
      operator->u.command[1] = temp7;
      operator->u.command[0] = temp8;
    }
    stackPush(&command_stack, operator);
    if ( DEBUG == 1 )
      printf("create_stack_tree: combined commands\n");
  }
  if ( DEBUG == 1 )
    printf("create_stack_tree: Finished\n");
  struct command* return_command = stackPop(&command_stack);
  if ( return_command == NULL )
  {
    if ( DEBUG == 1 )
      printf("create_stack_tree: function returning NULL\n");
    return NULL;
  }
  if ( DEBUG == 1 )
    printf("create_stack_tree: returning command\n");
  return return_command;
}

//convert tokens into operations and commands
// use NULL for word[number_of_words] equating
// use '\0' for word[number_of_words][BLANKK] equating
struct node_list* tokensToNodes(struct token_list *t_list)
{
    if ( DEBUG == 1 )
        printf("tokensToNodes: Begin converting tokens to nodes\n");
    struct node_list *n_list = (struct node_list *) checked_malloc(sizeof(struct node_list));
    n_list->start = NULL;
    n_list->end = NULL;
    
    int counter = 0;
    bool end_is_SIMPLE = false;
    bool empty_input = true;
    bool end_is_PAREN = false;
    
    while ( t_list->start != NULL ) 
      {
        // If token is an operator, create an operator
        struct node *n = (struct node *) checked_malloc(sizeof(struct node));
        n->next = NULL;
        n->is_AND = false; 
        n->is_LEFT_PAREN = false;
        
        // Operator
        if ( specialToken(t_list->start->word[0]) ) 
        {
          if ( DEBUG == 1) 
            printf("tokensToNodes: token is an operator\n");
          if (counter == 0 && t_list->start->word[0] != '(')
            return NULL;
            
          n->command_SIMPLE = NULL;
          switch (t_list->start->word[0]) 
            {
              default:
                break;
              case ';':
                if ( end_is_SIMPLE == false && end_is_PAREN == false )
                  return NULL;
                n->type = SEMI;
                break;
              case '(':
                if (end_is_SIMPLE) {
                    return NULL;
                }
                n->type = LEFT_PAREN;
                n->is_LEFT_PAREN = true;
                break;
              case ')':
                n->type = RIGHT_PAREN;
                end_is_PAREN=true;
                break;
              case '|':
                if (t_list->start->word[1] == '|')
                  n->type = OR;
                else
                  n->type = PIPE;
                  break;
              case '&':
                n->type = AND;
                n->is_AND = true;
                break;
            }
            end_is_SIMPLE = false;
          }
          // toke is a SIMPLE command
          else 
          {
            if ( DEBUG == 1 )
              printf("tokensToNodes: token is a SIMPLE command\n");
            n->type = SIMPLE;
            end_is_SIMPLE = true;
            
            // Create a simple command
            struct command *command_simple = (struct command *) checked_malloc(sizeof(struct command));
            command_simple->type = SIMPLE_COMMAND;
            command_simple->status = -1;
            command_simple->input = 0;
            command_simple->output = 0;
            command_simple->u.word = (char **) checked_malloc(sizeof(char *) * 100);
            int number_of_words = 0;
            int word_position = 0;
            int next_word_pos = 0;
            int in_char_pos = 0; 
            int out_char_pos = 0;
            bool next_word = false;
            bool in_redirect = false; 
            bool out_redirect = false;
            bool in_word = false; 
            bool out_word= false;
            char *word = t_list->start->word;
            
            while ( true ) 
              {
                if ( DEBUG == 1 )
                {
                printf("tokensToNodes: counter:%d, word_pos:%d, next_word_pos:%d, number_of_words:%d\n",
                  counter, word_position, next_word_pos, number_of_words);
                }
                if ( word[word_position] == '\0' ) 
                {
                  if ( in_word ) 
                  {
                    command_simple->input[in_char_pos] = '\0';
                    in_redirect = false; 
                    in_word = false;
                    if ( DEBUG == 1 )
                      printf("tokensToNodes: command->input updated: command->input:%s in_redirect:false\n", 
                        command_simple->input);  
                  }
                  else if ( out_word ) 
                  {
                      if (out_char_pos == 0)
                      {
                        if ( DEBUG == 1 )
                          printf("tokensToNodes: command->output word is empty\n");
                        return NULL;
                      }
                      command_simple->output[out_char_pos] = '\0';
                      in_redirect = false; 
                      out_word = false;
                      if ( DEBUG == 1 )
                        printf("tokensToNodes: command->output: %s out_redirect:false\n", command_simple->output);
                  }
                  else if ( next_word ) 
                  {
                      command_simple->u.word[number_of_words][next_word_pos] = '\0';
                      if ( DEBUG == 1 )
                        printf("tokensToNodes: word added:%s to command->word[%d], next_word_pos=%d\n", 
                          command_simple->u.word[number_of_words], number_of_words, next_word_pos);
                      number_of_words++;
                      command_simple->u.word[number_of_words] = NULL;
                      next_word = false;
                  } 
                  else 
                  {
                      command_simple->u.word[number_of_words] = NULL;
                  }
                  
                  break;
                }
                else if ( word[word_position] == '<' ) 
                {
                  if ( DEBUG == 1 )
                    printf("tokensToNodes: Encountered a <LEFT_REDIRECT<\n");
                  if ( out_word || out_redirect )
                    return NULL;
                  if ( next_word ) 
                  {
                    command_simple->u.word[number_of_words][next_word_pos] = '\0';
                    if ( DEBUG == 1 )
                      printf("tokensToNodes: word: %s inserted into u.word:%d", 
                        command_simple->u.word[number_of_words], number_of_words);
                    number_of_words++;
                    next_word = false;
                  }
                  command_simple->u.word[number_of_words] = NULL;
                  
                  // if IO redirection is at beginning
                  if (number_of_words == 0 || in_redirect)
                    return NULL;                    
                  command_simple->input = (char *)checked_malloc(sizeof(char)*100);
                  if ( DEBUG == 1 )
                    printf("tokensToNodes: in_redirect: true - redirect to input\n");
                  in_char_pos = 0;
                  in_redirect = true;
                  word_position++;
                }
                else if (word[word_position] == '>') 
                {
                  if ( DEBUG == 1 )
                  {
                    printf("tokensToNodes: Encountered a >RIGHT_REDIRECT>\n");
                    printf("tokensToNodes: in_redirect:%d, in_word:%d, out_redirect:%d, out_word:%d",
                      in_redirect, in_word, out_redirect, out_word);
                  }
                  if ( in_redirect && empty_input )
                    return NULL;
                  
                  // If last char was part of in_word
                  if ( in_word ) 
                  {
                    command_simple->input[in_char_pos] = '\0';
                    in_redirect = false; 
                    in_word = false;
                  }
                  if ( next_word ) 
                  {
                    command_simple->u.word[number_of_words][next_word_pos] = '\0';
                    if ( DEBUG == 1 )
                      printf("tokensToNodes: word %s added to index: %d\n", command_simple->u.word[number_of_words], number_of_words);
                    number_of_words++;
                    next_word = false;
                  }
                  command_simple->u.word[number_of_words] = NULL;
                  
                  if ( number_of_words == 0 || out_redirect )
                    return NULL;
                  
                  command_simple->output = (char *)checked_malloc(sizeof(char)*100);
                  out_char_pos = 0;
                  in_redirect = false;
                  out_redirect = true;
                  if ( DEBUG == 1 )
                    printf("tokensToNodes: IO redirect output: true\n");
                  word_position++; 
                }
                // If char is equal to white space
                else if ( word[word_position] == ' ' ) 
                {
                  if ( DEBUG == 1 )
                    printf("tokensToNodes: Encountered     WHITE_SPACE    \n");
                  if ( in_redirect && in_word ) 
                  {
                    command_simple->input[in_char_pos] = '\0';
                    in_word = false;
                    if ( DEBUG == 1 )
                      printf("tokensToNodes: command->input:%s, in_redirect:false\n", command_simple->input);
                  }
                  else if ( out_redirect && out_word ) 
                  {
                    command_simple->output[out_char_pos] = '\0';
                    out_word = false;
                    if ( DEBUG == 1 )
                      printf("tokensToNodes: command->output:%s, out_redirect:false\n", command_simple->output);
                  }
                  // If current_listy a word, parse the word
                  else if ( next_word ) 
                  {
                    command_simple->u.word[number_of_words][next_word_pos] = '\0';
                    if ( DEBUG == 1 ) 
                      printf("tokensToNodes: word:%s added to command->word[%d]  next_word_pos=%d\n", command_simple->u.word[number_of_words], number_of_words, next_word_pos);
                    if ( DEBUG == 1 )
                    {
                      printf("%s-",command_simple->u.word[number_of_words]);
                      int tempPos = 0;
                      while (command_simple->u.word[number_of_words][tempPos] != '\0') 
                      {
                        printf("%c", command_simple->u.word[number_of_words][tempPos]);
                        tempPos++;
                      }
                      printf("|\n");
                    }
                    word_position++;
                    number_of_words++;
                    next_word = false; 
                    next_word_pos = 0;
                  }
                  else // cannot parse
                  {
                    if ( DEBUG == 1 ) printf("tokensToNodes: Blank whitespace\n");
                    word_position++;
                  }
                }
                else // all that is left is for char to be a character
                {
                    if ( in_redirect ) 
                    {
                      if ( DEBUG == 1 ) 
                        printf("tokensToNodes: word is input\n");
                      command_simple->input[in_char_pos] = word[word_position];
                      in_char_pos++; word_position++;
                      in_word = true;
                      empty_input = false;
                    }
                    else if ( out_redirect ) 
                    {
                      if ( DEBUG == 1 )
                        printf("tokensToNodes: word is output\n");
                      command_simple->output[out_char_pos] = word[word_position];
                      out_char_pos++; 
                      word_position++;
                      out_word = true;
                    }
                    else if ( next_word == false ) // If not in a word, set next_word and create a new word
                    {
                      command_simple->u.word[number_of_words] = (char *)checked_malloc(sizeof(char)*100);
                      next_word_pos = 0;
                      command_simple->u.word[number_of_words][next_word_pos] = word[word_position];
                      next_word_pos++;
                      word_position++;
                      next_word = true;
                    }
                    else //continue
                    {
                      command_simple->u.word[number_of_words][next_word_pos] = word[word_position];
                      next_word_pos++;
                      word_position++;
                    }
                }
            }
            n->command_SIMPLE = command_simple;
        }
        
        // add completed node to n_list
        if ( DEBUG == 1 )
          printf("tokensToNodes: Completed iteration %d, node->type added: %d\n", counter, n->type);
        addNode(n, n_list);
        
        // free token and iterate to next
        struct token *temp = t_list->start;
        t_list->start = t_list->start->next;
        free(temp);
        counter++;
    }
    if ( DEBUG == 1 ) 
      printf("Leaving tokensToNodes\n");
    return n_list;
}


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  if ( DEBUG == 1 )
    printf("make_command_stream: Starting!\n");
  //create return command_stream
  struct command_stream *c_stream = (struct command_stream *) checked_malloc(sizeof(struct command_stream));
  //read line from script
  char *c_line = (char *) checked_malloc(sizeof(char *) * SIZE_BUFFER);
  c_line = pullBytes(*get_next_byte, get_next_byte_argument);

  //create token list and initialize to NULL
  struct token_list *t_list = (struct token_list *) checked_malloc(sizeof(struct token_list));
  t_list->start = NULL;
  t_list->end = NULL;

  int c_line_pos = 0;
  int start_pos = 0;
  int num_lines = 1;
  int num_parens = 0;
  bool new_word = false;
  bool line_empty = true;
  bool line_ignore = false;
  bool line_NULL = true;
  bool special_token_finished = false;


  while( c_line != NULL )
  {
    if ( DEBUG == 1 )
      printf("make_command_stream: current line:%d contents: %s\n", num_lines, c_line);
    line_NULL = false;
    while ( true )
    {
      //check if line is a comment or null byte and go to next lin
      if ( c_line[0] == '#' || c_line[0] == '\0')
      {
        num_lines++;
        line_ignore = true;
        break;
      }
      //check for special tokens
      if ( specialToken(c_line[c_line_pos]) )
      {
        if ( DEBUG == 1 )
          printf("make_command_stream: is special token\n");
        if ( c_line_pos == 0 && c_line[c_line_pos] != '(' )
          printError(num_lines);
        if ( new_word )
        {
          char *word_string = getString(c_line, start_pos, c_line_pos-1);
          struct token *new_token = (struct token *) checked_malloc(sizeof(struct  token));
          new_token->word = word_string;
          new_token->next = NULL;
          addToken(new_token, t_list);
          if ( DEBUG == 1 )
          {
            printf("make_command_stream: new token added\n");
          }
        }
        start_pos = c_line_pos;
        //make special token to add to list
        struct token *token_special = (struct token *) checked_malloc(sizeof(struct token));
        token_special->next = NULL;
        // OR "||" operator and PIPE '|' operator
        if ( c_line[c_line_pos] == '|' ) //PIPE token
        {
          if ( c_line[c_line_pos + 1] == '|' ) // OR token
          {
            token_special->word = getString(c_line, start_pos, c_line_pos + 1);
            if ( DEBUG == 1 )
              printf("make_command_stream: added token is OR operator\n");
            c_line_pos += 2;
          }
          else
          {
            if ( DEBUG == 1 )
              printf("make_command_stream: token is PIPE operator\n");
            token_special->word = getString(c_line, start_pos, c_line_pos);
            c_line_pos++;
          }
          special_token_finished = true;
        }
        else if ( c_line[c_line_pos] == '&' )
        {
          if ( c_line[c_line_pos + 1] == '&' )
          {
            token_special->word = getString(c_line, start_pos, c_line_pos + 1);
            c_line_pos += 2;
            special_token_finished = true;
            if ( DEBUG == 1 )
              printf("make_command_stream: token is AND operator\n");
          } 
          else
            printError(num_lines);
        }
        else if ( c_line[c_line_pos] == '(' )
        {
          token_special->word = getString(c_line, start_pos, c_line_pos);
          num_parens++;
          if ( DEBUG == 1 )
            printf("make_command_stream: added token is LEFT_PAREN operator\n");
          c_line_pos++;
          special_token_finished = true;
        }
        else if ( c_line[c_line_pos] == ')' )
        {
          num_parens--;
          if ( num_parens == 0 )
            special_token_finished = false;
          else
            special_token_finished = true;
          if ( num_parens < 0 )
            printError(num_lines);
          token_special->word = getString(c_line, start_pos, c_line_pos);
          if ( DEBUG == 1 )
            printf("make_command_stream: added token is a RIGHT_PAREN operator\n");
          c_line_pos++;
        }
        else if ( c_line[c_line_pos] == ';' )
        {
          if ( DEBUG == 1 )
            printf("make_command_stream: added token is a SEMI operator\n");
          token_special->word = getString(c_line, start_pos, c_line_pos);
          special_token_finished = false;
          c_line_pos++;
        }
        else 
        {
          token_special->word = getString(c_line, start_pos, c_line_pos);
          c_line_pos++;
          if ( DEBUG == 1 )
            printf("make_command_stream: added token is a %s operator", token_special->word);
        }
        start_pos = c_line_pos;
        addToken(token_special, t_list);
        new_word = false;
        line_empty = false;
      }
      else //not a special token
      {
        if ( validWord(c_line[c_line_pos]) || c_line[c_line_pos] == ' ' )
        {
          if ( c_line[c_line_pos] != ' ' )
            if ( new_word == false )
            {
              new_word = true;
              line_empty = false;
            }
        }
        else
        {
          printError(num_lines);
        }
        c_line_pos++;
      }
      if ( c_line[c_line_pos] == '#' || c_line[c_line_pos] == '\0' )
      {
        if ( new_word )
        {
          char *word_string = getString(c_line, start_pos, c_line_pos - 1);
          struct token *token_simple = (struct token *) checked_malloc(sizeof(struct token));
          token_simple->word = word_string;
          token_simple->next = NULL;
          addToken(token_simple, t_list);
          special_token_finished = false;
          line_empty = false;
        }
        if ( (c_line[c_line_pos] == '\0' && special_token_finished) || (num_parens > 0) || line_empty) 
        {
          if ( DEBUG == 1 )
            printf("make_command_stream: Last token reached\n");
          line_ignore = true;
          if ( num_parens > 0 ) //newline in between sub-shell commands
          {
            struct token *semi_token = (struct token *) checked_malloc(sizeof(struct token));
            semi_token->next = NULL;
            char *s_colon = (char *) checked_malloc(sizeof(char *) * 2);
            s_colon[0] = ';';
            s_colon[1] = '\0';
            semi_token->word = s_colon;
            addToken(semi_token, t_list);
            if ( DEBUG == 1 )
              printf("make_command_stream: semi colon token created from newline\n");
          }
        }
        break;
      }
    }
    if ( DEBUG == 1 )
    {
      printf("make_command_stream: Tokens created and put into t_list\n");
      int i = 0;
      struct token *t_list_h = t_list->start;
      struct token *t_list_t = t_list->end;
      while ( t_list->start != NULL )
      {
        printf("make_command_stream: Word %d:%s\n", i, t_list->start->word);
        t_list->start = t_list->start->next;
        i++;
      }
      t_list->start = t_list_h;
      t_list->end = t_list_t;
      printf("\n\n");
    }
    // c_line is not empty/ignored
    if ( line_ignore == false )
    {
      struct node_list *n_list = tokensToNodes(t_list);
      if ( n_list == NULL )
        printError(num_lines);
      // create stack tree from our node list and add command
      struct command *command_root = create_stack_tree(n_list);
      if ( command_root == NULL )
      {
        if ( DEBUG == 1 )
          printf("make_command_stream: command_root is NULL\n");
        printError(num_lines);
      }
      addCommand(c_stream, command_root);
    }
    free(c_line);
    c_line = pullBytes(*get_next_byte, get_next_byte_argument);
    num_lines++;
    c_line_pos = 0;
    start_pos = 0;
    new_word = false;
    line_ignore = false;
    line_empty = true;
    line_NULL = true;
    if ( DEBUG == 1 )
    printf("make_command_stream: Iterations completed\n");
  }
  if ( num_parens > 0 && line_NULL )
  {
    printError(num_lines);
  }
  if ( special_token_finished && line_NULL )
  {
    printError(num_lines);
  }
  if ( DEBUG == 1 )
    printf("make_command_stream: Reached end of function\n");
  return c_stream;
}

command_t
read_command_stream (command_stream_t s)
{
  if( s->start == NULL ) return 0;
  struct command *temp = s->start->command;
  struct command_list *c_list = s->start;
  s->start = s->start->next;
  free(c_list);
  return temp;
}