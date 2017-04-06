#ifndef HELPER
#define HELPER
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define LINE_SIZE 256
#define MAXARGS 128
#define WORD_SIZE 50
extern char* OLDPWD;
extern int input_flag;
extern int output_flag;
extern long int output_num;
extern long int input_num;

void init();

void free_init();
/*Checks to see if the first word is a valid command*/
int parse_cmd(char* cmd, char* name);

char*** parse_commands(char* cmd_line, char*** commands);

int command_counter (char *cmd_line);
/*Returns argument size*/
int arg_size(char* cmd);

/*Finds the color and bold code*/
char *find_cb(char* color, char* bold);

/*Edits the prompt to reflect with respect to home*/
void edit_prompt(char* prompt);

/*Finds executable if it exists and returns it*/
char* find_executable (char* filepath, char* command);

#endif