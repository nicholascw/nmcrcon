#ifndef __TYP_CMDS_H__
#define __TYP_CMDS_H__

#define STD_CMDH_ARGS int rconfd, const char *input
int cmdh_exit(STD_CMDH_ARGS);
int cmdh_clear(STD_CMDH_ARGS);
int cmdh_auth(STD_CMDH_ARGS);

typedef struct {
  const char *cmd;
  int (*func)(STD_CMDH_ARGS);
} cmd_t;

static const cmd_t cmds[] = {
    (cmd_t){.cmd = "exit", .func = &cmdh_exit},
    (cmd_t){.cmd = "clear", .func = &cmdh_clear},
    (cmd_t){.cmd = "auth", .func = &cmdh_auth},
};

#define NUM_OF_CMD_HANDLERS (sizeof(cmds) / sizeof(cmd_t))
#endif
