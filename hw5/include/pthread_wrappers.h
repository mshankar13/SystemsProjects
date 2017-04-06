#ifndef PTHREAD_WRAPPERS_H
#define PTHREAD_WRAPPERS_H

#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
 #include <errno.h>

void Pthread_mutex_init(pthread_mutex_t *mutex);

void Pthread_mutex_lock(pthread_mutex_t *mutex);

void Pthread_mutex_unlock(pthread_mutex_t *mutex);

void Pthread_create (pthread_t *thread, const pthread_attr_t *attr, 
	void *(*start_routine) (void *), void *arg);

void Pthread_join (pthread_t thread, void **retval);

void Pthread_mutex_destroy (pthread_mutex_t *mutex);

void Sem_init (sem_t *mutex, int pshared, unsigned int value);

void P (sem_t *mutex);

void V (sem_t *mutex);

void Pthread_setname_np (pthread_t thread, const char *name);
#endif