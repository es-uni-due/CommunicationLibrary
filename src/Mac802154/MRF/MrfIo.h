#ifndef COMMUNICATIONMODULE_MRFIO_H_H
#define COMMUNICATIONMODULE_MRFIO_H_H

#include <stdint.h>
#include "PeripheralInterface/PeripheralInterface.h"
#include "CommunicationModule/Mac802154MRFImpl.h"


/**
 * IO Module for the MRF Network Chip
 */

typedef struct MrfIo MrfIo;
typedef struct MrfIoCallback MrfIoCallback;
typedef struct  MrfIo_NonBlockingWriteContext MrfIo_NonBlockingWriteContext;


void MrfIo_writeBlockingToLongAddress(MrfIo *mrf, const uint8_t *payload, uint8_t size, uint16_t address);
void MrfIo_writeBlockingToShortAddress(MrfIo *mrf, const uint8_t *payload, uint8_t size, uint8_t address);

/**
 * Evaluates the register address to determine if it belongs to the short or long address space of
 * the mrf chip. Then synchronously writes that value to the address.
 * @param mrf
 * @param register_address
 * @param value
 */
void MrfIo_setControlRegister(MrfIo *mrf, uint16_t register_address, uint8_t value);
uint8_t MrfIo_readControlRegister(MrfIo *mrf, uint16_t register_address);
void MrfIo_readBlockingFromLongAddress(MrfIo *mrf, uint16_t register_address, uint8_t *buffer, uint8_t size);
void MrfIo_readNonBlockingFromLongAddress(MrfIo *mrf, const uint8_t *payload, uint8_t size);
void MrfIo_readBlockingFromShortAddress(MrfIo *mrf, const uint8_t *payload, uint8_t size);
void MrfIo_readNonBlockingFromShortAddress(MrfIo *mrf, const uint8_t *payload, uint8_t size);




#endif //COMMUNICATIONMODULE_MRFIO_H_H
