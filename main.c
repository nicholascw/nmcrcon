
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shell.h"
#include "socket.h"

#define MAXDATASIZE 8192

int main(int argc, char *argv[]) {
  char *host = NULL;
  char *port = NULL;
  if (argc > 1) {
    if (argc > 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
      goto print_usage;

    // parse host:port
    char *sp;
    if ((sp = strchr(argv[1], ':'))) {
      host = strndup(argv[1], sp - argv[1]);
      port = strdup(sp + 1);
    } else {
      host = strdup(argv[1]);
      port = strdup("25575");
    }

    if (!host || !port) goto print_usage;
  } else {
    host = strdup("localhost");
    port = strdup("25575");
  }

  int connfd = socket_tryconnect(host, port);
  if (connfd < 0) {
    fprintf(stderr, "Failed to connect to %s:%s\n", host, port);
    goto print_usage;
  }
  if (host) free(host);
  if (port) free(port);
  shell_loop(connfd);
  close(connfd);
  return 0;

print_usage:
  if (host) free(host);
  if (port) free(port);
  printf("Usage: %s [HOST:[PORT]]\n", argv[0]);
  return 1;
}