#include <argp.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "shell.h"
#include "socket.h"

const char *argp_program_ver = "v0.1.0";
const char *argp_program_bug_address = "<me@nicho1as.wang> <jasinc90@gmail.com>";
static char doc[] =
    "\n    Nicholas' Minecraft RCON Client (nmcrcon)\n"
    "    Copyright (C) 2025 Nicholas Wang <me@nicho1as.wang>\n"
    "\n"
    "    This program is free software: you can redistribute it and/or "
    "modify\n"
    "    it under the terms of the GNU General Public License as published "
    "by\n"
    "    the Free Software Foundation, either version 3 of the License.\n"
    "\n"
    "    This program is distributed in the hope that it will be useful,\n"
    "    but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "    GNU General Public License for more details.\n"
    "\n"
    "    You should have received a copy of the GNU General Public License\n"
    "    along with this program.  If not, see "
    "<https://www.gnu.org/licenses/>.\n\n";
static char args_doc[] = "[host:[port]]";
static struct argp_option options[] = {
  {"verbose", 'v', 0, OPTION_ARG_OPTIONAL, "Show verbose messages"},
                                       {0}
};
struct arguments {
  int verbose;
  char *host;
  char *port;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
    case 'v':
      arguments->verbose = 1;
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num == 0) {
        char *sp = NULL;
        if (!arg){
          arguments->host = "127.0.0.1";
          arguments->port = "25565";
        }else if (!(sp = strchr(arguments->host, ':'))){
          arguments->host = arg;
          arguments->port = "25565";
        }else{
          arguments->host = strndup(arg, sp-arg);
          arguments->port = strdup(sp+1);
        }
      } else {
        argp_usage(state);
      }
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char *argv[]) {
  struct arguments arguments;

  /* Default values. */
  arguments.verbose = 0;
  arguments.host = "127.0.0.1";
  arguments.port = "25565";


  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  if (arguments.verbose){
    printf("Connect to %s:%s\n", arguments.host, arguments.port);
  }
  int connect_fd = socket_tryconnect(arguments.host, arguments.port);
  if (connect_fd < 0){
    fprintf(stderr, "Failed to connect to %s:%s", arguments.host, arguments.port);
  }
  shell_loop(connect_fd);
  close(connect_fd);

  exit(0);
}
