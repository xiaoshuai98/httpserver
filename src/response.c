/**
 * @file response.c
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 */

#include "response.h"
#include "buffer.h"
#include "log.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

char file_path[256];
int initial_length;

int cur_entity_length = 0;
int fetched_entity_length = 0;

const char *ok = "HTTP/1.1 200 OK\r\n";
const char *bad_request = "HTTP/1.1 400 Bad Request\r\n";
const char *not_exist = "HTTP/1.1 404 Not Found\r\n";
const char *request_timeout = "HTTP/1.1 408 Request Timeout\r\n";
const char *not_implemented = "HTTP/1.1 501 Not Implemented\r\n";
const char *bad_version = "HTTP/1.1 505 HTTP Version Not Supported\r\n";

const char *server = "Server: Knight/1.0\r\n";
const char *conn_close = "Connection: Close\r\n";
const char *conn_keep = "Connection: Keep-Alive\r\n";

const char *file_type[6] = {
  "html",
  "css",
  "txt",
  "png",
  "jpeg",
  "gif"
};

const char *mime_type[6] = {
  "text/html",
  "text/css",
  "text/plain",
  "image/png",
  "image/jpeg",
  "image/gif"
};

void response_ending(struct hsevent *event) {
  hsbuffer_ncpy(event->outbound, "\r\n", 2);
}

int find_type(Request *request) {
  int index = strlen(request->http_uri);
  int i;
  while (index >= 0) {
    if (request->http_uri[index] == '.') {
      break;
    }
    index--;
  }
  for (i = 0; i < 5; i++) {
    if (!strcmp(request->http_uri + index + 1, file_type[i])) {
      break;
    }
  }

  return i;
}

void find_file(struct hsevent *event, Request *request, int *fd, size_t *length) {
  strcat(file_path, request->http_uri);
  *fd = open(file_path, O_RDONLY);
  file_path[initial_length] = '\0'; // remove the uri
  int status;
  if (*fd < 0) {
    if (errno == EACCES) {
      hsbuffer_ncpy(event->outbound, bad_request, strlen(bad_request));
      status = 400;
    } else {
      hsbuffer_ncpy(event->outbound, not_exist, strlen(not_exist));
      status = 404;
    }
    hsbuffer_ncpy(event->outbound, "Content-length: 0\r\n", 19);
    *length = 0;
    hslog_log(event->remote, request, status, (int)*length);
  } else {
    hsbuffer_ncpy(event->outbound, ok, strlen(ok));
    char buf[128];

    int index = find_type(request); 
    snprintf(buf, 128, "Content-type: %s\r\n", mime_type[index]);
    hsbuffer_ncpy(event->outbound, buf, strlen(buf));
  }
}

void response_timeout(struct hsevent *event) {
  hsbuffer_ncpy(event->outbound, request_timeout, strlen(request_timeout));
  hsbuffer_ncpy(event->outbound, server, strlen(server));
  hsbuffer_ncpy(event->outbound, "Content-length: 0\r\n", 19);
  response_ending(event);
  hslog_log(event->remote, NULL, 408, 0);
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

/**
 * @note
 * If the HTTP request has the wrong version, 
 * then this function will directly generate a response, 
 * and other functions can return directly and let write_conn() send the response.
 * 
 * @return 1 means wrong HTTP version, 0 means normal.
 */
int response_badversion(struct hsevent *event, Request *request) {
  if (!strcmp("HTTP/1.1", request->http_version)) {
    return 0;
  } else {
    hsbuffer_ncpy(event->outbound, bad_version, strlen(bad_version));
    response_server_conn(event, request);
    hsbuffer_ncpy(event->outbound, "Content-length: 0\r\n", 19);
    response_ending(event);
    hslog_log(event->remote, request, 505, 0);
  }
  return 1;
}

void response_head(struct hsevent *event, Request *request, int *fd, size_t *length) {
  find_file(event, request, fd, length);
  if (*fd > 0) {
    hsbuffer_ncpy(event->outbound, "Content-length: 0\r\n", 19);
    hslog_log(event->remote, request, 200, 0);
    *fd = -1; // Don't let write_conn() send file.
  }
  response_server_conn(event, request);
  response_ending(event);
}

void response_get(struct hsevent *event, Request *request, int *fd, size_t *length) {
  find_file(event, request, fd, length);
  if (*fd > 0) {
    char buf[128];
    struct stat file_stat;
    fstat(*fd, &file_stat);
    *length = file_stat.st_size;
    snprintf(buf, 128, "Content-length: %ld\r\n", *length);
    hsbuffer_ncpy(event->outbound, buf, strlen(buf));
    hslog_log(event->remote, request, 200, (int)*length);
  }
  response_server_conn(event, request);
  response_ending(event);
}

void response_post(struct hsevent *event, Request *request) {
  hsbuffer_ncpy(event->outbound, not_implemented, strlen(not_implemented));
  response_server_conn(event, request);
  hsbuffer_ncpy(event->outbound, "Content-length: 0\r\n", 19);
  response_ending(event);
}

void response_other_method(struct hsevent *event, Request *request) {
  hsbuffer_ncpy(event->outbound, not_implemented, strlen(not_implemented));
  response_server_conn(event, request);
  hsbuffer_ncpy(event->outbound, "Content-length: 0\r\n", 19);
  response_ending(event);
  hslog_log(event->remote, request, 501, 0);
}

void response_method(struct hsevent *event, Request *request, int *fd, size_t *length) {
  if (!strcmp(request->http_method, "GET")) {
    response_get(event, request, fd, length);
  } else if (!strcmp(request->http_method, "HEAD")) {
    response_head(event, request, fd, length);
  } else if (!strcmp(request->http_method, "POST")) {
    response_post(event, request);
  } else {
    response_other_method(event, request);
  }
}

void response_invalid(struct hsevent *event, Request *request) {
  hsbuffer_ncpy(event->outbound, bad_request, strlen(bad_request));
  response_server_conn(event, request);
  hsbuffer_ncpy(event->outbound, "Content-length: 0\r\n", 19);
  response_ending(event);
  hslog_log(event->remote, request, 400, 0);
}

/**
 * @return 1 means a entity of the required length has been detected, and 0 means not yet.
 */
int fetch_entitybody(struct hsevent *event, Request *request) {
  if (request) {
    for (int i = 0; i < request->header_count; i++) {
      if (!strcmp(request->headers[i].header_name, "Content-length")) {
        cur_entity_length = atoi(request->headers[i].header_value);
        break;
      }
    }
  }
  int readable = (int)hsbuffer_readable(event->inbound);
  int need = cur_entity_length - fetched_entity_length;
  fetched_entity_length += (readable > need ? need : readable);
  if (fetched_entity_length == cur_entity_length) {
    return 1;
  } else {
    return 0;
  }
}

int create_response(struct hsevent *event, int *fd, size_t *file_length) {
  Request *request;
  int size = (int)hsbuffer_readable(event->inbound);
  int result = parse(hsbuffer_pos(event->inbound, READ_POS), &size, &request);
  hsbuffer_consume(event->inbound, (size_t)size);
  if (result == HSPARSE_VALID) {
    if (!response_badversion(event, request)) {
      if (fetch_entitybody(event, request)) {
        hsbuffer_consume(event->inbound, cur_entity_length);
        cur_entity_length = 0;
        fetched_entity_length = 0;
        response_method(event, request, fd, file_length);
      }
    }
    parse_free(request);
  } else if (result == HSPARSE_INVALID) {
    response_invalid(event, request);
  } else {
    if (fetch_entitybody(event, NULL)) {
      cur_entity_length = 0;
      fetched_entity_length = 0;
      hsbuffer_consume(event->inbound, cur_entity_length);
      response_method(event, request, fd, file_length);
    }
  }

  return result;
}
