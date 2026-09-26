#include "builtin_cmds.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bestline.h"
#include "shell.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

static const char *cmd_args(const char *input) {
  const char *sp = strchr(input, ' ');
  if (!sp) return NULL;
  while (*sp == ' ') sp++;
  return *sp ? sp : NULL;
}

int cmdh_exit(STD_CMDH_ARGS) { return -1; }

int cmdh_clear(STD_CMDH_ARGS) {
  bestlineClearScreen(STDOUT_FILENO);
  return 1;
}

int cmdh_auth(STD_CMDH_ARGS) {
  const char *password = cmd_args(input);
  return shell_auth(state, password) == SHELL_RESULT_OK ? 1 : 0;
}

int cmdh_sleep(STD_CMDH_ARGS) {
  const char *arg = cmd_args(input);
  if (!arg) {
    fprintf(stderr, "Usage: sleep SECONDS\n");
    return 0;
  }

  char *end = NULL;
  errno = 0;
  double seconds = strtod(arg, &end);
  if (errno || end == arg || (end && *end != '\0') || seconds < 0) {
    fprintf(stderr, "sleep: invalid duration: %s\n", arg);
    return 0;
  }

  useconds_t usec = (useconds_t)(seconds * 1000000.0);
  usleep(usec);
  return 1;
}

int cmdh_disconnect(STD_CMDH_ARGS) {
  return shell_disconnect(state) == SHELL_RESULT_OK ? 1 : 0;
}

int cmdh_connect(STD_CMDH_ARGS) {
  return shell_connect(state, cmd_args(input)) == SHELL_RESULT_OK ? 1 : 0;
}

int cmdh_source(STD_CMDH_ARGS) {
  const char *path = cmd_args(input);
  if (!path) {
    fprintf(stderr, "Usage: source FILE\n");
    return 0;
  }

  FILE *stream = fopen(path, "r");
  if (!stream) {
    perror(path);
    return 0;
  }

  shell_result_t ret = shell_run_stream(state, stream, path);
  fclose(stream);
  if (ret == SHELL_RESULT_EXIT) return -1;
  return ret == SHELL_RESULT_OK ? 1 : 0;
}

#pragma GCC diagnostic pop
