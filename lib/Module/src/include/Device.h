/**
 * Device.h
 * @author: Renan Vaz <renan.c.vaz@gmail.com>
 */

#ifndef Device_h
#define Device_h

struct DeviceStruct
{
  char ID[37]; // guid: 3438bbe2-0c55-44be-a4f8-36994414eae5
  char TYPE[26]; // Max 25 characters
  char VERSION[12]; // 999.999.999
};

extern DeviceStruct Device;

#endif
