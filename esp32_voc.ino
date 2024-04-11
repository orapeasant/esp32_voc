#define USE_ESP32_VOC  true

#define BLYNK_TEMPLATE_ID "TMPL2FssVYEU4"
#define BLYNK_TEMPLATE_NAME "LED ESP32"

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

//#define APP_DEBUG

#define LED_PIN 2  // Use pin 2 for LED (change it, if your board uses another pin)


#include "BlynkEdgent.h"
#include "Multichannel_Gas_GM.h"
#include <Wire.h>

GAS_GM<TwoWire> gas;

boolean gbCaptureData = false;

void get_sensor_data() 
{
    uint8_t len = 0;
    uint8_t addr = 0;
    uint8_t i;
    uint32_t val = 0;

    val = gas.getGM102B(); Serial.print("GM102B: "); Serial.print(val); Serial.print("  =  "); //NO2 
    Serial.print(gas.calcVol(val)); Serial.println("V");
    val = gas.getGM302B(); Serial.print("GM302B: "); Serial.print(val); Serial.print("  =  "); //C2H5CH 
    Serial.print(gas.calcVol(val)); Serial.println("V");
    val = gas.getGM502B(); Serial.print("GM502B: "); Serial.print(val); Serial.print("  =  "); //VOC 
    Serial.print(gas.calcVol(val)); Serial.println("V");
    val = gas.getGM702B(); Serial.print("GM702B: "); Serial.print(val); Serial.print("  =  "); //CO 
    Serial.print(gas.calcVol(val)); Serial.println("V");
    /*
    val = gas.measure_NO2(); Serial.print("NO2: "); Serial.print(val); Serial.print("  =  ");
    Serial.print(gas.calcVol(val)); Serial.println("V");
    val = gas.measure_C2H5OH(); Serial.print("C2H5OH: "); Serial.print(val); Serial.print("  =  ");
    Serial.print(gas.calcVol(val)); Serial.println("V");
    val = gas.measure_VOC(); Serial.print("VOC: "); Serial.print(val); Serial.print("  =  ");
    Serial.print(gas.calcVol(val)); Serial.println("V");
    val = gas.measure_CO(); Serial.print("CO: "); Serial.print(val); Serial.print("  =  ");
    Serial.print(gas.calcVol(val)); Serial.println("V");
    */
    delay(2000);

}


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

void setup()
{
  pinMode(LED_PIN, OUTPUT);

  // Debug console. Make sure you have the same baud rate selected in your serial monitor
  Serial.begin(115200);
  delay(100);

  BlynkEdgent.begin();
  gas.begin(Wire, 0x08); // use the hardware I2C

}

void loop() {
  BlynkEdgent.run();
  delay(10);
  if (gbCaptureData) {
    Serial.println("in capturing...");
    get_sensor_data();
  }
}

