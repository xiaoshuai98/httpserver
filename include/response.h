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

/**
 * @brief Parse event->inbound and write the response to event->outbound.
 * 
 * @param[in] event The target of parsing and generating response.
 * @param[out] request Request headers, may be NULL.
 * 
 * @return The result of parsing the HTTP request.
 */
int create_response(struct hsevent *event, Request **request);

#endif  // HS_RESPONSE
