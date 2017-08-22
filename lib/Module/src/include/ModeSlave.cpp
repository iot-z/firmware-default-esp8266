/**
 * ModeSlave.cpp
 * @author: Renan Vaz <renan.c.vaz@gmail.com>
 */

#include "ModeSlave.h"

ModeSlave::ModeSlave()
{
  protocol = new UDPZ(Device.ID, Config.data.deviceName, Device.TYPE, Device.VERSION);
}

ModeSlave::~ModeSlave()
{
}

void ModeSlave::send(const char* topic)
{
  protocol->send(topic);
}

void ModeSlave::send(const char* topic, JsonObject& data)
{
  protocol->send(topic, data);
}

void ModeSlave::on(const char* eventName, std::function<void(JsonObject& in, JsonObject& out)> cb)
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
      Serial.print("UDP address assigned on port: ");
      Serial.println(localPort);
    #endif

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

    protocol->onMessage([&](const char* topic, JsonObject& inData, JsonObject& outData){
      _onMessage(topic, inData, outData);
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

void ModeSlave::_onMessage(const char* topic, JsonObject& inData, JsonObject& outData)
{
  int8_t foundIndex = _findEventIndex(topic);

  if (foundIndex != -1) {
    _cbFunctions[foundIndex](inData, outData);
  } else {
    #ifdef MODULE_CAN_DEBUG
      Serial.print("Command not found: ");
      Serial.println((const char*) topic);
    #endif
  }
}
