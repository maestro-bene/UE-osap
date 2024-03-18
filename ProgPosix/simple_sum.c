#include <stdio.h>
#include <stdlib.h>

#define N_MAX 1000000000 // Nombre d'itérations
long double sum = 0.0;   // Somme pour le calcul de π

void simple_sum() {
  unsigned long long n;
  long double sign = 1.0;
  for (n = 0; n < N_MAX; n++, sign = -sign) {
    sum += sign / (2.0 * n + 1.0);
  }
}

int main() {
  simple_sum();
  printf("pi = %.20Lf\n", 4.0 * sum);
  exit(EXIT_SUCCESS);
}
