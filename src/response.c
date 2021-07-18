/**
 * @file response.c
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 */

#include "response.h"
#include "buffer.h"

const char *ok = "HTTP/1.1 200 OK\r\n";
const char *bad_request = "HTTP/1.1 400 Bad Request\r\n";
const char *not_implemented = "HTTP/1.1 501 Not Implemented\r\n";

const char *server = "Server: Knight/1.0\r\n";
const char *conn_close = "Connection: Close\r\n";
const char *conn_keep = "Connection: Keep-Alive\r\n";

void response_ending(struct hsevent *event) {
  hsbuffer_ncpy(event->outbound, "\r\n", 2);
}

void response_server_conn(struct hsevent *event, Request *request) {
  hsbuffer_ncpy(event->outbound, server, strlen(server));
  Request_header *header = find_key(request, "Connection");
  if (!header || !strcmp(header->header_value, "Close")) {
    hsbuffer_ncpy(event->outbound, conn_close, strlen(conn_close));
    event->closed = 1;
  } else if (!strcmp(header->header_value, "Keep-Alive")) {
    hsbuffer_ncpy(event->outbound, conn_keep, strlen(conn_keep));
  }
}

void response_head(struct hsevent *event, Request *request) {
  hsbuffer_ncpy(event->outbound, ok, strlen(ok));
  response_server_conn(event, request);
  hsbuffer_ncpy(event->outbound, "Content-Length: 0\r\n", 19);
  response_ending(event);
}

void response_get(struct hsevent *event, Request *request) {
  hsbuffer_ncpy(event->outbound, ok, strlen(ok));
  response_server_conn(event, request);
  hsbuffer_ncpy(event->outbound, "Content-Length: 0\r\n", 19);
  response_ending(event);
}

void response_post(struct hsevent *event, Request *request) {
  hsbuffer_ncpy(event->outbound, not_implemented, strlen(not_implemented));
  response_server_conn(event, request);
  hsbuffer_ncpy(event->outbound, "Content-Length: 0\r\n", 19);
  response_ending(event);
}

void response_other_method(struct hsevent *event, Request *request) {
  hsbuffer_ncpy(event->outbound, not_implemented, strlen(not_implemented));
  response_server_conn(event, request);
}

void response_method(struct hsevent *event, Request *request) {
  if (!strcmp(request->http_method, "GET")) {
    response_get(event, request);
  } else if (!strcmp(request->http_method, "HEAD")) {
    response_head(event, request);
  } else if (!strcmp(request->http_method, "POST")) {
    response_post(event, request);
  } else {
    response_other_method(event, request);
  }
}

void response_invalid(struct hsevent *event, Request *request) {
  hsbuffer_ncpy(event->outbound, bad_request, strlen(bad_request));
  response_server_conn(event, request);
}

int create_response(struct hsevent *event, Request **request) {
  int size = (int)hsbuffer_readable(event->inbound);
  int result = parse(hsbuffer_pos(event->inbound, READ_POS), &size, request);
  hsbuffer_consume(event->inbound, (size_t)size);
  if (result == HSPARSE_VALID) {
    response_method(event, *request);
  } else if (result == HSPARSE_INVALID) {
    response_invalid(event, *request);
  } else {
    if (hsbuffer_remain(event->inbound) < 256) {
      hsbuffer_expand(event->inbound, hsbuffer_capacity(event->inbound) * 2);
    }
  }

  return result;
}
