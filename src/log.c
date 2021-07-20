/**
 * @file log.c
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 */

#include "log.h"

#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>

int logfd = -1;

int hslog_init(const char *logfile) {
  logfd = open(logfile, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  if (logfd < 0) {
    fprintf(stderr, "Open %s failed: ", logfile);
    perror("");
  } else {
    printf("Open %s succeed\n", logfile);
  }

  return logfd;
}

void hslog_log(struct sockaddr_in *remote, Request *request, int status, int length) {
  if (logfd < 0) {
    return ;
  }

  char logbuf[1024];
  int used_length = 0;

  memset(logbuf, 0, 1024);
  
  /* Log client's IP address */
  inet_ntop(AF_INET, &remote->sin_addr, logbuf, INET_ADDRSTRLEN);
  used_length += (INET_ADDRSTRLEN - 1);

  assert(request);

  /* Log identd and uesrid */
  snprintf(logbuf + used_length, 5, " - -");
  used_length += 4;

  /* Log time */
  time_t t;
  struct tm *tmp;
  time(&t);
  tmp = localtime(&t);
  strftime(logbuf + used_length, 30, " [%d/%b/%G:%H:%M:%S %z]", tmp);
  used_length += 29;

  /* Log the request line */
  size_t total_line = 5 + strlen(request->http_method) + strlen(request->http_uri) + strlen(request->http_version); 
  snprintf(logbuf + used_length, total_line + 1,
                                 " \"%s %s %s\"",
                                 request->http_method,
                                 request->http_uri,
                                 request->http_version);
  used_length += total_line;

  /* Log status code and length of content */
  sprintf(logbuf + used_length, " %d %d\n", status, length);

  /* Write common log to logfd */
  used_length = strlen(logbuf);
  int bytes_written = write(logfd, logbuf, used_length);
  if (bytes_written < used_length) {
    printf("hslog_log(): write() failed\n"); 
  }
}
