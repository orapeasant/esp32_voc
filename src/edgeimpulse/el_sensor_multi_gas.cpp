/* Include ----------------------------------------------------------------- */
#include <stdint.h>
#include <stdlib.h>
#include "el_sensor_multi_gas.h"
#include "ei_sensors_utils.h"

GAS_GM<TwoWire> gasGM;

void ei_multigas_init(void)
{   
    Serial.println("before i2c scan");
    //if(!i2c_scanner(0x08,Wire))
    //    return false;
    Serial.println("after i2c scan");

    gasGM.begin(Wire, 0x08); // use the hardware I2C
    Serial.println("after begin");
    }

void _getData(float *g0,float *g1,float *g2,float *g3)
{

    *g0 = gasGM.getGM102B(); // NO2
    *g1 = gasGM.getGM302B(); // C2H5CH
    *g2 = gasGM.getGM502B(); // VOC
    *g3 = gasGM.getGM702B(); // CO

}
