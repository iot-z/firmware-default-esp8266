/**
 * Config.h
 * @author: Renan Vaz <renan.c.vaz@gmail.com>
 */

#ifndef Config_h
#define Config_h

struct ConfigStruct
{
  char deviceMode[2];
  char deviceName[33];
  char ssid[33];
  char password[64];
};

extern ConfigStruct Config;

#endif
