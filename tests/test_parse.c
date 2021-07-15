#include "parse.h"

#include "string.h"
#include "assert.h"

int main() {
  Request *request = NULL;
  int parse_result;

  // TODO(qds): Add more tests.
  /* Valid request */
  char *request_1 = "GET /index.html HTTP/1.1\r\nHost: www.w3.org\r\nConnection: close\r\n\r\n";
  parse_result = parse(request_1, strlen(request_1) + 1, &request);
  assert(request);
  assert(parse_result == HSPARSE_VALID);
  parse_free(request);

  /* Invalid request */
  char *request_2 = "GET /index.html HTTP/1.1\rHost: www.w3.org\r\nConnection: close\r\n\r\n";
  parse_result = parse(request_2, strlen(request_2) + 1, &request);
  assert(!request);
  assert(parse_result == HSPARSE_INVALID);
  parse_free(request);

  /* Incomplete request */
  char *request_3 =  "GET /index.html HTTP/1.1\r\nHost: www.w3.org\r\nConnection: close\r\n";
  parse_result = parse(request_3, strlen(request_3) + 1, &request);
  assert(!request);
  assert(parse_result == HSPARSE_INCOMPLETE);
  parse_free(request);

  return 0;
}
