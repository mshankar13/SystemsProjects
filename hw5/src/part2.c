#include "lott.h"
#include "helpers.h"
#include "pthread_wrappers.h"
#include "wrappers.h"

static void* map(void*);
static void* reduce(void*);

struct file_node *file_node_head2 = NULL;
struct threads *threads_head = NULL;

volatile int threads_count2 = 0;


int part2(size_t nthreads){

    if (nthreads == 0) {
        return -1;
    }
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

                    if (threads_head == NULL) {
                        threads_head = thread_node;
                        threads_head->next = NULL;
                        threads_head->prev = NULL;
                    } else {   
                        threads_head->prev = thread_node;
                        thread_node->next = threads_head;
                        threads_head = thread_node;
                        threads_head->prev = NULL;
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
                if (file_node_head2 == NULL) { /*Add the file to the list for reduce*/
                    file_node_head2 = node;
                    file_node_head2->next = NULL;
                    file_node_head2->prev = NULL;
                } else {
                    file_node_head2->prev = node;
                    node->next = file_node_head2;
                    file_node_head2 = node;
                    file_node_head2->prev = NULL;
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
        pthread_setname_np(thread, thread_name (threads_count2, "Map"));
        threads_count2++;
        thread_node->thread = thread;
        thread_num++;
        file_size = 0; /*Reset file size*/
        num_files_read = 0; /*Reset num_files_read*/
        index = 0;
    } 
    struct threads *current_thread = threads_head;
    while (current_thread != NULL) {
        Pthread_join(current_thread->thread, NULL); /*Reap the threads*/
        current_thread = current_thread->next;
    }

    reduce(NULL);
    
    free(entry);
    struct file_node *current_node = file_node_head2;
    /*Free filenames*/
    while (current_node != NULL) {   
        free(current_node->filename);
        current_node = current_node->next;
    }

    /*Free countries and country*/
    current_node = file_node_head2;
    if (current_query == E) {
        while (current_node != NULL) {
            for (int i = 0; i < 200; i++) {
                if (current_node->countries[i] != NULL) {
                    free(current_node->countries[i]);
                } else {
                    break;
                }
            }
            free(current_node->country);
            current_node = current_node->next;
        }

    }

    /*Free nodes*/
    current_node = file_node_head2;
    struct file_node *temp = current_node;
    while (current_node != NULL) {
        temp = current_node->next;
        free(current_node);
        current_node = temp;
    }

    

    return 0;
}

static void* map(void* v){
    struct threads *thread_node = (struct threads*) v;
    for (int i = 0; i < thread_node->num_files; i++) { /*Go through each file in the thread*/
        struct file_node *node = thread_node->nodes[i];
        /*Open file*/
        char* filepath = malloc(strlen(DATA_DIR) + strlen(node->filename) + 6);
        getpath(filepath, DATA_DIR, node->filename);
        FILE *fp;
        fp = fopen(filepath, "r");
        char line[256];
        while (fgets(line, sizeof(line), fp) != NULL) { /*Read each line from the file*/
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
                /*Keep track of each file's country count*/
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

        fclose(fp); /*Close the filepath*/
        free(filepath);
    }
   
    return NULL;
}

static void* reduce(void* v){
   
    /*Keep track of query results*/
    double results = 0;
    char *resultstring = NULL;

    /*Keep track of Query E results*/
    int country_counts[200]; 
    for (int i = 0; i < 200; i++) {
        country_counts[i] = 0;
    }
    char *countries[200];
    for (int i = 0; i < 200; i++) {
        countries[i] = NULL;
    }

    struct file_node *current_node = file_node_head2;
    resultstring = current_node->filename;
    while (current_node != NULL) {
        if (current_query == A || current_query == C) {
            /*Check for greater than or equal to*/
            if ((current_node->result >= results)) {
                if (current_node->result == results) {
                    /*Check for alphabetical ordering*/
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
            /*Check for less than or equal to*/
           if ((current_node->result <= results || results == 0)) {
                if (current_node->result == results) {
                    /*Check for alphabetical ordering*/
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
        current_node = current_node->next;
    }

    if (current_query == E) { /*Do a countries count on the results if Query E*/
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


    if (current_query == E) { /*Print depending on the query*/
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

    if (current_query == E) { /*Free memory*/
        for (int i = 0; i < 200; i++) {
            if (countries[i] != NULL) {
                free(countries[i]);
            } else {
                break;
            }
        }
    }
    return NULL;
}
