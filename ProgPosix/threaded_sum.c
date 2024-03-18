#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define N_MAX 1000000000
#define T_COUNT 8 // Nombre de threads à utiliser

pthread_mutex_t mutex_sum = PTHREAD_MUTEX_INITIALIZER;
long double sum = 0.0;

typedef struct {
  unsigned long long start;
  unsigned long long end;
} ThreadArg;

void *thread_func(void *arg) {
  ThreadArg *t_arg = (ThreadArg *)arg;
  long double local_sum = 0.0;
  for (unsigned long long n = t_arg->start; n < t_arg->end; n++) {
    long double sign = (n % 2 == 0) ? 1.0 : -1.0;
    local_sum += sign / (2.0 * n + 1.0);
  }

  // Mise à jour sécurisée de la somme globale
  pthread_mutex_lock(&mutex_sum);
  sum += local_sum;
  pthread_mutex_unlock(&mutex_sum);

  return NULL;
}

int main() {
  pthread_t threads[T_COUNT];
  ThreadArg t_args[T_COUNT];
  unsigned long long range = N_MAX / T_COUNT;

  // Création des threads
  for (int i = 0; i < T_COUNT; i++) {
    t_args[i].start = i * range;
    t_args[i].end = (i + 1) * range;
    pthread_create(&threads[i], NULL, thread_func, &t_args[i]);
  }

  // Attente de la fin des threads
  for (int i = 0; i < T_COUNT; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("pi = %.20Lf\n", 4.0 * sum);
  exit(EXIT_SUCCESS);
}
