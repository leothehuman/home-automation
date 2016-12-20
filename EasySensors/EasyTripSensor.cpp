#include "EasyTripSensor.h"

EasyTripSensor::EasyTripSensor(const char* name, uint8_t sensorId, int pin, uint8_t sensorType, int trippedValue, bool pullUp, unsigned long debounce, const char* debugMsg)
  : EasySensor(name)
  , msgTripped(sensorId, V_TRIPPED)
  , sensorPin(pin)
  , type(sensorType)
  , trippedState(trippedValue)
  , debounceDelay(debounce)
  , debugMessage(debugMsg)
  , nextDebounce(0)
{
  pinMode(sensorPin, pullUp ? INPUT_PULLUP : INPUT);
}

void EasyTripSensor::present()
{
  ::present(msgTripped.sensor, type);
}

void EasyTripSensor::process(unsigned long now)
{
  bool newTripped = (digitalRead(sensorPin) == trippedState);

  if (newTripped && tripped)
  {
    nextDebounce = now + debounceDelay;
    return;
  }

  if (newTripped != tripped && nextDebounce < now)
  {
    tripped = newTripped;
#ifdef EASY_DEBUG
    Serial.print(debugMessage);
    Serial.println(tripped ? "1" : "0");
#endif
    send(msgTripped.set(tripped ? "1" : "0"));

    if (newTripped) 
    {
      nextDebounce = now + debounceDelay;
    }
  }
}

bool EasyTripSensor::getTripped() { return tripped; }

EasySoundSensor::EasySoundSensor(const char* name, uint8_t sensorId, int pin, unsigned long debounce)
  : EasyTripSensor(name, sensorId, pin, S_SOUND, LOW, false, debounce, "Sound: ") {}

EasyMotionSensor::EasyMotionSensor(const char* name, uint8_t sensorId, int pin)
  : EasyTripSensor(name, sensorId, pin, S_MOTION, HIGH, false, 0, "Motion: ") {}
