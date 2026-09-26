#include <argp.h>
#include <error.h>
#include <stdio.h>
#include <unistd.h>

#include "conf.h"
#include "dbg.h"
#include "shell.h"

int main(int argc, char *argv[]) {
  conf_init_state(argc, argv);

  shell_state_t shell_state = {
      .rconfd = -1,
      .connected = 0,
      .interactive = isatty(STDIN_FILENO),
  };

  if (shell_connect(&shell_state, NULL) != SHELL_RESULT_OK) {
    if (nmcrcon_state.cmds_c > 0 || !shell_state.interactive) return 1;
    conf_usage(argv[0]);
    return 1;
  }

  shell_result_t ret = SHELL_RESULT_OK;
  if (nmcrcon_state.cmds_c > 0) {
    ret = shell_run_commands(&shell_state);
  } else if (!shell_state.interactive) {
    ret = shell_run_stream(&shell_state, stdin, "stdin");
  } else {
    ret = shell_loop(&shell_state);
  }

  shell_disconnect(&shell_state);
  return ret == SHELL_RESULT_ERROR ? 1 : 0;
}
