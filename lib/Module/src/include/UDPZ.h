/**
 * UDPZ.h
 * @author: Renan Vaz <renan.c.vaz@gmail.com>
 */

#ifndef UDPZ_h
#define UDPZ_h

#include <Arduino.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <cstdint>
#include <functional>

// PACKET BUFFER SIZE
#define PACKET_SIZE 512

 /**
  * Message JSON structure
  * {
  *   "moduleId": "",
  *   "topic": "",
  *   "data": {}
  * }
  */

class UDPZ
{
  public:
    UDPZ(const char* id, const char* name, const char* type, const char* version);
    ~UDPZ();

    int16_t setup();

    void connect(IPAddress, uint16_t);
    void reconnect();
    void disconnect();
    bool connected();
    void loop();
    void send(const char* topic);
    void send(const char* topic, JsonObject& data);
    void onConnected(std::function<void()> cb);
    void onDisconnected(std::function<void()> cb);
    void onMessage(std::function<void(const char* topic, JsonObject& inData, JsonObject& outData)> cb);
  private:
    std::function<void()> _onConnectedCb;
    std::function<void()> _onDisconnectedCb;
    std::function<void(const char* topic, JsonObject& inData, JsonObject& outData)> _onMessageCb;

    const char* _id;
    const char* _name;
    const char* _type;
    const char* _version;

    IPAddress _ip;
    uint16_t _port;
    unsigned long _lastTalkTime;
    bool _isConnected;

    IPAddress _remoteIP;
    uint16_t _remotePort;

    uint16_t _timeout;

    uint16_t _packetSize;
    char _packetBuffer[PACKET_SIZE];

    void _connect();
};

#endif
