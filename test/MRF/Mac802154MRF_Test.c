#include <string.h>
#include <stdlib.h>
#include "lib/include/Mac802154MRFImpl.h"
#include "test/Mocks/RuntimeLibraryImplMock.h"
#include "lib/src/Mac802154/MRF/MRFInternalConstants.h"

#include "unity.h"
#include "lib/src/Mac802154/MRF/MockMRFHelperFunctions.h"
#include "lib/src/Mac802154/MRF/MockMRFState.h"
#include "test/MRF/MockMac802154MRF_TestHelper.h"
#include "lib/src/Mac802154/MRF/MockMrfIo.h"
#include "lib/src/Mac802154/MRF/Mrf.h"

MemoryManagement *inspected_memory;

Peripheral *device;
PeripheralInterface *interface;
Mac802154 *mrf;
Mac802154Config mrf_config;

void setUp(void) {
  inspected_memory = MemoryManagement_createMockImpl();

  mrf_config.interface = interface;
  mrf_config.device = device;
  mrf_config.pan_id = 0;
  mrf_config.extended_source_address = 0;
  mrf_config.short_source_address = 0;
  mrf_config.channel = 11;
  mrf = Mac802154_createMRF(inspected_memory, fakeDelay);
}

void tearDown(void) {
  Mac802154_destroy(mrf);
  TEST_ASSERT_EQUAL(0, MockMemoryManagement_numberOfAllocatedObjects(inspected_memory));
}

static void setUpInitializationValues(MrfIo *impl, const Mac802154Config *config);

void test_channelSelectionRegisterValueIsCalculatedCorrectly(void) {
  TEST_ASSERT_EQUAL_UINT8(0x23, MRF_getRegisterValueForChannelNumber(13));
  TEST_ASSERT_EQUAL_UINT8(0x43, MRF_getRegisterValueForChannelNumber(15));
  TEST_ASSERT_EQUAL_UINT8(0xF3, MRF_getRegisterValueForChannelNumber(26));
}

void test_initPerformsSetupLikeShownInDatasheet(void) {
  Mrf *impl = (Mrf *) mrf;
  setUpInitializationValues(&impl->io, &mrf_config);
  MrfState_init_Expect(&impl->state);
  MrfState_setPanId_Expect(&impl->state, mrf_config.pan_id);
  MrfState_setShortSourceAddress_Expect(&impl->state, mrf_config.short_source_address);
  uint64_t coordinators_address = 0;
  MrfState_setExtendedDestinationAddress_Expect(&impl->state, coordinators_address);

  Mac802154_init(mrf, &mrf_config);
  TEST_ASSERT_EQUAL_PTR(impl->io.device, device);
  TEST_ASSERT_EQUAL_PTR(impl->io.interface, interface);
}

void test_initWithDifferentConfig(void) {
  mrf_config.extended_source_address = 0xFFFFABABCDCD1234;
  mrf_config.short_source_address = 0xFFAB;
  mrf_config.channel = 22;
  mrf_config.pan_id = 0xABCD;
  Mrf *impl = (Mrf *) mrf;

  setUpInitializationValues(&impl->io, &mrf_config);
  MrfState_init_Expect(&impl->state);
  MrfState_setPanId_Expect(&impl->state, mrf_config.pan_id);
  MrfState_setShortSourceAddress_Expect(&impl->state, mrf_config.short_source_address);
  uint64_t coordinators_address = 0;
  MrfState_setExtendedDestinationAddress_Expect(&impl->state, coordinators_address);

  Mac802154_init(mrf, &mrf_config);
}


void setUpInitializationValues(MrfIo *impl, const Mac802154Config *config) {
  MrfIo_setControlRegister_Expect(impl, mrf_register_software_reset, mrf_value_full_software_reset);
  MrfIo_setControlRegister_Expect(impl, mrf_register_power_amplifier_control2, (1 << mrf_bit_fifo_enable) | mrf_value_recommended_transmitter_on_time_before_beginning_a_packet);
  MrfIo_setControlRegister_Expect(impl, mrf_register_tx_stabilization, mrf_value_recommended_interframe_spacing);
  MrfIo_setControlRegister_Expect(impl, mrf_register_rf_control0, mrf_value_recommended_rf_optimize_control0);
  MrfIo_setControlRegister_Expect(impl, mrf_register_rf_control1, mrf_value_recommended_rf_optimize_control1);
  MrfIo_setControlRegister_Expect(impl, mrf_register_rf_control2, mrf_value_phase_locked_loop_enabled);
  MrfIo_setControlRegister_Expect(impl, mrf_register_rf_control6, mrf_value_enable_tx_filter | mrf_value_20MHz_clock_recovery_less_than_1ms);
  MrfIo_setControlRegister_Expect(impl, mrf_register_rf_control7, mrf_value_use_internal_100kHz_oscillator);
  MrfIo_setControlRegister_Expect(impl, mrf_register_sleep_clock_control1, mrf_value_disable_deprecated_clkout_sleep_clock_feature | mrf_value_minimum_sleep_clock_divisor_for_internal_oscillator);
  MrfIo_setControlRegister_Expect(impl, mrf_register_base_band2, mrf_value_clear_channel_assessment_energy_detection_only);
  MrfIo_setControlRegister_Expect(impl, mrf_register_energy_detection_threshold_for_clear_channel_assessment, mrf_value_recommended_energy_detection_threshold);
  MrfIo_setControlRegister_Expect(impl, mrf_register_base_band6, mrf_value_append_rssi_value_to_rxfifo);
  MrfIo_setControlRegister_Expect(impl, mrf_register_interrupt_control, mrf_value_rx_interrupt_enabled);

  // select channel 11, afterwards the rf state machine should be reset
  MrfIo_setControlRegister_Expect(impl, mrf_register_rf_control0, MRF_getRegisterValueForChannelNumber(config->channel));
  MrfIo_setControlRegister_Expect(impl, mrf_register_rf_mode_control, mrf_value_rf_state_machine_reset_state);
  MrfIo_setControlRegister_Expect(impl, mrf_register_rf_mode_control, mrf_value_rf_state_machine_operating_state);

  fakeDelay_Expect(mrf_value_delay_interval_after_state_machine_reset);

  // set transmitter power to -30dB
  MrfIo_setControlRegister_Expect(impl, mrf_register_rf_control3, mrf_value_transmitter_power_minus30dB);

  // here the addresses are required to be stored in ascending byte order (big endian)
  MrfIo_writeBlockingToShortAddress_Expect(impl, (uint8_t *)&config->short_source_address, 2, mrf_register_short_address_low_byte);
  MrfIo_writeBlockingToShortAddress_Expect(impl, (uint8_t *)&config->extended_source_address, 8, mrf_register_extended_address0);
  MrfIo_writeBlockingToShortAddress_Expect(impl, (uint8_t *)&config->pan_id, 2, mrf_register_pan_id_low_byte);
}

void test_sendBlocking(void) {
  Mrf *impl = (Mrf *) mrf;
  uint8_t payload[] = "hello, world!";
  uint8_t payload_length = (uint8_t) strlen((const char *) payload);

  MrfState_setPayload_Expect(NULL, payload, payload_length);
  MrfState_setPayload_IgnoreArg_mrf();

  Mac802154_setPayload(mrf, payload, payload_length);

  uint8_t fake_header_data[] = "123456789";
  uint8_t fake_header_length = 9;
  uint8_t fake_header_memory_address = 0;
  MrfField full_header = {
          .data = fake_header_data,
          .size = fake_header_length,
          .address = fake_header_memory_address,
  };
  MrfState_getFullHeaderField_ExpectAnyArgsAndReturn(full_header);

  MrfField payload_field = {
          .data = payload,
          .size = payload_length,
          .address = fake_header_length,
  };
  MrfState_getPayloadField_ExpectAnyArgsAndReturn(payload_field);

  MrfIo_writeBlockingToLongAddress_Expect(NULL, full_header.data, full_header.size, full_header.address);
  MrfIo_writeBlockingToLongAddress_IgnoreArg_mrf();

  MrfIo_writeBlockingToLongAddress_Expect(NULL, payload_field.data, payload_field.size, payload_field.address);
  MrfIo_writeBlockingToLongAddress_IgnoreArg_mrf();

  MrfIo_setControlRegister_Expect(&impl->io, mrf_register_tx_normal_fifo_control, 1);

  Mac802154_sendBlocking(mrf);
}
