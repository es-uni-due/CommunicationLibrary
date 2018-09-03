#include "unity.h"
#include "lib/src/Mac802154/MRF/MrfIo.h"
#include "lib/include/MockPeripheral.h"
#include "lib/src/Mac802154/MRF/MRFHelperFunctions.h"
#include "lib/src/Mac802154/MRF/MRFInternalConstants.h"

static void captureWriteCallback(PeripheralInterface *interface, PeripheralCallback callback, int number_of_calls);

static PeripheralCallback last_write_callback;

void debug(const uint8_t *message) {}

void test_writeBlockingToLongAddress(void) {
  MrfIo mrf;
  uint8_t size = 2;
  uint8_t payload[size];
  uint16_t address = 19;
  uint8_t command[2] = {MRF_writeLongCommandFirstByte(address),
                        MRF_writeLongCommandSecondByte(address)};
  PeripheralInterface_selectPeripheral_Expect(mrf.interface, mrf.device);
  PeripheralInterface_writeBlocking_ExpectWithArray(mrf.interface, command, 2, 2);
  PeripheralInterface_writeBlocking_ExpectWithArray(mrf.interface, payload, size, size);
  PeripheralInterface_deselectPeripheral_Expect(mrf.interface, mrf.device);
  MrfIo_writeBlockingToLongAddress(&mrf, payload, size, address);
}

void test_writeBlockingToShortAddress(void) {
  MrfIo mrf;
  uint8_t size = 5;
  uint8_t address = 15;
  uint8_t payload[size];
  uint8_t command[1] = {MRF_writeShortCommand(address)};
  PeripheralInterface_selectPeripheral_Expect(mrf.interface, mrf.device);
  PeripheralInterface_writeBlocking_ExpectWithArray(mrf.interface, command, 1, 1);
  PeripheralInterface_writeBlocking_ExpectWithArray(mrf.interface, payload, size, size);
  PeripheralInterface_deselectPeripheral_Expect(mrf.interface, mrf.device);
  MrfIo_writeBlockingToShortAddress(&mrf, payload, size, address);
}

void test_writeNonBlockingToShortAddressCheckingBothCallbacks(void) {
  MrfIo mrf;
  mrf.callback.function = NULL;
  uint8_t size = 20;
  uint8_t payload[size];
  uint8_t address;
  uint8_t command[1] = {MRF_writeShortCommand(address)};
  PeripheralInterface_setWriteCallback_StubWithCallback(captureWriteCallback);

  PeripheralInterface_selectPeripheral_Expect(mrf.interface, mrf.device);
  PeripheralInterface_writeNonBlocking_Expect(mrf.interface, command, 1);
  MrfIo_writeNonBlockingToShortAddress(&mrf, payload, size, address);

  PeripheralInterface_writeNonBlocking_Expect(mrf.interface, payload, size);
  last_write_callback.function(last_write_callback.argument);

  PeripheralInterface_deselectPeripheral_Expect(mrf.interface, mrf.device);
  last_write_callback.function(last_write_callback.argument);

  TEST_ASSERT_NULL(last_write_callback.function);
  TEST_ASSERT_NULL(last_write_callback.argument);
}

void test_writeNonBlockingToLongAddress(void) {
  MrfIo mrf;
  mrf.callback.function = NULL;
  uint8_t payload_size = 13;
  uint8_t payload[payload_size];
  uint16_t address;
  uint8_t command_size = 2;
  uint8_t command[] = {
          MRF_writeLongCommandFirstByte(address),
          MRF_writeLongCommandSecondByte(address)
  };
  PeripheralInterface_setWriteCallback_StubWithCallback(captureWriteCallback);

  PeripheralInterface_selectPeripheral_Expect(mrf.interface, mrf.device);
  PeripheralInterface_writeNonBlocking_Expect(mrf.interface, command, command_size);

  MrfIo_writeNonBlockingToLongAddress(&mrf, payload, payload_size, address);

  PeripheralInterface_writeNonBlocking_Expect(mrf.interface, payload, payload_size);
  last_write_callback.function(last_write_callback.argument);

  PeripheralInterface_deselectPeripheral_Expect(mrf.interface, mrf.device);
  last_write_callback.function(last_write_callback.argument);

  TEST_ASSERT_NULL(last_write_callback.function);
  TEST_ASSERT_NULL(last_write_callback.argument);
}

static void writeCallback(void *arg) {
  *(bool *) arg = true;
}

void test_writeNonBlockingToLongAddressTwoTimesUsingCallback(void) {
  MrfIo mrf;
  uint8_t first_payload_size = 24;
  uint8_t first_payload[first_payload_size];
  uint16_t address;
  uint8_t command_size = 2;
  uint8_t command[] = {
          MRF_writeLongCommandFirstByte(address),
          MRF_writeLongCommandSecondByte(address)
  };
  uint8_t second_payload_size = 16;
  uint8_t second_payload[second_payload_size];
  bool write_callback_called = false;
  MrfIoCallback callback = {
          .function = writeCallback,
          .argument = &write_callback_called,
  };
  MrfIo_setWriteCallback(&mrf, callback);
  PeripheralInterface_setWriteCallback_StubWithCallback(captureWriteCallback);

  PeripheralInterface_selectPeripheral_Expect(mrf.interface, mrf.device);
  PeripheralInterface_writeNonBlocking_Expect(mrf.interface, command, command_size);

  MrfIo_writeNonBlockingToLongAddress(&mrf, first_payload, first_payload_size, address);

  PeripheralInterface_writeNonBlocking_Expect(mrf.interface, first_payload, first_payload_size);
  last_write_callback.function(last_write_callback.argument);

  TEST_ASSERT_FALSE(write_callback_called);
  PeripheralInterface_deselectPeripheral_Expect(mrf.interface, mrf.device);
  last_write_callback.function(last_write_callback.argument);
  TEST_ASSERT_TRUE(write_callback_called);
}

void captureWriteCallback(PeripheralInterface *interface, PeripheralCallback callback, int number_of_calls) {
   last_write_callback = callback;
}

void test_setShortAddress(void){
  MrfIo mrf;
  uint8_t command = MRF_writeShortCommand(mrf_register_software_reset);
  PeripheralInterface_selectPeripheral_Expect(mrf.interface, mrf.device);
  PeripheralInterface_writeBlocking_Expect(mrf.interface, &command, 1);
  uint8_t value = 0xAB;
  PeripheralInterface_writeBlocking_Expect(mrf.interface, &value, 1);
  PeripheralInterface_deselectPeripheral_Expect(mrf.interface, mrf.device);
  MrfIo_setControlRegister(&mrf, mrf_register_software_reset, value);
}

void test_setLongAddress(void) {
  MrfIo mrf;
  uint8_t command[] = {
          MRF_writeLongCommandFirstByte(mrf_register_rf_control6),
          MRF_writeLongCommandSecondByte(mrf_register_rf_control6),
  };
  uint8_t value;
  PeripheralInterface_selectPeripheral_Expect(mrf.interface, mrf.device);
  PeripheralInterface_writeBlocking_ExpectWithArray(mrf.interface, command, 2, 2);
  PeripheralInterface_writeBlocking_Expect(mrf.interface, &value, 1);
  PeripheralInterface_deselectPeripheral_Expect(mrf.interface, mrf.device);
  MrfIo_setControlRegister(&mrf, mrf_register_rf_control6, value);
}

void test_readShortAddressControlRegister(void) {
  MrfIo mrf;
  uint8_t command = MRF_readShortCommand(mrf_register_interrupt_status);
  PeripheralInterface_selectPeripheral_Expect(mrf.interface, mrf.device);
  PeripheralInterface_writeBlocking_Expect(mrf.interface, &command, 1);
  uint8_t value = 0xAB;
  PeripheralInterface_readBlocking_Expect(mrf.interface, &value, 1);
  PeripheralInterface_readBlocking_IgnoreArg_buffer();
  PeripheralInterface_readBlocking_ReturnThruPtr_buffer(&value);
  PeripheralInterface_deselectPeripheral_Expect(mrf.interface, mrf.device);
  uint8_t expected = MrfIo_readControlRegister(&mrf, mrf_register_interrupt_status);
  TEST_ASSERT_EQUAL_HEX8(expected, value);
}

void test_readLongAddressControlRegister(void) {
  MrfIo mrf;
  uint8_t command[] = {MRF_readLongCommandFirstByte(mrf_register_rf_control6),
          MRF_readLongCommandSecondByte(mrf_register_rf_control6)};
  PeripheralInterface_selectPeripheral_Expect(mrf.interface, mrf.device);
  PeripheralInterface_writeBlocking_ExpectWithArray(mrf.interface, command, 2, 2);
  uint8_t value = 0xAB;
  PeripheralInterface_readBlocking_Expect(mrf.interface, &value, 1);
  PeripheralInterface_readBlocking_IgnoreArg_buffer();
  PeripheralInterface_readBlocking_ReturnThruPtr_buffer(&value);
  PeripheralInterface_deselectPeripheral_Expect(mrf.interface, mrf.device);
  uint8_t expected = MrfIo_readControlRegister(&mrf, mrf_register_rf_control6);
  TEST_ASSERT_EQUAL_HEX8(expected, value);
}