#ifndef EASYTRIPSENSOR_H
#define EASYTRIPSENSOR_H

#include "EasySensor.h"

class EasyTripSensor : EasySensor
{
public:
  EasyTripSensor(uint8_t sensorId, int pin, uint8_t type, int trippedValue, bool pullUp, unsigned long debounce, const char* debugMsg);
  void process(unsigned long now);
  bool getTripped();
private:
  MyMessage msgTripped;
  int sensorPin;
  int trippedState;
  unsigned long debounceDelay;
  const char* debugMessage;

  bool tripped;
  unsigned long nextDebounce;
};

class EasySoundSensor : public EasyTripSensor
{
public:
  EasySoundSensor(uint8_t sensorId, int pin, unsigned long debounce) : EasyTripSensor(sensorId, pin, S_SOUND, LOW, false, debounce, "Sound: ") {};
};

class EasyMotionSensor : public EasyTripSensor
{
public:
  EasyMotionSensor(uint8_t sensorId, int pin) : EasyTripSensor(sensorId, pin, S_MOTION, HIGH, false, 0, "Motion: ") {};
};

#endif//EASYTRIPSENSOR_H
