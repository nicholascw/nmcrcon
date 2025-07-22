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
#define HINTS_IMPL 0  // from example.c, TODO
#define HIST2DISK 0   // TODO

#if HINTS_IMPL
void completion(const char *buf, int pos, bestlineCompletions *lc) {
  (void)pos;
  if (buf[0] == 'h') {
    bestlineAddCompletion(lc, "hello");
    bestlineAddCompletion(lc, "hello there");
  }
}

char *hints(const char *buf, const char **ansi1, const char **ansi2) {
  if (!strcmp(buf, "hello")) {
    *ansi1 = "\033[35m"; /* magenta foreground */
    *ansi2 = "\033[39m"; /* reset foreground */
    return " World";
  }
  return NULL;
}
#endif
int _shell_builtin(int rconfd, const char *input) {
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
  if (!in_cmd) return -1;
  for (size_t i = 0; i < NUM_OF_CMD_HANDLERS; i++) {
    if (!strcmp(in_cmd, cmds[i].cmd)) {
      ret = cmds[i].func(rconfd, input);
      break;
    }
  }
  free(in_cmd);
  return ret;
}

int shell_rcon_auth(int rconfd) {
  int auth_result = -1;
  char *password = NULL;
  int failed_cnt = 0;
  do {
    if (password) {
      memset(password, '\0', strlen(password));
      free(password);
      password = NULL;
    }
    failed_cnt++;
    bestlineMaskModeEnable();
    password = bestline("Password: ");
    bestlineMaskModeDisable();
    if (!password) password = strdup("");
  } while ((auth_result = rcon_auth(rconfd, password)) < 0 && failed_cnt < 3);
  if (password) {
    memset(password, '\0', strlen(password));
    free(password);
  }
  return auth_result < 0 ? -1 : 0;
}

static bool gotint = false;
void _shell_sigint_handler(int signum) {
  if (signum == SIGINT) gotint = true;
}

void shell_loop(int rconfd) {
#if HINTS_IMPL
  /* Set the completion callback. This will be called every time the
   * user uses the <tab> key. */
  bestlineSetCompletionCallback(completion);
  bestlineSetHintsCallback(hints);
#if HIST2DISK
  bestlineHistoryLoad("history.txt");
#endif
#endif
  int auth_result = -1;
  if (nmcrcon_state.password) {
    auth_result = rcon_auth(rconfd, nmcrcon_state.password);
  }
  if (auth_result < 0) auth_result = shell_rcon_auth(rconfd);
  if (auth_result < 0) return;
  char *input_buf;
  // ignore SIGINT
  struct sigaction sa;
  sa.sa_handler = &_shell_sigint_handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, 0);

  while (1) {
    gotint = false;
    input_buf = bestline("nmcrcon> ");
    if (!input_buf) {
      // Ctrl-D or Ctrl-C.
      if (gotint) {
        putchar('\n');
        continue;
      }
      break;
    }
    if (input_buf[0] == '\0') {
      free(input_buf);
      continue;
    }
    bestlineHistoryAdd(input_buf);
#if HIST2DISK
    bestlineHistorySave("history.txt");
#endif
    int builtin_ret = _shell_builtin(rconfd, input_buf);
    if (!builtin_ret) rcon_exec(rconfd, input_buf);
    if (builtin_ret < 0) break;
    free(input_buf);
  }
}
