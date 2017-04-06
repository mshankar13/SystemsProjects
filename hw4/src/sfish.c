#include "sfish.h"
#include "builtins.h"
#include "helper.h"

int return_value = 0;
char *prompt;
char *cmd;
char *machine;
char *cwd_buf;
int executable_flag = 0;

int input_flag = 0;
int output_flag = 0;
int input_state = -1;
int output_state = -1;
long int output_num = -1;
long int input_num = -1;

int pipe_count = 0;
int symbol_num = 0;

int total_commands = 0;

char* SFISH_INFO1 = {"\n----Info----\nhelp\nprt\n"};
char* SFISH_INFO2 = {"----CTRL----\ncd\nchclr\nchpmt\npwd\nexit\n"};
char* SFISH_INFO3 = {"----Job Control----\nbg\nfg\ndisown\njobs\n"};
char* SFISH_INFO4 = {"---Number of Commands Run---\n"};
char* SFISH_INFO5 = {"----Process Table----\n"};
char* SFISH_INFO6 = {"PGID\tPID\tTIME\tCMD"};

int readline_funcB (int count, int key) {
    rl_on_new_line ();
    return 0;
}

int readline_funcG (int count, int key) {
    rl_on_new_line ();
    return 0;
}

int readline_funcH (int count, int key) {
    print_help();
    rl_on_new_line ();
    return 0;
}

int readline_funcP (int count, int key) {
    printf("%s\n", SFISH_INFO1);
    printf("%s\n", SFISH_INFO2);
    printf("%s\n", SFISH_INFO3);
    printf("%s\n", SFISH_INFO4);
    printf("%d\n", total_commands);
    printf("%s\n", SFISH_INFO5);
    printf("%s\n", SFISH_INFO6);
    rl_on_new_line ();
    return 0;
}

int readline_funcC (int count, int key) {
    pid_t id = getpid();
    kill(id, SIGINT);
    rl_on_new_line ();
    return 0;
}

int readline_funcZ (int count, int key) {
    pid_t id = getpid();
    kill(id, SIGTSTP);
    rl_on_new_line ();
    return 0;
}
int main(int argc, char** argv) {
    //DO NOT MODIFY THIS. If you do you will get a ZERO.
    rl_catch_signals = 0;
    //This is disable readline's default signal handlers, since you are going
    //to install your own.

    rl_bind_keyseq ("\\C-b", readline_funcB);
    rl_bind_keyseq ("\\C-g", readline_funcG);
    rl_bind_keyseq ("\\C-h", readline_funcH);
    rl_bind_keyseq ("\\C-p", readline_funcP);
    rl_bind_keyseq ("\\C-c", readline_funcC);
    rl_bind_keyseq ("\\C-z", readline_funcZ);
    init(); //Initialize OLDPWD

    prompt = malloc (LINE_SIZE);

    edit_prompt(prompt);

    //pid_t SPID;
    while((cmd = readline(prompt)) != NULL) {
        evaluate_line(cmd); // Evaluate input

        //printf("%s\n",cmd);

        //All your debug print statments should be surrounded by this #ifdef
        //block. Use the debug target in the makefile to run with these enabled.
        #ifdef DEBUG
        //fprintf(stderr, "Length of command entered: %ld\n", strlen(cmd));
        #endif
        //You WILL lose points if your shell prints out garbage values.

    }

    //Don't forget to free allocated memory, and close file descriptors.
    free(cmd);
    free(prompt);
    free(machine);
    free(cwd_buf);
    //WE WILL CHECK VALGRIND!

    return EXIT_SUCCESS;
}

void evaluate_line (char* cmd) {
    output_state = -1;
    input_state = -1;
    pipe_count = 0;
    symbol_num = 0;
    int redir = has_redirection(cmd);

    if (redir == -2) {
        return; //Invalid input
    } else if (redir == 1) {
        //Piping!
        char ***commands = NULL;
        int cmd_num = command_counter(cmd);
        commands = parse_commands(cmd, commands);
        piping(commands, cmd_num);
        //execute piping
    } else if (redir == 0) {
        //Redirection!
        redirect_flags(cmd);
        total_commands++;
        redirection(cmd);
    } else {
        total_commands++;
        handle_execution(cmd);
    }
    
}

int (*get_builtin(char* cmd)) () {
    executable_flag = 0;
    if (strcmp(cmd, "") == 0) {
        return NULL;
    } else if (parse_cmd(cmd, "exit") == 0) {
        //free before exiting
        free(cmd);
        free(prompt);
        free(machine);
        free(cwd_buf);
        exit_shell(); // Don't fork
        return NULL;
    } else if (parse_cmd(cmd, "help") == 0) {
        return &print_help;
    } else if (parse_cmd(cmd, "cd") == 0) {
        return_value = cd(cmd, prompt, arg_size(cmd)); // Don't fork
        return NULL;
    } else if (parse_cmd(cmd, "pwd") == 0) {
        return &pwd;
    } else if (parse_cmd(cmd, "prt") == 0) {
        return &prt;
    } else if (parse_cmd(cmd, "chpmt") == 0) {
        return_value = chpmt(cmd, prompt, arg_size(cmd));
        return NULL; // Don't fork
    } else if (parse_cmd(cmd, "chclr") == 0) {
        return_value = chclr(cmd, prompt, arg_size(cmd));
        return NULL; // Don't fork
    } else if (parse_cmd(cmd, "jobs") == 0) {
        jobs();
        return NULL;
    } else if (parse_cmd(cmd, "bg") == 0) {
        return &bg;
    } else if (parse_cmd(cmd, "fg") == 0) {
        return &fg;
    } else if (parse_cmd(cmd, "kill") == 0) {
        kills(cmd);
        return NULL;
    } else if (parse_cmd(cmd, "disown") == 0) {
        return &disown;
    } else if (strcmp(cmd, "&") == 0) {
        return NULL;
    } else {
        executable_flag = 1;
        return NULL;
    }
    return NULL;   
}

int prt() {
    printf("%d\n", return_value);
    return 0;
}

void handle_execution(char* cmd) {
    pid_t child;
    int (*function_ptr) ();
    int status = 0;
    char* args[MAXARGS];
    if ((function_ptr = get_builtin(cmd)) != NULL) { // Create child and executes program
        if ((child = fork()) == 0) { // In child
            // Call builtin function
            return_value = function_ptr();
            exit(0);
        } else {
            // Wait for child 
            wait(&status);
        }
    } else if (executable_flag == 1) { // Handle executables
        char words [LINE_SIZE];
        memset(words, 0, LINE_SIZE);
        strcpy(words, cmd);
        char* word = strtok(words, " ");
        char* command = malloc(strlen(word) + 1);
        strcpy(command, word);
        char* filepath = malloc(LINE_SIZE); 
        int size = 0;   
        if ((child = fork()) == 0) {
            if (find_executable(filepath, command) != NULL) { //File exists
                parseline(filepath, cmd, args);
                
                if (execv(args[0], args) < 0) {
                    printf("%s\n", "Unknown command");
                    free(filepath);
                    free(command);
                    free_parseline(args, size);
                    exit(0);
                }
            }
            if (size != 0) {
                free_parseline(args, size);
            }
            free(command);
            free(filepath);
            exit(0);
        } else {
            wait(&status);
        }
    }
}

int parseline(char* filepath, char* cmd_line, char** argv) {
    int index = 0;
    char args[LINE_SIZE];
    memset(args, 0, LINE_SIZE);
    strcpy(args, cmd_line);
    char* word = strtok(args, " ");
    if (word == NULL) {
        return -1;
    }
    argv[index] = malloc(strlen(filepath) + 1);
    strcpy(argv[index], filepath);
    index++;
    while ((word = strtok(NULL, " ")) != NULL) {
        if (strstr(word, "<") != NULL) { //One of the redirection flags triggered
            if ((word = strtok(NULL, " ")) == NULL) {
                break;
            }
        } else if (strstr(word, ">") != NULL) {
            if ((word = strtok(NULL, " ")) == NULL) {
                break;
            }
        } else {
            argv[index] = malloc(strlen(word) + 1);
            strcpy(argv[index], word);
            index++;
        }
    }
    argv[index] = malloc(1);
    argv[index] = NULL;
    return index;
}

void redirect_flags(char* cmd) {
    char args[LINE_SIZE];
    memset(args, 0, LINE_SIZE);
    strcpy(args, cmd);
    char* word = strtok(args, " ");
    if (word == NULL) {
        return;
    }
    while ((word = strtok(NULL, " ")) != NULL) {
        if (strstr(word, "<") != NULL) { //One of the redirection flags triggered
            if (input_state == -1 && output_state == -1) {
                input_state = 0;
            } else if (output_state == 0) {
                input_state = 1;
            }
        } else if (strstr(word, ">") != NULL) {
            if (input_state == -1 && output_state == -1) {
                output_state = 0;
            } else if (input_state == 0) {
                output_state = 1;
            }
        }
    }
}

void free_parseline(char** argv, int argsize) {
    for (int i = 0; i < argsize; i++) {
        free(argv[i]);
    }
}

int has_redirection(char* cmd) {
    //Redirection format: evec > filename || exec < filename || exec < file > out
    int nothing = -1;
    char args[LINE_SIZE];
    memset(args, 0, LINE_SIZE);
    strcpy(args, cmd);
    char* word;
    word = strtok(args, " ");
    while (word!= NULL) { //Gets filenames is possible
        if (strstr(word, ">") != NULL) {
            output_num = strtol(word, NULL, 10);
            if ((word = strtok(NULL, " ")) == NULL) {
                //Do nothing
                return -2;
            }
            nothing = 0;
        } else if (strstr(word, "<") != NULL) {
            input_num = strtol(word, NULL, 10);
            if ((word = strtok(NULL, " ")) == NULL) {
                //Do nothing
                return -2;
            }
            nothing = 0;
        } else if (strcmp(word, "|") == 0) {
            return 1;
        }
        word = strtok(NULL, " ");
    }
    return nothing;
}

int redirection(char* cmd) {
    //Redirection format: evec > filename || exec < filename || exec < file > out
    char* filename1 = malloc(MAXARGS);
    memset(filename1, 0, MAXARGS);
    char* filename2 = malloc(MAXARGS);
    memset(filename2, 0, MAXARGS);
    char args[LINE_SIZE];
    memset(args, 0, LINE_SIZE);
    strcpy(args, cmd);
    char* word;
    int bracket1;
    int bracket2;
    word = strtok(args, " ");
    while (word!= NULL) { //Gets filenames is possible
        if (strstr(word, ">") != NULL) {
            bracket1 = strlen(word);
            output_num = strtol(word, NULL, 10);
            if ((word = strtok(NULL, " ")) == NULL) {
                //Do nothing
                return -1;
            } else {
                if (output_state == 0 && input_state == -1) {
                    //Writing to standard out
                    strcpy(filename1, word);
                } else if (output_state == 1 && input_state == 0) {
                    strcpy(filename2, word);
                }
            }
        } else if (strstr(word, "<") != NULL) {
            bracket2 = strlen(word);
            input_num = strtol(word, NULL, 10);
            if ((word = strtok(NULL, " ")) == NULL) {
                //Do nothing
                return -1;
            } else {
                if (input_state == 0 && output_state == -1) {
                    //standard in
                    strcpy(filename1, word);
                } else if (input_state == 0 && output_state == 1) {
                    strcpy(filename1, word);
                }
            }
        }
        word = strtok(NULL, " ");
    }
    pid_t child;
    int status;
    //Open/Create file
    if(input_state == 0 && output_state == -1) { //Only reading
        if ((child = fork()) == 0) {
            int fd = open(filename1, O_RDONLY);
            if (fd == -1) {
                //Invalid filename
                close(fd);
                exit(0);
            } else if (bracket2 == 1) {
                dup2(fd, 0); //Reading from file
                redirection_execution(cmd);
            } else {
                //Custom stream
                dup2(fd, input_num);
                redirection_execution(cmd);
            }
            close(fd);
            exit(0);
        } else {
            wait(&status);
        }   
    } else if (input_state == -1 && output_state == 0) { //Only writing
        if ((child = fork()) == 0) {
            int fd = open(filename1, O_WRONLY | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (fd == -1) {
                //Invalid filename
                close(fd);
                exit(0);
            } else if (bracket1 == 1) {
                dup2(fd, 1); //Writing from stdout to file
                redirection_execution(cmd);
            } else {
                //Custom stream
                dup2(fd, output_num);
                redirection_execution(cmd);
            }
            close(fd);
            exit(0);
        } else {
            wait(&status);
        }   
    } else if (input_state == 0 && output_state == 1) { // Reading and writing
        if ((child = fork()) == 0) {
            int fd1 = open(filename1, O_RDONLY);
            int fd2 = open(filename2, O_WRONLY | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (fd1 == -1 || fd2 == -1) {
                close(fd1);
                close(fd2);
                exit(0);
            } else if (bracket1 == 1 && bracket2 == 1) {
                dup2(fd1, 0);
                dup2(fd2, 1);
                redirection_execution(cmd);
            } else if (bracket1 == 1 && bracket2 > 1) {
                dup2(fd1, 0);
                dup2(fd2, output_num);
                redirection_execution(cmd);
            } else if (bracket1 > 1 && bracket2 == 1) {
                dup2(fd1, input_num);
                dup2(fd2, 1);
                redirection_execution(cmd);
            } else if (bracket1 > 1 && bracket2 > 1) {
                dup2(fd1, input_num);
                dup2(fd2, output_num);
                redirection_execution(cmd);
            }
            close(fd1);
            close(fd2);
            exit(0);
        } else {
            wait(&status);
        }
    }
    //dup2
    return 0;
}

void redirection_execution(char* cmd) {
    int (*function_ptr) ();
    char* args[MAXARGS];
    if ((function_ptr = get_builtin(cmd)) != NULL) { // Create child and executes program
        
    } else if (executable_flag == 1) { // Handle executables
        char words [LINE_SIZE];
        memset(words, 0, LINE_SIZE);
        strcpy(words, cmd);
        char* word = strtok(words, " ");
        char* command = malloc(strlen(word) + 1);
        strcpy(command, word);
        char* filepath = malloc(LINE_SIZE); 
        int size = 0;   
        if (find_executable(filepath, command) != NULL) { //File exists
            parseline(filepath, cmd, args);
            
            if (execv(args[0], args) < 0) {
                printf("%s\n", "Unknown command");
            }
        }
        if (size != 0) {
            free_parseline(args, size);
        }
        free(command);
        free(filepath);
    }
}

int piping(char*** commands, int cmd_num) {
    //loop through each command in the pipe
    int cmd_index = 0;
    int x = 0;
    int status = 0;
    int pipe_fd[(cmd_num - 1) * 2]; //0 - Reading, 1 - Writing
    int num_pipes = ((cmd_num - 1) * 2); 
    total_commands += num_pipes;
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(&pipe_fd[2*i]) == -1) {
            errno = EFAULT;
            exit(EXIT_FAILURE);
        }
    }

    //int y = 0;
    while (cmd_index < cmd_num) {
        total_commands++;
        //Pipe
        //fork for each command to be executed
        //char** command = commands[cmd_index];
        //Pipe for each process minus parent
        int m = 0;
        char* cmd_ptr = malloc(LINE_SIZE);
        memset(cmd_ptr, 0, LINE_SIZE);
        while (commands[x][m] != NULL) {
            strcat(cmd_ptr, commands[x][m]);
            strcat(cmd_ptr, " ");
            m++;
        } 
        x++;
        m = 0;

        if (fork() == 0) { //be childish
            //exec
            //int (*function_ptr) ();
            //char* args[MAXARGS];

            if (cmd_index != 0) { //Not First
                dup2(pipe_fd[2*cmd_index - 2], 0);
            }
            if (cmd_index != (cmd_num - 1)) { //Not Last
                dup2(pipe_fd[2*cmd_index + 1], 1);
            }

            for (int z = 0; z < ((cmd_num - 1) * 2); z++) {
                    close(pipe_fd[z]);
            }
            evaluate_line (cmd_ptr);
            // if ((function_ptr = get_builtin(cmd_ptr)) != NULL) { // Create child and executes program
                
            // } else if (executable_flag == 1) { // Handle executables
            //     char words [LINE_SIZE];
            //     memset(words, 0, LINE_SIZE);
            //     strcpy(words, cmd_ptr);
            //     char* word = strtok(words, " ");
            //     char* command1 = malloc(strlen(word) + 1);
            //     strcpy(command1, word);
            //     char* filepath = malloc(LINE_SIZE); 
            //     int size = 0;  

            //     if (find_executable(filepath, command1) != NULL) { //File exists
            //         parseline(filepath, cmd_ptr, args); 
            //         if (execv(args[0], args) < 0) {
            //             printf("%s\n", "Unknown command");
            //         }
            //     }
            //     if (size != 0) {
            //         free_parseline(args, size);
            //     }
            //     free(command1);
            //     free(filepath);
            // }
            exit(0);
        }
        if (cmd_index != 0){
            close(pipe_fd[2*cmd_index - 2]);
        }
        if (cmd_index != (cmd_num - 1)) {
            close(pipe_fd[2*cmd_index + 1]);
        }

        cmd_index++;
    }
    for (int i = 0; i < cmd_num; i++) {
        wait(&status);
    }
    for (int z = 0; z < ((cmd_num - 1) * 2); z++) {
            close(pipe_fd[z]);
    }
    return 0;
}