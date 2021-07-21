/**
 * @file utils.c
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 */

#include "event_handler.h"
#include "utils.h"
#include "parse.h"
#include "response.h"

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/sendfile.h>

void outbound_send(struct hsevent *event) {
  ssize_t total_written = hsbuffer_readable(event->outbound);
  while (total_written > 0) {
    ssize_t bytes_written = hsbuffer_send(event->sockfd, event->outbound, total_written);
    if (bytes_written < 0) {
      perror("send() failed");
      break;
    }
    total_written -= bytes_written;
  }
}

void close_event(struct hsevent *event) {
  hsevent_base_update(EPOLL_CTL_DEL, event, event->event_base);
  close(event->sockfd);
  close(event->timerfd);
  hsevent_free(event);
}

void accept_conn(struct hsevent *event) {
  int conn_sockfd;
  while (1) {
    socklen_t socklen = sizeof(struct sockaddr_in);
    conn_sockfd = accept(event->sockfd, (struct sockaddr*)event->remote, &socklen);
    if (conn_sockfd < 0) {
      if (errno == EAGAIN) {
        break;
      } else {
        perror("accept() failed:");
      }
    } else {
      set_nonblocking(conn_sockfd);
      struct hsevent *new_event = hsevent_init(conn_sockfd, EPOLLIN | EPOLLET | EPOLLRDHUP, event->event_base);
      hsevent_update_cb(new_event, HSEVENT_RDHUP, rdhup_conn);
      hsevent_update_cb(new_event, HSEVENT_READ, read_conn);
      hsevent_update_cb(new_event, HSEVENT_WRITE, write_conn);
    }
  }
}

void rdhup_conn(struct hsevent *event) {
  close_event(event);
}

void read_conn(struct hsevent *event) {
  uint64_t timerfd_buf;
  if (read(event->timerfd, &timerfd_buf, sizeof(uint64_t)) < 0) {
    if (errno == EAGAIN) {
      struct itimerspec timeout;
      timeout.it_value.tv_nsec = 0;
      timeout.it_value.tv_sec = HSINTERVAL;
      timeout.it_interval.tv_nsec = 0;
      timeout.it_interval.tv_sec = HSINTERVAL;
      timerfd_settime(event->timerfd, 0, &timeout, NULL);
    } else {
      perror("timerfd");
    }
  } else {
    response_timeout(event);
    outbound_send(event);
    close_event(event);
    return;
  }

  while (1) {
    ssize_t bytes_read = hsbuffer_recv(event->sockfd, event->inbound, hsbuffer_remain(event->inbound));
    if (bytes_read < 0) {
      if (errno == EAGAIN) {
        break;
      } else {
        perror("recv() failed");
      }
    }
  }
  hsevent_update(event, event->events | EPOLLOUT);
}

void write_conn(struct hsevent *event) {
  while (hsbuffer_readable(event->inbound)) {
    int fd;
    size_t file_length;
    int result = create_response(event, &fd, &file_length);
    if (result == HSPARSE_INCOMPLETE) {
      break;
    }
    outbound_send(event);
    if (fd > 0) {
      sendfile(event->sockfd, fd, 0, file_length);
      close(fd);
    }
  }

  if (event->closed) {
    close_event(event);
  }
}
