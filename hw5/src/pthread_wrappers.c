#include "pthread_wrappers.h"

void Pthread_mutex_init(pthread_mutex_t *mutex) {
	if (pthread_mutex_init(mutex, NULL) != 0){
		errno = EINVAL;
    }
}

void Pthread_mutex_lock(pthread_mutex_t *mutex) {
	if (pthread_mutex_lock(mutex) != 0) {
		errno = EINVAL;
	}
}

void Pthread_mutex_unlock(pthread_mutex_t *mutex) {
	if (pthread_mutex_unlock(mutex) != 0) {
		errno = EINVAL;
	}
}

void Pthread_create (pthread_t *thread, const pthread_attr_t *attr, 
	void *(*start_routine) (void *), void *arg) {
	if (pthread_create(thread, attr, start_routine, arg) != 0) {
		errno = EINVAL;
	}
}

void Pthread_join (pthread_t thread, void **retval) {
	if (pthread_join(thread, retval) != 0) {
		errno = EINVAL;
	}
}

void Pthread_mutex_destroy (pthread_mutex_t *mutex) {
	if (pthread_mutex_destroy(mutex) != 0) {
		errno = EINVAL;
	}
}

void Sem_init (sem_t *mutex, int pshared, unsigned int value) {
	if (sem_init(mutex, pshared, value) < 0) {
		errno = EINVAL;
	}
}

void P (sem_t *mutex) {
	if (sem_wait(mutex) < 0) {
		errno = EINVAL;
	}
}

void V (sem_t *mutex) {
	if (sem_post(mutex) < 0) {
		errno = EINVAL;
	}
}

void Pthread_setname_np (pthread_t thread, const char *name) {
	if (pthread_setname_np(thread, name) != 0) {
		errno = ERANGE;
	}
}