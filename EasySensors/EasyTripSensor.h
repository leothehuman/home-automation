#ifndef EASYTRIPSENSOR_H
#define EASYTRIPSENSOR_H

#include "EasySensor.h"

class EasyTripSensor : public EasySensor
{
public:
  EasyTripSensor(uint8_t sensorId, int sensorPin, uint8_t sensorType, int trippedValue, bool pullUp, unsigned long debounce, const char* debugMsg);
  virtual void present();
  virtual void process(unsigned long now);
  bool getTripped();
private:
  MyMessage msgTripped;
  int pin;
  uint8_t type;
  int trippedState;
  unsigned long debounceDelay;
  const char* debugMessage;

  bool tripped;
  unsigned long nextDebounce;
};

class EasyMotionSensor : public EasyTripSensor
{
public:
  EasyMotionSensor(uint8_t sensorId, int sensorPin);
};

class EasyDoorSensor : public EasyTripSensor
{
public:
  EasyDoorSensor(uint8_t sensorId, int sensorPin);
};

class EasySoundSensor : public EasyTripSensor
{
public:
  EasySoundSensor(uint8_t sensorId, int sensorPin, unsigned long debounce);
};

#endif//EASYTRIPSENSOR_H
