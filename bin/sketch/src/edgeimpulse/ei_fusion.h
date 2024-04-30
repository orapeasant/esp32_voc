#line 1 "/home/ubuntu/proj/esp32_voc/esp32_voc/src/edgeimpulse/ei_fusion.h"
/* Edge Impulse Arduino examples
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Includes ---------------------------------------------------------------- */
#ifndef _EI_FUSION_H
#define _EI_FUSION_H

#define EI_CLASSIFIER_SENSOR EI_CLASSIFIER_SENSOR_ACCELEROMETER
#define EI_INFERENCING  0

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
void fusion_setup();

/**
* @brief      Get data and run inferencing
*/
void fusion_loop();

void capture_data();

#endif