/**
 * ModuleCore.cpp
 * @author: Renan Vaz <renan.c.vaz@gmail.com>
 */

#include "ModuleCore.h"

ModuleCore::ModuleCore()
{
}

ModuleCore::~ModuleCore()
{
}

void ModuleCore::setup(String& id, String& type, String& version)
{
  id.toCharArray(Device.ID, 37);
  type.toCharArray(Device.TYPE, 26);
  version.toCharArray(Device.VERSION, 12);

  Data.load();

  _isModeConfig = strcmp(Config.deviceMode, CONFIG) == 0;
  _isModeSlave  = strcmp(Config.deviceMode, SLAVE) == 0;

  if (_isModeSlave) {
    bool error = false;
    uint8_t connectionTries = 0;
    uint8_t maxConnectionTries = 40;
    int16_t localPort;

    IPAddress ip; ip.fromString(Config.serverIp);
    uint16_t port = String(Config.serverPort).toInt();

    #ifdef MODULE_CAN_DEBUG
      Serial.print("Try to connect to: ");
      Serial.println(Config.networkSsid);
      Serial.print("With password: ");
      Serial.println(Config.networkPassword);
    #endif

    WiFi.mode(WIFI_STA);
    WiFi.begin(Config.networkSsid, Config.networkPassword);

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      #ifdef MODULE_CAN_DEBUG
        Serial.print(".");
      #endif

      if (connectionTries++ < maxConnectionTries) {
        delay(500);
      } else {
        error = true;
        break;
      }
    }

    if (!error) {
      #ifdef MODULE_CAN_DEBUG
        Serial.println();
        Serial.print("Connected to: ");
        Serial.println(Config.networkSsid);
      #endif

      _modeSlave = new ModeSlave();
      _modeSlave->setup(ip, port);
    } else {
      #ifdef MODULE_CAN_DEBUG
        Serial.println("Connection failure... Restarting in mode CONFIG...");
      #endif

      strcpy(Config.deviceMode, CONFIG);
      Data.save();
      ESP.restart();
    }
  } else if (_isModeConfig) {
    String ssid = "Module - ";
    ssid += Device.TYPE;
    ssid += " (";
    ssid += Config.deviceName[0] != '\0' ? Config.deviceName : String("Unamed ") + String(random(0xffff), HEX);
    ssid += ")";

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid.c_str(), AP_PASSWORD);

    #ifdef MODULE_CAN_DEBUG
      Serial.print("SSID: ");
      Serial.println(ssid);
      Serial.print("PASS: ");
      Serial.println(AP_PASSWORD);
      Serial.print("Local IP: ");
      Serial.println(WiFi.softAPIP());
    #endif

    _modeConfig = new ModeConfig();
    _modeConfig->setup();
  } else {
    #ifdef MODULE_CAN_DEBUG
      Serial.println("Setup mode Format");
    #endif

    Data.clear();
    Data.load();

    strcpy(Config.deviceMode, CONFIG);

    Data.save();

    #ifdef MODULE_CAN_DEBUG
      Serial.println("Restarting...");
    #endif

    ESP.restart();
  }
}

void ModuleCore::loop()
{
  if (_isModeSlave) {
    _modeSlave->loop();
  } else {
    _modeConfig->loop();
  }
}

bool ModuleCore::isModeSlave()
{
  return _isModeSlave;
}

bool ModuleCore::isModeConfig()
{
  return _isModeConfig;
}

void ModuleCore::send(const char* topic)
{
  StaticJsonBuffer<PACKET_SIZE> jsonBuffer;
  JsonObject& message = jsonBuffer.createObject();
  message["topic"] = topic;

  _modeSlave->send(message);
}

void ModuleCore::send(const char* topic, JsonObject& data)
{
  StaticJsonBuffer<PACKET_SIZE> jsonBuffer;
  JsonObject& message = jsonBuffer.createObject();
  message["topic"] = topic;
  message["data"] = data;

  _modeSlave->send(message);
}

void ModuleCore::send(const char* topic, String& data)
{
  StaticJsonBuffer<PACKET_SIZE> jsonBuffer;
  JsonObject& message = jsonBuffer.createObject();
  message["topic"] = topic;
  message["data"] = data.c_str();

  _modeSlave->send(message);
}

void ModuleCore::send(const char* topic, const char* data)
{
  StaticJsonBuffer<PACKET_SIZE> jsonBuffer;
  JsonObject& message = jsonBuffer.createObject();
  message["topic"] = topic;
  message["data"] = data;

  _modeSlave->send(message);
}

void ModuleCore::on(const char* eventName, std::function<void(JsonObject&, JsonObject&)> cb)
{
  _modeSlave->on(eventName, cb);
}

void ModuleCore::createDefaultAPI()
{
  StaticJsonBuffer<0> b;
  JsonObject& r = b.createObject();

  on("pinMode", [](JsonObject& in, JsonObject& out) {
    uint8_t pin = in["pin"];
    String mode = in["mode"];

    pinMode(pin, mode == "OUTPUT" ? OUTPUT : INPUT);
  });

  on("digitalWrite", [](JsonObject& in, JsonObject& out) {
    uint8_t pin  = in["pin"];
    String level = in["level"];
    Serial.println(pin);

    digitalWrite(pin, level == "1" ? HIGH : LOW);
  });

  on("digitalRead", [](JsonObject& in, JsonObject& out) {
    uint8_t pin    = in["pin"];
    uint8_t level  = digitalRead(pin);

    out["level"] = level;
  });
}
