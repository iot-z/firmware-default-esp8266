/**
 * ModeSlave.cpp
 * @author: Renan Vaz <renan.c.vaz@gmail.com>
 */

#include "ModeSlave.h"

// Add interupts for button press (more performance)
uint8_t resetButtonPin;
long _startPressReset;

void _onReleaseReset(){
  uint16_t holdTime = (uint16_t) (millis() - _startPressReset);

  detachInterrupt(resetButtonPin);

  if (holdTime > 5000) {
    #ifdef MODULE_CAN_DEBUG
      Serial.println("Long button reset press.");
    #endif

    Data.clear();
  } else {
    #ifdef MODULE_CAN_DEBUG
      Serial.println("Button reset press.");
    #endif
  }

  strcpy(Config.deviceMode, CONFIG);
  Data.save();

  #ifdef MODULE_CAN_DEBUG
    Serial.println("Restarting...");
  #endif

  ESP.restart();
}

void _onPressReset(){
  _startPressReset = millis();
  detachInterrupt(resetButtonPin);
  attachInterrupt(resetButtonPin, _onReleaseReset, FALLING);
}

ModeSlave::ModeSlave()
{
  protocol = new UDPZ(Device.ID, Config.deviceName, Device.TYPE, Device.VERSION);
}

ModeSlave::~ModeSlave()
{
}

void ModeSlave::send(JsonObject& data)
{
  protocol->send(data);
}

void ModeSlave::on(const char* eventName, std::function<void(JsonObject&, JsonObject&)> cb)
{
  int8_t foundIndex = _findEventIndex(eventName);

  if (foundIndex == -1) {
    if (_cbIndex < MAX_CALLBACKS) {
      _cbNames[_cbIndex] = eventName;
      _cbFunctions[_cbIndex] = cb;

      _cbIndex++;

      #ifdef MODULE_CAN_DEBUG
        Serial.print("Command created: ");
        Serial.println(eventName);
      #endif
    } else {
      #ifdef MODULE_CAN_DEBUG
        Serial.print("The callbacks limit has been reached: ");
        Serial.println(MAX_CALLBACKS);
      #endif
    }
  } else {
    #ifdef MODULE_CAN_DEBUG
      Serial.print("Cannot override command: ");
      Serial.println(eventName);
    #endif
  }
}

int8_t ModeSlave::_findEventIndex(const char* eventName)
{
  for (uint8_t i = 0; i < _cbIndex; i++) {
    if (strcmp(_cbNames[i], eventName) == 0) {
      return i;
    }
  }

  return -1;
}

void ModeSlave::setup(IPAddress ip, uint16_t port)
{
  uint16_t localPort = protocol->setup();

  if (localPort != -1) {
    #ifdef MODULE_CAN_DEBUG
      Serial.print("UDP connection successful on port: ");
      Serial.println(localPort);
    #endif

    // Setup button reset to config mode pin
    pinMode(RESET_BUTTON_PIN, INPUT);
    resetButtonPin = RESET_BUTTON_PIN;

    attachInterrupt(RESET_BUTTON_PIN, _onPressReset, RISING);

    protocol->onConnected([&](){
      #ifdef MODULE_CAN_DEBUG
        Serial.println("Connected to the server");
      #endif
    });

    protocol->onDisconnected([&](){
      _lastConnectionTry = millis();

      #ifdef MODULE_CAN_DEBUG
        Serial.println("Disconnected from the server");
      #endif
    });

    protocol->onMessage([&](JsonObject& payload){
      _onMessage(payload);
    });

    _lastConnectionTry = millis();

    protocol->connect(ip, port);
  } else {
    #ifdef MODULE_CAN_DEBUG
      Serial.println("UDP Connection failed");
    #endif
  }
}

void ModeSlave::loop()
{
  protocol->loop();

  if (!protocol->connected()) {
    unsigned long now = millis();

    if ((now - _lastConnectionTry) > RECONNECT_DELAY) {
      _lastConnectionTry = now;

      protocol->reconnect();
    }
  }
}

void ModeSlave::_onMessage(JsonObject& payload)
{
  int8_t foundIndex = _findEventIndex(payload["topic"]);

  if (foundIndex != -1) {
    StaticJsonBuffer<PACKET_SIZE> jsonBuffer;
    JsonObject& out = jsonBuffer.createObject();
    _cbFunctions[foundIndex](payload["data"], out);

    if (payload.containsKey("messageId")) {
      StaticJsonBuffer<PACKET_SIZE> jsonBuffer;
      JsonObject& message = jsonBuffer.createObject();
      message["topic"] = payload["messageId"];
      message["data"] = out;

      send(message);
    }
  } else {
    #ifdef MODULE_CAN_DEBUG
      Serial.print("Command not found: ");
      Serial.println((const char *) payload["topic"]);
    #endif
  }
}
