/**
 * ModeConfig.data.cpp
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

void ModeConfig::setup()
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
  String serverIp   = server->arg("server-ip");
  String serverPort = server->arg("server-port");

  #ifdef MODULE_CAN_DEBUG
    Serial.print("Device name: ");
    Serial.println(deviceName);
    Serial.print("SSID:");
    Serial.println(ssid);
    Serial.print("Password:");
    Serial.println(password);
  #endif

  strcpy(Config.data.deviceName, deviceName.c_str());
  strcpy(Config.data.networkSsid, ssid.c_str());
  strcpy(Config.data.networkPassword, password.c_str());
  strcpy(Config.data.serverIp, serverIp.c_str());
  strcpy(Config.data.serverPort, serverPort.c_str());
  strcpy(Config.data.deviceMode, SLAVE);

  Config.save();

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
  html.replace("{{ device-name }}", Config.data.deviceName);
  html.replace("{{ server-ip }}", Config.data.serverIp);
  html.replace("{{ server-port }}", Config.data.serverPort);
  html.replace("{{ ssid }}", Config.data.networkSsid);
  html.replace("{{ password }}", Config.data.networkPassword);

  return html;
}
