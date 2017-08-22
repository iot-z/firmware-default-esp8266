/**
 * UDPZ.cpp
 * @author: Renan Vaz <renan.c.vaz@gmail.com>
 */

#include "UDPZ.h"

WiFiUDP Udp;
unsigned long now;

UDPZ::UDPZ(const char* id, const char* name, const char* type, const char* version)
{
  _id           = id;
  _name         = name;
  _type         = type;
  _version      = version;
  _isConnected  = false;
}

UDPZ::~UDPZ()
{
}

void UDPZ::_connect()
{
  StaticJsonBuffer<PACKET_SIZE> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["name"]     = _name;
  data["type"]     = _type;
  data["version"]  = _version;

  send("connect", data);
}

void UDPZ::connect(IPAddress ip, uint16_t port)
{
  _ip   = ip;
  _port = port;

  _connect();
}

void UDPZ::reconnect()
{
  _connect();
}

void UDPZ::disconnect()
{
  send("disconnect");
}

bool UDPZ::connected()
{
  return _isConnected;
}

void UDPZ::onConnected(std::function<void()> cb)
{
  _onConnectedCb = cb;
}

void UDPZ::onDisconnected(std::function<void()> cb)
{
  _onDisconnectedCb = cb;
}

void UDPZ::onMessage(std::function<void(const char* topic, JsonObject& inData, JsonObject& outData)> cb)
{
  _onMessageCb = cb;
}

int16_t UDPZ::setup()
{
  uint16_t port      = 4001; // 4000 is the server
  uint16_t portLimit = 5000;

  bool error = false;

  while (Udp.begin(port) != 1) {
    port++;

    if (port > portLimit) {
      return -1;
    }
  }

  return port;
}

void UDPZ::loop()
{
  _packetSize = Udp.parsePacket();

  if (_packetSize) {
    _remoteIP    = Udp.remoteIP();
    _remotePort  = Udp.remotePort();

    if (_remotePort == _port && _remoteIP == _ip) {
      _lastTalkTime = millis();

      Udp.read(_packetBuffer, _packetSize);
      _packetBuffer[_packetSize] = '\0';

      StaticJsonBuffer<PACKET_SIZE> jsonBuffer;
      JsonObject& payload = jsonBuffer.parseObject(_packetBuffer);

      StaticJsonBuffer<PACKET_SIZE> outJsonBuffer;
      JsonObject& outData = outJsonBuffer.createObject();

      if (strcmp(payload["topic"], "ping") == 0) { // Ping
        send(payload["messageId"]);
      } else if (strcmp(payload["topic"], "disconnect") == 0) { // Disconnect
        _isConnected = false;

        _onDisconnectedCb();

        send(payload["messageId"]);
      } else if (strcmp(payload["topic"], "connect") == 0) { // Connect
        _isConnected = true;

        _timeout = payload["data"]["timeout"];

        _onConnectedCb();

        send(payload["messageId"]);
      } else { // Message
        #ifdef MODULE_CAN_DEBUG
        Serial.print(_packetSize);
        Serial.print("B packet received from: ");
        Serial.print(_remoteIP);
        Serial.print(":");
        Serial.println(_remotePort);
        Serial.print("Message: ");
        payload.printTo(Serial);
        Serial.println();
        #endif

        _onMessageCb(payload["topic"], payload["data"], outData);

        send(payload["messageId"], outData);
      }

    }
  } else if (_isConnected) {
    now = millis();

    if (now - _lastTalkTime > _timeout) {
      _isConnected = false;

      _onDisconnectedCb();
    }
  }
}

void UDPZ::send(const char* topic)
{
  StaticJsonBuffer<PACKET_SIZE> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  send(topic, data);
}

void UDPZ::send(const char* topic, JsonObject& data)
{
  StaticJsonBuffer<PACKET_SIZE> jsonBuffer;
  JsonObject& message = jsonBuffer.createObject();
  message["moduleId"] = _id;
  message["topic"]    = topic;
  message["data"]     = data;

  #ifdef MODULE_CAN_DEBUG
    Serial.print("Send message: ");
    message.printTo(Serial);
    Serial.println();
  #endif

  Udp.beginPacket(_ip, _port);
  message.printTo(Udp);
  Udp.println();
  Udp.endPacket();
}
