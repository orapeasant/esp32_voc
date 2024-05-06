/* Include ----------------------------------------------------------------- */
#include <stdint.h>
#include <stdlib.h>
#include "ei_sensor_multi_gas.h"
#include "ei_sensors_utils.h"

#define BOARD_TYPE_ESP32_VOC
//#define BOARD_TYPE_ESP32_WROOM

#ifdef BOARD_TYPE_ESP32_WROOM
#include <Multichannel_Gas_GM.h>

GAS_GM<TwoWire> gas;
#endif

void vocSetup() {
#ifdef BOARD_TYPE_ESP32_WROOM
    gas.begin(Wire, 0x08);
#endif
}

void getGasData(uint16_t *g0,uint16_t *g1,uint16_t *g2,uint16_t *g3)
{

#ifdef BOARD_TYPE_ESP32_VOC
    *g0 = analogReadMilliVolts(4);
    *g1 = analogReadMilliVolts(5);
    *g2 = analogReadMilliVolts(6);
    *g3 = analogReadMilliVolts(7);    
#endif

#ifdef BOARD_TYPE_ESP32_WROOM
    Serial.println("getting BOARD_TYPE_ESP32_WROOM");
    *g0 = gas.getGM102B(); // NO2
    *g1 = gas.getGM302B(); // C2H5CH
    *g2 = gas.getGM502B(); // VOC
    *g3 = gas.getGM702B(); // CO
#endif   

}
