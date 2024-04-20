#line 1 "/home/ubuntu/proj/esp32_voc/esp32_voc/src/edgeimpulse/ei_sensors_utils.h"
#include "Arduino.h"
#include "Wire.h"

//Scan the specified address I2C device to determine whether it is connected.
bool i2c_scanner(uint8_t address, TwoWire &wire = Wire);