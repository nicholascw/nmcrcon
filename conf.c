#include <argp.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __GNUC__
#define __CC__ "GCC "__VERSION__
#else
#define __CC__ __VERSION__
#endif

#ifdef NMCRCON_VERSION
const char *argp_program_version = NMCRCON_VERSION;
#else
const char *argp_program_version = "nmcrcon (custom build, "__DATE__" "__TIME__", "__CC__")";
#endif

const char *argp_program_bug_address =
    "https://github.com/nicholascw/nmcrcon/issues/new";

static char doc[] =
    "Nicholas' Minecraft RCON Client / nmcrcon -- Yet another Minecraft RCON "
    "client that isn't against humanity and actually works.\v"
    "You may also specify server host, port, and password in environment "
    "variables MCRCON_HOST, MCRCON_PORT, MCRCON_PASS.";

static char args_doc[] = "[HOST[:PORT]] [COMMAND]";

/* Keys for options without short-options. */
#define OPT_ABORT 1 /* –abort */

/* The options we understand. */
static struct argp_option options[] = {
    {"host", 'H', "HOST", 0, "server host"},
    {"port", 'P', "PORT", 0, "server port"},
    {"pass", 'p', "PASSWORD", 0, "password"},
    {"silent", 's', 0, 0, "Silent mode"},
    {"wait", 'w', "SECONDS", 0, "Wait duration in between each command"},
    {0}};

/* Used by main to communicate with parse_opt. */
struct arguments {
  char *hostport;                 /* arg1 */
  char **cmds;             /* [string…] */
  char *host;
  char *port;
  char *password;
  float wait_sec;
};

/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key) {
    case 'q':
    case 's':
      break;
    case 'v':
      break;
    case 'o':
      break;
    case 'r':
      break;
    case ARGP_KEY_NO_ARGS:
      return 0;
    case ARGP_KEY_ARG:
      /* Here we know that state->arg_num == 0, since we
         force argument parsing to end before any more arguments can
         get here. */
      arguments->hostport = arg;

      /* Now we consume all the rest of the arguments.
         state->next is the index in state->argv of the
         next argument to be parsed, which is the first string
         we’re interested in, so we can just use
         &state->argv[state->next] as the value for
         arguments->strings.

         In addition, by setting state->next to the end
         of the arguments, we can force argp to stop parsing here and
         return. */
      arguments->cmds = &state->argv[state->next];
      state->next = state->argc;

      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

/* Our argp parser. */
static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char **argv) {
  int i, j;
  struct arguments arguments;

  /* Default values. */

  /* Parse our arguments; every option seen by parse_opt will be
     reflected in arguments. */
  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  exit(0);
}
