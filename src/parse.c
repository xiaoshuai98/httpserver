/**
 * @file buffer.c
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 */

#include "parse.h"

#include <string.h>

int parse(char *buffer, int *size, Request **request) {
  // Differant states in the state machine
	enum {
		STATE_START = 0, STATE_CR, STATE_CRLF, STATE_CRLFCR, STATE_CRLFCRLF
	};

	int i = 0, state;
	size_t offset = 0;
	char ch;
	char buf[8192];
	memset(buf, 0, 8192);

	state = STATE_START;
	while (state != STATE_CRLFCRLF) {
		char expected = 0;

		if (i == *size)
			break;

		ch = buffer[i++];
		buf[offset++] = ch;

		switch (state) {
		case STATE_START:
		case STATE_CRLF:
			expected = '\r';
			break;
		case STATE_CR:
		case STATE_CRLFCR:
			expected = '\n';
			break;
		default:
			state = STATE_START;
			continue;
		}

		if (ch == expected)
			state++;
		else
			state = STATE_START;

	}

	if (state == STATE_CRLFCRLF) {
		*request = (Request *) malloc(sizeof(Request));
    (*request)->header_count = 0;
		(*request)->header_capacity = 4;
    (*request)->headers = (Request_header *)malloc(sizeof(Request_header) * 4);
		set_parsing_options(buf, i, *request);

		*size = i;
		if (yyparse() == HSPARSE_VALID) {
      return HSPARSE_VALID;
		} else {
      free((*request)->headers);
      free(*request);
      *request = NULL;
      return HSPARSE_INVALID;
    }
	}
	*size = 0;
	return HSPARSE_INCOMPLETE;
}

Request_header* find_key(Request *request, const char *key) {
	for (int i = 0; request && i < request->header_count; i++) {
		if (!strcmp(request->headers[i].header_name, key)) {
			return &request->headers[i];
		}
	}
	return NULL;
}

void parse_free(Request *request) {
	if (!request) {
		return ;
	}
	if (request->headers) {
		free(request->headers);
	}
	free(request);
}
