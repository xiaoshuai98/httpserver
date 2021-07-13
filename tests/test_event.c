#include "event.h"

#include "sys/socket.h"
#include "netinet/in.h"
#include "sys/epoll.h"
#include "string.h"
#include "stdio.h"
#include "assert.h"
// TODO(qds): Handling connection closed event。
void echo(struct hsevent *event) {
  hsbuffer_recv(event->sockfd, event->inbound, HS_BUFFER_SIZE);
  assert(hsbuffer_length(event->inbound) != 0);
  hsbuffer_send(event->sockfd, event->inbound, HS_BUFFER_SIZE);
  assert(hsbuffer_length(event->inbound) == 0);
}

void accept_conn(struct hsevent *event) {
  printf("New connection arrived.\n");
  int conn_sockfd = accept(event->sockfd, NULL, NULL);

  struct hsevent *new_event = hsevent_init(conn_sockfd, EPOLLIN, event->event_base);
  hsevent_update_cb(new_event, HSEVENT_READ, echo);
}

int main() {
  struct hsevent_base *base = hsevent_base_init();

  /* Create socket */
  int i = 1;
  int serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(serv_sockfd, SOL_SOCKET, SO_REUSEPORT, &i, 0);
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(struct sockaddr_in));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(9999);
  bind(serv_sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
  listen(serv_sockfd, 16);

  /* Start Event Loop */
  struct hsevent *listen_event = hsevent_init(serv_sockfd, EPOLLIN, base);
  hsevent_update_cb(listen_event, HSEVENT_READ, accept_conn);
  hsevent_base_loop(base);

  return 0;
}
