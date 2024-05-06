#line 1 "/home/ubuntu/proj/esp32_voc/esp32_voc/src/edgeimpulse/ei_fusion.h"

/* Includes ---------------------------------------------------------------- */
#ifndef _EI_FUSION_H
#define _EI_FUSION_H

#include "ei_const.h"

//#define EI_CLASSIFIER_SENSOR EI_CLASSIFIER_SENSOR_ACCELEROMETER
//#define EI_INFERENCING  1


//#define SAMPLE_TIME   3 // seconds
//#define SAMPLE_RATE   20 // Hz  1000/20


/** Struct to link sensor axis name to sensor value function */
typedef struct{
    const char *name;
    uint16_t *value;
    uint8_t (*poll_sensor)(void);
    bool (*init_sensor)(void);
    int8_t status;  // -1 not used 0 used(unitialized) 1 used(initalized) 2 data sampled
} eiSensors;

/* Constant defines -------------------------------------------------------- */
/** Number sensor axes used */
#define N_SENSORS 4

#define MULTIGAS_SENSOR_ADDR 0x08

/* Forward declarations ------------------------------------------------------- */
static bool ei_connect_fusion_list(const char *input_list);

bool init_IMU(void);
bool init_ADC(void);
uint8_t poll_IMU(void);
uint8_t poll_ADC(void);

/**
* @brief      Arduino setup function
*/
void fusion_setup(bool debug = false);

/**
* @brief      Get data and run inferencing
*/
void fusion_loop(bool debug = false);

void capture_data(bool debug = false);

#endif