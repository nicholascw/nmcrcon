#pragma once

#include <stdio.h>

typedef enum {
  SHELL_RESULT_ERROR = -1,
  SHELL_RESULT_OK = 0,
  SHELL_RESULT_EXIT = 1,
} shell_result_t;

typedef struct shell_state {
  int rconfd;
  int connected;
  int interactive;
} shell_state_t;

shell_result_t shell_auth(shell_state_t *state, const char *password);
shell_result_t shell_connect(shell_state_t *state, const char *hostport);
shell_result_t shell_disconnect(shell_state_t *state);
shell_result_t shell_exec_line(shell_state_t *state, const char *input);
shell_result_t shell_loop(shell_state_t *state);
shell_result_t shell_run_commands(shell_state_t *state);
shell_result_t shell_run_stream(shell_state_t *state, FILE *stream,
                                const char *source_name);