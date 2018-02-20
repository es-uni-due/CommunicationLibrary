
#ifndef COMMUNICATIONMODULE_SPIMESSAGE_H
#define COMMUNICATIONMODULE_SPIMESSAGE_H


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct SPIMessage SPIMessage;

/*
 * When creating a new SPIMessage, the memory
 * areas provided through outgoing_data and incoming_data,
 * must have enough space for "length" number of bytes.
 * E.g.:
 * ```
 * uint8_t out[i];
 * uint8_t int[j];
 * SPIMessage m = { .length = minimum(i,j),
 *                  .outgoing_data = out,
 *                  .incoming_data = in };
 * ```
 *
 */
struct SPIMessage {
  uint8_t length;
  uint8_t* outgoing_data;
  uint8_t* incoming_data;
  SPIMessage *next;
};

static inline void SPIMessage_append(SPIMessage *head, SPIMessage *tail) {
  head->next = tail;
}

static inline void SPIMessage_init(SPIMessage *self) {
  self->length = 0;
  self->incoming_data = NULL;
  self->outgoing_data = NULL;
  self->next = NULL;
}

/*
 * This function compares the complete outgoing data of two messages byte by byte.
 * The comparation is done over a linked list of uint8_t arrays.
 */
bool SPIMessage_equal(const SPIMessage *left, const SPIMessage *right);

#endif //COMMUNICATIONMODULE_SPIMESSAGE_H
