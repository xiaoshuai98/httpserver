#include "parse.h"

#include <string.h>
#include <assert.h>

int main() {
  Request *request = NULL;
  int parse_result;
  int size;

  /* Valid request */
  char *request_1 = "GET /index.html HTTP/1.1\r\nHost: www.w3.org\r\nConnection: close\r\n\r\n";
  size = strlen(request_1);
  parse_result = parse(request_1, &size, &request);
  assert(request);
  assert(parse_result == HSPARSE_VALID);
  assert(size == (int)strlen(request_1));
  parse_free(request);

  char *request_2 = "GET /index.html HTTP/1.1\r\nHost: www.w3.org\r\nConnection: keep-alive\r\nAccept: text/html\r\nUser-Agent: Mozilla/5.0\r\nAccept-Encoding: gzip\r\nAccept-Language: en-US\r\n\r\n";
  size = strlen(request_2);
  parse_result = parse(request_2, &size, &request);
  assert(request);
  assert(parse_result == HSPARSE_VALID);
  assert(size == (int)strlen(request_2));
  parse_free(request);

  /* Invalid request */
  char *request_3 = "GET /index.html HTTP/1.1\rHost: www.w3.org\r\nConnection: close\r\n\r\n";
  size = strlen(request_3);
  parse_result = parse(request_3, &size, &request);
  assert(!request);
  assert(parse_result == HSPARSE_INVALID);
  assert(size == (int)strlen(request_3));
  parse_free(request);

  char *request_4 = "GET /index.html HTTP/1.1\r\r\nHost: www.w3.org\r\nConnection: close\r\n\r\n";
  size = strlen(request_4);
  parse_result = parse(request_4, &size, &request);
  assert(!request);
  assert(!request);
  assert(parse_result == HSPARSE_INVALID);
  assert(size == (int)strlen(request_4));
  parse_free(request);

  /* Incomplete request */

  char *request_5 = "GET /index.html HTTP/1.1\r\nHost: www.w3.org\r\nConnection: close\r\n\r\r\n";
  size = strlen(request_5);
  parse_result = parse(request_5, &size, &request);
  assert(!request);
  assert(!request);
  assert(parse_result == HSPARSE_INCOMPLETE);
  assert(!size);
  parse_free(request);

  char *request_6 =  "GET /index.html HTTP/1.1\r\nHost: www.w3.org\r\nConnection: close\r\n";
  size = strlen(request_6);
  parse_result = parse(request_6, &size, &request);
  assert(!request);
  assert(parse_result == HSPARSE_INCOMPLETE);
  assert(!size);
  parse_free(request);

  char *request_7 =  "GET /index.html HTTP/1.1\r\nHost: www.w3.org\r\nConnection: clo";
  size = strlen(request_7);
  parse_result = parse(request_7, &size, &request);
  assert(!request);
  assert(parse_result == HSPARSE_INCOMPLETE);
  assert(!size);
  parse_free(request);

  return 0;
}
