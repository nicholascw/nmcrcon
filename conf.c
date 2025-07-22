#include "conf.h"

#include <argp.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dbg.h"
#include "shell.h"
#include "socket.h"

nmcrcon_state_t nmcrcon_state;

#ifdef __GNUC__
#define __CC__ "GCC " __VERSION__
#else
#define __CC__ __VERSION__
#endif

#ifdef NMCRCON_VERSION
const char *argp_program_version = NMCRCON_VERSION;
#else
const char *argp_program_version =
    "nmcrcon (custom build, " __DATE__ " " __TIME__ ", " __CC__ ")";
#endif

const char *argp_program_bug_address =
    "https://github.com/nicholascw/nmcrcon/issues/new";

static char doc[] =
    "Nicholas' Minecraft RCON Client / nmcrcon -- Yet another Minecraft RCON "
    "client that isn't against humanity and actually works.\v"
    "You may also specify server host, port, and password in environment "
    "variables (N)MCRCON_HOST, (N)MCRCON_PORT, (N)MCRCON_PASS. Please note the "
    "NMCRCON_ variables would have higher priorities than MCRCON_ ones.";

static char args_doc[] = "[HOST[:PORT]] [COMMAND]";

static struct argp_option options[] = {
    {"host", 'H', "HOST", 0, "Server host"},
    {"port", 'P', "PORT", 0, "Server port"},
    {"pass", 'p', "PASSWORD", 0, "RCON Password"},
    {"quiet", 'q', 0, 0, "Don't produce any output"},
    {"silent", 's', 0, OPTION_ALIAS},
    {"verbose", 'v', 0, 0, "Output more information"},
    {"wait", 'w', "SECONDS", 0, "Wait duration in between each command"},
    {0}};

struct arguments {
  char *hostport;
  char *host;
  char *port;
  char *password;
  float wait_sec;
  int verbose;
  int silent;
  int cmds_c;
  char **cmds_v;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key) {
    case 'H':
      arguments->host = arg;
      break;
    case 'P':
      arguments->port = arg;
      break;
    case 'p':
      arguments->password = arg;
      break;
    case 'q':
    case 's':
      arguments->silent++;
      break;
    case 'v':
      arguments->verbose++;
      break;
    case 'w':
      sscanf(arg, "%f", &arguments->wait_sec);
      break;
    case ARGP_KEY_ARG:
      arguments->hostport = arg;
      arguments->cmds_v = &state->argv[state->next];
      arguments->cmds_c = state->argc - state->next;
      state->next = state->argc;
      return 0;
    case ARGP_KEY_NO_ARGS:
    case ARGP_KEY_INIT:
    case ARGP_KEY_END:
    case ARGP_KEY_SUCCESS:
    case ARGP_KEY_FINI:
      return 0;
    default:
      argp_usage(state);
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int conf_init_state(int argc, char *argv[]) {
  struct arguments arguments;
  memset(&arguments, 0, sizeof(arguments));
  if (argc > 1) {
    argp_parse(&argp, argc, argv, 0, 0, &arguments);
  }

  if (arguments.host || arguments.port) {
    // host and/or port aleady explicitly specified, so hostport is actually
    // part of cmds
    if (arguments.hostport) {
      arguments.hostport = 0;
      arguments.cmds_v--;
      arguments.cmds_c++;
    }
  } else {
    if (arguments.hostport) {
      // fill hostport into host & port
      arguments.host = arguments.hostport;

      char *sp = strrchr(arguments.hostport, ':');
      char *test_ipv6 = strrchr(arguments.hostport, ']');
      if (sp && test_ipv6 < sp) {
        arguments.port = sp + 1;
        *sp = '\0';
      }
    }
  }
  // everything from cmdline should be filled in arguments at this point
  if (!arguments.host) {
    arguments.host = getenv("NMCRCON_HOST");
  }
  if (!arguments.port) {
    arguments.port = getenv("NMCRCON_PORT");
  }
  if (!arguments.password) {
    arguments.password = getenv("NMCRCON_PASS");
  }

  // provide compatibility to Tiiffi/mcrcon
  if (!arguments.host) {
    arguments.host = getenv("MCRCON_HOST");
  }
  if (!arguments.port) {
    arguments.port = getenv("MCRCON_PORT");
  }
  if (!arguments.password) {
    arguments.password = getenv("MCRCON_PASS");
  }

  // copy the values from arguments to nmcrcon_state,
  // which should be persistent throught the program's lifecycle

  nmcrcon_state.host = strdup(arguments.host ? arguments.host : "127.0.0.1");
  if (!nmcrcon_state.host) {
    perror("strdup(host)");
    exit(1);
  }
  nmcrcon_state.port = strdup(arguments.port ? arguments.port : "25575");
  if (!nmcrcon_state.port) {
    perror("strdup(port)");
    exit(1);
  }
  if (arguments.password) {
    nmcrcon_state.password = strdup(arguments.password);
    if (!nmcrcon_state.password) {
      perror("strdup(password)");
      exit(1);
    }
  } else {
    nmcrcon_state.password = NULL;
  }
  nmcrcon_state.wait_sec = arguments.wait_sec;
  nmcrcon_state.verbose = arguments.verbose;
  nmcrcon_state.silent = arguments.silent;
  nmcrcon_state.cmds_c = arguments.cmds_c;
  nmcrcon_state.cmds_v = calloc(nmcrcon_state.cmds_c, sizeof(char *));
  if (!nmcrcon_state.cmds_v) {
    perror("calloc(cmds_v)");
    exit(1);
  }
  for (int i = 0; i < nmcrcon_state.cmds_c; i++) {
    nmcrcon_state.cmds_v[i] = strdup(arguments.cmds_v[i]);
    if (!nmcrcon_state.cmds_v[i]) {
      perror("strdup(cmds_v)");
      exit(1);
    }
  }

  return 0;
}

void conf_usage(char *argv0) {
  argp_help(&argp, stderr, ARGP_HELP_STD_USAGE, argv0);
}