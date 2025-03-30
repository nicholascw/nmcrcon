#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

ssize_t send_all(int sockfd, void *buffer, size_t length) {
  size_t sent = 0;
  ssize_t ret;
  do {
    ret = send(sockfd, buffer + sent, length - sent, 0);
    if (ret > 0) {
      sent += ret;
    } else {
      if (errno == EINTR)
        continue;
      else {
        perror("send");
        return -1;
      }
    }
  } while (sent < length);
  return sent;
}

ssize_t recv_numbytes(int sockfd, void *buffer, size_t length) {
  size_t recved = 0;
  ssize_t ret;
  do {
    ret = recv(sockfd, buffer + recved, length - recved, 0);
    if (ret > 0) {
      recved += ret;
    } else if (ret == 0) {
      break;
    } else {
      if (errno == EINTR) {
        ret = 0;
        continue;
      } else {
        perror("recv");
        return -1;
      }
    }
  } while (recved < length);
  return recved;
}

/*

// the logic is wrong, int32s would definitely have 'nul nul' sequence as well

ssize_t recv_pkt(int sockfd, char *buf, size_t bufsize) {
  char *ptr = buf;
  ssize_t ret = 0;
  memset(buf, 0xff, bufsize);
  do {
    ptr += ret;
    if (ptr - buf > (ssize_t)bufsize) return -1;
    ret = recv(sockfd, ptr, 1, 0);
    if (ret < 0) {
      if (errno == EINTR) {
        ret = 0;
        continue;
      } else {
        perror("recv");
        return -1;
      }
    }
  } while (!memmem(buf, bufsize, "\0\0", 2));
  return (ptr - buf) + ret;
}

*/

static void *_socket_get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

static int _socket_set_nonblk(int fd) {
  return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int socket_tryconnect(char *hostname, char *port) {
  int connfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((connfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("socket");
      continue;
    }
    if (connect(connfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(connfd);
      perror("connect");
      continue;
    }
    char s[INET6_ADDRSTRLEN];
    inet_ntop(p->ai_family, _socket_get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    fprintf(stderr, "Connecting to %s (%s:%s)...\n", hostname, s, port);
    break;
  }

  freeaddrinfo(servinfo);
  if (!p) {
    fprintf(stderr, "Failed to connect.\n");
    return -1;
  }
  return connfd;
}
