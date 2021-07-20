/**
 * @file log.h
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 * 
 * @details 
 * This file declares some functions to record common logs.
 */

#ifndef HS_LOG
#define HS_LOG

#include "parse.h"

#include <netinet/in.h>

extern int logfd; // The fd of the log file

/**
 * @brief Open log file.
 * 
 * @param logfile The path of the log file.
 * 
 * @return 0 means failure to open the log file, non-zero means success.
 */
int hslog_init(const char *logfile);

/**
 * @brief Write common log to log file.
 * 
 * @param remote The client's IP address.
 * @param request HTTP request sent by the client.
 * @param status The status code of the HTTP response sent by the server.
 * @param length The length of the content sent by the server, excluding the response headers.
 */
void hslog_log(struct sockaddr_in *remote, Request *request, int status, int length);

#endif  // HS_LOG
