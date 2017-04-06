#include "helpers.h"

void getpath (char *filepath, char *root, char *filename) {
	size_t path_length = strlen(root) + strlen(filename) + 2; 
	char filep[path_length];
	memset(filep, 0, path_length);
	strcat(filep, "../");
	strcat(filep, root);
	strcat(filep, "/");
	strcat(filep, filename);
	strcpy(filepath, filep);
}

void average_duration (char *duration, double *result) {
    /*Sums up all durations*/
	*result += convert_number(duration);
}

double convert_number (char *line) {
	return (double)(atof(line));
}

void year_count (char *timestamp, int years[]) {
	time_t unixtime = atoi(timestamp);
	const char *format = "%a %b %d %Y";
	struct tm timeformat;
    char time[64];
    localtime_r(&unixtime, &timeformat);
    strftime(time, sizeof(time), format, &timeformat);

    /*time is a string where the last word is the year*/
    char *saveptr;
    char copy[64];
    strcpy(copy, time);
    strtok_r(copy, " ", &saveptr);
    strtok_r(NULL, " ", &saveptr);
    strtok_r(NULL, " ", &saveptr);
    char* year = strtok_r(NULL, " ", &saveptr);
    int y = atoi(year);
    
    /*Count the unique occurrences of year to get denominator*/
    int found = -1;
    for (int i = 0; i < 100; i++) { /*Check to see if the year is already stored*/
        if (years[i] == y) {
            found = 0;
        }
    }
    if (found == -1) { /*Store the unique year*/
        for (int j = 0; j < 100; j++) {
            if (years[j] == 0) {
                years[j] = y;
                break;
            }
        }
    }
}

void country_count (char *country, int country_count[], char *countries[], int incrementer) {
    int found = -1;
    for (int i = 0; i < 200; i++) { /*Sees if country is already in the list*/
        if (countries[i] != NULL) {
            if (strcmp(country, countries[i]) == 0) {
                found = 0;
                country_count[i] += incrementer;
            }
        }
    }
    if (found == -1) {
        for (int j = 0; j < 200; j++) { /*Inserts unique country into the list*/
            if (countries[j] == NULL) {
                countries[j] = malloc(strlen(country) + 1);
                strcpy(countries[j], country);
                country_count[j] += incrementer; /*Increments count*/
                break;
            }
        }
    }
}

int get_num_files () {
    DIR *dir_ptr = Opendir("../data"); /*Open dir*/
    /*Get the number of files in the directory excluding . and .. */
    struct dirent *entry = malloc(sizeof(struct dirent) + NAME_MAX + 1);
    struct dirent *result;
    /*Read dir*/
    int counter = 0;
    while (Readdir_r(dir_ptr, entry, &result) == 0 && result != NULL) { /*Loop through each file*/
        if (strcmp(result->d_name, ".") != 0 && strcmp(result->d_name, "..") != 0) {
            counter++;
        }
    }
    return counter;
}

char *thread_name (int num, char *n) {
    char *name = malloc(strlen(n) + 10);
    memset(name, 0, strlen(n) + 10);
    strcat(name, n);
    char res[5] = { '\0' };
    sprintf(res, "%d",num);
    strcat(name, res);
    return name;
}

void create_socketpair(int **fds, int nthreads) {
    for (int i = 0; i < nthreads; i++) {
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds[i])) {
            errno = ENFILE;
        }
    }
}