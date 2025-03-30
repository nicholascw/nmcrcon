#include <sys/types.h>

int socket_tryconnect(char *hostname, char *port);
ssize_t send_all(int sockfd, void *buffer, size_t length);
ssize_t recv_numbytes(int sockfd, void *buffer, size_t length);
/*ssize_t recv_pkt(int sockfd, char *buf, size_t bufsize);*/
