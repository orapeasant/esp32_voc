#line 1 "/home/ubuntu/proj/esp32_voc/esp32_voc/src/edgeimpulse/ei_sensor_multi_gas.cpp"
/* Include ----------------------------------------------------------------- */
#include <stdint.h>
#include <stdlib.h>
#include "ei_sensor_multi_gas.h"
#include "ei_sensors_utils.h"


void getGasData(uint16_t *g0,uint16_t *g1,uint16_t *g2,uint16_t *g3)
{
    *g0 = analogReadMilliVolts(4);
    *g1 = analogReadMilliVolts(5);
    *g2 = analogReadMilliVolts(6);
    *g3 = analogReadMilliVolts(7);

}
