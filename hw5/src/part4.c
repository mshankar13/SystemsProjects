#include "lott.h"
#include "helpers.h"
#include "pthread_wrappers.h"
#include "wrappers.h"

static void* map(void*);
static void* reduce(void*);

sem_t reading4, writing4;
sem_t x, y, z;

volatile int threads_count4 = 0;
int readcount4 = 0;
int signal4 = -1;

struct file_node *file_node_head4 = NULL;
struct threads *threads_head4 = NULL;


int part4(size_t nthreads){

    if (nthreads == 0) {
        return -1;
    }

    Sem_init(&reading4, 0, 1); /*Initialize*/
    Sem_init(&writing4, 0, 1); /*Initialize*/

    int file_num = get_num_files(); /*Calculate the number of files*/
    int num_files_read = 0;

    /*Calculate the number of files per thread*/
    int files_per_thread = file_num / nthreads;
    int remainder = 0;

    if ((remainder = file_num % nthreads) != 0) { /*Check for even division*/
        files_per_thread++;
    }

    DIR *dir_ptr = Opendir("../data"); /*Open dir*/
    struct dirent *entry = malloc(sizeof(struct dirent) + NAME_MAX + 1);
    struct dirent *result;


    pthread_t rthread; /*Reduce thread for reading*/
    Pthread_create(&rthread, NULL, reduce, &rthread);

    int total = 0;
    int thread_num = 0;
    int file_size = 0;  
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

                    if (threads_head4 == NULL) {
                        threads_head4 = thread_node;
                        threads_head4->next = NULL;
                        threads_head4->prev = NULL;
                    } else {   
                        threads_head4->prev = thread_node;
                        thread_node->next = threads_head4;
                        threads_head4 = thread_node;
                        threads_head4->prev = NULL;
                    }
                }

                /*Increment counts*/
                num_files_read++;
                file_size++;
                total++;

                struct file_node *node = malloc(sizeof(struct file_node));
                memset(node, 0, sizeof(struct file_node));
                node->filename = malloc(strlen(result->d_name) + 1);
                strcpy(node->filename, result->d_name);

                node->prev = NULL;
                node->next = NULL;
                if (file_node_head4 == NULL) { /*Add the file to the list for reduce*/
                    file_node_head4 = node;
                    file_node_head4->next = NULL;
                    file_node_head4->prev = NULL;
                } else {
                    file_node_head4->prev = node;
                    node->next = file_node_head4;
                    file_node_head4 = node;
                    file_node_head4->prev = NULL;
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
        pthread_setname_np(thread, thread_name (threads_count4, "Map"));
        threads_count4++;
        thread_node->thread = thread;
        thread_num++;
        file_size = 0; /*Reset file size*/
        num_files_read = 0; /*Reset num_files_read*/
        index = 0;
    } 
    struct threads *current_thread = threads_head4;
    while (current_thread != NULL) {
        Pthread_join(current_thread->thread, NULL);
        current_thread = current_thread->next;
    }
    signal4 = 0;
    Pthread_join(rthread, NULL);

    return 0;
}

static void* map(void* v){ /*Producer -- Writer <<Priority*/
    P(&writing4);
    readcount4++;
    if (readcount4 == 1) {
        P(&reading4);
    }
    V(&writing4);

    /*Critical Section*/
    struct threads *thread_node = (struct threads*) v;
    for (int i = 0; i < thread_node->num_files; i++) { /*Go through each file in the thread*/
        struct file_node *node = thread_node->nodes[i];
        /*Open file*/
        char* filepath = malloc(strlen(DATA_DIR) + strlen(node->filename) + 6);
        getpath(filepath, DATA_DIR, node->filename);
        FILE *fp;
        fp = fopen(filepath, "r");
        char line[256];
        while (fgets(line, sizeof(line), fp) != NULL) {
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
        } else if (current_query == E) {
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
    }
    /*Critical Section End*/

    P(&writing4);
    readcount4--;
    if (readcount4 == 0) {
        V(&reading4);
    }
    V(&writing4);

    return NULL;
}

static void* reduce(void* v){ /*Consumer -- Reader*/
    pthread_t *thr = (pthread_t *) v;

    /*Keep track of the results*/
    double results = 0;
    char *resultstring = NULL;

    /*Results for query E*/
    int country_counts[200]; 
    for (int i = 0; i < 200; i++) {
        country_counts[i] = 0;
    }
    char *countries[200];
    for (int i = 0; i < 200; i++) {
        countries[i] = NULL;
    }

    /*old_head keeps track of what was already appended and where to stop when reading from the list*/
    struct file_node *current_node = file_node_head4;
    struct file_node *old_head = NULL;
    if (current_node != NULL) {
        resultstring = current_node->filename;
    }
    while (1) {
        P(&reading4);
        /*Critical region*/
        current_node = file_node_head4;
        old_head = current_node;
        /*Check to see if any nodes have been written to*/
        while (current_node != NULL && old_head != NULL) {
            if (current_query == A || current_query == C) {
                if ((current_node->result >= results)) {
                    /*Value is greater than or equal*/
                    if (current_node->result == results) {
                        /*Check alphabetical positioning*/
                        if (strcmp(current_node->filename, resultstring) < 0) {
                            results = current_node->result;
                            resultstring = current_node->filename;
                        }
                    } else {
                        results = current_node->result;
                        resultstring = current_node->filename;
                    }
                }
            } else if (current_query == B || current_query == D) {
               if ((current_node->result <= results || results == 0)) { 
               /*Checks to see if value is less than first*/
                    if (current_node->result == results) {
                        /*Value equal, check for alphabetical positioning*/
                        if (strcmp(current_node->filename, resultstring) < 0) {
                            results = current_node->result;
                            resultstring = current_node->filename;
                        }
                    } else {
                        results = current_node->result;
                        resultstring = current_node->filename;
                    }
               }
            } else {
                /*Query E!*/  
                /*country and result from node*/
                country_count (current_node->country, country_counts, countries, current_node->result);
            }
            if (current_node != NULL) {
                current_node = current_node->next;
            }  
        }

        if (old_head != NULL) { 
        /*Checks to see if there is even a filenode before checking for query e*/
            if (current_query == E) {
                /*Continuation of query calculations with populated countries and country_count lists*/
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
        }
        /*End Critical region*/
        V(&reading4);
        usleep(0.5);
        if (signal4 == 0) {
            if (current_query == E) { /*Print out depending on the query*/
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
            pthread_cancel(*thr);
            return NULL;
        }
    }
    return NULL;
}
