/**
 * @file parse.h
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 * 
 * @details 
 * This file declares some structs and functions. 
 * These structs and functions are used to parse HTTP request.
 */

#ifndef HSHTTP_PARSE
#define HSHTTP_PARSE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define HSPARSE_VALID      0
#define HSPARSE_INVALID    1
#define HSPARSE_INCOMPLETE 2

typedef struct
{
	char header_name[128];
	char header_value[256];
} Request_header;

typedef struct
{
	char http_version[16];
	char http_method[16];
	char http_uri[1024];
	Request_header *headers;
	int header_count;
  int header_capacity;
} Request;

/**
 * @brief Parse HTTP request.
 * 
 * @details
 * There are two reasons for the failure of parsing: 
 * one is that it does not end with \r\n\r\n, we think this request is incomplete, 
 * and we should continue to read data from the socket; 
 * the other is that the format is incorrect, such as a certain request header without \r\n, 
 * a response should be returned to notify this error.
 * 
 * @param[in] buffer A char array for storing HTTP request.
 * @param[in] size The number of bytes to be parsed.
 * @param[out] request The result after parsing
 * 
 * @return A flag indicating whether the parsing was successful.
 */
int parse(char *buffer, int size, Request **request);

int yyparse();
void set_parsing_options(char *buf, size_t i, Request *request);

#endif  // HSHTTP_PARSE
