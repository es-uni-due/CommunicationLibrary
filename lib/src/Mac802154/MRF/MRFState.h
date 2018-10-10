#ifndef COMMUNICATIONMODULE_MRFSTATE_H
#define COMMUNICATIONMODULE_MRFSTATE_H

#include "lib/src/Mac802154/FrameHeader802154.h"

/**
 * This module manages the data, that needs to be
 * written to the mrf chip in order to transmit
 * a message. For each field to be updated it
 * returns the destination memory address, the
 * size and the pointer to the actual data.
 * All changes made to fields through this module
 * will result in the corresponding data to be
 * included in the set of fields returned by
 * getNextField(). The module yields all data
 * that needs to be updated on the mrf due to
 * field changes.
 */

typedef struct MrfHeader {
  uint8_t frame_header_length;
  uint8_t frame_length;
  FrameHeader802154 frame_header;
} MrfHeader;

typedef struct MrfField {
  uint8_t size;
  uint16_t address;
  const uint8_t *data;
} MrfField;

typedef struct MrfState {
  uint8_t state;
  MrfHeader header;
  const uint8_t *payload;
} MrfState;

enum {
  MRF_STATE_DESTINATION_ADDRESS_CHANGED = 1,
};

void MrfState_init(MrfState *mrf_state);
void MrfState_setExtendedDestinationAddress(MrfState *mrf, uint64_t address);
void MrfState_setShortDestinationAddress(MrfState *mrf, uint16_t address);
void MrfState_setShortSourceAddress(MrfState *mrf, uint16_t address);
void MrfState_setPanId(MrfState *mrf, uint16_t pan_id);
void MrfState_setPayload(MrfState *mrf, const uint8_t *payload, uint8_t payload_length);
const uint8_t *MrfState_getPayload(MrfState *mrf);
uint8_t MrfState_getPayloadLength(MrfState *mrf);
MrfField MrfState_getPayloadField(MrfState *mrf_state);
void MrfState_setSequenceNumber(MrfState *mrf);
void MrfState_disableSequenceNumber(MrfState *mrf);
void MrfState_enableSequenceNumber(MrfState *mrf);
const uint8_t *MrfState_getFullHeaderData(MrfState *mrf);
bool MrfState_nextField(MrfState *mrf);
MrfField MrfState_getCurrentField(MrfState *mrf);
uint8_t MrfState_getFullHeaderLength(MrfState *mrf);
uint8_t MrfState_getCurrentFieldsOffset(MrfState *mrf);
const uint8_t *MrfState_getCurrentFieldsData(MrfState *mrf);
MrfField MrfState_getFullHeaderField(MrfState *mrf_state);
#endif