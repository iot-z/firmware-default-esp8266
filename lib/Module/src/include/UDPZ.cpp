/**
 * UDPZ.cpp
 * @author: Renan Vaz <renan.c.vaz@gmail.com>
 */

#include "UDPZ.h"

WiFiUDP Udp;
unsigned long now;

UDPZ::UDPZ()
{
  _isConnected = false;
}

UDPZ::~UDPZ()
{
}

void UDPZ::connect(const char* id, IPAddress ip, uint16_t port)
{
  _id   = id;
  _ip   = ip;
  _port = port;

  send("connect", _id);
}

void UDPZ::reconnect()
{
  send("connect", _id);
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

void UDPZ::onMessage(std::function<void(JsonObject& params)> cb)
{
  _onMessageCb = cb;
}

int16_t UDPZ::setup()
{
  uint16_t port      = 4000;
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
    now = millis();

    Udp.read(_packetBuffer, _packetSize);
    _packetBuffer[_packetSize] = '\0';
    _lastTalkTime = now;

    StaticJsonBuffer<PACKET_SIZE> jsonBuffer;
    JsonObject& params = jsonBuffer.parseObject(_packetBuffer);

    if (strcmp(params["topic"], "ping") == 0) { // Ping
      send("ping");
    } else if (strcmp(params["topic"], "disconnect") == 0) { // Disconnect
      _isConnected = false;

      _onDisconnectedCb();
    } else if (strcmp(params["topic"], "connect") == 0) { // Connect
      _isConnected = true;

      _onConnectedCb();
    } else { // Message
      #ifdef MODULE_CAN_DEBUG
      _remoteIP    = Udp.remoteIP();
      _remotePort  = Udp.remotePort();

      Serial.print(_packetSize);
      Serial.print("B packet received from: ");
      Serial.print(_remoteIP);
      Serial.print(":");
      Serial.println(_remotePort);
      Serial.print("Message: ");
      params.printTo(Serial);
      Serial.println();
      #endif

      _onMessageCb(params);
    }
  } else if (_isConnected) {
    now = millis();

    if (now - _lastTalkTime > TIMEOUT) {
      _isConnected = false;

      _onDisconnectedCb();
    }
  }
}

void UDPZ::send(const char* topic)
{
    send(topic, "");
}

void UDPZ::send(const char* topic, const char* data)
{
  StaticJsonBuffer<PACKET_SIZE> jsonBuffer;
  JsonObject& message = jsonBuffer.createObject();
  message["topic"] = topic;
  message["data"] = data;

  String v;
  message.printTo(v);

  #ifdef MODULE_CAN_DEBUG
  if (strcmp(topic, "ping") != 0 && strcmp(topic, "connect") != 0 && strcmp(topic, "disconnect") != 0) {
    Serial.print("Send message: ");
    message.printTo(Serial);
    Serial.println();
  }
  #endif

  Udp.beginPacket(_ip, _port);
  message.printTo(Udp);
  Udp.println();
  Udp.endPacket();
}
