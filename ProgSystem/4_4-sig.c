/*
 * Auteur(s): Cudennec Gaël
 */

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void hdl_sys1(int n) { printf("hdl_sys1: signal received: %d\n", (n - 1)); }
void hdl_sys2(int n) { printf("hdl_sys1: signal received: %d\n", n); }

void toggle_to_hdl_sys1(int sig);

void toggle_to_hdl_sys2(int sig) {
  struct sigaction sa;
  sa.sa_handler = hdl_sys2;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
  sa.sa_handler =
      toggle_to_hdl_sys1; // Set the next SIGQUIT action to toggle back
  sigaction(SIGQUIT, &sa, NULL);
}

void toggle_to_hdl_sys1(int sig) {
  struct sigaction sa;
  sa.sa_handler = hdl_sys1;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
  sa.sa_handler = toggle_to_hdl_sys2; // Set the next SIGQUIT action to toggle
  sigaction(SIGQUIT, &sa, NULL);
}

void travail() __attribute__((noreturn));

void travail() {
  const char msg[] = "-\\|/";
  const int sz = strlen(msg);
  int i = 0;

  for (;;) {
    write(STDOUT_FILENO, "\r", 1);
    usleep(100000);
    write(STDOUT_FILENO, " => ", 4);
    write(STDOUT_FILENO, &msg[i++], 1);
    if (i == sz)
      i = 0;
  }
}

int main() {
  printf("PID: %d\n", getpid());

  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  // Set initial handlers
  sa.sa_handler = hdl_sys1;
  sigaction(SIGINT, &sa, NULL);

  sa.sa_handler =
      toggle_to_hdl_sys2; // Prepare to toggle to hdl_sys2 upon SIGQUIT
  sigaction(SIGQUIT, &sa, NULL);

  travail();
}
