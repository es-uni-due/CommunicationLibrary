#include "MRFState.h"

void MrfState_setShortDestinationAddress(MrfState *mrf) {
  mrf->state = 1;
}

uint8_t MrfState_getCurrentFieldsSize(MrfState *mrf) {
  return FrameHeader802154_getHeaderSize(mrf->header) + 2;
}

