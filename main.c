
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shell.h"
#include "socket.h"

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
  printf(
      "\n    Nicholas' Minecraft RCON Client (nmcrcon): yet another Minecraft "
      "RCON client that isn't against humanity and actually works.\n"
      "    Copyright (C) 2025 Nicholas Wang <me@nicho1as.wang>\n"
      "\n"
      "    This program is free software: you can redistribute it and/or "
      "modify\n"
      "    it under the terms of the GNU General Public License as published "
      "by\n"
      "    the Free Software Foundation, either version 3 of the License, or\n"
      "    (at your option) any later version.\n"
      "\n"
      "    This program is distributed in the hope that it will be useful,\n"
      "    but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
      "    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
      "    GNU General Public License for more details.\n"
      "\n"
      "    You should have received a copy of the GNU General Public License\n"
      "    along with this program.  If not, see "
      "<https://www.gnu.org/licenses/>.\n\n");
  printf("Usage: %s [HOST:[PORT]]\n", argv[0]);
  return 1;
}