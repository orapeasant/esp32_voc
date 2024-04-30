#pragma once

#include <WiFi.h>

class WiFiClnt : public WiFiClient {
public:
  void flush() override;
};