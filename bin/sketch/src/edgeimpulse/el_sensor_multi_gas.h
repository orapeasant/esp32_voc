#line 1 "/home/ubuntu/proj/esp32_voc/esp32_voc/src/edgeimpulse/el_sensor_multi_gas.h"
#ifndef _EI_MUTLGASSENSOR_H
#define _EI_MUTLGASSENSOR_H

#include "Multichannel_Gas_GM.h"

typedef float sample_format_t;
#define N_GAS_SAMPLED			4
#define SIZEOF_N_GAS_SAMPLED	(sizeof(sample_format_t) * N_GAS_SAMPLED)

void ei_multigas_init(void);
void getGasData(uint16_t *g0,uint16_t *g1,uint16_t *g2,uint16_t *g3);

#endif
