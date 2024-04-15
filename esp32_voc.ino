/* Edge Impulse ingestion SDK
 * Copyright (c) 2020 EdgeImpulse Inc.
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

// replace these accordingly
#define API_PATH      "http://ingestion.edgeimpulse.com/api/training/data"
#define API_KEY       "ei_6439c5b0db8ae8e54c767b41ccfe526502d8e36b389a1a6e4d8c950f8aed073d"
#define HMAC_KEY      "7c8f7f289a331c36a2c3e4a52ab449c0"
#define SSID_NET      "Audrialand"
#define PASSWORD      "Adirondax@1234"

#define SAMPLE_TIME   1 // seconds
#define SAMPLE_RATE   1 // Hz

#define USE_ESP32_VOC  true

#define ESP32

//Blynk related Begin: 
#define BLYNK_TEMPLATE_ID "TMPL2FssVYEU4"
#define BLYNK_TEMPLATE_NAME "LED ESP32"

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

//#define APP_DEBUG
//Blynk related End

#define LED_PIN 2  // Use pin 2 for LED (change it, if your board uses another pin)




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qcbor.h"
#include "nonposix.h"
#include "sensor_aq.h"
#include "sensor_aq_mbedtls_hs256.h"
#include "lg.h"

#include "BlynkEdgent.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>

// avoid brownout - use with care
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include <Multichannel_Gas_GMXXX.h>

GAS_GMXXX<TwoWire> gas;

boolean gbCaptureData = false;

// V0 is a datastream used to transfer and store LED switch state.
// Evey time you use the LED switch in the app, this function
// will listen and update the state on device
BLYNK_WRITE(V0)
{
  // Local variable `value` stores the incoming LED switch state (1 or 0)
  // Based on this value, the physical LED on the board will be on or off:
  int value = param.asInt();

  if (value == 1) {
    Serial.print("value =");
    Serial.println(value);
    gbCaptureData=true;
  } else {
    Serial.print("value = ");
    Serial.println(value);
    gbCaptureData=false;
  }
}

void ei_gas_init(void)
{   
    Serial.println("before i2c scan");
    //if(!i2c_scanner(0x08))
    //    return false;
    Serial.println("after i2c scan");

    gas.begin(Wire, 0x08); // use the hardware I2C
    Serial.println("after begin");
}

void getData(uint32_t *g0,uint32_t *g1,uint32_t *g2,uint32_t *g3)
{

    *g0 = gas.getGM102B(); // NO2
    *g1 = gas.getGM302B(); // C2H5CH
    *g2 = gas.getGM502B(); // VOC
    *g3 = gas.getGM702B(); // CO

}


void getGasData(uint16_t *g0,  uint16_t *g1,  uint16_t *g2, uint16_t *g3 ){
    
    uint32_t a,b,c,d ;
   
    getData(&a,&b,&c,&d);

    *g0=a;
    *g1=b;
    *g2=c;
    *g3=c;
}

void post_data(){
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
        "ac:87:a3:0a:2d:1b",
        // Device type (required), use the same device type for similar devices
        "ESP32-VOC-001",
        // How often new data is sampled in ms. (100Hz = every 10 ms.)
        1/SAMPLE_TIME,
        // The axes which you'll use. The units field needs to comply to SenML units (see https://www.iana.org/assignments/senml/senml.xhtml)
        //{ { "GM102", "ppm" }, { "GM302", "ppm" }, { "GM502", "ppm" }, { "GM702", "ppm" } }
        { { "GM102", "ppm" }, { "GM302", "ppm" }, { "GM502", "ppm" } }
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
    int16_t values[SAMPLE_TIME * SAMPLE_RATE][3] = { (int16_t)0 }; // 100Hz * 10 seconds
    uint16_t values_ix = 0;
    lg( "2.");
    while(values_ix < 1000){
        lg( "3.");
        uint64_t next_tick = micros() + SAMPLE_RATE * 1000;
        
        uint16_t g0, g1, g2, g3;

        getGasData(&g0, &g1, &g2, &g3);
        values[values_ix][0] = (int16_t)g0;
        values[values_ix][1] = (int16_t)g1;
        values[values_ix][2] = (int16_t)g2;
        //values[values_ix][3] = (int16_t)g3;

        values_ix++;

        while (micros() < next_tick) {
            /* blocking loop */
        }
    }

    lg("4.");
    for (size_t ix = 0; ix < sizeof(values) / sizeof(values[0]); ix++) {
        res = sensor_aq_add_data_i16(&ctx, values[ix], 3);
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
    if(http.begin(API_PATH)){
      Serial.println("[HTTP] begin...");
    } else {
      Serial.println("[HTTP] failed...");
    }
    
    http.addHeader("content-type", "application/cbor");
    http.addHeader("x-api-key",  API_KEY);
    http.addHeader("x-file-name", "embeddedtest");
    http.addHeader("x-label", "test");
  
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
}     

void get_gas(void) {
    uint8_t len = 0;
    uint8_t addr = 0;
    uint8_t i;
    uint32_t val = 0;

    val = gas.getGM102B(); Serial.print("GM102B: "); Serial.print(val); Serial.print("  =  ");
    Serial.print(gas.calcVol(val)); Serial.println("V");
    val = gas.getGM302B(); Serial.print("GM302B: "); Serial.print(val); Serial.print("  =  ");
    Serial.print(gas.calcVol(val)); Serial.println("V");
    val = gas.getGM502B(); Serial.print("GM502B: "); Serial.print(val); Serial.print("  =  ");
    Serial.print(gas.calcVol(val)); Serial.println("V");
    val = gas.getGM702B(); Serial.print("GM702B: "); Serial.print(val); Serial.print("  =  ");
    Serial.print(gas.calcVol(val)); Serial.println("V");

    delay(2000);    
}

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

    Serial.begin(115200);
    WiFi.begin(SSID_NET, PASSWORD);
    
    Serial.println("Connecting to WiFi..");
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    delay(100);
    BlynkEdgent.begin();

    //ei_gas_init();
    gas.begin(Wire, 0x08);

}

void loop(){
    BlynkEdgent.run();
    delay(10);
    if (gbCaptureData) {
        Serial.println("in capturing...");
        post_data();
        //get_gas();
    }

}
