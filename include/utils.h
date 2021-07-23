/**
 * @file utils.h
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 * 
 * @details 
 * This file declares some useful utility functions and macros.
 */

#ifndef HS_UTILS
#define HS_UTILS

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

extern int http_port;
extern int https_port;

/**
 * @brief Process the successfully parsed command line argument.
 */
void process_argument(const char *option, const char *argument);

/**
 * @brief Return a listening socket.
 * 
 * @note
 * If you use http_port or https_port as parameter, 
 * check whether they have been initialized.
 */
int hssocket(int port);

/**
 * @brief Set sockfd to non-blocking mode.
 */
void set_nonblocking(int sockfd);

/**
 * @brief Convert all characters of the string to cgi format.
 */
void convertstr(char *str);

#endif  // HS_UTILS
