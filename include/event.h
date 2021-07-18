/**
 * @file event.h
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 * 
 * @details 
 * This file declares some structs and functions. 
 * These structs and functions are used to implement an asynchronous event loop.
 */

#ifndef HS_EVENT
#define HS_EVENT

#include "buffer.h"

#define HSEVENT_READ  0 // EPOLLIN
#define HSEVENT_WRITE 1 // EPOLLOUT
#define HSEVENT_RDHUP 2 // EPOLLRDHUP
#define HSEVENT_ERR   3 // EPOLLERR

/**
 * @brief The polling unit in the event loop.
 * 
 * @details
 * We can register some callback functions to hsevent in response to network IO.
 * 
 * HTTPServer should assign a struct hsevent to each socket 
 * (including listening socket and connected sockets), 
 * and add the hsevent to the event loop to monitor these sockets.
 * 
 * @note
 * Some functions require a pointer to hsevent as parameter, 
 * and the caller should be responsible for checking whether this pointer is legal.
 */
struct hsevent;

typedef void (*hsevent_cb)(struct hsevent *event);
struct hsevent {
  int sockfd;                       // Associated socket
  struct hsbuffer *inbound;         // Input buffer, read data from socket
  struct hsbuffer *outbound;        // Output buffer, write data to socket
  uint32_t events;                  // Types of events monitored
  struct hsevent_base *event_base;  // Point to the hsevent_base that polls the hsevent
  hsevent_cb read_cb;               // Callback function for EPOLLIN
  hsevent_cb write_cb;              // Callback function for EPOLLOUT
  hsevent_cb rdhup_cb;              // Callback function for EPOLLHUP
  hsevent_cb err_cb;                // Callback function for EPOLLERR
  int closed;                       // Indicate whether to close the connection after sending the response.
};

/**
 * @brief Responsible for polling the hsevent and opening the event loop.
 * 
 * @details 
 * We add struct hsevent to hsevent_base, 
 * hsevent_base will poll all struct hsevent, 
 * and automatically call the callback functions 
 * of all activated struct hsevent after the polling ends.
 * 
 * @note
 * Some functions require a pointer to hsevent_base as parameter, 
 * and the caller should be responsible for checking whether this pointer is legal.
 */
struct hsevent_base;

/**
 * @brief Initialize a hsevent.
 * 
 * @details
 * If events is zero, event_base will be ignored.
 * 
 * @param[in] sockfd The monitored listening socket or connected socket.
 * @param[in] events The events of interest to the caller.
 * @param[in] event_base The hsevent_base to which hsevent belongs.
 * 
 * @return A pointer to the allocated hsevent, or NULL if failed.
 */
struct hsevent* hsevent_init(int sockfd, int events, struct hsevent_base* event_base);

/**
 * @brief Free the memory space of a hsevent.
 * 
 * @param[in] event A pointer to the allocated hsevent. 
 */
void hsevent_free(struct hsevent *event);

/**
 * @brief Modify the events monitored by hsevent.
 * 
 * @param[in] event A pointer to the allocated hsevent.
 * @param[in] events The events of interest to the caller.
 */
void hsevent_update(struct hsevent *event, int events);

/**
 * @brief Modify the callback function of an event.
 * 
 * @param[in] event A pointer to the allocated hsevent.
 * @param[in] event_type Event monitored.
 * @param[in] event_cb The call back function, can be NULL.
 */
void hsevent_update_cb(struct hsevent *event, int event_type, hsevent_cb event_cb);

/**
 * @brief Initialize a hsevent_base. 
 * 
 * @return A pointer to the allocated hsevent_base, or NULL if failed.
 */
struct hsevent_base* hsevent_base_init();

/**
 * @brief Free the memory space of the hsevent_base.
 * 
 * @param[in] event_base A pointer to the allocated hsevent_base.
 */
void hsevent_base_free(struct hsevent_base *event_base);

/**
 * @brief Perform operation on hsevent_base
 * 
 * @param[in] op The operation to be performed on hsevent_base.
 * @param[in] event The target of the operation.
 * @param[in] event_base The target of the operation.
 */
void hsevent_base_update(int op, 
                         struct hsevent *event,
                         struct hsevent_base *event_base);

/**
 * @brief Clear all registered hsevent of hsevent_base.
 */
void hsevent_base_clear(struct hsevent_base *base);

/**
 * @brief Start the event loop.
 * 
 * @param[in] event_base A pointer to the allocated hsevent_base.
 */
void hsevent_base_loop(struct hsevent_base *event_base);

/* For debug */
void hsevent_base_exit(struct hsevent_base *event_base);

#endif  // HS_EVENT
