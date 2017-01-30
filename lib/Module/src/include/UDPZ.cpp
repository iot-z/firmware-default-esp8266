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
  JsonObject& message = jsonBuffer.createObject();
  message["topic"] = "connect";

  JsonObject& data = message.createNestedObject("data");
  data["name"] = _name;
  data["type"] = _type;
  data["version"] = _version;

  send(message);
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
  StaticJsonBuffer<PACKET_SIZE> jsonBuffer;
  JsonObject& message = jsonBuffer.createObject();
  message["topic"] = "disconnect";

  send(message);
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

void UDPZ::onMessage(std::function<void(JsonObject& message)> cb)
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
      now = millis();

      Udp.read(_packetBuffer, _packetSize);
      _packetBuffer[_packetSize] = '\0';
      _lastTalkTime = now;

      StaticJsonBuffer<PACKET_SIZE> jsonBuffer;
      JsonObject& message = jsonBuffer.parseObject(_packetBuffer);

      if (strcmp(message["topic"], "ping") == 0) { // Ping
        StaticJsonBuffer<PACKET_SIZE> jsonBuffer;
        JsonObject& messagePing = jsonBuffer.createObject();
        messagePing["topic"] = "ping";

        send(messagePing);
      } else if (strcmp(message["topic"], "disconnect") == 0) { // Disconnect
        _isConnected = false;

        _onDisconnectedCb();
      } else if (strcmp(message["topic"], "connect") == 0) { // Connect
        _isConnected = true;

        _onConnectedCb();
      } else { // Message
        #ifdef MODULE_CAN_DEBUG
        Serial.print(_packetSize);
        Serial.print("B packet received from: ");
        Serial.print(remoteIP);
        Serial.print(":");
        Serial.println(remotePort);
        Serial.print("Message: ");
        message.printTo(Serial);
        Serial.println();
        #endif

        _onMessageCb(message);
      }
    }
  } else if (_isConnected) {
    now = millis();

    if (now - _lastTalkTime > TIMEOUT) {
      _isConnected = false;

      _onDisconnectedCb();
    }
  }
}

void UDPZ::send(JsonObject& message)
{
  message["module"] = _id;

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
