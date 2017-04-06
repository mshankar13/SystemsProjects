#include "wrappers.h"



DIR *Opendir (const char *name) {
	DIR *dir_ptr;
	if ((dir_ptr = opendir(name)) == NULL) {
		errno = ENOENT;
	}
	return dir_ptr;
}

int Readdir_r (DIR *dirp, struct dirent *entry, struct dirent **result) {
	int ret = 0;
	if ((ret = readdir_r(dirp, entry, result)) != 0) {
		errno = EBADF;
	}
	return ret;
}

void Closedir (DIR *dirp) {
	if (closedir(dirp) != 0) {
		errno = EBADF;
	}
}

void Reader (pthread_mutex_t *mutex) {
	Pthread_mutex_lock(mutex);

	Pthread_mutex_unlock(mutex);
}

void Writer (pthread_mutex_t *mutex) {
	Pthread_mutex_lock(mutex);
	Pthread_mutex_unlock(mutex);
}