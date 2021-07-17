/**
 * @file event_handler.h
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 * 
 * @details 
 * This file declares some event handling functions, 
 * which can be registered as callback functions of struct hsevent.
 */

#ifndef HS_EVENT_HANDLER
#define HS_EVENT_HANDLER

#include "event.h"

/**
 * @brief Establish a new TCP connection.
 */
void accept_conn(struct hsevent *event);

/**
 * @brief Unregister an event, and close the connection.
 */
void close_conn(struct hsevent *event);

/**
 * @brief Responding to read-ready event.
 */
void read_conn(struct hsevent *event);

/**
 * @brief Responding to write-ready event.
 */
void write_conn(struct hsevent *event);

#endif  // HS_EVENT_HANDLER
