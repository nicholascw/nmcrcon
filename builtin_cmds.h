#ifndef __TYP_CMDS_H__
#define __TYP_CMDS_H__

struct shell_state;

#define STD_CMDH_ARGS struct shell_state *state, const char *input
int cmdh_exit(STD_CMDH_ARGS);
int cmdh_clear(STD_CMDH_ARGS);
int cmdh_auth(STD_CMDH_ARGS);
int cmdh_sleep(STD_CMDH_ARGS);
int cmdh_disconnect(STD_CMDH_ARGS);
int cmdh_connect(STD_CMDH_ARGS);
int cmdh_source(STD_CMDH_ARGS);

typedef struct {
  const char *cmd;
  int (*func)(STD_CMDH_ARGS);
} cmd_t;

static const cmd_t cmds[] = {
    (cmd_t){.cmd = "exit", .func = &cmdh_exit},
    (cmd_t){.cmd = "clear", .func = &cmdh_clear},
    (cmd_t){.cmd = "auth", .func = &cmdh_auth},
    (cmd_t){.cmd = "sleep", .func = &cmdh_sleep},
    (cmd_t){.cmd = "disconnect", .func = &cmdh_disconnect},
    (cmd_t){.cmd = "connect", .func = &cmdh_connect},
    (cmd_t){.cmd = "source", .func = &cmdh_source},
};

#define NUM_OF_CMD_HANDLERS (sizeof(cmds) / sizeof(cmd_t))
#endif
