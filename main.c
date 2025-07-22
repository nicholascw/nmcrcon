#include <argp.h>
#include <error.h>
#include <stdio.h>
#include <unistd.h>

#include "conf.h"
#include "dbg.h"
#include "shell.h"
#include "socket.h"

int main(int argc, char *argv[]) {
  conf_init_state(argc, argv);
  int connect_fd =
      socket_tryconnect(nmcrcon_state.host ? nmcrcon_state.host : "127.0.0.1",
                        nmcrcon_state.port ? nmcrcon_state.port : "25575");
  if (connect_fd < 0) {
    fprintf(stderr, "Failed to connect to %s:%s\n", nmcrcon_state.host,
            nmcrcon_state.port);
    conf_usage(argv[0]);
    exit(1);
  }
  shell_loop(connect_fd);
  close(connect_fd);

  return 0;
}
