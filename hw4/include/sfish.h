#ifndef SFISH
#define SFISH
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

int readline_funcB (int count, int key);
int readline_funcG (int count, int key);
int readline_funcH (int count, int key);
int readline_funcP (int count, int key);
int readline_funcC (int count, int key);
int readline_funcZ (int count, int key);

/*Evaluates input line*/
void evaluate_line (char* cmd);

/*Checks to see is a program is a builtin*/
/*@param cmd line input by the user*/
/*@return functionptr ptr to the builtin if it exists*/
int (*get_builtin(char* cmd)) ();

/*Prints return code of the command that was last executed*/
int prt ();
/*Handles execution without redirection*/
void handle_execution(char* cmd);

/*Parses the command line into an array*/
int parseline(char* filepath, char* cmd_line, char** argv);

void redirect_flags(char* cmd);

/*Frees parseline argument*/
void free_parseline(char** argv, int argsize);

/**Checks for any redirection
*@param unfiltered cmd line to parse
*@return -2 incomplete arguments, -1 no redirection, 0 redirection
**/
int has_redirection(char* cmd);

int redirection(char* cmd);

void redirection_execution(char* cmd);

int piping(char*** commands, int cmd_num);

#endif