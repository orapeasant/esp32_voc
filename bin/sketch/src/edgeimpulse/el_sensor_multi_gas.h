#line 1 "/home/ubuntu/proj/ei-projects/esp32-examples/ei-mbed-qcbor-http-acc/src/edgeimpulse/el_sensor_multi_gas.h"
#ifndef _EI_MUTLGASSENSOR_H
#define _EI_MUTLGASSENSOR_H

#include "Multichannel_Gas_GM.h"

typedef float sample_format_t;
#define N_GAS_SAMPLED			4
#define SIZEOF_N_GAS_SAMPLED	(sizeof(sample_format_t) * N_GAS_SAMPLED)

void ei_multigas_init(void);
void getData(float *g0,float *g1,float *g2,float *g3);

#endif
