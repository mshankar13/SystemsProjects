#include "lott.h"
#include "helpers.h"
#include "pthread_wrappers.h"
#include "wrappers.h"

static void* map(void*);
static void* reduce(void*);
static void* Write(void*);
static void* Read(void*);
volatile int readcount = 0;

volatile int signal = -1;
sem_t reading, writing;
volatile int loop = 0;
volatile int threads_count3 = 0;

struct file_node *file_node_head3 = NULL;
struct threads *threads_head3 = NULL;

int part3(size_t nthreads){

    if (nthreads == 0) {
        return -1;
    }

    Sem_init(&reading, 0, 1); /*Initialize*/
    Sem_init(&writing, 0, 1); /*Initialize*/

    int file_num = get_num_files(); /*Calculate the number of files*/
    int num_files_read = 0;
    
    /*Calculate the number of files per thread*/
    int files_per_thread = file_num / nthreads;
    int remainder = 0;
    //int rem_flag = -1;
    if ((remainder = file_num % nthreads) != 0) { /*Check for even division*/
        //rem_flag = 0;
        files_per_thread++;
    }

    DIR *dir_ptr = Opendir("../data"); /*Open dir*/
    struct dirent *entry = malloc(sizeof(struct dirent) + NAME_MAX + 1);
    struct dirent *result;

    int total = 0; /*Ensure all files are read -- For debugging purposes*/
    int thread_num = 0; /*Keeps track of the number of threads made*/
    int file_size = 0;
    
    FILE *writes = fopen("mapred.tmp", "w");
    fclose(writes);

    pthread_t rthread; /*Reduce thread for reading*/
    Pthread_create(&rthread, NULL, reduce, &rthread);
    pthread_setname_np(rthread, "Reduce");

    while (thread_num < nthreads) {
        struct threads *thread_node; /*Thread node per thread to pass to map*/
        int index = 0; /*Keep track of the position of filenodes in threads*/
        while (Readdir_r(dir_ptr, entry, &result) == 0 && (result != NULL)) { /*Loop through each file*/
            if (strcmp(result->d_name, ".") != 0 && strcmp(result->d_name, "..") != 0) { /*Valid file?*/
                
                if (num_files_read == 0) {
                    /*Create a thread*/
                    thread_node = malloc(sizeof(struct threads));
                    memset(thread_node, 0, sizeof(struct threads));
                    thread_node->nodes = malloc(sizeof(struct file_node*) * files_per_thread);

                    if (threads_head3 == NULL) { /*Add thread to the linked list of threads*/
                        threads_head3 = thread_node;
                        threads_head3->next = NULL;
                        threads_head3->prev = NULL;
                    } else {   
                        threads_head3->prev = thread_node;
                        thread_node->next = threads_head3;
                        threads_head3 = thread_node;
                        threads_head3->prev = NULL;
                    }
                }

                /*Increment counts*/
                num_files_read++;
                file_size++;
                total++;

                /*Filenode for each file to store data*/
                struct file_node *node = malloc(sizeof(struct file_node));
                memset(node, 0, sizeof(struct file_node));
                node->filename = malloc(strlen(result->d_name) + 1);
                strcpy(node->filename, result->d_name);

                node->prev = NULL;
                node->next = NULL;
                if (file_node_head3 == NULL) { /*Add the file to the list for reduce*/
                    file_node_head3 = node;
                    file_node_head3->next = NULL;
                    file_node_head3->prev = NULL;
                } else {
                    file_node_head3->prev = node;
                    node->next = file_node_head3;
                    file_node_head3= node;
                    file_node_head3->prev = NULL;
                }
                thread_node->nodes[index] = node;
                index++;

            }
            
            /*Check to see if avg files per thread met*/
            if ((num_files_read) == files_per_thread) { 
                break; /*Move on to next thread*/
            }
        }
        thread_node->num_files = num_files_read; /*Update num_files read*/
        /*Create new thread*/
        pthread_t thread;
        Pthread_create(&thread, NULL, map, thread_node);
        pthread_setname_np(thread, thread_name (threads_count3, "Map"));
        threads_count3++;
        thread_node->thread = thread;
        thread_num++;
        file_size = 0; /*Reset file size*/
        num_files_read = 0; /*Reset num_files_read*/
        index = 0;
    } 

    struct threads *current_thread = threads_head3;
    while (current_thread != NULL) { /*Reap threads*/
        Pthread_join(current_thread->thread, NULL);
        current_thread = current_thread->next;
    }
    signal = 0;
    Pthread_join(rthread, NULL); /*Join the reduce thread once all map threads are completed*/
    //reduce(NULL);
    return 0;
}

/*Writing*/
static void* map(void* v){
    Write(v);
    return NULL;
}

/*Reading - preference*/
static void* reduce(void* v){
    Read(v);
    return NULL;
}

void *Write (void *v) {
    //usleep(1); /*Sleep to stall slightly*/

    P(&writing); /*Semwait*/

    /*Critical Section where writing will happen --------------------------------*/
    FILE *writes = fopen("mapred.tmp", "a");

    struct threads *thread_node = (struct threads*) v;
    for (int i = 0; i < thread_node->num_files; i++) { /*Go through each file in the thread*/
        struct file_node *node = thread_node->nodes[i];
        /*Open file*/
        char* filepath = malloc(strlen(DATA_DIR) + strlen(node->filename) + 6);
        getpath(filepath, DATA_DIR, node->filename);
        FILE *fp;
        fp = fopen(filepath, "r");
        char line[256];
        while (fgets(line, sizeof(line), fp) != NULL) { /*Get data from a file and store it*/
            char cpy[256];
            strcpy(cpy, line);
            char *saveptr;
            char *timestamp = strtok_r(cpy, ",", &saveptr);
            strtok_r(NULL, ",", &saveptr);
            char *duration = strtok_r(NULL, ",", &saveptr);
            char* country = strtok_r(NULL, ",", &saveptr);
            if (current_query == A || current_query == B) {
                average_duration (duration, &node->result);
                node->denominator ++;
            } else if (current_query == C || current_query == D) {
                node->result++;
                /*Calculate the total number of unique years*/
                year_count (timestamp, node->years);
            } else {
                country_count (country, node->country_count, node->countries, 1);
            }

        }
        
        if (current_query == C || current_query == D) {
            for (int i = 0; i < 100; i++) { /*Check to see if the year is already stored*/
                if (node->years[i] != 0) {
                    node->denominator++;
                }
            }
        } else if (current_query == E) { /*Check to see which country is higher*/
            for (int i = 0; i < 200; i++) {
                if ((double)node->country_count[i] > node->result) {
                    node->result = (double)node->country_count[i];
                    node->country = malloc(strlen(node->countries[i]) + 1);
                    strcpy(node->country, node->countries[i]);
                }
            }
        }

        if (current_query != E) {
            node->result = node->result / node->denominator;
        } else {
        }

        fclose(fp);
        free(filepath);

        /*Write the result and filename to file*/ 
        /*For query E write the country*/
        char res[16] = { '\0' };
        sprintf(res, "%.8g", node->result); 
        fputs(res, writes); 
        fputs(",", writes);
        fputs(node->filename, writes);   
        if (current_query == E) {
            fputs(",", writes);
            fputs(node->country, writes);
        } else {
            fputs("\n", writes);
        }
        loop++;
    }


    fclose(writes); 
    V(&writing); /*Sempost*/
    /*End Critical Section ------------------------------------------------------*/
    usleep(.5);
    return NULL;
}

void *Read (void *v) {
    pthread_t *thr = (pthread_t *) v;
    FILE *reads = fopen("mapred.tmp", "r");
    char line [256];
    char *filename;
    //int number = 0;

    double results = 0;
    char *resultstring = malloc(16);
    strcpy(resultstring, "~~~~~~~~~~~~~~~");

    int country_counts[200]; 
    for (int i = 0; i < 200; i++) {
        country_counts[i] = 0;
    }
    char *countries[200];
    for (int i = 0; i < 200; i++) {
        countries[i] = NULL;
    }

    while (1) {
        //usleep(.50); /*Sleep to allow map time to work*/

        P(&reading); /*SemWait*/
        readcount++;

        if (readcount == 1) { /*First*/
            P(&writing); /*SemWait*/
        }
        V(&reading); /*SemPost*/

        /*Critical section*/
        //memset(line, 0, 256);
        char *status;
        /*Read each line from the file of results*/
        while (((status = fgets(line, sizeof(line), reads)) != NULL)) {
            char *string = strdup(line);
            /*String contains the values to parse*/
            char cpy[256];
            strcpy(cpy, string);
            char *saveptr;
            if (current_query != E) {
                char *value = strtok_r(cpy, ",", &saveptr);
                double convert = convert_number (value);
                filename = strtok_r(NULL, ",", &saveptr);
                if (current_query == A || current_query == C) {
                    if ((convert >= results)) {
                        if (convert == results) { 
                            /*Check for alphabetical*/
                            if (strcmp(filename, resultstring) < 0) {
                                results = convert;
                                free(resultstring);
                                resultstring = malloc(strlen(filename) + 1);
                                strcpy(resultstring, filename);
                            }
                        } else {
                            results = convert;
                            free(resultstring);
                            resultstring = malloc(strlen(filename) + 1);
                            strcpy(resultstring, filename);
                        }
                    }
                } else if (current_query == B || current_query == D) {
                   if ((convert <= results || results == 0)) {
                        if (convert == results) {
                            /*Alphabetical ordering*/
                            if (strcmp(filename, resultstring) < 0) {
                                results = convert;
                                free(resultstring);
                                resultstring = malloc(strlen(filename) + 1);
                                strcpy(resultstring, filename);
                            }
                        } else {
                            results = convert;
                            free(resultstring);
                            resultstring = malloc(strlen(filename) + 1);
                            strcpy(resultstring, filename);
                        }
                    }
                } 
            } else {
                /*Query E line parsing*/
                char *value = strtok_r(cpy, ",", &saveptr);
                double convert = convert_number (value);
                strtok_r(NULL, ",", &saveptr);
                char *country = strtok_r(NULL, ",", &saveptr);

                country_count (country, country_counts, countries, convert);
            }

            free(string);
        }

        if (current_query == E) { /*Finishing calculating Query E*/
            resultstring = countries[0]; //default value
            for (int i = 0; i < 200; i++) {
                if ((country_counts[i] >= results)) {
                    if (country_counts[i] == results) {
                        if (strcmp(countries[i], resultstring) < 0) {
                            results = country_counts[i];
                            resultstring = countries[i];
                        }
                    } else {
                        results = country_counts[i];
                        resultstring = countries[i];
                    }
                }
            }
        }
        /*Reading happens*/

        P(&reading); /*SemWait*/
        readcount--;
        if (readcount == 0) {
            V(&writing); /*SemPost*/
        }
        V(&reading); /*SemPost*/
        if (signal == 0) {
            if (current_query == E) {
                printf(
                "Part: %s\n"
                "Query: %s\n"
                "Result: %d, %s\n",
                PART_STRINGS[current_part], QUERY_STRINGS[current_query], (int)results, resultstring);
            } else {
                printf(
                "Part: %s\n"
                "Query: %s\n"
                "Result: %.5g, %s\n",
                PART_STRINGS[current_part], QUERY_STRINGS[current_query], results, resultstring);
            }
            /*End this thread*/
            fclose(reads);
            unlink("mapred.tmp");
            pthread_cancel(*thr);
            return NULL;
        }
    }
    return NULL;
}