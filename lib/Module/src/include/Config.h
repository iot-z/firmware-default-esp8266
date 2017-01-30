/**
 * Config.h
 * @author: Renan Vaz <renan.c.vaz@gmail.com>
 */

#ifndef Config_h
#define Config_h

#include <Arduino.h>
#include <EEPROM.h>
#include <cstdint>

// EEPROM memory address
#define ADDRESS_CONFIG 0
#define EEPROM_SIZE 512

struct DataStruct
{
  char deviceMode[2];
  char deviceName[33];
  char networkSsid[33];
  char networkPassword[64];
  char serverIp[16]; // uint8_t serverIp[4];
  char serverPort[6]; //uint16_t serverPort;
};

struct ConfigStruct
{
  DataStruct data;

  void load()
  {
    EEPROM.begin(EEPROM_SIZE);

    for (uint16_t i = 0, l = sizeof(data); i < l; i++){
      *((char*)&data + i) = EEPROM.read(ADDRESS_CONFIG + i);
    }

    EEPROM.end();
  }

  void save()
  {
    EEPROM.begin(EEPROM_SIZE);

    for (uint16_t i = 0, l = sizeof(data); i < l; i++){
      EEPROM.write(ADDRESS_CONFIG + i, *((char*)&data + i));
    }

    EEPROM.end();
  }

  void clear()
  {
    #ifdef MODULE_CAN_DEBUG
      Serial.println("Cleaning data...");
    #endif

    EEPROM.begin(EEPROM_SIZE);

    for (uint16_t i = 0; i < EEPROM_SIZE; i++) {
      EEPROM.write(i, '\0');
    }

    for (uint16_t i = 0, l = sizeof(data); i < l; i++){
      *((char*)&data + i) = EEPROM.read(ADDRESS_CONFIG + i);
    }

    EEPROM.end();
  }
};


extern ConfigStruct Config;

#endif
