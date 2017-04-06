#include "lott.h"
#include "helpers.h"
#include "pthread_wrappers.h"
#include "wrappers.h"

static void* map(void*);
static void* reduce(void*);
struct file_node *file_node_head = NULL;
sem_t mutex;

int counter = 0;
int threads_count = 0;

double results = 0;
char *resultstring = NULL;

int part1(){
    
    Sem_init(&mutex, 0, 1); /*Initialize*/
    DIR *dir_ptr = Opendir("../data"); /*Open dir*/

    struct dirent *entry = malloc(sizeof(struct dirent) + NAME_MAX + 1);
    struct dirent *result;
    /*Read dir*/
    while (Readdir_r(dir_ptr, entry, &result) == 0 && result != NULL) { /*Loop through each file*/
        if (strcmp(result->d_name, ".") != 0 && strcmp(result->d_name, "..") != 0) {
            counter++;
            /*Create thread for each file to open*/
            pthread_t thread;
            struct file_node *node = malloc(sizeof(struct file_node));
            memset(node, 0, sizeof(struct file_node));
            node->filename = malloc(strlen(result->d_name) + 1);
            strcpy(node->filename, result->d_name);

            node->prev = NULL;
            node->next = NULL;
            if (file_node_head == NULL) { /*Add the filenode to the linked list*/
                file_node_head = node;
                file_node_head->next = NULL;
                file_node_head->prev = NULL;
            } else {
                file_node_head->prev = node;
                node->next = file_node_head;
                file_node_head = node;
                file_node_head->prev = NULL;
            }
            /*Call map*/

            Pthread_create(&thread, NULL, map, node); /*Create a thread for each filenode*/
            pthread_setname_np(thread, thread_name (threads_count, "Map")); /*Set the name for the thread*/
            node->thread = thread;
            threads_count++;
        }
        
    }

    /*Call Pthread join*/
    struct file_node *current_node = file_node_head;
    while (current_node != NULL) {
        Pthread_join(current_node->thread, NULL);
        current_node = current_node->next;
    }
    /*Call reduce in main thread*/
    reduce(NULL);
    /*Free all mallocs*/
    free(entry);
    /*Free filenames*/
    current_node = file_node_head;
    while (current_node != NULL) {   
        free(current_node->filename);
        current_node = current_node->next;
    }

    /*Free countries and country*/
    current_node = file_node_head;
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
    current_node = file_node_head;
    struct file_node *temp = current_node;
    while (current_node != NULL) {
        temp = current_node->next;
        free(current_node);
        current_node = temp;
    }

    Closedir(dir_ptr);

    return 0;
}

static void* map(void* v){
    // P (&mutex);
    struct file_node *node = (struct file_node *) v;
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
    return NULL;
}

static void* reduce(void* v){
    int country_counts[200]; 
    for (int i = 0; i < 200; i++) {
        country_counts[i] = 0;
    }
    char *countries[200];
    for (int i = 0; i < 200; i++) {
        countries[i] = NULL;
    }

    struct file_node *current_node = file_node_head;
    resultstring = current_node->filename;
    while (current_node != NULL) { /*Iterate through the list and get the calculated values per filenode*/
        if (current_query == A || current_query == C) {
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

    if (current_query == E) { /*Culmination of all Query E file results*/
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
    if (current_query == E) { /*Query E print result as int as per PIAZZA*/
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

    if (current_query == E) {
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
