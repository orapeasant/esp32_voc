#define SSID_NET      "Audrialand"
#define PASSWORD      "Adirondax@1234"

#define BOARD_TYPE_ESP32_VOC
//#define BOARD_TYPE_ESP32_WROOM
#define USE_ESP32_VOC  true

#define ESP32
//Blynk related Begin: 
#define BLYNK_TEMPLATE_ID           "TMPL2nFwFHGwJ"
#define BLYNK_TEMPLATE_NAME         "Quickstart Template"
#define BLYNK_AUTH_TOKEN            "yuxh2-g6cNgI0hQrYJndjftoxtllCLwB"

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

//#define APP_DEBUG
//Blynk related End

#define LED_PIN 2  // Use pin 2 for LED (change it, if your board uses another pin)

// avoid brownout - use with care
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"


#include <BlynkSimpleEsp32.h>

#include "ei_fusion.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>



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


void setup() {
    //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

    Serial.begin(115200);
    /*
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
    */
    Blynk.begin(BLYNK_AUTH_TOKEN, SSID_NET, PASSWORD);
    //BlynkEdgent.begin();

    //byte mac[6];
    //WiFi.macAddress(mac);
    //sscanf(gsMACAddress, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
    


}

void loop(){
    Serial.println("in loop...");
    //BlynkEdgent.run();
    Blynk.run();
    delay(1000);
    if (gbCaptureData) {
        Serial.println("in capturing...");
        delay(1000);
        capture_data();
    } else {
       Serial.println("in infuerencing...");
       delay(1000);
       fusion_loop();
    }

}
