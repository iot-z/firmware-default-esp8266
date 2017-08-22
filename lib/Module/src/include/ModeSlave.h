/**
 * ModeSlave.h
 * @author: Renan Vaz <renan.c.vaz@gmail.com>
 */

#ifndef ModeSlave_h
#define ModeSlave_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <cstdint>

#include "Modes.h"
#include "Device.h"
#include "Config.h"
#include "UDPZ.h"

// Limit of binded commands
#define MAX_CALLBACKS 100

// Access point info
#define RECONNECT_DELAY 500

class ModeSlave
{
  public:
  ModeSlave();
  ~ModeSlave();

  uint8_t RESET_BUTTON_PIN = D1;
  uint8_t LED_STATUS_PIN = LED_BUILTIN;

  void setup(IPAddress ip, uint16_t port);
  void loop();

  void send(const char* topic);
  void send(const char* topic, JsonObject& data);

  void on(const char* eventName, std::function<void(JsonObject&, JsonObject&)> cb);

  void createDefaultAPI();

  private:
  UDPZ* protocol;
  unsigned long _startPressReset;
  unsigned long _lastConnectionTry = 0;

  uint8_t _cbIndex = 0;
  const char* _cbNames[MAX_CALLBACKS];
  std::function<void(JsonObject& in, JsonObject& out)> _cbFunctions[MAX_CALLBACKS];

  void _onMessage(const char* topic, JsonObject& in, JsonObject& out);
  int8_t _findEventIndex(const char* eventName);
};

#endif
