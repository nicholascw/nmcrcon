#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "socket.h"

#define RCON_TYPE_RESP 0
#define RCON_TYPE_CMD 2
#define RCON_TYPE_AUTH_RESP 2
#define RCON_TYPE_AUTH 3
#define RCON_TYPE_EOF 0xdeadbeef

static uint32_t id = 114514;

ssize_t _rcon_pkt_send(int fd, uint32_t id, uint32_t type, char *payload) {
  size_t len = 4 + 4 + strlen(payload) + 1 + 1;
  char *buf = malloc(4 + len);
  if (!buf) {
    perror("malloc");
    return -1;
  }
  uint32_t *r_len = (uint32_t *)buf;
  uint32_t *r_id = (uint32_t *)(buf + 4);
  uint32_t *r_type = (uint32_t *)(buf + 8);
  *r_len = htole32(len);
  *r_id = htole32(id);
  *r_type = htole32(type);
  strcpy(buf + 12, payload);
  buf[len + 4 - 2] = '\0';
  buf[len + 4 - 1] = '\0';

  ssize_t ret = send_all(fd, buf, 4 + len);
  if (ret > 0 && ret != (ssize_t)(4 + len)) {
    ret = -1;
  }
  free(buf);
  return ret;
}

char *_rcon_pkt_recv(int fd, uint32_t *id) {
  uint32_t len;
  ssize_t ret = recv_numbytes(fd, &len, sizeof(len));
  if (ret != sizeof(len)) {
    if (ret <= 0) {
      perror("recv");
    } else {
      fprintf(stderr, "recv_numbytes: expected %zu bytes, got %zd\n", sizeof(len), ret);
    }
    return NULL;
  }
  len = le32toh(len);
  char *buf = malloc(len);
  if (!buf) {
    perror("malloc");
    return NULL;
  }
  ret = recv_numbytes(fd, buf, len);
  if (ret != len) {
    if (ret <= 0) {
      perror("recv");
    } else {
      fprintf(stderr, "recv_numbytes: expected %zu bytes, got %zd\n", sizeof(len), ret);
    }
    free(buf);
    return NULL;
  }
  *id = le32toh(*(uint32_t *)buf);
  uint32_t type = le32toh(*(uint32_t *)(buf + 4));
  if (type != RCON_TYPE_RESP && type != RCON_TYPE_AUTH_RESP) {
    fprintf(stderr, "_rcon_pkt_recv: unexpected packet type %u\n", type);
  }

  char *payload = strdup(buf + 8);
  free(buf);
  return payload;
}

char *_rcon_resp_recv(int fd, uint32_t cmd_id, uint32_t eof_id) {
  char *buf;
  char *ret = NULL;
  uint32_t id;
  do {
    buf = _rcon_pkt_recv(fd, &id);
    if (!buf) return NULL;
    if (id == cmd_id) {
      if (ret) {
        ret = realloc(ret, strlen(ret) + strlen(buf) + 1);
        if (!ret) {
          perror("realloc");
          free(buf);
          return NULL;
        }
        strcat(ret, buf);
      } else {
        ret = strdup(buf);
        if (!ret) {
          perror("strdup");
        }
      }
    } else if (id != eof_id) {
      if (id == 0xffffffff) {
        fprintf(stderr, "You are not authenticated.\n");
      } else {
        fprintf(stderr,
                "_rcon_resp_recv: "
                "unexpected packet id %u, expecting cmd:%u or eof: %u\n",
                id, cmd_id, eof_id);
      }
    }
    free(buf);
  } while (id != eof_id);
  return ret;
}

int rcon_exec(int fd, char *cmd) {
  uint32_t cmd_id = id++;
  uint32_t eof_id = id++;
  ssize_t ret = _rcon_pkt_send(fd, cmd_id, RCON_TYPE_CMD, cmd);
  if (ret < 0) return -1;
  id += 1;
  ret = _rcon_pkt_send(fd, eof_id, RCON_TYPE_EOF, "done?");
  if (ret < 0) return -1;
  char *resp = _rcon_resp_recv(fd, cmd_id, eof_id);
  if (!resp) return -1;
  printf("%s\n", resp);
  free(resp);
  return 0;
}

int rcon_auth(int fd, char *password) {
  uint32_t auth_id = id++;
  uint32_t auth_resp_id;
  ssize_t ret = _rcon_pkt_send(fd, auth_id, RCON_TYPE_AUTH, password);
  if (ret < 0) return -1;
  char *resp = _rcon_pkt_recv(fd, &auth_resp_id);
  if (resp) free(resp);
  if (!resp || auth_resp_id == 0xffffffff) {
    printf("Authentication failed.\n");
    return -1;
  } else
    printf("Authenticated.\n");
  return 0;
}
