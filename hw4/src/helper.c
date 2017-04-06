#include "helper.h"
#include "builtins.h"

/*Contains all helper functions for the shell*/
char* OLDPWD;

/*Check to see is the command if valid with junk*/
void init() {
	OLDPWD = malloc(DEFAULT_BUFFER_SIZE);
}

void free_init() {
	free(OLDPWD);
}
int parse_cmd(char* cmd, char* name) {
	char line[256];
	strcpy(line, cmd);
	int x = memcmp(line, name, strlen(name));

	if (x != 0) { // Check to see if first argument is a command
		return -1;
	}
	return 0;
}

/*Only when piping is involved*/
char*** parse_commands(char* cmd_line, char*** commands) {
	/*Parses for redirection and piping?*/
	//Count the number of pipes
	int pipe_counter = 0;
	int command_counter = 0;
	int arg_counter = 0;
	char line[strlen(cmd_line) + 1];
	strcpy(line, cmd_line);
	char* word = strtok (line," "); // Filter out spaces
	while (word != NULL) {
		if (strcmp(word, "|") == 0) {
			pipe_counter++;
			word = strtok (NULL," ");
			if (word == NULL) {

			}
		} else {
			word = strtok (NULL," ");
		}
		
	}

	if (pipe_counter != 0) {
		command_counter = pipe_counter + 1; //Increment if piping is a thing
	}

	int args_count_list[command_counter]; //Stores number of elements for each command. Includes >, <
	int m = 0; //Index of int array
	strcpy(line, cmd_line);
	word = strtok (line," "); // Filter out spaces
	while (word != NULL) {
		if (strcmp(word, "|") == 0) {
			args_count_list[m] = arg_counter;
			m++;
			arg_counter = 0;
		} else {
			arg_counter++;
		}
		word = strtok (NULL," ");
		if (word == NULL) {
			args_count_list[m] = arg_counter;
		}
	}

	commands = malloc (sizeof(char**) * command_counter);
	memset(commands, 0, sizeof(char**) * command_counter); 

	char cmds[strlen(cmd_line) + 1];
	strcpy(cmds, cmd_line);
	word = strtok (cmds," "); // Filter out spaces
	int cmd_counter = 0;
	int i = 0;
	int word_counter = 0;
	char** cmd = malloc (sizeof(char*) * args_count_list[i] + 1);
	while (word != NULL) {
		if (strcmp(word, "|") == 0) {
			cmd[word_counter] = malloc(1);
			cmd[word_counter] = NULL;
			word_counter = 0;
			i++;
			commands[cmd_counter] = cmd;
			cmd_counter++;
			cmd = malloc (sizeof(char*) * args_count_list[i] + 1);
		} else {
			cmd[word_counter] = malloc(strlen(word) + 1);
			strcpy(cmd[word_counter], word);
			word_counter++;
		}
		word = strtok(NULL, " ");
		if (word == NULL) {
			cmd[word_counter] = malloc(1);
			cmd[word_counter] = NULL;
			commands[cmd_counter] = cmd;
		}
	}
	return commands;
}

int command_counter (char *cmd_line) {
	int pipe_counter = 0;
	int command_counter = 0;
	char line[strlen(cmd_line) + 1];
	strcpy(line, cmd_line);
	char* word = strtok (line," "); // Filter out spaces
	while (word != NULL) {
		if (strcmp(word, "|") == 0) {
			pipe_counter++;
			word = strtok (NULL," ");
			if (word == NULL) {

			}
		} else {
			word = strtok (NULL," ");
		}
		
	}

	if (pipe_counter != 0) {
		command_counter = pipe_counter + 1; //Increment if piping is a thing
	}

	return command_counter;
}

int arg_size(char* cmd) {
	int argsize = 0;
	char s [LINE_SIZE];
	strcpy(s, cmd);
	strtok (s," "); // Filter out spaces
	argsize++;
	while (strtok (NULL, " ") != NULL) {
		argsize++;
	}
	return argsize - 1; //omit builtin name
}



char *find_cb(char* color, char* bold) {
	if (strcmp(color, "red") == 0) {
		if (strcmp(bold, "0") == 0) {
			return RED;
		} else if (strcmp(bold, "1") == 0) {
			return BRED;
		}
	} else if (strcmp(color, "blue") == 0) {
		if (strcmp(bold, "0") == 0) {
			return BLU;
		} else if (strcmp(bold, "1") == 0) {
			return BBLU;
		}
	} else if (strcmp(color, "green") == 0) {
		if (strcmp(bold, "0") == 0) {
			return GRN;
		} else if (strcmp(bold, "1") == 0) {
			return BGRN;
		}
	} else if (strcmp(color, "yellow") == 0) {
		if (strcmp(bold, "0") == 0) {
			return YEL;
		} else if (strcmp(bold, "1") == 0) {
			return BYEL;
		}
	} else if (strcmp(color, "cyan") == 0) {
		if (strcmp(bold, "0") == 0) {
			return CYN;
		} else if (strcmp(bold, "1") == 0) {
			return BCYN;
		}
	} else if (strcmp(color, "magenta") == 0) {
		if (strcmp(bold, "0") == 0) {
			return MAG;
		} else if (strcmp(bold, "1") == 0) {
			return BMAG;
		}
	} else if (strcmp(color, "black") == 0) {
		if (strcmp(bold, "0") == 0) {
			return BLK;
		} else if (strcmp(bold, "1") == 0) {
			return BBLK;
		}
	} else if (strcmp(color, "white") == 0) {
		if (strcmp(bold, "0") == 0) {
			return WHT;
		} else if (strcmp(bold, "1") == 0) {
			return BWHT;
		}
	} else {
		return NULL;
	}
	return NULL;
}

void edit_prompt(char* prompt) {


	// Loop through, if equal don't include
	char *tilde = "~";
	char *fish = "sfish-";
    char *at = "@";
    char *colon = ":";
    char *carrot = ">";
    char *space = " ";
    char *user = getenv("USER");
    char *machine = malloc(HOSTNAMESIZE);
    gethostname(machine, HOSTNAMESIZE);

    char* home_dir = getenv("HOME");
    char *cwd_buf = malloc(PATH_SIZE);
    getcwd(cwd_buf, PATH_SIZE);
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
    memset(prompt, 0, strlen(prompt) + 1);
    strcat(prompt, fish);
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

char* find_executable (char* filepath, char* command) {
	struct stat buffer;
	char* PATH = getenv("PATH");
	char PATHS [400];
	strcpy(PATHS, PATH);
	char* path;
	char* slash = "/";
	char* cmd = malloc(strlen(command) + 1);
	strcpy(cmd, command);
	path = strtok(PATHS, ":");
	//if (stat(command, &buffer) == 0) { //command is an executable
	//	return command;
	//}
	while (path != NULL) {
		memset(filepath, 0, strlen(filepath) + 1);
		strcat(filepath, path);
		strcat(filepath, slash);
		strcat(filepath, cmd);
		path = strtok(NULL, ":");
		
		if (stat(filepath, &buffer) == 0) {
			free(cmd);
			return filepath;
		}
	}
	//Break down paths
	//Concat command
	//Use stat to see if file exists
	//Return filename
	free(cmd);
	return NULL;
}

