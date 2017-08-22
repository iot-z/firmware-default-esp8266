/**
 * ModuleCore.cpp
 * @author: Renan Vaz <renan.c.vaz@gmail.com>
 */

#include "ModuleCore.h"


// Add interupts for button press (more performance)
uint8_t resetButtonPin;
long _startPressReset;

void _onReleaseReset()
{
  uint16_t holdTime = (uint16_t) (millis() - _startPressReset);

  detachInterrupt(resetButtonPin);

  if (holdTime > 15000) {
    #ifdef MODULE_CAN_DEBUG
      Serial.println("Long button reset press (15s). Clear data.");
    #endif

    Config.clear();
  } else if (holdTime > 5000) {
    #ifdef MODULE_CAN_DEBUG
      Serial.println("Long button reset press (5s). Config mode.");
    #endif

    strcpy(Config.data.deviceMode, CONFIG);
    Config.save();
  } else {
    #ifdef MODULE_CAN_DEBUG
      Serial.println("Button reset press.");
    #endif
  }

  #ifdef MODULE_CAN_DEBUG
    Serial.println("Restarting...");
  #endif

  ESP.restart();
}

void _onPressReset()
{
  _startPressReset = millis();
  detachInterrupt(resetButtonPin);
  attachInterrupt(resetButtonPin, _onReleaseReset, FALLING);
}

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

  Config.load();

  // Setup button reset to config mode pin
  pinMode(RESET_BUTTON_PIN, INPUT);
  resetButtonPin = RESET_BUTTON_PIN;

  attachInterrupt(RESET_BUTTON_PIN, _onPressReset, RISING);

  _isModeConfig = strcmp(Config.data.deviceMode, CONFIG) == 0;
  _isModeSlave  = strcmp(Config.data.deviceMode, SLAVE) == 0;

  if (_isModeSlave) {
    bool error = false;

    IPAddress ip; ip.fromString(Config.data.serverIp);
    uint16_t port = String(Config.data.serverPort).toInt();

    #ifdef MODULE_CAN_DEBUG
      Serial.print("Try to connect to: ");
      Serial.println(Config.data.networkSsid);
      Serial.print("With password: ");
      Serial.println(Config.data.networkPassword);
    #endif

    WiFi.mode(WIFI_STA);
    WiFi.begin(Config.data.networkSsid, Config.data.networkPassword);

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      #ifdef MODULE_CAN_DEBUG
        Serial.print(".");
      #endif

      delay(500);
    }

    #ifdef MODULE_CAN_DEBUG
      Serial.println();
      Serial.print("Connected to: ");
      Serial.println(Config.data.networkSsid);
    #endif

    _modeSlave = new ModeSlave();
    _modeSlave->setup(ip, port);
  } else if (_isModeConfig) {
    String ssid = "Module - ";
    ssid += Device.TYPE;
    ssid += " (";
    ssid += Config.data.deviceName[0] != '\0' ? Config.data.deviceName : String("Unamed ") + String(random(0xffff), HEX);
    ssid += ")";

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid.c_str(), AP_PASSWORD);

    #ifdef MODULE_CAN_DEBUG
      Serial.print("SSID: ");
      Serial.println(ssid);
      Serial.print("PASS: ");
      Serial.println(AP_PASSWORD);
      Serial.print("Local server address: ");
      Serial.println(WiFi.softAPIP());
    #endif

    _modeConfig = new ModeConfig();
    _modeConfig->setup();
  } else {
    #ifdef MODULE_CAN_DEBUG
      Serial.println("Setup mode Format");
    #endif

    Config.clear(); // Clear EEPROM data
    Config.load(); // Load a clear data

    strcpy(Config.data.deviceMode, CONFIG);

    Config.save();

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
  _modeSlave->send(topic);
}

void ModuleCore::send(const char* topic, JsonObject& data)
{
  _modeSlave->send(topic, data);
}

void ModuleCore::on(const char* eventName, std::function<void(JsonObject& in, JsonObject& out)> cb)
{
  _modeSlave->on(eventName, cb);
}

void ModuleCore::createDefaultAPI()
{
  on("pinMode", [](JsonObject& in, JsonObject& out) {
    uint8_t pin = in["pin"];
    String mode = in["mode"];

    pinMode(pin, mode == "OUTPUT" ? OUTPUT : INPUT);
  });

  on("digitalWrite", [](JsonObject& in, JsonObject& out) {
    uint8_t pin  = in["pin"];
    String level = in["level"];
    Serial.println(pin);
    Serial.println(level);

    digitalWrite(pin, level == "1" ? HIGH : LOW);
  });

  on("digitalRead", [](JsonObject& in, JsonObject& out) {
    uint8_t pin    = in["pin"];
    uint8_t level  = digitalRead(pin);

    out["level"] = level;
  });
}
