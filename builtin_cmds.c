

#include "builtin_cmds.h"

#include <unistd.h>

#include "bestline.h"
#include "shell.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int cmdh_exit(STD_CMDH_ARGS) { return -1; }

int cmdh_clear(STD_CMDH_ARGS) {
  bestlineClearScreen(STDOUT_FILENO);
  return 1;
}

int cmdh_auth(STD_CMDH_ARGS) {
  if (shell_rcon_auth(rconfd) < 0) return -1;
  return 1;
}

#pragma GCC diagnostic pop