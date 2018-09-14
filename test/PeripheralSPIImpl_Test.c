#include "unity.h"
#include "lib/include/Peripheral/PeripheralSPIImpl.h"
#include <memory.h>
#include "lib/src/Peripheral/SpiPinNumbers.h"

#define NUMBER_OF_REGISTERS 5
static uint8_t registers[NUMBER_OF_REGISTERS];
static SPIConfig config;

void setUp(void) {
  uint8_t *registers_local_copy = registers;
  config.control_register = registers_local_copy++;
  config.data_register = registers_local_copy++;
  config.status_register = registers_local_copy++;
  config.io_lines_data_direction_register = registers_local_copy++;
  config.io_lines_data_register = registers_local_copy;
  config.clock_pin = 0;
  config.miso_pin = 1;
  config.mosi_pin = 2;
  memset(registers, 0, NUMBER_OF_REGISTERS);
}

/**
 * TestCheckList of minimum functionality:
 * - configures the clock divider correctly
 * - configures the io line correctly
 * - configures io pin for peripheral
 * - writes correct pin for peripheral select
 * - writes correct pin for peripheral deselect
 *
 */

void test_initSetsSPIEnableBit(void) {
  uint8_t memory[PeripheralInterfaceSPI_requiredSize()];
  PeripheralInterface interface = PeripheralInterfaceSPI_createNew(memory, &config);
  PeripheralInterface_init(interface);

  TEST_ASSERT_BIT_HIGH(spi_enable_bit, *config.control_register);
}

void test_initConfiguresInterfaceAsMaster(void) {
  uint8_t memory[PeripheralInterfaceSPI_requiredSize()];
  PeripheralInterface interface = PeripheralInterfaceSPI_createNew(memory, &config);
  PeripheralInterface_init(interface);

  TEST_ASSERT_BIT_HIGH(master_slave_select_bit, *config.control_register);
}