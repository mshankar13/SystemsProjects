#include "builtins.h"
#include "helper.h"

char* HELP_MENU_PRINT = {"\nHelp:\n bg PID|JID\n cd [dir]\n chclr SETTING COLOR TOGGLE\n chpmt SETTING TOGGLE\n disown[PID|JID]\n exit [n]\n fg PID|JID\n help\n jobs\n kill [signal] PID|JID\n prt\n pwd\n"};
int user_switch = -1;
int machine_switch = -1;
char *user_color = NULL;
char *machine_color = NULL;

job *first_job = NULL; //Head of the list of jobs

int print_help() {
	printf("%s\n", HELP_MENU_PRINT);
	return 0;
}

void exit_shell() {
	free_init();
	exit(0);
}

int cd(char* cmd, char* prompt, int arg_size) {
	char* home_buf = NULL;
	char* new_dir;
	char* cwd_buf = malloc(DEFAULT_BUFFER_SIZE);
	memset(cwd_buf, 0, DEFAULT_BUFFER_SIZE);
	getcwd(cwd_buf, DEFAULT_BUFFER_SIZE);
	
	char words [LINESIZE];
	strcpy(words, cmd);
	char* cmd_arg = strtok (words," "); // Get second argument after builtin name
	cmd_arg = strtok (NULL," ");
	if (arg_size == 0 || strcmp(cmd_arg, "~") == 0) { //no argument for cd. Home directory
		
		home_buf = getenv("HOME");	
		if ( chdir(home_buf) == 0) { //Success
			OLDPWD = realloc(OLDPWD, strlen(cwd_buf) + 1);
			strcpy(OLDPWD, cwd_buf);
			free(cwd_buf);
			edit_prompt(prompt);
			return 0;
		} else {
			fprintf(stderr, "%s\n", "Cannot change directory to home");
			free(cwd_buf);
			return -1;
		}
	} else {

		if (strcmp(cmd_arg, "-") == 0) { //Previous directory
			if (chdir(OLDPWD) == 0) { //Set old to current
				OLDPWD = realloc(OLDPWD, strlen(cwd_buf) + 1);
				strcpy(OLDPWD, cwd_buf);
				free(cwd_buf);
				edit_prompt(prompt);
				return 0;
			} else {
				fprintf(stderr, "%s\n", "cd: OLDPWD not set");
				free(cwd_buf);
				return -1;
			}
		} else if (strcmp(cmd_arg, ".") == 0) { //Current directory
			if (chdir(cwd_buf) == 0) { //Success
				OLDPWD = realloc(OLDPWD, strlen(cwd_buf) + 1);
				strcpy(OLDPWD, cwd_buf);
				free(cwd_buf);
				edit_prompt(prompt);
				return 0;
			} else {
				fprintf(stderr, "%s\n", "Cannot change directory");
				free(cwd_buf);
				return -1;
			}
		} else if (strcmp(cmd_arg, "..") == 0) { //Parent directory
			new_dir = malloc(strlen(cwd_buf) + 1);
			memset(new_dir, 0, strlen(cwd_buf) + 1);
			char* child_dir = strrchr(cwd_buf, '/'); //Get pointer to last occurrence of /
			strncpy(new_dir, cwd_buf, strlen(cwd_buf) - strlen(child_dir));
			//Try to change directories
			if (chdir(new_dir) == 0) { //Success
				OLDPWD = realloc(OLDPWD, strlen(cwd_buf) + 1);
				strcpy(OLDPWD, cwd_buf);
				free(new_dir);
				free(cwd_buf);
				edit_prompt(prompt);
				return 0;
			} else {
				fprintf(stderr, "%s\n", "Cannot change directory");
				free(new_dir);
				free(cwd_buf);
				return -1;
			}
		} else {
			// Check to see if file or directory exists
			new_dir = malloc(PATH_SIZE);
			strcat(new_dir, cwd_buf);
			strcat(new_dir, "/");
			strcat(new_dir, cmd_arg);

			if (chdir(new_dir) == 0) {
				OLDPWD = realloc(OLDPWD, strlen(cwd_buf) + 1);
				strcpy(OLDPWD, cwd_buf);
				free(new_dir);
				free(cwd_buf);
				edit_prompt(prompt);
				return 0;
			} else {
				free(new_dir);
				free(cwd_buf);
				fprintf(stderr ,"%s\n", "No such file or directory");
				return -1;
			}
		}
	}

	return -1;
}

int pwd() {
	char* cwd_buf = malloc(DEFAULT_BUFFER_SIZE);
	memset(cwd_buf, 0, DEFAULT_BUFFER_SIZE);
	getcwd(cwd_buf, DEFAULT_BUFFER_SIZE);
	if (cwd_buf == NULL) {
		fprintf(stderr, "%s\n", "Cannot locate current working directory");
		free(cwd_buf);
		return -1;
	} else {
		printf("%s\n", cwd_buf);
		free(cwd_buf);
		return 0;
	}
}

int chpmt(char *cmd, char *prompt, int arg_size) {

	char words [LINESIZE];
	strcpy(words, cmd);
	char* cmd_arg = strtok (words," "); // Get second argument after builtin name
	cmd_arg = strtok (NULL," ");

	char *fish = "sfish";
	char* tilde = "~";	
	char *dash = "-";
    char *at = "@";
    char *colon = ":";
    char *carrot = ">";
    char *space = " ";
    char *user = getenv("USER");
    char *machine = calloc(1, HOSTNAMESIZE);
    gethostname(machine, HOSTNAMESIZE);
    char *cwd_buf = calloc(1, PATH_SIZE);
    getcwd(cwd_buf, PATH_SIZE);

    char* home_dir = getenv("HOME");
    char cwd[strlen(cwd_buf) - strlen(home_dir) + 1];
    if (strcmp(home_dir, cwd_buf) == 0) {
    	//Relative to home dir...
    	cwd_buf = tilde;
    } else {
    	int index = 0;
    	for (int i = strlen(home_dir); i < strlen(cwd_buf); i++) {
    		cwd[index] = cwd_buf[i];
    		index++;
    	}
    	cwd[index] = '\0';
    }

   	if (arg_size == 0 || arg_size == 1) {
   		return -1;
   	}

   	if (user_switch == -1) {
   		user_switch = 1;
   	}

   	if (machine_switch == -1) {
   		machine_switch = 1;
   	}
   	
	if (strcmp(cmd_arg, "user") == 0) {
		cmd_arg = strtok (NULL," ");
		if (strcmp(cmd_arg, "1") == 0) { //ON
			memset(prompt, 0, strlen(prompt) + 1);
			user_switch = 1;
			if (machine_switch == 0) { //User
				strcat(prompt, fish);
				strcat(prompt, dash);
    			strcat(prompt, user);
    			strcat(prompt, colon);
    			if (strcmp(home_dir, cwd_buf) == 0) {
    				strcat(prompt, cwd_buf);
    			} else {
    				strcat(prompt, tilde);
    				strcat(prompt, cwd);
    			}
    			strcat(prompt, carrot);
    			strcat(prompt, space);
			} else {				  //User and Machine
				strcat(prompt, fish);
				strcat(prompt, dash);
    			strcat(prompt, user);
    			strcat(prompt, at);
    			strcat(prompt, machine);
    			strcat(prompt, colon);
    			if (strcmp(home_dir, cwd_buf) == 0) {
    				strcat(prompt, cwd_buf);
    			} else {
    				strcat(prompt, tilde);
    				strcat(prompt, cwd);
    			}
    			strcat(prompt, carrot);
    			strcat(prompt, space);
			}
			free(machine);
			free(cwd_buf);
			return 0;
		} else if (strcmp(cmd_arg, "0") == 0) { //OFF
			memset(prompt, 0, strlen(prompt) + 1);
			user_switch = 0;
			if (machine_switch == 0) { //None
				strcat(prompt, fish);
    			strcat(prompt, colon);
    			if (strcmp(home_dir, cwd_buf) == 0) {
    				strcat(prompt, cwd_buf);
    			} else {
    				strcat(prompt, tilde);
    				strcat(prompt, cwd);
    			}
    			strcat(prompt, carrot);
    			strcat(prompt, space);
			} else {				   //Machine
				strcat(prompt, fish);
				strcat(prompt, dash);
    			strcat(prompt, machine);
    			strcat(prompt, colon);
    			if (strcmp(home_dir, cwd_buf) == 0) {
    				strcat(prompt, cwd_buf);
    			} else {
    				strcat(prompt, tilde);
    				strcat(prompt, cwd);
    			}
    			strcat(prompt, carrot);
    			strcat(prompt, space);
			}
			free(machine);
			free(cwd_buf);
			return 0;
		} else {
			free(machine);
			free(cwd_buf);
			return -1;
		}
	} else if (strcmp(cmd_arg, "machine") == 0) {
		cmd_arg = strtok (NULL," ");
		if (strcmp(cmd_arg, "1") == 0) { //ON
			memset(prompt, 0, strlen(prompt) + 1);
			machine_switch = 1;
			if (user_switch == 0) {   //Machine
				strcat(prompt, fish);
				strcat(prompt, dash);
    			strcat(prompt, machine);
    			strcat(prompt, colon);
    			if (strcmp(home_dir, cwd_buf) == 0) {
    				strcat(prompt, cwd_buf);
    			} else {
    				strcat(prompt, tilde);
    				strcat(prompt, cwd);
    			}
    			strcat(prompt, carrot);
    			strcat(prompt, space);
			} else {				  //Machine and User
				strcat(prompt, fish);
				strcat(prompt, dash);
    			strcat(prompt, user);
    			strcat(prompt, at);
    			strcat(prompt, machine);
    			strcat(prompt, colon);
    			if (strcmp(home_dir, cwd_buf) == 0) {
    				strcat(prompt, cwd_buf);
    			} else {
    				strcat(prompt, tilde);
    				strcat(prompt, cwd);
    			}
    			strcat(prompt, carrot);
    			strcat(prompt, space);
			}
			free(machine);
			free(cwd_buf);
			return 0;
		} else if (strcmp(cmd_arg, "0") == 0) { //OFF
			memset(prompt, 0, strlen(prompt) + 1);
			machine_switch = 0;
			if (user_switch == 0) {	 //None
				strcat(prompt, fish);
    			strcat(prompt, colon);
    			if (strcmp(home_dir, cwd_buf) == 0) {
    				strcat(prompt, cwd_buf);
    			} else {
    				strcat(prompt, tilde);
    				strcat(prompt, cwd);
    			}
    			strcat(prompt, carrot);
    			strcat(prompt, space);
			} else {				 //User
				strcat(prompt, fish);
				strcat(prompt, dash);
    			strcat(prompt, user);
    			strcat(prompt, colon);
    			if (strcmp(home_dir, cwd_buf) == 0) {
    				strcat(prompt, cwd_buf);
    			} else {
    				strcat(prompt, tilde);
    				strcat(prompt, cwd);
    			}
    			strcat(prompt, carrot);
    			strcat(prompt, space);
			}
			free(machine);
			free(cwd_buf);
			return 0;
		} else {
			free(machine);
			free(cwd_buf);
			return -1;
		}
	} else {
		free(machine);
		free(cwd_buf);
		return -1;
	}

    free(machine);
	free(cwd_buf);
	return -1;
}

int chclr(char *cmd, char *prompt, int arg_size) {
	char words [LINESIZE];
	strcpy(words, cmd);
	char* cmd_arg = strtok (words," "); // Get second argument after builtin name
	char* setting = malloc(SETTING);
	char* color = malloc(COLOR);
	char* toggle = malloc(TOGGLE);
	char *cb;

	char *fish = "sfish";
	char *tilde = "~";
	char *dash = "-";
    char *at = "@";
    char *colon = ":";
    char *carrot = ">";
    char *space = " ";
    char *user = getenv("USER");
    char *machine = calloc(1, HOSTNAMESIZE);
    gethostname(machine, HOSTNAMESIZE);
    char *cwd_buf = calloc(1, PATH_SIZE);
    getcwd(cwd_buf, PATH_SIZE);

    char* home_dir = getenv("HOME");
    char cwd[strlen(cwd_buf) - strlen(home_dir) + 1];
    if (strcmp(home_dir, cwd_buf) == 0) {
    	//Relative to home dir...
    	cwd_buf = tilde;
    } else {
    	int index = 0;
    	for (int i = strlen(home_dir); i < strlen(cwd_buf); i++) {
    		cwd[index] = cwd_buf[i];
    		index++;
    	}
    	cwd[index] = '\0';
    }

   	if (arg_size == 0) { //No arguments after SETTING
   		return -1;
   	}

   	if (user_color == NULL) { // Set default colors
   		user_color = DEFAULT;
   	}

   	if (machine_color == NULL) {
   		machine_color = DEFAULT;
   	}

   	cmd_arg = strtok (NULL," ");
	if (strcmp(cmd_arg, "user") == 0) { //USER Setting
		cmd_arg = strtok (NULL," ");;
		if (cmd_arg != NULL) { // Possible Color input
			strcpy(color, cmd_arg);
			cmd_arg = strtok (NULL," ");
			if (cmd_arg != NULL) { //Possible toggle
				strcpy(toggle, cmd_arg);
				//Get color setting
				cb = find_cb(color,toggle);
				if (cb != NULL) { //Value!
					user_color = cb;
					memset(prompt, 0, strlen(prompt) + 1);
					strcat(prompt, fish);
					strcat(prompt, dash);
					if (user_switch != 0) {
						strcat(prompt, user_color);
    					strcat(prompt, user);
    					strcat(prompt, RESET);
    					strcat(prompt, at);
					}
					if (machine_switch != 0) {
						strcat(prompt, machine_color);
    					strcat(prompt, machine);
    					strcat(prompt, RESET);
					}

    				strcat(prompt, colon);
    				if (strcmp(home_dir, cwd_buf) == 0) {
    					strcat(prompt, cwd_buf);
    				} else {
    					strcat(prompt, tilde);
    					strcat(prompt, cwd);
    				}
    				strcat(prompt, carrot);
    				strcat(prompt, space);
				}
			}
		}
	} else if (strcmp(cmd_arg, "machine") == 0) {
		cmd_arg = strtok (NULL," ");;
		if (cmd_arg != NULL) { // Possible Color input
			strcpy(color, cmd_arg);
			cmd_arg = strtok (NULL," ");
			if (cmd_arg != NULL) { //Possible toggle
				strcpy(toggle, cmd_arg);
				//Get color setting
				cb = find_cb(color,toggle);
				if (cb != NULL) { //Value!
					machine_color = cb;
					memset(prompt, 0, strlen(prompt) + 1);
					strcat(prompt, fish);
					strcat(prompt, dash);
					if (user_switch != 0) {
						strcat(prompt, user_color);
    					strcat(prompt, user);
    					strcat(prompt, RESET);
    					strcat(prompt, at);
					}
					if (machine_switch != 0) {
						strcat(prompt, machine_color);
    					strcat(prompt, machine);
    					strcat(prompt, RESET);
					}
    				strcat(prompt, colon);
    				if (strcmp(home_dir, cwd_buf) == 0) {
    					strcat(prompt, cwd_buf);
    				} else {
    					strcat(prompt, tilde);
    					strcat(prompt, cwd);
    				}
    				strcat(prompt, carrot);
    				strcat(prompt, space);
				}
			}
		}
	} else {
	}

	free(setting);
	free(color);
	free(toggle);
    free(machine);
	free(cwd_buf);
	return -1;
}

int jobs () {
	//List all jobs running in bg: name, pid, job #, status, exit status code

	return 0;
}

void add_job(char* cmd) {

}

int fg () {
	return 0;
}

int bg () {
	return 0;
}

int kills () {
	pid_t id = getpid();
    kill(id, SIGTERM);
	return 0;
}

int disown () {
	return 0;
}