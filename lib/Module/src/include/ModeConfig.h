/**
 * ModeConfig.h
 * @author: Renan Vaz <renan.c.vaz@gmail.com>
 */

#ifndef ModeConfig_h
#define ModeConfig_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <FS.h>
#include <cstdint>

#include "Modes.h"
#include "Device.h"
#include "Config.h"
#include "Data.h"

class ModeConfig
{
  public:
  ModeConfig();
  ~ModeConfig();

  void setup(const char*ssid, const char* pass);
  void loop();

  private:
  ESP8266WebServer* server;

  // HTML data for config mode
  String _htmlRoot;
  String _htmlSuccess;

  void _handleRootGET();
  void _handleRootPOST();

  String _parseHTML(String html);
};

#endif
