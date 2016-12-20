#ifndef EASYTRIPSENSOR_H
#define EASYTRIPSENSOR_H

#include "EasySensor.h"

class EasyTripSensor : public EasySensor
{
public:
  EasyTripSensor(const char* name, uint8_t sensorId, int pin, uint8_t sensorType, int trippedValue, bool pullUp, unsigned long debounce, const char* debugMsg);
  virtual void present();
  virtual void process(unsigned long now);
  bool getTripped();
private:
  MyMessage msgTripped;
  int sensorPin;
  uint8_t type;
  int trippedState;
  unsigned long debounceDelay;
  const char* debugMessage;

  bool tripped;
  unsigned long nextDebounce;
};

class EasySoundSensor : public EasyTripSensor
{
public:
  EasySoundSensor(const char* name, uint8_t sensorId, int pin, unsigned long debounce);
};

class EasyMotionSensor : public EasyTripSensor
{
public:
  EasyMotionSensor(const char* name, uint8_t sensorId, int pin);
};

#endif//EASYTRIPSENSOR_H
