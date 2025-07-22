typedef struct __nmcrcon_state {
  char *host;
  char *port;
  char *password;
  float wait_sec;
  int verbose;
  int silent;
  int cmds_c;
  char **cmds_v;
} nmcrcon_state_t;

extern nmcrcon_state_t nmcrcon_state;

int conf_init_state(int argc, char *argv[]);
void conf_usage(char *argv0);