#ifndef HELPERS_H
#define HELPERS_H

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/socket.h>
#include "wrappers.h"

/*There are under 200 countries in the world*/
#define COUNTRIES 200

/*Linked list to keep track of each file's data*/
struct file_node {
	double result; /*Result of the file's query calculations*/
	double denominator; /*Denominator for calculations*/
	char *filename; /*Resultant filename*/
	char *country;
	pthread_t thread;

	size_t size;

	int years [100];
	int country_count[COUNTRIES];
	char *countries[COUNTRIES]; /*There are less than 200 countries in the world*/

	struct file_node *prev;
	struct file_node *next;
};

/*For part 2. Keeps track of threads which contain an array of pointers file_nodes for map analysis*/
struct threads {

	pthread_t thread;
	struct threads *prev;
	struct threads *next;

	int num_files;

	struct file_node **nodes;
};

/*Gets the path to each file in the directory*/
void getpath (char *filepath, char *root, char *filename);

/*Calculates the avg duration -- Query A*/
void average_duration (char *duration, double *result);

/*Converts string to double*/
double convert_number (char *line);

/*Counts the number of years for Queries C and D*/
void year_count (char *timestamp, int years[]);

/*Query E -- Calculated the number of countries*/
void country_count (char *country, int country_count[], char *countries[], int incrementer);

/*Gets the number of files in the data directory*/
int get_num_files ();

/*Creates the threadname for renaming each map thread*/
char *thread_name (int num, char *n);

/*Create socketpairs to be passed to the map and reduce threads*/
void create_socketpair(int **fds, int nthreads);

#endif