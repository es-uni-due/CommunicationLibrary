//
// Created by Peter Zdankin on 01.04.18.
//


#include <stdbool.h>
#include "lib/include/TransferLayer/PeripheralInterface.h"
#include "lib/include/TransferLayer/PeripheralSPIImpl.h"
#include "lib/include/TransferLayer/InterruptData.h"
#include "CException.h"
#include "lib/include/Exception.h"

struct InterruptData{
    uint8_t *buffer;
    uint16_t length;
    uint16_t index;
    bool busy;
};

typedef struct PeripheralInterfaceImpl {
    PeripheralInterface interface;
    PeripheralCallback readCallback;
    void *readCallbackParameter;
    PeripheralCallback writeCallback;
    void *writeCallbackParameter;

    volatile uint8_t *ddr;
    volatile uint8_t *port;
    volatile uint8_t *spcr;
    volatile uint8_t *spdr;
    volatile uint8_t *spsr;
    bool clearWriteCallback :1;
    bool clearReadCallback :1;
    uint8_t f_osc : 4;
    void (*handleInterrupt)(void);
    InterruptData interruptData;
} PeripheralInterfaceImpl;



static PeripheralInterfaceImpl *interfacePTR;


struct Peripheral{
    volatile uint8_t *DDR;
    uint8_t PIN;
    volatile  uint8_t *PORT;
};



static void destroy(PeripheralInterface *self);
static void init(PeripheralInterface *self);
static void writeBlocking(PeripheralInterface *self, uint8_t *buffer, uint16_t length);
static void readBlocking(PeripheralInterface *self, uint8_t *buffer, uint16_t length);
static void writeNonBlocking(PeripheralInterface *self, uint8_t *buffer, uint16_t length);
static void readNonBlocking(PeripheralInterface *self, uint8_t *buffer, uint16_t length);
static void setReadCallback(PeripheralInterface *self, PeripheralCallback callback, void *callback_parameter);
static void setWriteCallback(PeripheralInterface *self, PeripheralCallback callback, void *callback_parameter);
static void setCallbackClearFlags(PeripheralInterface *self, bool clearReadCallbackOnCall, bool clearWriteCallbackOnCall);
static void configurePeripheral(PeripheralInterface *self, Peripheral *device);
static void selectPeripheral(PeripheralInterface *self, Peripheral *device);
static void deselectPeripheral(PeripheralInterface *self, Peripheral *device);

Deallocator (deallocator);

/**
 * Create a PeripheralInterface, given a TransferlayerConfig and a SPIConfig
 * @param transferLayerConfig - struct with function Pointers to allocation and deallocation function
 * @param spiConfig - struct with &DDRB, &PORTB, &SPCR, &SPDR, &SPSR, and the SPI Speed f_osc
 * @return Pointer to PeripheralInterface
 */
PeripheralInterface * PeripheralInterface_create(TransferLayerConfig transferLayerConfig, SPIConfig spiConfig){
    PeripheralInterfaceImpl *PIImpl = transferLayerConfig.allocate(sizeof(PeripheralInterfaceImpl));
    PIImpl->ddr = spiConfig.ddr;
    PIImpl->port = spiConfig.port;
    PIImpl->spcr = spiConfig.spcr;
    PIImpl->spdr = spiConfig.spdr;
    PIImpl->spsr = spiConfig.spsr;
    PIImpl->f_osc = spiConfig.sck_rate;

    PIImpl->interface.init = init;
    PIImpl->interface.writeNonBlocking = writeNonBlocking;
    PIImpl->interface.readNonBlocking = readNonBlocking;
    PIImpl->interface.writeBlocking = writeBlocking;
    PIImpl->interface.readBlocking = readBlocking;
    PIImpl->interface.destroy = destroy;
    PIImpl->interface.setReadCallback = setReadCallback;
    PIImpl->interface.setWriteCallback = setWriteCallback;
    PIImpl->interface.setCallbackClearFlags = setCallbackClearFlags;
    PIImpl->interface.configurePeripheral = configurePeripheral;
    PIImpl->interface.selectPeripheral = selectPeripheral;
    PIImpl->interface.deselectPeripheral = deselectPeripheral;

    deallocator = transferLayerConfig.deallocate;

    return (PeripheralInterface *) PIImpl;
}

/**
 * Set a specified bit high
 *
 * uint8_t val = 0;
 * uint8_t *ptr = &val;
 * set_bit(ptr, 3);
 * TEST_ASSERT_EQUAL_UINT8(0b00001000, val);
 * @param value - Pointer to a 8 bit value
 * @param pin - The bit to set high
 */
static void set_bit(volatile uint8_t *value, uint8_t pin){
    *(value) |= (1<<pin);
}
/**
 * Set a specified bit low
 *
 * uint8_t val = 255;
 * uint8_t *ptr = &val;
 * unset_bit(ptr, 3);
 * TEST_ASSERT_EQUAL_UINT8(0b11110111, val);
 * @param value - Pointer to a 8 bit value
 * @param pin - The bit to set low
 */
static void unset_bit(volatile uint8_t *value, uint8_t pin){
    *(value) &= ~(1<<pin);
}

/**
 * Set Master Out Slave In (MOSI) and Serial Clock (SCK) to Output as we are using Master Mode.
 * Configure SS as output and pull high
 * @param self - The PeripheralInterface
 */
static void set_ddr(PeripheralInterfaceImpl *self){
    set_bit(self->ddr, spi_mosi_pin);
    set_bit(self->ddr, spi_sck_pin);
    set_bit(self->ddr, spi_ss_pin);
    set_bit(self->port, spi_ss_pin);
}

/**
 * Enable SPI and use Master Mode. Additionally set the SPI speed
 * @param self - The PeripheralInterface
 */
static void set_spcr(PeripheralInterfaceImpl *self){
    set_bit(self->spcr, spi_enable);
    set_bit(self->spcr, spi_master_slave_select);

    //Last 2 bits are f_osc
    *(self->spcr)|=(0b00000011 & self->f_osc);
}

/**
 * Enable SPI specific interrupts
 * @param self - The PeripheralInterface
 */
static void enableInterrupts(PeripheralInterfaceImpl *self){
    //TODO global Interrupts?
    set_bit(self->spcr, spi_interrupt_enable);
}

/**
 * Disable SPI specific interrupts
 * @param self - The PeripheralInterface
 */
static void disableInterrupts(PeripheralInterfaceImpl *self){
    unset_bit(self->spcr, spi_interrupt_enable);
}

/**
 * Initialize all necessary things to enable SPI
 * @param self - The PeripheralInterface
 */
void init(PeripheralInterface * self){
    PeripheralInterfaceImpl *peripheralImpl = (PeripheralInterfaceImpl *)self;
    set_ddr(peripheralImpl);
    set_spcr(peripheralImpl);
}

/**
 * Write a byte to the SPI Data Register
 * @param self - The PeripheralInterface Implementation
 * @param byte - The value to transmit
 */
static void write(PeripheralInterfaceImpl *self, uint8_t byte){
    *(self->spdr)  = byte;
}

/**
 * Read a byte from the SPI Data Register
 * @param self - The PeripheralInterface Implementation
 * @return - The content of the SPI Data Register
 */
static uint8_t read(PeripheralInterfaceImpl *self){
    return *(self->spdr);
}

/**
 * Returns, whether the PeripheralInterface is currently busy with communication
 * When the transmission is finished, the spi_interrupt_flag in the SPSR is set.
 * Therefore we check whether this flag is not set, which indicates that it is in fact busy.
 *
 * Working:
 * SPSR = 0b0xxxxxxx
 *
 * Done:
 * SPSR = 0b1xxxxxxx
 *
 * @param self - The PeripheralInterface
 * @return - true if currently busy
 */
static bool spiBusy(PeripheralInterfaceImpl *self){
    return !(*(self->spsr) & (1<<spi_interrupt_flag));
}


/**
 * Check if a callback is set and call it, check if clear flag is set and clear if so
 * @param impl - The PeripheralInterface
 */
static void readCallback(PeripheralInterfaceImpl *impl){
    if (impl->readCallback != NULL) {
        impl->readCallback(impl->readCallbackParameter);
    }
    if (impl->clearReadCallback){
        impl->readCallback = NULL;
        impl->readCallbackParameter = NULL;
    }
}

static void writeCallback(PeripheralInterfaceImpl *impl){
    if (impl->writeCallback != NULL) {
        impl->writeCallback(impl->writeCallbackParameter);
    }
    if(impl->clearWriteCallback){
        impl->writeCallback = NULL;
        impl->writeCallbackParameter = NULL;
    }
}

/**
 * One method, which may be called when calling handleInterrupt() inside the ISR
 * This method is called when a SPI transmission is finished, therefore a previous byte is sent.
 * If a callback is included, it will be called after the last byte.
 * The callback won't be cleared
 * The Interrupts are disabled with sending the last byte
 *
 * First Byte is written in writeNonBlocking
 *
 * Last Byte is written in writeInInterrupt and completion triggers WriteCallback
 *
 */
static void writeInInterrupt() {
    if (interfacePTR->interruptData.index < interfacePTR->interruptData.length) {
        write(interfacePTR, interfacePTR->interruptData.buffer[interfacePTR->interruptData.index]);
        ++interfacePTR->interruptData.index;
    }
    else{
        writeCallback(interfacePTR);
        interfacePTR->interruptData.busy = false;
        disableInterrupts(interfacePTR);
    }
}

/**
 * The other method, which may be called when calling handleInterrupt() inside the ISR
 * This method is called, when a SPI transmission is completed, i.e. a byte was received.
 * If a callback is included, it will be called with the last read byte
 */
static void readInInterrupt() {
    interfacePTR->interruptData.buffer[interfacePTR->interruptData.index] = read(interfacePTR);
    (interfacePTR->interruptData.index)++;
    if (interfacePTR->interruptData.index >= interfacePTR->interruptData.length) {
        readCallback(interfacePTR);
        interfacePTR->interruptData.busy = false;
        disableInterrupts(interfacePTR);
    }
    //TODO activate this on real hardware for SPI to start the next transmission
    //else{
    //    write(interfacePTR, 0x00);
    //}
}

/**
 * Sets up the InterruptData and the Interrupthandler for both Reading and Writing
 * @param impl - The PeripheralInterfaceImplementation
 * @param buffer - The Buffer to transmit/write in
 * @param length - Max Length of the buffer
 * @param interruptHandler - The method to call when handling an interrupt
 */
static void setupNonBlocking(PeripheralInterfaceImpl *impl, uint8_t *buffer, uint16_t length, void (*interruptHandler)(void)){
    impl->interruptData.busy = true;
    impl->handleInterrupt = interruptHandler;
    impl->interruptData.length = length;
    impl->interruptData.buffer = buffer;
    impl->interruptData.index = 0;
    interfacePTR = impl;
    enableInterrupts(impl);
}

//TODO test this on real hardware
/**
 * Write a bytebuffer with a specified length over SPI in a non blocking way
 * @param self - The PeripheralInterface
 * @param buffer - a buffer which is to be transmitted
 * @param length - The maximum transmission length
 * @warning - Length shouldn't be bigger than the buffer
 */
void writeNonBlocking(PeripheralInterface *self, uint8_t *buffer, uint16_t length){
    PeripheralInterfaceImpl *impl = (PeripheralInterfaceImpl *)self;
    if(impl->interruptData.busy == false) {
        setupNonBlocking(impl, buffer, length, writeInInterrupt);
        interfacePTR->handleInterrupt(); //Write First Byte
    }else{
        Throw(SPI_BUSY_EXCEPTION);
    }

}

//TODO test this on real hardware
/**
 * Read "length" bytes into a buffer in a non blocking way
 * @param self - The PeripheralInterface
 * @param buffer - The destination buffer
 * @param length - Maximum length to read
 * @warning - Length shouldn't be bigger than the buffer
 */
void readNonBlocking(PeripheralInterface *self, uint8_t *buffer, uint16_t length){
    PeripheralInterfaceImpl *impl = (PeripheralInterfaceImpl *)self;
    if(impl->interruptData.busy == false) {
        setupNonBlocking(impl, buffer, length, readInInterrupt);
        write(impl, 0x00); // Start transmission
    }
    else{
        Throw(SPI_BUSY_EXCEPTION);
    }
}


/**
 * Write a bytebuffer with a specified length over SPI and block until it is finished
 * @param self - The PeripheralInterface
 * @param buffer - The buffer
 * @param length - Length of the buffer
 * @warning - Length shouldn't be bigger than the buffer
 */
//TODO test this on real hardware
void writeBlocking(PeripheralInterface *self, uint8_t *buffer, uint16_t length) {
    PeripheralInterfaceImpl *peripheralSPI = (PeripheralInterfaceImpl *) self;
    for (uint16_t i = 0; i < length; ++i) {
        write(peripheralSPI, buffer[i]);
        while (spiBusy(peripheralSPI)) {}
    }
    writeCallback(peripheralSPI);
}

/**
 * Read "length" bytes into a buffer and wait until it is finished
 * @param self - The PeripheralInterface
 * @param buffer - The destination buffer
 * @param length - The amount to read
 * @warning - Length shouldn't be bigger than the buffer
 */
//TODO test this on real hardware
void readBlocking(PeripheralInterface *self, uint8_t *buffer, uint16_t length) {
    PeripheralInterfaceImpl *peripheralSPI = (PeripheralInterfaceImpl *) self;
    for (uint16_t i = 0; i < length; ++i) {
        write(peripheralSPI, 0x00);
        while (spiBusy(peripheralSPI)) {}
        buffer[i] = read(peripheralSPI);
    }
    readCallback(peripheralSPI);
}


/**
 * Free allocated memory
 * @param self - The PeripheralInterface
 */
void destroy(PeripheralInterface *self){
    deallocator(self);
}

/**
 * Set the function to be called, when any read is finished
 * @param self - The PeripheralInterface
 * @param callback - The function to be called
 * @param callback_parameter - The parameters to pass the function
 */
void setReadCallback(PeripheralInterface *self, PeripheralCallback callback, void *callback_parameter){
    PeripheralInterfaceImpl *peripheralSPI = (PeripheralInterfaceImpl *)self;
    peripheralSPI->readCallback = callback;
    peripheralSPI->readCallbackParameter = callback_parameter;
}

/**
 * Set the function to be called, when any write is finished
 * @param self - The PeripheralInterface
 * @param callback - The function to be called
 * @param callback_parameter - The parameters to pass the function
 */
void setWriteCallback(PeripheralInterface *self, PeripheralCallback callback, void *callback_parameter){
    PeripheralInterfaceImpl *peripheralSPI = (PeripheralInterfaceImpl *)self;
    peripheralSPI->writeCallback = callback;
    peripheralSPI->writeCallbackParameter = callback_parameter;
}

/**
 * Sets whether after calling one of the callbacks, the callback should be cleared and not called at the next write/read
 * @param self - The PeripheralInterface
 * @param clearReadCallbackOnCall - Clear ReadCallback after calling it?
 * @param clearWriteCallbackOnCall - Clear WriteCallback after calling it?
 */
void setCallbackClearFlags(PeripheralInterface *self,bool clearReadCallbackOnCall, bool clearWriteCallbackOnCall){
    PeripheralInterfaceImpl *peripheralSPI = (PeripheralInterfaceImpl *)self;
    peripheralSPI->clearReadCallback = clearReadCallbackOnCall;
    peripheralSPI->clearWriteCallback = clearWriteCallbackOnCall;
}

/**
 * Configure a Peripheral to be a slave
 * @param self - The PeripheralInterface
 * @param device - The device to become a slave
 */
void configurePeripheral(PeripheralInterface *self, Peripheral *device){
    set_bit(device->DDR, device->PIN);
    set_bit(device->PORT, device->PIN);
}

/**
 * Pull down the SS for the specified device to indicate the start of a transmission
 * @param self - The PeripheralInterface
 * @param device - The Device
 */
void selectPeripheral(PeripheralInterface *self, Peripheral *device){
    unset_bit(device->PORT, device->PIN);
}

/**
 * Pull up the SS for the specified device to indicate the end of a transmission
 * @param self - The PeripheralInterface
 * @param device - The Device
 */
void deselectPeripheral(PeripheralInterface *self, Peripheral *device){
    set_bit(device->PORT, device->PIN);
}