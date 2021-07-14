/**
 * @file buffer.h
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 * 
 * @details 
 * This file declares a struct hsbuffer and some functions related to the struct. 
 */

#ifndef HS_BUFFER
#define HS_BUFFER

#include "stddef.h"
#include "sys/types.h"
#include "stdint.h"

#define READ_POS 0
#define WRITE_POS 1

#define HS_BUFFER_SIZE 8192

/**
 * @brief A buffer that can be used for network IO.
 * 
 * @details
 * httpserver should allocate an hsbuffer for every connected socket. 
 * The socket can only perform IO on hsbuffer, and it is better to use hsbuffer's IO functions.
 * 
 * @note
 * All functions require a pointer to hsbuffer as parameter, 
 * and the caller should be responsible for checking whether this pointer is legal.
 */
struct hsbuffer;

/**
 * @brief Allocate and initialize an hsbuffer.
 * 
 * @details
 * The only reason this function failed is out of memory. 
 * If this function returns a null pointer, the ENOMEM error needs to be handled.
 * 
 * @param[in] buffer_size The initial capacity of struct hsbuffer.
 * 
 * @return A pointer to the allocated hsbuffer, or NULL if the request failed.
 */
struct hsbuffer* hsbuffer_init(uint32_t buffer_size);

/**
 * @brief Free the hsbuffer pointed to by ptr.
 * 
 * @param[in] ptr A pointer to the allocated hsbuffer.
 */
void hsbuffer_free(struct hsbuffer *ptr);

/**
 * @brief Return the readable or writable position of the hsbuffer pointed by ptr.
 * 
 * @param[in] ptr A pointer to the allocated hsbuffer.
 * @param[in] pos_type The type of hsbuffer position, READ_POS or WRITE_POS.
 * 
 * @return A pointer to readable or writeable position, or NULL if pos_type is invalid.
 */
char* hsbuffer_pos(struct hsbuffer *ptr, int pos_type);

/**
 * @brief Return the size of the hsbuffer pointed by ptr that has been used.
 * 
 * @param[in] ptr A pointer to the allocated hsbuffer.
 * 
 * @return The size of the hsbuffer that has been used.
 */
size_t hsbuffer_length(const struct hsbuffer *ptr);

/**
 * @brief Return the capacity of the hsbuffer pointed by ptr.
 * 
 * @param[in] ptr A pointer to the allocated hsbuffer.
 * 
 * @return The capacity of the hsbuffer.
 */
size_t hsbuffer_capacity(const struct hsbuffer *ptr);

/**
 * @brief Return the remain writeable memory space of the hsbuffer pointed by ptr.
 * 
 * @param[in] ptr A pointer to the allocated hsbuffer.
 * 
 * @return The remain writeable memory space of the hsbuffer.
 */
size_t hsbuffer_remain(const struct hsbuffer *ptr);

/**
 * @brief Return the number of readable bytes of the hsbuffer pointed by ptr.
 * 
 * @param[in] ptr A pointer to the allocated hsbuffer.
 * 
 * @return The number of readable bytes of the hsbuffer.
 */
size_t hsbuffer_readable(const struct hsbuffer *ptr);

/**
 * @brief Receive length bytes from sockfd into the hsbuffer pointed by ptr.
 * 
 * @param[in] sockfd A connected socket.
 * @param[in] ptr A pointed to the allocated hsbuffer.
 * @param[in] length The maximum length of message received from the socket.
 * 
 * @return The number of bytes received, or -1 if an error occured.
 */
ssize_t hsbuffer_recv(int sockfd, struct hsbuffer *ptr, size_t length);

/**
 * @brief Send length bytes from the hsbuffer pointed by ptr to the sockfd.
 * 
 * @param[in] sockfd A connected socket.
 * @param[in] ptr A pointed to the allocated hsbuffer.
 * @param[in] length The maximum length of message send to the socket.
 * 
 * @return The number of bytes sent, or -1 if an error occured.
 */
ssize_t hsbuffer_send(int sockfd, struct hsbuffer *ptr, size_t length);

/**
 * @brief Increase the memory space of the hsbuffer pointed by ptr.
 * 
 * @details 
 * If new_capacity is less than the capacity of the hsbuffer,
 * no operation is performed.
 * 
 * @param[in] ptr A pointer to the allocated hsbuffer.
 * @param[in] new_capacity The new capacity after increasing the hsbuffer.
 */
void hsbuffer_expand(struct hsbuffer *ptr, size_t new_capacity);

/**
 * @brief Consume the readable space in the hsbuffer pointed by ptr.
 * 
 * @details
 * If length is bigger than the size of readble space in the hsbuffer,
 * the length will be truncated.
 * 
 * @param[in] ptr A pointer to the allocated hsbuffer.
 * @param[in] length The size of the readable space consumed in the hsbuffer.
 */
void hsbuffer_consume(struct hsbuffer *ptr, size_t length);

#endif  // HS_BUFFER
