#line 1 "/home/ubuntu/proj/esp32_voc/esp32_voc/src/edgeimpulse/WiFiClnt.h"
#pragma once

#include <WiFi.h>

class WiFiClnt : public WiFiClient {
public:
  void flush() override;
};