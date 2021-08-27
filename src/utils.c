/**
 * @file utils.c
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 */

#include "utils.h"
#include "log.h"
#include "response.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

int http_port = -1;
int https_port = -1;

static void print_help() {
  printf("Usage: ./server [options]\n");
  printf("Option:\n");
  printf("  --help  %s\n", "Display this information.");
  printf("  --http  %s\n", "The port for the HTTP (or echo) server to listen on.");
  printf("  --log   %s\n", "File to send log messages to (debug, info, error).");
  printf("  --www   %s\n", "Folder containing a tree to serve as the root of a website.");
  printf("  --cgi   %s\n", "File that should be a script where you redirect all /cgi/* URIs.");
}

static void get_port(int server_type, const char *argument) {
  if (server_type == 0) {
    http_port = atoi(argument);
  } else if (server_type == 1) {
    https_port = atoi(argument);
  }
}

void process_argument(const char *option, const char *argument) {
  if (!strcmp(option, "help")) {
    print_help();
  } else if (!strcmp(option, "http")) {
    get_port(0, argument);
  } else if (!strcmp(option, "log")) {
    int ret = hslog_init(argument);
    if (ret < 0) {
      exit(-1);
    }
  } else if (!strcmp(option, "www")) {
    strncpy(file_path, argument, 255);
    initial_length = strlen(file_path);
  } else if (!strcmp(option, "cgi")) {
    strncpy(cgi_folder, argument, 127);
    cgifolder_length = strlen(cgi_folder);
  }
}

int hssocket(int port) {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(struct sockaddr_in));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  bind(sockfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in));
  listen(sockfd, 1024);

  return sockfd;
}

void set_nonblocking(int sockfd) {
  int val = fcntl(sockfd ,F_GETFL, 0);
  fcntl(sockfd, F_SETFL, val | O_NONBLOCK);
}

void convertstr(char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    if (str[i] >= 'a' && str[i] <= 'z') {
      str[i] -= 32;
    } else if (str[i] == '-') {
      str[i] = '_';
    }
  }
}
