/**
 * ModeConfig.cpp
 * @author: Renan Vaz <renan.c.vaz@gmail.com>
 */

#include "ModeConfig.h"

ModeConfig::ModeConfig()
{
  server = new ESP8266WebServer(80);
}

ModeConfig::~ModeConfig()
{
}

void ModeConfig::setup(const char*ssid, const char* password)
{
  // Init SPIFFS for load the index.html file
  SPIFFS.begin();

  // HTML da pagina principal
  File fileIndex = SPIFFS.open("/index.html", "r");
  File fileSuccess = SPIFFS.open("/success.html", "r");

  if (fileIndex) {
    _htmlRoot = fileIndex.readString();
  } else {
    #ifdef MODULE_CAN_DEBUG
      Serial.println("ERROR on loading \"index.html\" file");
    #endif
  }

  if (fileSuccess) {
    _htmlSuccess = fileSuccess.readString();
  } else {
    #ifdef MODULE_CAN_DEBUG
      Serial.println("ERROR on loading \"success.html\" file");
    #endif
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  #ifdef MODULE_CAN_DEBUG
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("PASS: ");
    Serial.println(password);
    Serial.print("Local IP: ");
    Serial.println(WiFi.softAPIP());
  #endif

  // Start the server
  server->on("/", HTTP_GET, std::bind(&ModeConfig::_handleRootGET, this));
  server->on("/", HTTP_POST, std::bind(&ModeConfig::_handleRootPOST, this));
  server->begin();
}

void ModeConfig::loop()
{
  server->handleClient();
}

void ModeConfig::_handleRootGET()
{
  server->send(200, "text/html", _parseHTML(_htmlRoot));
}

void ModeConfig::_handleRootPOST()
{
  String deviceName = server->arg("device-name");
  String ssid       = server->arg("ssid");
  String password   = server->arg("password");

  #ifdef MODULE_CAN_DEBUG
    Serial.print("Device name: ");
    Serial.println(deviceName);
    Serial.print("SSID:");
    Serial.println(ssid);
    Serial.print("Password:");
    Serial.println(password);
  #endif

  strcpy(Config.deviceName, deviceName.c_str());
  strcpy(Config.ssid, ssid.c_str());
  strcpy(Config.password, password.c_str());
  strcpy(Config.deviceMode, SLAVE);

  Data.save();

  server->send(200, "text/html", _parseHTML(_htmlSuccess));

  #ifdef MODULE_CAN_DEBUG
    Serial.println("Restarting...");
  #endif

  ESP.restart();
}

String ModeConfig::_parseHTML(String html)
{
  html.replace("{{ device-type }}", Device.TYPE);
  html.replace("{{ firmware-version }}", Device.VERSION);
  html.replace("{{ device-id }}", Device.ID);
  html.replace("{{ device-name }}", Config.deviceName);
  html.replace("{{ ssid }}", Config.ssid);
  html.replace("{{ password }}", Config.password);

  return html;
}
