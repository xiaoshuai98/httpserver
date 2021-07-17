/**
 * @file utils.c
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 */

#include "event_handler.h"
#include "utils.h"
#include "parse.h"

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

void accept_conn(struct hsevent *event) {
  int conn_sockfd;
  while (1) {
    conn_sockfd = accept(event->sockfd, 0, 0);
    if (conn_sockfd < 0) {
      if (errno == EAGAIN) {
        break;
      } else {
        // TODO(qds): log.
        perror("accept() failed:");
      }
    } else {
      set_nonblocking(conn_sockfd);
      struct hsevent *new_event = hsevent_init(conn_sockfd, EPOLLIN | EPOLLET, event->event_base);
      hsevent_update_cb(new_event, EPOLLRDHUP, close_conn);
      hsevent_update_cb(new_event, EPOLLIN, read_conn);
      hsevent_update_cb(new_event, HSEVENT_WRITE, write_conn);
    }
  }
}

void close_conn(struct hsevent *event) {
  hsevent_base_update(EPOLL_CTL_DEL, event, event->event_base);
  close(event->sockfd);
  hsevent_free(event);
}

void read_conn(struct hsevent *event) {
  while (1) {
    ssize_t bytes_read = hsbuffer_recv(event->sockfd, event->inbound, hsbuffer_remain(event->inbound));
    if (bytes_read < 0) {
      if (errno == EAGAIN) {
        break;
      } else {
        // TODO(qds): log.
        perror("recv() failed:");
      }
    }
  }
  hsevent_update(event, event->events | EPOLLOUT);
}
