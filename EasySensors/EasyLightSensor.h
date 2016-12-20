#ifndef EASYLIGHTSENSOR_H
#define EASYLIGHTSENSOR_H

#include "EasySensor.h"

class EasyLightSensor : public EasySensor
{
public:
  EasyLightSensor(uint8_t sensorId, int pin);
  virtual void present();
  virtual void process(unsigned long now);
  uint8_t getLevel();
private:
  MyMessage msgLightLevel; 
  int sensorPin;
  uint8_t level;
  unsigned long nextCheck;
};

#endif//EASYLIGHTSENSOR_H

