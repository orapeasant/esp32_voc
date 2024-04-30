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
#include <Arduino.h>

#include "ei_fusion.h"
#include "ei_sensor_multi_gas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qcbor.h"
#include "nonposix.h"
#include "sensor_aq.h"
#include "sensor_aq_mbedtls_hs256.h"

#if EI_INFERENCING == 1
#include "nut_inferencing.h"
#endif

#include "lg.h"

#include <WiFi.h>
#include <WiFiClnt.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "esp_system.h"


// replace these accordingly
#define API_PATH      "http://ingestion.edgeimpulse.com/api/training/data"
#define API_KEY       "ei_6439c5b0db8ae8e54c767b41ccfe526502d8e36b389a1a6e4d8c950f8aed073d"
#define HMAC_KEY      "7c8f7f289a331c36a2c3e4a52ab449c0"	

#define SAMPLE_TIME   2 // seconds
#define SAMPLE_RATE   2 // Hz

#define INFEURENCE_TIME   2 // seconds
#define INFEURENCE_RATE   2 // Hz


#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

//#define APP_DEBUG
//Blynk related End


/* Private variables ------------------------------------------------------- */
static const bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
static uint16_t data[N_SENSORS];
static int8_t fusion_sensors[N_SENSORS];
static int fusion_ix = 0;
char* gsMACAddress ="";

/** Used sensors value function connected to label name */
eiSensors sensors[] =
{
    "NO2",    &data[0], &poll_IMU, &init_IMU, -1,
    "C2H5CH", &data[1], &poll_IMU, &init_IMU, -1,
    "VOC",    &data[2], &poll_IMU, &init_IMU, -1,
    "CO",     &data[3], &poll_IMU, &init_IMU, -1,
};



char* getMacAddress() {
	uint8_t baseMac[6];
	// Get MAC address for WiFi station
	esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
	char* baseMacChr = "" ;
	sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
	return baseMacChr;
}


//"NO2 + C2H5CH + VOC + CO"

/**
* @brief      Arduino setup function
*/
void fusion_setup()
{
    byte mac[6];
    WiFi.macAddress(mac);
    sscanf(gsMACAddress, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

#if EI_INFERENCING == 1        
    /* Connect used sensors */
    if(ei_connect_fusion_list(EI_CLASSIFIER_FUSION_AXES_STRING) == false) {
        ei_printf("ERR: Errors in sensor list detected\r\n");
        return;
    }
    /* Init & start sensors */

    for(int i = 0; i < fusion_ix; i++) {
        if (sensors[fusion_sensors[i]].status == 0) {
            sensors[fusion_sensors[i]].status = sensors[fusion_sensors[i]].init_sensor();
            if (!sensors[fusion_sensors[i]].status) {
              ei_printf("%s axis sensor initialization failed.\r\n", sensors[fusion_sensors[i]].name);
            }
            else {
              ei_printf("%s axis sensor initialization successful.\r\n", sensors[fusion_sensors[i]].name);
            }
        }
    }
#endif    
}

/**
* @brief      Get data and run inferencing
*/
void fusion_loop()
{
#if EI_INFERENCING == 1    
    ei_printf("\nStarting inferencing in 2 seconds...\r\n");

    delay(2000);

    if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != fusion_ix) {
        ei_printf("ERR: Sensors don't match the sensors required in the model\r\n"
        "Following sensors are required: %s\r\n", EI_CLASSIFIER_FUSION_AXES_STRING);
        return;
    }

    ei_printf("Sampling...\r\n");

    // Allocate a buffer here for the values we'll read from the sensor
    float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };

    for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME) {
        // Determine the next tick (and then sleep later)
        int64_t next_tick = (int64_t)micros() + ((int64_t)EI_CLASSIFIER_INTERVAL_MS * 1000);

        for(int i = 0; i < fusion_ix; i++) {
            if (sensors[fusion_sensors[i]].status == 1) {
                sensors[fusion_sensors[i]].poll_sensor();
                sensors[fusion_sensors[i]].status = 2;
            }
            if (sensors[fusion_sensors[i]].status == 2) {
                buffer[ix + i] = *sensors[fusion_sensors[i]].value;
                //ei_printf("%d %f\n", fusion_sensors[i], buffer[ix + i]);
                sensors[fusion_sensors[i]].status = 1;
            }
        }

        int64_t wait_time = next_tick - (int64_t)micros();

        if(wait_time > 0) {
            delayMicroseconds(wait_time);
        }
    }

    // Turn the raw buffer in a signal which we can the classify
    signal_t signal;
    int err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) {
        ei_printf("ERR:(%d)\r\n", err);
        return;
    }

    // Run the classifier
    ei_impulse_result_t result = { 0 };

    err = run_classifier(&signal, &result, debug_nn);
    if (err != EI_IMPULSE_OK) {
        ei_printf("ERR:(%d)\r\n", err);
        return;
    }

    // print the predictions
    ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.):\r\n",
        result.timing.dsp, result.timing.classification, result.timing.anomaly);
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("%s: %.5f\r\n", result.classification[ix].label, result.classification[ix].value);
    }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("    anomaly score: %.3f\r\n", result.anomaly);
#endif
#endif
}


#if !defined(EI_CLASSIFIER_SENSOR) || (EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_FUSION && EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_ACCELEROMETER)
#error "Invalid model for current sensor"
#endif


/**
 * @brief Go through sensor list to find matching axis name
 *
 * @param axis_name
 * @return int8_t index in sensor list, -1 if axis name is not found
 */
static int8_t ei_find_axis(char *axis_name)
{
#if EI_INFERENCING == 1       
    int ix;
    for(ix = 0; ix < N_SENSORS; ix++) {
        if(strstr(axis_name, sensors[ix].name)) {
            return ix;
        }
    }
#endif    
    return -1;
}

/**
 * @brief Check if requested input list is valid sensor fusion, create sensor buffer
 *
 * @param[in]  input_list      Axes list to sample (ie. "accX + gyrY + magZ")
 * @retval  false if invalid sensor_list
 */
static bool ei_connect_fusion_list(const char *input_list)
{
    bool is_fusion = false;

#if EI_INFERENCING == 1   
    char *buff;


    /* Copy const string in heap mem */
    char *input_string = (char *)ei_malloc(strlen(input_list) + 1);
    if (input_string == NULL) {
        return false;
    }
    memset(input_string, 0, strlen(input_list) + 1);
    strncpy(input_string, input_list, strlen(input_list));

    /* Clear fusion sensor list */
    memset(fusion_sensors, 0, N_SENSORS);
    fusion_ix = 0;

    buff = strtok(input_string, "+");

    while (buff != NULL) { /* Run through buffer */
        int8_t found_axis = 0;

        is_fusion = false;
        found_axis = ei_find_axis(buff);

        if(found_axis >= 0) {
            if(fusion_ix < N_SENSORS) {
                fusion_sensors[fusion_ix++] = found_axis;
                sensors[found_axis].status = 0;
            }
            is_fusion = true;
        }

        buff = strtok(NULL, "+ ");
    }

    ei_free(input_string);
#endif
    return is_fusion;
}


bool init_IMU(void) {
  static bool init_status = false;
  if (!init_status) {
    init_status=true;
    //init_status = lis.isConnection();

    //if(init_status == false) {
    //    ei_printf("Failed to connect to Inertial sensor!\n");
    //    return false;
    //}

#if EI_INFERENCING == 1 
    ei_sleep(100);
#endif
  }
  return init_status;
}

bool init_ADC(void) {
  static bool init_status = false;
  if (!init_status) {
    init_status = true;
  }
  return init_status;
}

uint8_t poll_IMU(void) {

    getGasData(&data[0], &data[1], &data[2],&data[3]);
    //ei_printf("getting gas data %d,%d,%d,%d",data[0],data[1],data[2],data[3]);
    return 0;
}

uint8_t poll_ADC(void) {
    // change to another pin if necessary
    //data[6] = analogRead(A0);
    return 0;
}



void capture_data(){
    //ei_gas_init();
    // The sensor format supports signing the data, set up a signing context
    sensor_aq_signing_ctx_t signing_ctx;

    // We'll use HMAC SHA256 signatures, which can be created through Mbed TLS
    // If you use a different crypto library you can implement your own context
    sensor_aq_mbedtls_hs256_ctx_t hs_ctx;
    // Set up the context, the last argument is the HMAC key
    sensor_aq_init_mbedtls_hs256_context(&signing_ctx, &hs_ctx, HMAC_KEY);

    // Set up the sensor acquisition basic context
    sensor_aq_ctx ctx = {
        // We need a single buffer. The library does not require any dynamic allocation (but your TLS library might)
        { (unsigned char*)malloc(1024), 1024 },

        // Pass in the signing context
        &signing_ctx,

        // And pointers to fwrite and fseek - note that these are pluggable so you can work with them on
        // non-POSIX systems too. Just override the EI_SENSOR_AQ_STREAM macro to your custom file type.
        &ms_fwrite,
        &ms_fseek,
        // if you set the time function this will add 'iat' (issued at) field to the header with the current time
        // if you don't include it, this will be omitted
        NULL
    };
    // Payload header
    sensor_aq_payload_info payload = {
        // Unique device ID (optional), set this to e.g. MAC address or device EUI **if** your device has one
        gsMACAddress,
        // Device type (required), use the same device type for similar devices
        "ESP32-VOC-001",
        // How often new data is sampled in ms. (100Hz = every 10 ms.)
        (float) 1000/SAMPLE_RATE,
        // The axes which you'll use. The units field needs to comply to SenML units (see https://www.iana.org/assignments/senml/senml.xhtml)
        { { "NO2", "ppm" }, { "C2H5CH", "ppm" }, { "VOC", "ppm" }, { "CO", "ppm" } }
        //{ { "NO2", "ppm" }, { "C2H5CH", "ppm" }, { "VOC", "ppm" } }
    };

    // Place to write our data.
    memory_stream_t stream;
    stream.length = 0;
    stream.current_position = 0;
    // Initialize the context, this verifies that all requirements are present
    // it also writes the initial CBOR structure
    int res;
    res = sensor_aq_init(&ctx, &payload, &stream, false);
    if (res != AQ_OK) {
        Serial.printf("sensor_aq_init failed (%d)\n", res);
        while(1);
    }

    lg( "1.");
    // Periodically call `sensor_aq_add_data` (every 10 ms. in this example) to append data
    int16_t values[SAMPLE_TIME * SAMPLE_RATE][4] = { (int16_t)0 }; // 100Hz * 10 seconds
    uint16_t values_ix = 0;
    while(values_ix < SAMPLE_TIME * SAMPLE_RATE){
        uint64_t next_tick = micros() + SAMPLE_RATE * 1000;
        
        uint16_t g0, g1, g2, g3;

        getGasData(&g0, &g1, &g2, &g3);
        Serial.printf("getGasData data (%d,%d,%d,%d)\n", g0,g1,g2,g3);
        values[values_ix][0] = (int16_t)g0;
        values[values_ix][1] = (int16_t)g1;
        values[values_ix][2] = (int16_t)g2;
        values[values_ix][3] = (int16_t)g3;

        values_ix++;

        while (micros() < next_tick) {
            /* blocking loop */
        }
    }

    lg("4.");
    //for (size_t ix = 0; ix < sizeof(values) / sizeof(values[0]); ix++) {
    for (size_t ix = 0; ix< SAMPLE_TIME * SAMPLE_RATE ; ix++) {
        res = sensor_aq_add_data_i16(&ctx, values[ix], 4);
        if (res != AQ_OK) {
            Serial.printf("sensor_aq_add_data failed (%d)\n", res);
            while(1);
        }
    }

    // When you're done call sensor_aq_finish - this will calculate the finalized signature and close the CBOR file
    res = sensor_aq_finish(&ctx);
    if (res != AQ_OK) {
        Serial.printf("sensor_aq_finish failed (%d)\n", res);
        while(1);
    }

    // For convenience we'll print the encoded file. 
    // You can throw this directly in http://cbor.me to decode
    Serial.printf("Encoded file:\n");

    // Print the content of the stream here:
    for (size_t ix = 0; ix < stream.length ; ix++) {
        Serial.printf("%02x ", stream.buffer[ix]);
    }
    Serial.printf("\n");
    /*
     * 
     * Here the binary data stored in the stream object is
     * uploaded to the API
     * 
     */
    lg("5.");
    HTTPClient http;
    WiFiClnt* wifi = new WiFiClnt();
    if(http.begin(*wifi,API_PATH)){
      Serial.println("[HTTP] begin...");
    } else {
      Serial.println("[HTTP] failed...");
    }
    
    http.addHeader("content-type", "application/cbor");
    http.addHeader("x-api-key",  API_KEY);
    http.addHeader("x-file-name", "mbdtest");
    http.addHeader("x-label", "Test");
  
    int httpCode = http.POST(stream.buffer, stream.length);
    
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        Serial.println(payload);
        Serial.println(httpCode);
     } else {
        Serial.printf("[HTTP] failed, error: %d %s\n", httpCode, http.errorToString(httpCode).c_str());
     }
     http.end();
     delay(2000);    
}//capture_data
