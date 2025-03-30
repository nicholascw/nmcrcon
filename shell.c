#include <bestline.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

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

static int exit_now = 0;
void _shell_sigint_hdlr(int sig) { exit_now = 1; }

void _shell_rcon_auth(int rconfd) {
  char *password;
  bestlineMaskModeEnable();
  password = bestline("Password: ");
  bestlineMaskModeDisable();
  if (!password) return;
  if (password[0] == '\0') return;
  if (rcon_auth(rconfd, password) < 0) exit_now = 1;
  free(password);
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
  signal(SIGINT, _shell_sigint_hdlr);
  _shell_rcon_auth(rconfd);
  char *input_buf;
  while ((input_buf = bestline("nmcrcon> ")) && !exit_now) {
    if (input_buf[0] != '\0') {
      bestlineHistoryAdd(input_buf);
#if HIST2DISK
      bestlineHistorySave("history.txt");
#endif
      if (!strcmp(input_buf, ".exit")) exit_now = 1;
      if (!strcmp(input_buf, ".auth")) {
        _shell_rcon_auth(rconfd);
      } else {
        if (rcon_exec(rconfd, input_buf) < 0) exit_now = 1;
      }
      free(input_buf);
    }
    return;
  }
}