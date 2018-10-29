#ifndef PERIPHERALINTERFACE_H
#define PERIPHERALINTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "include/Callback.h"

/*!
 * \file Peripheral.h
 * The PeripheralInterface provides a software interface
 * for different peripheral interface drivers (USART, SPI).
 * The following example illustrates writing a string
 * to a peripheral device, the initialization of the
 * Peripheral struct (software representation of the peripheral
 * device you want to interact with) and the PeripheralInterface itself
 * are documented in the header file corresponding to their implementation:
 * ~~~.c
 * uint8_t string[] = "hello, world!";
 * PeripheralInterface_selectPeripheral(interface, peripheral);
 * PeripheralInterface_writeBlocking(interface, string, strlen(string));
 * PeripheralInterface_deselectPeripheral(interface, peripheral);
 * ~~~
 *
 * For now the support for non blocking io is not fully implemented.
 */

typedef void Peripheral;
typedef struct PeripheralInterface* PeripheralInterface;
typedef CommunicationModule_Callback PeripheralInterface_Callback;

typedef struct PeripheralInterface_NonBlockingWriteContext
{
  PeripheralInterface_Callback callback;
  const uint8_t *output_buffer;
  size_t length;
} PeripheralInterface_NonBlockingWriteContext;

typedef struct PeripheralInterface_NonBlockingReadContext
{
  PeripheralInterface_Callback callback;
  uint8_t *input_buffer;
  size_t length;
} PeripheralInterface_NonBlockingReadContext;

void
PeripheralInterface_writeBlocking (PeripheralInterface self,
                                   const uint8_t *buffer,
                                   size_t size);

void
PeripheralInterface_writeNonBlocking(PeripheralInterface self,
                                     PeripheralInterface_NonBlockingWriteContext context);

void
PeripheralInterface_readBlocking (PeripheralInterface self,
                                  uint8_t *buffer,
                                  size_t length);

void
PeripheralInterface_readNonBlocking(PeripheralInterface self,
                                    uint8_t *buffer,
                                    uint16_t size);

/**
 * Runs the necessary setup steps to allow data transfers
 * to the specified peripheral device. Make sure the
 * specified peripheral matches the implementation used,
 * otherwise behaviour is undefined. Additionally
 * this locks the PeripheralInterface ADT for exclusive
 * access.
 */
void
PeripheralInterface_selectPeripheral(PeripheralInterface self,
                                     Peripheral *device);

/**
 * Unlocks the PeripheralInterface ADT and performs clean
 * up steps where necessary, e.g. disabling slave select
 * lines for SPI.
 */
void
PeripheralInterface_deselectPeripheral(PeripheralInterface self,
                                       Peripheral *device);

/**
 * Use this function inside the interrupt service routine,
 * triggering when a write operation was completed, to allow
 * for non blocking write operations.
 */
void
PeripheralInterface_handleWriteInterrupt(PeripheralInterface self);

/**
 * Use this function inside the interrupt service routine,
 * triggering when a read operation was completed, to allow
 * for non blocking read operations.
 */
void
PeripheralInterface_handleReadInterrupt(PeripheralInterface self);


#endif /* PERIPHERALINTERFACE_H */