#include "buffer.h"

#undef MAX_BUFFER_SIZE
#define MAX_BUFFER_SIZE 16

#include "assert.h"
#include "netinet/in.h"
#include "string.h"
#include "stdio.h"
#include "unistd.h"

#define ERROR_POS 3

int main() {
  struct hsbuffer* buffer = hsbuffer_init();

  /* Basic tests */
  assert(buffer);
  assert(hsbuffer_pos(buffer, READ_POS));
  assert(hsbuffer_pos(buffer, WRITE_POS));
  assert(!hsbuffer_pos(buffer, ERROR_POS));
  assert(hsbuffer_length(buffer) == 0);
  assert(hsbuffer_capacity(buffer) == 16);
  assert(hsbuffer_remain(buffer) == 16);
  assert(hsbuffer_readable(buffer) == 0);

  /* Create socket */
  int i = 1;
  int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEPORT, &i, 0);
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(struct sockaddr_in));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(9999);
  bind(server_sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
  listen(server_sockfd, 16);
  int conn_sockfd = accept(server_sockfd, NULL, NULL);

  /* Network IO tests */
  hsbuffer_recv(conn_sockfd, buffer, 100);
  assert(hsbuffer_length(buffer) == 8);
  assert(hsbuffer_remain(buffer) == 8);
  assert(hsbuffer_readable(buffer) == 8);
  assert(!strcmp("12345678", hsbuffer_pos(buffer, READ_POS)));

  hsbuffer_send(conn_sockfd, buffer, 100);
  assert(hsbuffer_length(buffer) == 0);
  assert(hsbuffer_remain(buffer) == 16);

  hsbuffer_recv(conn_sockfd, buffer, 100);
  assert(hsbuffer_length(buffer) == 16);
  assert(hsbuffer_remain(buffer) == 0);
  assert(!strcmp("1234567812345678", hsbuffer_pos(buffer, READ_POS)));

  hsbuffer_send(conn_sockfd, buffer, 100);
  assert(hsbuffer_length(buffer) == 0);
  assert(hsbuffer_remain(buffer) == 16);

  sleep(2);
  hsbuffer_expand(buffer, 4);
  assert(hsbuffer_capacity(buffer) == 16);
  hsbuffer_expand(buffer, 32);
  assert(hsbuffer_capacity(buffer) == 32);
  hsbuffer_recv(conn_sockfd, buffer, 100);
  assert(hsbuffer_length(buffer) == 24);
  assert(hsbuffer_remain(buffer) == 8);

  hsbuffer_send(conn_sockfd, buffer, 18);
  assert(hsbuffer_length(buffer) == 24);
  assert(hsbuffer_remain(buffer) == 8);
  assert(hsbuffer_readable(buffer) == 6);
  hsbuffer_send(conn_sockfd, buffer, 32);
  assert(hsbuffer_length(buffer) == 0);
  assert(hsbuffer_remain(buffer) == 32);
  assert(hsbuffer_readable(buffer) == 0);

  hsbuffer_free(buffer);
  return 0;
}
