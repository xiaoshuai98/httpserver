/**
 * @file response.c
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 */

#include "utils.h"
#include "response.h"
#include "buffer.h"
#include "log.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

char file_path[256];
int initial_length;

char cgi_folder[128];
int cgifolder_length;

int cur_entity_length = 0;
int fetched_entity_length = 0;

const char *ok = "HTTP/1.1 200 OK\r\n";
const char *bad_request = "HTTP/1.1 400 Bad Request\r\n";
const char *not_exist = "HTTP/1.1 404 Not Found\r\n";
const char *request_timeout = "HTTP/1.1 408 Request Timeout\r\n";
const char *server_error = "HTTP/1.1 500 Internal Server Error\r\n";
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

static void response_ending(struct hsevent *event) {
  hsbuffer_ncpy(event->outbound, "\r\n", 2);
}

static int find_type(Request *request) {
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

static void find_file(struct hsevent *event, Request *request, int *fd, size_t *length) {
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

static void response_server_conn(struct hsevent *event, Request *request) {
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
static int response_badversion(struct hsevent *event, Request *request) {
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

static void response_server_error(struct hsevent *event, Request *request) {
  hsbuffer_ncpy(event->outbound, server_error, strlen(server_error));
  response_server_conn(event, request);
  hsbuffer_ncpy(event->outbound, "Content-length: 0\r\n", 19);
  response_ending(event);
}

static int get_content_length(Request *request) {
  for (int i = 0; i < request->header_count; i++) {
    if (!strcmp(request->headers[i].header_name, "Content-length")) {
      return atoi(request->headers[i].header_value);
    }
  }
  return 0;
}

static const char* get_content_type(Request *request) {
  for (int i = 0; i < request->header_count; i++) {
    if (!strcmp(request->headers[i].header_name, "Content-type")) {
      return request->headers[i].header_value;
    }
  }
  return NULL;
}

static const char* get_query_string(Request *request) {
  for (size_t i = 0; i < strlen(request->http_uri); i++) {
    if (request->http_uri[i] == '?') {
      return request->http_uri + i + 1;
    }
  }
  return NULL;
}

/**
 * @return 1 means the response is generated by the cgi script, 
 * and 0 means the response is generated by the server.
 */
static int response_cgi(struct hsevent *event, Request *request) {
  if (strncmp(request->http_uri, "/cgi/", 5)) {
    return 0;
  }

  pid_t pid;
  int stdin_pipe[2];
  int stdout_pipe[2];
  if (pipe(stdin_pipe) || pipe(stdout_pipe)) {
    response_server_error(event, request);
    return 1;
  }
  strcat(cgi_folder, request->http_uri);
  char environment[32][128];
  char *env[32];
  char *args[2];
  args[0] = cgi_folder;
  args[1] = NULL;
  int num_of_env = 11;
  char remote_addr[17];
  memset(remote_addr, 0, 17);
  inet_ntop(AF_INET, &event->remote->sin_addr, remote_addr, INET_ADDRSTRLEN);
  sprintf(environment[0],  "CONTENT_LENGTH=%d", get_content_length(request));
  sprintf(environment[1],  "CONTENT_TYPE=%s", get_content_type(request));
  sprintf(environment[2],  "GATEWAY_INTERFACE=%s", "CGI/1.1");
  sprintf(environment[3],  "QUERY_STRING=%s", get_query_string(request));
  sprintf(environment[4],  "REMOTE_ADDR=%s", remote_addr);
  sprintf(environment[5],  "REQUEST_METHOD=%s", request->http_method);
  sprintf(environment[6],  "REQUEST_URI=%s", request->http_uri);
  sprintf(environment[7],  "SCRIPT_NAME=%s", request->http_uri + 5);
  sprintf(environment[8],  "SERVER_PORT=%d", https_port);
  sprintf(environment[9],  "SERVER_PROTOCOL=%s", "HTTP/1.1");
  sprintf(environment[10], "SERVER_SOFTNAME=%s", "Knight/1.0");
  for (int i = 0; i < request->header_count; i++) {
    if (!strcmp(request->headers[i].header_name, "Content-length") || 
        !strcmp(request->headers[i].header_name, "Content-type")) {
      continue;    
    }
    convertstr(request->headers[i].header_name);
    sprintf(environment[num_of_env], "HTTP_%s=%s", request->headers[i].header_name,
                                                   request->headers[i].header_value);
    num_of_env++;
  }
  for (int i = 0; i < num_of_env; i++) {
    env[i] = environment[i];
  }
  env[num_of_env] = NULL;
  

  pid = fork();
  if (pid < 0) {

  }
  if (pid == 0) {
    close(stdin_pipe[1]);
    close(stdout_pipe[0]);
    dup2(stdin_pipe[0], fileno(stdin));
    dup2(stdout_pipe[1], fileno(stdout));
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);
    if (execve(cgi_folder, args, env) < 0) {
      perror("execve()");
      response_server_error(event, request);
      return 1;
    }
  } 
  if (pid > 0) {
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);
    event->pipe_rfd = stdout_pipe[0]; // parent process reads from the stout of child process
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    ev.data.fd = event->pipe_rfd;
    event->event_base->sockets[stdout_pipe[0]] = event;
    epoll_ctl(event->event_base->epollfd, EPOLL_CTL_ADD, stdout_pipe[0], &ev);
    set_nonblocking(stdout_pipe[0]);
    ssize_t bytes_written;
    bytes_written = write(stdin_pipe[1], hsbuffer_pos(event->inbound, READ_POS), hsbuffer_readable(event->inbound));
    if (bytes_written < 0) {
      response_server_error(event, request);
      return 1;
    } else {
      hsbuffer_consume(event->inbound, bytes_written);
    }
    close(stdin_pipe[1]);
  }
  return 1;
}

static void response_head(struct hsevent *event, Request *request, int *fd, size_t *length) {
  if (response_cgi(event, request)) {
    return ;
  }
  find_file(event, request, fd, length);
  if (*fd > 0) {
    hsbuffer_ncpy(event->outbound, "Content-length: 0\r\n", 19);
    hslog_log(event->remote, request, 200, 0);
    *fd = -1; // Don't let write_conn() send file.
  }
  response_server_conn(event, request);
  response_ending(event);
}

static void response_get(struct hsevent *event, Request *request, int *fd, size_t *length) {
  if (response_cgi(event, request)) {
    return ;
  }
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

static void response_post(struct hsevent *event, Request *request) {
  if (response_cgi(event, request)) {
    return ;
  }
  hsbuffer_ncpy(event->outbound, not_implemented, strlen(not_implemented));
  response_server_conn(event, request);
  hsbuffer_ncpy(event->outbound, "Content-length: 0\r\n", 19);
  response_ending(event);
}

static void response_other_method(struct hsevent *event, Request *request) {
  hsbuffer_ncpy(event->outbound, not_implemented, strlen(not_implemented));
  response_server_conn(event, request);
  hsbuffer_ncpy(event->outbound, "Content-length: 0\r\n", 19);
  response_ending(event);
  hslog_log(event->remote, request, 501, 0);
}

static void response_method(struct hsevent *event, Request *request, int *fd, size_t *length) {
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

static void response_invalid(struct hsevent *event, Request *request) {
  hsbuffer_ncpy(event->outbound, bad_request, strlen(bad_request));
  response_server_conn(event, request);
  hsbuffer_ncpy(event->outbound, "Content-length: 0\r\n", 19);
  response_ending(event);
  hslog_log(event->remote, request, 400, 0);
}

/**
 * @return 1 means a entity of the required length has been detected, and 0 means not yet.
 */
static int fetch_entitybody(struct hsevent *event, Request *request) {
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
        response_method(event, request, fd, file_length);
      }
    }
    parse_free(request);
  } else if (result == HSPARSE_INVALID) {
    response_invalid(event, request);
  } else {
    
  }

  return result;
}
