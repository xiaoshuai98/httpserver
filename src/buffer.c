/**
 * @file buffer.c
 * @author Quan.Dashuai
 * @version 1.0
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 */

#include "buffer.h"
#include "utils.h"

#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>

struct hsbuffer {
  char* data;           // Point to the memory allocated for the hsbuffer
  size_t read_pos;      // Readable position in the hsbuffer
  size_t write_pos;     // Writable position in the hsbuffer
  size_t used_length;   // The size of the hsbuffer that has been used
  size_t capacity;      // The maximum number of bytes that the hsbuffer can hold
};

struct hsbuffer* hsbuffer_init(uint32_t buffer_size) {
  struct hsbuffer *ptr = (struct hsbuffer*)malloc(sizeof(struct hsbuffer));
  if (!ptr) {
    return NULL;
  }
  ptr->data = (char*)malloc(buffer_size + 1);   // Reserve a space for the null terminator
  if (!ptr->data) {
    free(ptr);
    return NULL;
  }
  ptr->capacity = buffer_size;
  ptr->used_length = 0;
  ptr->read_pos = 0;
  ptr->write_pos = 0;

  return ptr;
}

void hsbuffer_free(struct hsbuffer *ptr) {
  if (!ptr) {
    return ;
  }
  if (ptr->data) {
    free(ptr->data);
  }
  free(ptr);
}

char* hsbuffer_pos(struct hsbuffer *ptr, int pos_type) {
  char *data = ptr->data;
  switch (pos_type) {
    case READ_POS: {
      data += ptr->read_pos;
      break;
    }
    case WRITE_POS: {
      data += ptr->write_pos;
      break;
    }
    default: {
      data = NULL;
      break;
    }
  }

  return data;
}

size_t hsbuffer_length(const struct hsbuffer *ptr) {
  return ptr->used_length;
}

size_t hsbuffer_capacity(const struct hsbuffer *ptr) {
  return ptr->capacity;
}

size_t hsbuffer_remain(const struct hsbuffer *ptr) {
  return ptr->capacity - ptr->used_length;
}

size_t hsbuffer_readable(const struct hsbuffer *ptr) {
  return ptr->write_pos - ptr->read_pos;
}

ssize_t hsbuffer_recv(int sockfd, struct hsbuffer *ptr, size_t length) {
  length = MIN(hsbuffer_remain(ptr), length);
  char *buf = hsbuffer_pos(ptr, WRITE_POS);
  ssize_t bytes_recv = 0;
  if ((bytes_recv = recv(sockfd, (void*)buf, length, 0)) > 0) {
    ptr->write_pos += (size_t)bytes_recv;
    ptr->used_length += (size_t)bytes_recv;
    ptr->data[ptr->write_pos] = '\0';
  }
  return bytes_recv;
}

ssize_t hsbuffer_send(int sockfd, struct hsbuffer *ptr, size_t length) {
  length = MIN(ptr->write_pos - ptr->read_pos, length);
  char *buf = hsbuffer_pos(ptr, READ_POS);
  ssize_t bytes_sent = 0;
  if ((bytes_sent = send(sockfd, (void*)buf, length, 0)) > 0) {
    hsbuffer_consume(ptr, bytes_sent);
  }

  return bytes_sent;
}

void hsbuffer_expand(struct hsbuffer *ptr, size_t new_capacity) {
  if (new_capacity <= ptr->capacity) {
    return ;
  }

  char *new_data;
  if ((new_data = (char*)realloc((void*)ptr->data, new_capacity + 1)) != NULL) { // Reserve a space for the null terminator
    ptr->data = new_data;
    ptr->capacity = new_capacity;
  } 
}

void hsbuffer_ncpy(struct hsbuffer *ptr, const char *src, size_t length) {
  strncpy(hsbuffer_pos(ptr, WRITE_POS), src, length);
  ptr->write_pos += length;
}

void hsbuffer_consume(struct hsbuffer *ptr, size_t length) {
  length = MIN(ptr->write_pos - ptr->read_pos, length);
  ptr->read_pos += length;
  if (ptr->read_pos == ptr->write_pos) {
    ptr->read_pos = ptr->write_pos = 0;
    ptr->used_length = 0;
  }
}
