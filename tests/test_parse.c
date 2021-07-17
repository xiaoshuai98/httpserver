#include "parse.h"

#include <string.h>
#include <assert.h>

int main() {
  Request *request = NULL;
  int parse_result;

  /* Valid request */
  char *request_1 = "GET /index.html HTTP/1.1\r\nHost: www.w3.org\r\nConnection: close\r\n\r\n";
  parse_result = parse(request_1, strlen(request_1) + 1, &request);
  assert(request);
  assert(parse_result == HSPARSE_VALID);
  parse_free(request);

  char *request_2 = "GET /index.html HTTP/1.1\r\nHost: www.w3.org\r\nConnection: keep-alive\r\nAccept: text/html\r\nUser-Agent: Mozilla/5.0\r\nAccept-Encoding: gzip\r\nAccept-Language: en-US\r\n\r\n";
  parse_result = parse(request_2, strlen(request_2) + 1, &request);
  assert(request);
  assert(parse_result == HSPARSE_VALID);
  parse_free(request);

  /* Invalid request */
  char *request_3 = "GET /index.html HTTP/1.1\rHost: www.w3.org\r\nConnection: close\r\n\r\n";
  parse_result = parse(request_3, strlen(request_3) + 1, &request);
  assert(!request);
  assert(parse_result == HSPARSE_INVALID);
  parse_free(request);

  char *request_4 = "GET /index.html HTTP/1.1\r\r\nHost: www.w3.org\r\nConnection: close\r\n\r\n";
  parse_result = parse(request_4, strlen(request_4) + 1, &request);
  assert(!request);
  assert(!request);
  assert(parse_result == HSPARSE_INVALID);
  parse_free(request);

  /* Incomplete request */

  char *request_5 = "GET /index.html HTTP/1.1\r\nHost: www.w3.org\r\nConnection: close\r\n\r\r\n";
  parse_result = parse(request_5, strlen(request_5) + 1, &request);
  assert(!request);
  assert(!request);
  assert(parse_result == HSPARSE_INCOMPLETE);
  parse_free(request);

  char *request_6 =  "GET /index.html HTTP/1.1\r\nHost: www.w3.org\r\nConnection: close\r\n";
  parse_result = parse(request_6, strlen(request_6) + 1, &request);
  assert(!request);
  assert(parse_result == HSPARSE_INCOMPLETE);
  parse_free(request);

  char *request_7 =  "GET /index.html HTTP/1.1\r\nHost: www.w3.org\r\nConnection: clo";
  parse_result = parse(request_7, strlen(request_7) + 1, &request);
  assert(!request);
  assert(parse_result == HSPARSE_INCOMPLETE);
  parse_free(request);

  return 0;
}
