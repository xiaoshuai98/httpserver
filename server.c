/**
 * @file server.c
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 * @brief Start the HTTP server.
 */

#include "parse.h"
#include "event.h"
#include "utils.h"
#include "event_handler.h"

#include <sys/socket.h>
#include <sys/epoll.h>
#include <getopt.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fputs("Server failed to start.\n", stderr);
    fputs("Run server --help to see more information.\n", stderr);
    exit(-1);
  }
  int val = 0;
  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
      {"help", no_argument, 0, 0}, 
      {"http", required_argument, 0, 0},
      {"https", required_argument, 0, 0},
      {"log", required_argument, 0, 0},
      {"lock", required_argument, 0, 0},
      {"www", required_argument, 0, 0},
      {"cgi", required_argument, 0, 0},
      {"key", required_argument, 0, 0},
      {"certificate", required_argument, 0, 0},
      {0, 0, 0, 0}
    };
    val = getopt_long(argc, argv, "", long_options, &option_index);
    if (val == -1) {
      break;
    }

    switch (val) {
      case 0: {
        process_argument(long_options[option_index].name, optarg);
        break;
      }
      case '?': {
        break;
      }
      default: {
        printf("?? getopt returned character code 0%o ??\n", val);
      }
    }
  }

  struct hsevent_base *base = hsevent_base_init();
  int serv_sockfd = hssocket(http_port);
  set_nonblocking(serv_sockfd);
  struct hsevent *listen_event = hsevent_init(serv_sockfd, EPOLLIN | EPOLLET, base);
  hsevent_update_cb(listen_event, HSEVENT_READ, accept_conn);

  hsevent_base_loop(base);

  hsevent_base_clear(base);
  hsevent_base_free(base);

  exit(0);
}
