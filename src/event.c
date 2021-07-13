/**
 * @file event.c
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 * 
 */

#include "event.h"

#include "sys/epoll.h"
#include "string.h"

#define MAXFD 1024

struct hsevent {
  int sockfd;                       // Associated socket
  struct hsbuffer *inbound;         // Input buffer, read data from socket
  struct hsbuffer *outbound;        // Output buffer, write data to socket
  uint32_t events;                  // Types of events monitored
  struct hsevent_base *event_base;  // Point to the hsevent_base that polls the hsevent
  hsevent_cb read_cb;               // Callback function for EPOLLIN
  hsevent_cb write_cb;              // Callback function for EPOLLOUT
  hsevent_cb hup_cb;                // Callback function for EPOLLHUP
  hsevent_cb err_cb;                // Callback function for EPOLLERR
};

struct hsevent_base {
  int epollfd;
  struct epoll_event activate_events[MAXFD];
  struct hsevent *sockets[MAXFD];
};

struct hsevent* hsevent_init(int sockfd, int events, struct hsevent_base *event_base) {
  struct hsevent *event = (struct hsevent*)malloc(sizeof(struct hsevent));
  if (event == NULL) {
    return event;
  }
  event->inbound = hsbuffer_init();
  event->outbound = hsbuffer_init();
  if (!event->inbound || !event->outbound) {
    free(event->inbound);
    free(event->outbound);
    free(event);
    return NULL;
  }
  event->sockfd = sockfd;
  event->events = events;
  if (event->events) {
    event->event_base = event_base;
  }

  return event;
}

void hsevent_free(struct hsevent *event) {
  free(event->inbound);
  free(event->outbound);
  free(event);
}

void hsevent_update(struct hsevent *event, int events) {
  event->events = events;
  if (event->event_base) {
    hsevent_base_update(HSEVENT_MOD, event, event->event_base);
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
    case HSEVENT_HUP: {
      event->hup_cb = event_cb;
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
  struct epoll_event ev;
  ev.events = event->events;
  epoll_ctl(base->epollfd, op, event->sockfd, &ev);
}

void hsevent_base_loop(struct hsevent_base *base) {
  while (1) {
    int nready = epoll_wait(base->epollfd, &base->activate_events, MAXFD, -1);
    for (int i = 0; i < nready; i++) {
      int sockfd = base->activate_events[i].data.fd;
      int events = base->activate_events[i].events;
      struct hsevent *activate_event = base->sockets[sockfd];
      if (events & EPOLLERR) {
        if (activate_event->err_cb) {
          activate_event->err_cb(sockfd, NULL);
        }
        return ;
      } else if(events & EPOLLHUP) {
        if (activate_event->hup_cb) {
          activate_event->hup_cb(sockfd, NULL);
        }
        return ;
      } else if(events & EPOLLIN) {
        if (activate_event->read_cb) {
          activate_event->read_cb(sockfd, activate_event->inbound);
        }
      } else if(events & EPOLLOUT) {
        if (activate_event->write_cb) {
          activate_event->write_cb(sockfd, activate_event->outbound);
        }
      }
    }
  }
}
