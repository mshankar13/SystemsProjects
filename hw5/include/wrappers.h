#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include "pthread_wrappers.h"

DIR *Opendir (const char *name);

int Readdir_r (DIR *dirp, struct dirent *entry, struct dirent **result);

void Closedir (DIR *dirp);

#endif