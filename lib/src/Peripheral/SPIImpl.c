//
// Created by Peter Zdankin on 28.02.18.
//

#include <stdint.h>
#include "lib/include/Peripheral/PeripheralInterface.h"
#include "lib/include/Peripheral/SPIImpl.h"

typedef struct SPIImpl {
    PeripheralInterface interface;
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    volatile uint8_t *spcr;
    volatile uint8_t *spdr;
    volatile uint8_t *spsr;
    uint8_t f_osc;
    void (*freeFunction)(void *);
    void (*readCallback)(void);
    void (*writeCallback)(void);
} SPIImpl;


static void init(PeripheralInterface *self);

static void setReadCallback(PeripheralInterface *self,void (*callback)(void));
static void setWriteCallback(PeripheralInterface *self,void (*callback)(void));

static void writeBlocking(PeripheralInterface *self, uint8_t *data, uint16_t length);
static void readBlocking(PeripheralInterface *self, uint8_t *data, uint16_t length);

static void writeToSPDR(PeripheralInterface *self, uint8_t data);
static uint8_t readFromSPDR(PeripheralInterface *self);
static uint8_t transfer(PeripheralInterface *self, uint8_t data);

static void selectSlave(volatile uint8_t *PORT, uint8_t pin);
static void deselectSlave(volatile uint8_t *PORT, uint8_t pin);
static void configureAsSlave(volatile uint8_t *ddr, uint8_t pin,volatile  uint8_t *port);

static void enableInterrupt (PeripheralInterface *self);
static void disableInterrupt(PeripheralInterface *self);

static void destroy(PeripheralInterface *self);




PeripheralInterface * SPI_createSPI(SPIConfig config) {
    SPIImpl *implementation = config.allocate(sizeof(SPIImpl));

    implementation->interface.init = init;
    implementation->interface.setReadCallback = setReadCallback;
    implementation->interface.setWriteCallback = setWriteCallback;
    implementation->interface.writeBlocking = writeBlocking;
    implementation->interface.readBlocking = readBlocking;
    implementation->interface.write = writeToSPDR;
    implementation->interface.read = readFromSPDR;
    implementation->interface.transfer = transfer;
    implementation->interface.configureAsSlave = configureAsSlave;
    implementation->interface.selectSlave = selectSlave;
    implementation->interface.deselectSlave = deselectSlave;
    implementation->interface.enableInterrupt = enableInterrupt;
    implementation->interface.disableInterrupt = disableInterrupt;
    implementation->interface.destroy = destroy;
    implementation->interface.interruptData = config.interruptData;

    implementation->freeFunction = config.deallocate;

    implementation->ddr = config.ddr;
    implementation->port = config.port;
    implementation->spcr = config.spcr;
    implementation->spdr = config.spdr;
    implementation->spsr = config.spsr;
    implementation->f_osc = config.sck_rate;


    return (PeripheralInterface*) implementation;
}


static void set_bit(volatile uint8_t *value, uint8_t pin){
    *(value) |= (1<<pin);
}

static void unset_bit(volatile uint8_t *value, uint8_t pin){
    *(value) &= ~(1<<pin);
}

static void set_ddr(SPIImpl *self){
    set_bit(self->ddr, spi_mosi_pin);
    set_bit(self->ddr, spi_sck_pin);
}

static void set_spcr(SPIImpl *self){
    set_bit(self->spcr, spi_enable);
    set_bit(self->spcr, spi_master_slave_select);

    //Last 2 bits for f_osc
    *(self->spcr)|=(0b00000011 & self->f_osc);
}

static void set_ss(SPIImpl *self){
    //Pull up Slave Select Line
    set_bit(self->ddr, spi_ss_pin);
}

void init(PeripheralInterface *self){
    SPIImpl *spi = (SPIImpl *)self;
    set_ddr(spi);
    set_spcr(spi);
    set_ss(spi);
}


void setReadCallback(PeripheralInterface *self,void (*callback)(void)){
    SPIImpl *spi = (SPIImpl *)self;
    spi->readCallback = callback;
}

void setWriteCallback(PeripheralInterface *self,void (*callback)(void)){
    SPIImpl *spi = (SPIImpl *)self;
    spi->writeCallback = callback;
}

void writeBlocking(PeripheralInterface *self, uint8_t *data, uint16_t length){

}


void readBlocking(PeripheralInterface *self, uint8_t *data, uint16_t length){

}


void writeToSPDR(PeripheralInterface *self, uint8_t data){
    SPIImpl *spi = (SPIImpl *)self;
    *(spi->spdr) = data;
}

uint8_t readFromSPDR(PeripheralInterface *self){
    SPIImpl *spi = (SPIImpl *)self;
    return *(spi->spdr);
}

uint8_t transfer(PeripheralInterface *self, uint8_t data){
    SPIImpl *spi = (SPIImpl *)self;
    *(spi->spdr) = data;
    while(!(*(spi->spsr) & (1<<spi_interrupt_flag))){} //Wait until SPIF is set
    return *(spi->spdr);
}

/**
 * Configure a slave by
 *  1) Setting their pin as output in the Data Definition Register
 *  2) Writing a '1' to their port
 */
void configureAsSlave(volatile uint8_t *ddr, uint8_t pin,volatile  uint8_t *port){
    set_bit(ddr, pin);
    set_bit(port,pin);
}


void selectSlave(volatile uint8_t *PORT, uint8_t pin){
    unset_bit(PORT, pin);
}

void deselectSlave(volatile uint8_t *PORT, uint8_t pin){
    set_bit(PORT, pin);
}

void enableInterrupt (PeripheralInterface *self){
    SPIImpl *spi = (SPIImpl *)self;
    set_bit(spi->spcr, spi_interrupt_enable);
}

void disableInterrupt(PeripheralInterface *self){
    SPIImpl *spi = (SPIImpl *)self;
    unset_bit(spi->spcr, spi_interrupt_enable);
}

void destroy(PeripheralInterface *self){
    SPIImpl *implementation = (SPIImpl *)self;
    implementation->freeFunction(self->interruptData);
    implementation->freeFunction(implementation);
}

