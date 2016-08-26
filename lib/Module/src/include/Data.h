/**
 * Data.h
 * @author: Renan Vaz <renan.c.vaz@gmail.com>
 */

#ifndef Data_h
#define Data_h

#include <Arduino.h>
#include <EEPROM.h>
#include <cstdint>

#include "Config.h"

// EEPROM memory address
#define ADDRESS_CONFIG 0
#define EEPROM_SIZE 512

struct DataStruct
{
  void load()
  {
    EEPROM.begin(EEPROM_SIZE);

    for (uint16_t i = 0, l = sizeof(Config); i < l; i++){
      *((char*)&Config + i) = EEPROM.read(ADDRESS_CONFIG + i);
    }

    EEPROM.end();
  }

  void save()
  {
    EEPROM.begin(EEPROM_SIZE);

    for (uint16_t i = 0, l = sizeof(Config); i < l; i++){
      EEPROM.write(ADDRESS_CONFIG + i, *((char*)&Config + i));
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

    for (uint16_t i = 0, l = sizeof(Config); i < l; i++){
      *((char*)&Config + i) = EEPROM.read(ADDRESS_CONFIG + i);
    }

    EEPROM.end();
  }
};

extern DataStruct Data;

#endif
