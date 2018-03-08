//
// Created by Peter Zdankin on 28.02.18.
//

#include "unity.h"
#include "test/Mocks/MockRuntimeLibraryImpl.h"
#include "lib/include/SPI_Layer/SPI.h"
#include "lib/include/SPI_Layer/SPIImpl.h"
#include "lib/include/platform/io.h"

static uint8_t memory[20];
static MockAllocateConfig alloc_config = {.size_allocated = 0, .returned_address = &memory};
static SPI *spi;

volatile uint8_t mySPCR = 0;
volatile uint8_t mySPDR = 0;
volatile uint8_t myDDR = 0;
volatile uint8_t myPORTB = 0;

static uint8_t f_osc = f_osc_16;

typedef struct SPIImpl {
    SPI interface;
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    volatile uint8_t *spcr;
    volatile uint8_t *spdr;
    uint8_t f_osc;
} SPIImpl;


#define SPCR (*(volatile uint8_t *)&mySPCR)

#define SPDR (*(volatile uint8_t *)&mySPDR)

#define DDRB (*(volatile uint8_t *)&myDDR)

#define PORTB (*(volatile uint8_t *)&myPORTB)

void configureMock(void){
    MockAllocate_configure(&alloc_config);
}

void setupDummyRegisters(void){
    SPCR = 0;
    SPDR = 0;
    DDRB = 0;
    PORTB = 0;
}

void setUp(void){
    setupDummyRegisters();
    configureMock();
    SPIConfig config = {&DDRB, &PORTB, &SPCR, &SPDR, f_osc, MockAllocate_allocate};
    spi = SPI_createSPI(config);
}

void test_createSPIConfigReturnsCorrectPointer(void){
    TEST_ASSERT_EQUAL_PTR(alloc_config.returned_address, spi);
}

void test_ddr(void){
    TEST_ASSERT_EQUAL(0, DDRB);
}

void test_spiInitNotNull(void){
    TEST_ASSERT_NOT_NULL(spi->initSPI);
}

void test_f_osc_4(void){
    TEST_ASSERT_BIT_LOW(0, f_osc_4);
    TEST_ASSERT_BIT_LOW(1, f_osc_4);
}

void test_f_osc_16(void){
    TEST_ASSERT_BIT_HIGH(0, f_osc_16);
    TEST_ASSERT_BIT_LOW (1, f_osc_16);
}

void test_f_osc_64(void){
    TEST_ASSERT_BIT_LOW (0, f_osc_64);
    TEST_ASSERT_BIT_HIGH(1, f_osc_64);
}

void test_f_osc_128(void){
    TEST_ASSERT_BIT_HIGH(0, f_osc_128);
    TEST_ASSERT_BIT_HIGH(1, f_osc_128);
}

void test_spiDDR(void){
    SPIImpl *spiImpl = (SPIImpl *)spi;
    spiImpl->interface.initSPI(spi);
    TEST_ASSERT_BIT_HIGH(spi_mosi_pin, *(spiImpl->ddr));
    TEST_ASSERT_BIT_HIGH(spi_sck_pin, *(spiImpl->ddr));
}

void test_spiSPCR(void){
    SPIImpl *spiImpl = (SPIImpl *)spi;
    spiImpl->interface.initSPI(spi);
    TEST_ASSERT_BIT_HIGH(spi_enable, *(spiImpl->spcr));
    TEST_ASSERT_BIT_HIGH(spi_master_slave_select, *(spiImpl->spcr));
    TEST_ASSERT_BIT_HIGH(spi_ss_pin, *(spiImpl->ddr));
    uint8_t tempSPCR = *(spiImpl->spcr);
    tempSPCR &= 0b00000011;
    TEST_ASSERT_EQUAL(f_osc, tempSPCR);
}

void test_spiWriteNotNull(void){
    TEST_ASSERT_NOT_NULL(spi->writeToSPDR);
}

void test_spiWriteChangesSPDR(void){
    SPIImpl *spiImpl = (SPIImpl *)spi;
    spiImpl->interface.initSPI(spi);
    spiImpl->interface.writeToSPDR(spi, 42);
    TEST_ASSERT_EQUAL(42, *(spiImpl->spdr));
}

void test_spiReadNotNull(void){
    TEST_ASSERT_NOT_NULL(spi->readFromSPDR);
}

void test_spiReadBeforeStartIsZero(void){
    SPIImpl *spiImpl = (SPIImpl *)spi;
    spiImpl->interface.initSPI(spi);
    TEST_ASSERT_EQUAL(0, spiImpl->interface.readFromSPDR(spi));
}

void test_spiWriteThenReadIsTheSame(void){
    SPIImpl *spiImpl = (SPIImpl *)spi;
    spiImpl->interface.initSPI(spi);
    spiImpl->interface.writeToSPDR(spi,42);
    TEST_ASSERT_EQUAL(42, spiImpl->interface.readFromSPDR(spi));
}

void test_spiSlaveSelectNotNull(void){
    TEST_ASSERT_NOT_NULL(spi->selectSlave);
}

void test_spiSlaveDeselectNotNull(void){
    TEST_ASSERT_NOT_NULL(spi->deselectSlave);
}

void test_spiConfigureSlaveNotNull(void){
    TEST_ASSERT_NOT_NULL(spi->configureAsSlave);
}

void test_spiConfigureSetsDDRAndPORT(void){
    uint8_t spi_slave = 1;
    SPIImpl *spiImpl = (SPIImpl *)spi;
    spiImpl->interface.initSPI(spi);
    spiImpl->interface.configureAsSlave(&DDRB, spi_slave, &PORTB);
    TEST_ASSERT_BIT_HIGH(spi_slave, *(spiImpl->ddr));
    TEST_ASSERT_BIT_HIGH(spi_slave, *(spiImpl->port));
}

void test_spiSelectSlave(void){
    uint8_t spi_slave = 1;
    SPIImpl *spiImpl = (SPIImpl *)spi;
    spiImpl->interface.initSPI(spi);
    spiImpl->interface.configureAsSlave(&DDRB, spi_slave, &PORTB);
    spiImpl->interface.selectSlave(&PORTB, spi_slave);
    TEST_ASSERT_BIT_LOW(spi_slave, *(spiImpl->port));
}

void test_spiDeselectSlave(void){
    uint8_t spi_slave = 1;
    SPIImpl *spiImpl = (SPIImpl *)spi;
    spiImpl->interface.initSPI(spi);
    spiImpl->interface.configureAsSlave(&DDRB, spi_slave, &PORTB);
    spiImpl->interface.selectSlave(&PORTB, spi_slave);
    spiImpl->interface.deselectSlave(&PORTB, spi_slave);
    TEST_ASSERT_BIT_HIGH(spi_slave, *(spiImpl->port));
}