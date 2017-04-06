#ifndef BUILTINS
#define BUILTINS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
/*Declare all function prototypes for builtins*/

#define DEFAULT_BUFFER_SIZE 200
#define HOSTNAMESIZE 20
#define PATH_SIZE 100
#define LINESIZE 256
#define PATHSIZE 400
#define SETTING 50
#define COLOR 20
#define TOGGLE 10

// Colors for the output
#define BLK   "\x1B[30m"
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"
#define DEFAULT "\x1B[0m"

// Colors for the output
#define BBLK   "\x1B[1;30m"
#define BRED   "\x1B[1;31m"
#define BGRN   "\x1B[1;32m"
#define BYEL   "\x1B[1;33m"
#define BBLU   "\x1B[1;34m"
#define BMAG   "\x1B[1;35m"
#define BCYN   "\x1B[1;36m"
#define BWHT   "\x1B[1;37m"

extern int return_value;
extern char *user_color;
extern char *machine_color;

typedef struct job {
	struct job *next;
	char* name;
	pid_t PID;
	int num;
	char* status;
} job;

/**Print a list of all builtin's and their basic usage 
*in a single column.*/
int print_help ();

/*Exits the shell*/
void exit_shell ();

/*Changes the current working directory of the shell*/
int cd (char* cmd_arg, char *prompt, int arg_size);

/*Prints absolute path of current working directoru*/
int pwd ();

int chpmt(char *cmd, char *prompt, int arg_size);

int chclr();

int jobs ();

void add_job();

int fg ();

int bg ();

int kills ();

int disown ();

#endif