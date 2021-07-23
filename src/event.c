/**
 * @file event.c
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 */

#include "event.h"

struct hsevent* hsevent_init(int sockfd, int events, struct hsevent_base *event_base) {
  struct hsevent *event = (struct hsevent*)malloc(sizeof(struct hsevent));
  if (event == NULL) {
    return event;
  }
  event->inbound = hsbuffer_init(HS_BUFFER_SIZE);
  event->outbound = hsbuffer_init(HS_BUFFER_SIZE);
  if (!event->inbound || !event->outbound) {
    hsbuffer_free(event->inbound);
    hsbuffer_free(event->outbound);
    free(event);
    return NULL;
  }
  event->sockfd = sockfd;
  event->events = events;
  event->pipe_rfd = -1;
  event->closed = 0;
  event->remote = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
  event->timerfd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
  struct itimerspec timeout;
  timeout.it_value.tv_nsec = 0;
  timeout.it_value.tv_sec = HSINTERVAL;
  timeout.it_interval.tv_nsec = 0;
  timeout.it_interval.tv_sec = HSINTERVAL;
  timerfd_settime(event->timerfd, 0, &timeout, NULL);
  if (event->events) {
    event->event_base = event_base;
    if (event->event_base) {
      hsevent_base_update(EPOLL_CTL_ADD, event, event_base);
    }
  }

  return event;
}

void hsevent_free(struct hsevent *event) {
  hsbuffer_free(event->inbound);
  hsbuffer_free(event->outbound);
  free(event->remote);
  free(event);
}

void hsevent_update(struct hsevent *event, int events) {
  event->events = events;
  if (event->event_base) {
    hsevent_base_update(EPOLL_CTL_MOD, event, event->event_base);
  }
}

void hsevent_update_cb(struct hsevent *event, int event_type, hsevent_cb event_cb) {
  switch (event_type) {
    case HSEVENT_READ: {
      event->read_cb = event_cb;
      break;
    }
    case HSEVENT_WRITE: {
      event->write_cb = event_cb;
      break;
    }
    case HSEVENT_RDHUP: {
      event->rdhup_cb = event_cb;
      break;
    }
    case HSEVENT_ERR: {
      event->err_cb = event_cb;
      break;
    }
    default: {
      break;
    }
  }
}

struct hsevent_base* hsevent_base_init() {
  struct hsevent_base *base = (struct hsevent_base*)malloc(sizeof(struct hsevent_base));
  if (!base) {
    return base;
  }
  base->epollfd = epoll_create1(0);
  base->exit = 0;
  memset(base->activate_events, 0, MAXFD * sizeof(struct epoll_event));
  for (int i = 0; i < MAXFD; i++) {
    base->sockets[i] = NULL;
  }

  return base;
}

void hsevent_base_free(struct hsevent_base *base) {
  free(base);
}

void hsevent_base_update(int op, struct hsevent *event, struct hsevent_base *base) {
  struct epoll_event ev, timerev;
  ev.events = event->events;
  ev.data.fd = event->sockfd;
  timerev.events = EPOLLIN | EPOLLET;
  timerev.data.fd = event->timerfd;
  epoll_ctl(base->epollfd, op, event->sockfd, &ev);
  epoll_ctl(base->epollfd, op, event->timerfd, &timerev);
  if (op == EPOLL_CTL_ADD) {
    base->sockets[event->sockfd] = event;
    base->sockets[event->timerfd] = event;
  } else if (op == EPOLL_CTL_DEL) {
    base->sockets[event->sockfd] = NULL;
    base->sockets[event->timerfd] = NULL;
    base->sockets[event->pipe_rfd] = NULL;
  }
}

void hsevent_base_clear(struct hsevent_base *base) {
  for (int i = 0; i < MAXFD; i++) {
    if (base->sockets[i]) {
      struct hsevent *event = base->sockets[i];
      base->sockets[event->sockfd] = NULL;
      base->sockets[event->timerfd] = NULL;
      base->sockets[event->pipe_rfd] = NULL;
      hsevent_free(event);
    }
  }
}

void hsevent_base_loop(struct hsevent_base *base) {
  while (1) {
    /* For debug */
    if (base->exit) {
      return ;
    }

    int nready = epoll_wait(base->epollfd, base->activate_events, MAXFD, -1);
    for (int i = 0; i < nready; i++) {
      int sockfd = base->activate_events[i].data.fd;
      int events = base->activate_events[i].events;
      struct hsevent *activate_event = base->sockets[sockfd];
      if (events & EPOLLERR) {
        if (activate_event->err_cb) {
          activate_event->err_cb(activate_event);
        }
        continue;
      } else if(events & EPOLLRDHUP) {
        if (activate_event->rdhup_cb) {
          activate_event->rdhup_cb(activate_event);
        }
        continue;
      } else if(events & EPOLLIN) {
        if (activate_event->read_cb) {
          activate_event->read_cb(activate_event);
        }
      } else if(events & EPOLLOUT) {
        if (activate_event->write_cb) {
          activate_event->write_cb(activate_event);
        }
      }
    }
  }
}

void hsevent_base_exit(struct hsevent_base *base) {
  base->exit = 1;
}
