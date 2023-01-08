#include "utils.h"

#include <pthread.h>
#include <stdlib.h>

void mutex_lock(pthread_mutex_t *lock) {
  if (pthread_mutex_lock(lock) != 0) {
    perror("pthread_mutex_lock");
    exit(EXIT_FAILURE);
  }
}

void mutex_unlock(pthread_mutex_t *lock) {
  if (pthread_mutex_unlock(lock) != 0) {
    perror("pthread_mutex_unlock");
    exit(EXIT_FAILURE);
  }
}

void mutex_destroy(pthread_mutex_t *lock) {
  if (pthread_mutex_destroy(lock) != 0) {
    perror("pthread_mutex_destroy");
    exit(EXIT_FAILURE);
  }
}

void mutex_init(pthread_mutex_t *lock) {
  if (pthread_mutex_init(lock, NULL) != 0) {
    perror("pthread_mutex_init");
    exit(EXIT_FAILURE);
  }
}