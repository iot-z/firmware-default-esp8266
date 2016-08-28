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
  char networkSsid[33];
  char networkPassword[64];
  char serverIp[16]; // uint8_t serverIp[4];
  char serverPort[6]; //uint16_t serverPort;
};

extern ConfigStruct Config;

#endif
