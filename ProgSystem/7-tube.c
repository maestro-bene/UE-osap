/*
 * Auteur(s): Cudennec Gaël
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void create_pipe(int pipe_fd[2]) {
  // Create the communication pipe
  if (pipe(pipe_fd) == -1) {
    perror("Error creating pipe");
    exit(EXIT_FAILURE);
  }
}

void parent_process(int pipe_fd[2]) {
  // Close the read end of the pipe for the parent
  close(pipe_fd[0]);

  // Read from standard input and write to the pipe indefinitely
  char buffer[256];
  ssize_t bytes_read;
  while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
    if (write(pipe_fd[1], buffer, bytes_read) == -1) {
      perror("Error writing to pipe in parent process");
      exit(EXIT_FAILURE);
    }
  }

  // Close the write end of the pipe
  close(pipe_fd[1]);
}

void child_process(int pipe_fd[2]) {
  // Close the write end of the pipe for the child
  close(pipe_fd[1]);

  // Read from the pipe and print to the console indefinitely
  char buffer[256];
  ssize_t bytes_read;
  while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
    // Prepare the string to be printed
    char output[256];
    snprintf(output, sizeof(output), "Child process: %s", buffer);

    // Write the content to stdout
    if (write(STDOUT_FILENO, output, bytes_read + 15) == -1) {
      perror("Error writing to stdout in child process");
      exit(EXIT_FAILURE);
    }
  }

  // Close the read end of the pipe
  close(pipe_fd[0]);
}

int main() {
  int pipe_fd[2];

  create_pipe(pipe_fd);

  pid_t pid = fork();

  if (pid == -1) {
    perror("Error creating child process");
    exit(EXIT_FAILURE);
  }

  if (pid > 0) { // Parent process
    parent_process(pipe_fd);
  } else { // Child process
    child_process(pipe_fd);
  }

  return 0;
}
