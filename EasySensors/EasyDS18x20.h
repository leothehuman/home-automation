#ifndef EASYDS18X20_H
#define EASYDS18X20_H

#include "EasySensor.h"
#include <OneWire.h>

class EasyDS18x20 : public EasySensor
{
public:
  EasyDS18x20(uint8_t sensorId, int sensorPin);
  virtual void present();
  virtual void process(unsigned long now);
  bool isOk() const;
  float getTemp() const;
private:
  void init(unsigned long now);

  MyMessage msgTemp;
  OneWire ds;
  uint8_t addr[8];
  uint8_t type;

  float temp;
  unsigned long nextCheck;
};

#endif//EASYDS18X20_H
