#line 1 "/home/ubuntu/proj/esp32_voc/esp32_voc/src/edgeimpulse/ei_sensor_multi_gas.h"
#ifndef _EI_MUTLGASSENSOR_H
#define _EI_MUTLGASSENSOR_H


typedef float sample_format_t;
#define N_GAS_SAMPLED			4
#define MULTIGAS_SENSOR_ADDR    0x08
#define SIZEOF_N_GAS_SAMPLED	(sizeof(sample_format_t) * N_GAS_SAMPLED)

void getGasData(uint16_t *g0,uint16_t *g1,uint16_t *g2,uint16_t *g3);

#endif
