#include "shell.h"

#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bestline.h"
#include "builtin_cmds.h"
#include "conf.h"
#include "rcon.h"
#include "socket.h"

static bool gotint = false;

static void shell_sigint_handler(int signum) {
  if (signum == SIGINT) gotint = true;
}

static char *shell_lstrip(char *s) {
  while (*s && isspace((unsigned char)*s)) s++;
  return s;
}

static void shell_rstrip(char *s) {
  size_t len = strlen(s);
  while (len > 0 && isspace((unsigned char)s[len - 1])) {
    s[--len] = '\0';
  }
}

static void shell_sleep_wait(float seconds) {
  if (seconds <= 0) return;
  useconds_t usec = (useconds_t)(seconds * 1000000.0f);
  usleep(usec);
}

static void shell_replace_string(char **dst, const char *src) {
  char *copy = strdup(src);
  if (!copy) {
    perror("strdup");
    exit(1);
  }
  free(*dst);
  *dst = copy;
}

static void shell_apply_hostport(const char *hostport) {
  char *copy = strdup(hostport);
  if (!copy) {
    perror("strdup");
    exit(1);
  }

  char *host = copy;
  char *port = NULL;
  char *sp = strrchr(copy, ':');
  char *test_ipv6 = strrchr(copy, ']');
  if (test_ipv6 && strchr(copy, '[')) {
    host = strchr(copy, '[') + 1;
    *test_ipv6 = '\0';
  }
  if (sp && test_ipv6 < sp && strchr(copy, ':') == sp) {
    port = sp + 1;
    *sp = '\0';
  }

  shell_replace_string(&nmcrcon_state.host, host);
  if (port && *port) shell_replace_string(&nmcrcon_state.port, port);
  free(copy);
}

static void shell_update_credential(const char *credential) {
  if (!credential) return;
  shell_replace_string(&nmcrcon_state.credential, credential);
}

static shell_result_t shell_prompt_auth(shell_state_t *state) {
  int auth_result = -1;
  char *credential = NULL;
  int failed_cnt = 0;

  while (failed_cnt < 3) {
    if (credential) {
      memset(credential, '\0', strlen(credential));
      free(credential);
      credential = NULL;
    }
    failed_cnt++;
    bestlineMaskModeEnable();
    credential = bestline("Password: ");
    bestlineMaskModeDisable();
    if (!credential) credential = strdup("");
    if (!credential) {
      perror("strdup");
      return SHELL_RESULT_ERROR;
    }
    auth_result = rcon_auth(state->rconfd, credential);
    if (auth_result == 0) {
      shell_update_credential(credential);
      break;
    }
  }

  if (credential) {
    memset(credential, '\0', strlen(credential));
    free(credential);
  }
  return auth_result == 0 ? SHELL_RESULT_OK : SHELL_RESULT_ERROR;
}

static shell_result_t shell_builtin(shell_state_t *state, const char *input) {
  int ret = 0;
  char *sp;
  char *in_cmd = NULL;

  if ((sp = strchr(input, ' '))) {
    *sp = '\0';
    in_cmd = strdup(input);
    *sp = ' ';
  } else {
    in_cmd = strdup(input);
  }
  if (!in_cmd) {
    fprintf(stderr, "shell_builtin: strdup failed\n");
    return SHELL_RESULT_ERROR;
  }

  for (size_t i = 0; i < NUM_OF_CMD_HANDLERS; i++) {
    if (!strcmp(in_cmd, cmds[i].cmd)) {
      ret = cmds[i].func(state, input);
      free(in_cmd);
      if (ret < 0) return SHELL_RESULT_EXIT;
      if (ret > 0) return SHELL_RESULT_OK;
      return SHELL_RESULT_ERROR;
    }
  }

  free(in_cmd);
  return 2;
}

static void shell_completion(const char *buf, int pos,
                             bestlineCompletions *lc) {
  (void)pos;

  const char *sp = strchr(buf, ' ');
  if (sp) return;

  size_t prefix_len = strlen(buf);
  for (size_t i = 0; i < NUM_OF_CMD_HANDLERS; i++) {
    if (!strncmp(cmds[i].cmd, buf, prefix_len)) {
      bestlineAddCompletion(lc, cmds[i].cmd);
    }
  }
}

shell_result_t shell_auth(shell_state_t *state, const char *credential) {
  if (!state->connected) {
    fprintf(stderr, "Not connected.\n");
    return SHELL_RESULT_ERROR;
  }

  if (credential && *credential) {
    if (rcon_auth(state->rconfd, (char *)credential) == 0) {
      shell_update_credential(credential);
      return SHELL_RESULT_OK;
    }
  } else if (nmcrcon_state.credential && *nmcrcon_state.credential) {
    if (rcon_auth(state->rconfd, nmcrcon_state.credential) == 0) {
      return SHELL_RESULT_OK;
    }
  }

  if (!isatty(STDIN_FILENO)) {
    fprintf(stderr,
            "Authentication failed and no interactive prompt is available.\n");
    return SHELL_RESULT_ERROR;
  }
  return shell_prompt_auth(state);
}

shell_result_t shell_disconnect(shell_state_t *state) {
  if (!state->connected) return SHELL_RESULT_OK;

  close(state->rconfd);
  state->rconfd = -1;
  state->connected = 0;
  if (nmcrcon_state.verbose && !nmcrcon_state.silent) {
    fprintf(stderr, "Disconnected.\n");
  }
  return SHELL_RESULT_OK;
}

shell_result_t shell_connect(shell_state_t *state, const char *hostport) {
  if (hostport && *hostport) shell_apply_hostport(hostport);
  if (state->connected) shell_disconnect(state);

  int rconfd =
      socket_tryconnect(nmcrcon_state.host ? nmcrcon_state.host : "127.0.0.1",
                        nmcrcon_state.port ? nmcrcon_state.port : "25575");
  if (rconfd < 0) {
    fprintf(stderr, "Failed to connect to %s:%s\n", nmcrcon_state.host,
            nmcrcon_state.port);
    return SHELL_RESULT_ERROR;
  }

  state->rconfd = rconfd;
  state->connected = 1;
  if (nmcrcon_state.verbose && !nmcrcon_state.silent) {
    fprintf(stderr, "Connected to %s:%s\n", nmcrcon_state.host,
            nmcrcon_state.port);
  }

  if (shell_auth(state, NULL) != SHELL_RESULT_OK) {
    shell_disconnect(state);
    return SHELL_RESULT_ERROR;
  }
  return SHELL_RESULT_OK;
}

shell_result_t shell_exec_line(shell_state_t *state, const char *input) {
  shell_result_t builtin_ret = shell_builtin(state, input);
  if (builtin_ret == SHELL_RESULT_OK || builtin_ret == SHELL_RESULT_EXIT) {
    return builtin_ret;
  }
  if (builtin_ret != 2) return builtin_ret;

  if (!state->connected) {
    fprintf(stderr, "Not connected.\n");
    return SHELL_RESULT_ERROR;
  }
  if (nmcrcon_state.verbose && !nmcrcon_state.silent) {
    fprintf(stderr, "Executing: %s\n", input);
  }
  return rcon_exec(state->rconfd, (char *)input) == 0 ? SHELL_RESULT_OK
                                                      : SHELL_RESULT_ERROR;
}

shell_result_t shell_run_commands(shell_state_t *state) {
  bool ran_command = false;

  for (int i = 0; i < nmcrcon_state.cmds_c; i++) {
    const char *cmd = nmcrcon_state.cmds_v[i];
    if (!cmd || !*cmd) continue;
    if (ran_command) shell_sleep_wait(nmcrcon_state.wait_sec);
    shell_result_t ret = shell_exec_line(state, cmd);
    if (ret != SHELL_RESULT_OK) return ret;
    ran_command = true;
  }

  return SHELL_RESULT_OK;
}

shell_result_t shell_run_stream(shell_state_t *state, FILE *stream,
                                const char *source_name) {
  char *line = NULL;
  size_t cap = 0;
  ssize_t len;
  bool ran_command = false;

  while ((len = getline(&line, &cap, stream)) >= 0) {
    if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
    char *trimmed = shell_lstrip(line);
    shell_rstrip(trimmed);
    if (trimmed[0] == '\0' || trimmed[0] == '#') continue;

    if (ran_command) shell_sleep_wait(nmcrcon_state.wait_sec);
    shell_result_t ret = shell_exec_line(state, trimmed);
    if (ret != SHELL_RESULT_OK) {
      if (ret == SHELL_RESULT_ERROR && source_name) {
        fprintf(stderr, "%s: command failed: %s\n", source_name, trimmed);
      }
      free(line);
      return ret;
    }
    ran_command = true;
  }

  free(line);
  return SHELL_RESULT_OK;
}

shell_result_t shell_loop(shell_state_t *state) {
  bestlineSetCompletionCallback(shell_completion);

  if (nmcrcon_state.history_path)
    bestlineHistoryLoad(nmcrcon_state.history_path);

  struct sigaction sa;
  sa.sa_handler = &shell_sigint_handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, 0);

  while (1) {
    gotint = false;
    char *input_buf = bestline(nmcrcon_state.prompt);
    if (!input_buf) {
      if (gotint) {
        putchar('\n');
        continue;
      }
      break;
    }

    char *trimmed = shell_lstrip(input_buf);
    shell_rstrip(trimmed);
    if (trimmed[0] == '\0' || trimmed[0] == '#') {
      free(input_buf);
      continue;
    }

    bestlineHistoryAdd(trimmed);
    if (nmcrcon_state.history_path)
      bestlineHistorySave(nmcrcon_state.history_path);

    shell_result_t ret = shell_exec_line(state, trimmed);
    free(input_buf);
    if (ret == SHELL_RESULT_EXIT) break;
  }

  return SHELL_RESULT_OK;
}
