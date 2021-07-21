/**
 * @file response.h
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 * 
 * @details 
 * This file declares some functions for generating responses.
 */

#ifndef HS_RESPONSE
#define HS_RESPONSE

#include "parse.h"
#include "event.h"

extern char file_path[256];
extern int initial_length;

/**
 * @brief Parse event->inbound and write the response to event->outbound.
 * 
 * @param[in] event The target of parsing and generating response.
 * @param[out] fd The fd of the requested object in the HTTP request.
 * 
 * @details
 * If opening the requested object fails, [fd] will be set to -1.
 * 
 * @return The result of parsing the HTTP request.
 */
int create_response(struct hsevent *event, int *fd, size_t *file_length);

/**
 * @brief Generate an HTTP response to notify the peer timeout.
 */
void response_timeout(struct hsevent *event);

#endif  // HS_RESPONSE
