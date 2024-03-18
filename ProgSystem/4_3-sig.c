/*
 * Auteur(s): Cudennec Gaël
 */

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void (*sig_avant)(int); /* for handling SIGINT and SIGQUIT */

void hdl_sys1(int n) { printf("hdl_sys1: Signal received: %d\n", (n - 1)); }
void hdl_sys2(int n) { printf("hdl_sys2: Signal received: %d\n", n); }

// Forward declaration of toggle functions to allow mutual references
void toggle_to_hdl_sys1(int sig);
void toggle_to_hdl_sys2(int sig);

// Toggle the signal handler to hdl_sys1 and set up the next toggle function
void toggle_to_hdl_sys1(int sig) {
  signal(SIGINT, hdl_sys1);            // Set SIGINT handler to hdl_sys1
  signal(SIGQUIT, toggle_to_hdl_sys2); // Next SIGQUIT will toggle to hdl_sys2
}

// Toggle the signal handler to hdl_sys2 and set up the next toggle function
void toggle_to_hdl_sys2(int sig) {
  signal(SIGINT, hdl_sys2); // Set SIGINT handler to hdl_sys2
  signal(SIGQUIT,
         toggle_to_hdl_sys1); // Next SIGQUIT will toggle back to hdl_sys1
}

void travail() {
  /* Je travaille tres intensement !    */
  /* Ne cherchez pas a comprendre ;-) */
  /* Il n'y a rien a modifier ici     */
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

void travail() __attribute__((noreturn));

int main() {
  printf("PID: %d\n", getpid());

  signal(SIGINT, hdl_sys1);            // Register hdl_sys1 for SIGINT
  signal(SIGQUIT, toggle_to_hdl_sys2); // Register toggle_mode for SIGQUIT

  travail();
}
