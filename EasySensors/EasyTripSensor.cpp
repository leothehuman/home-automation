#include "EasyTripSensor.h"

EasyTripSensor::EasyTripSensor(uint8_t sensorId, int sensorPin, uint8_t sensorType, int trippedValue, bool pullUp, unsigned long debounce, const char* debugMsg)
  : msgTripped(sensorId, V_TRIPPED)
  , pin(sensorPin)
  , type(sensorType)
  , trippedState(trippedValue)
  , debounceDelay(debounce)
  , debugMessage(debugMsg)
  , tripped(0)
  , nextDebounce(0)
{
  pinMode(pin, pullUp ? INPUT_PULLUP : INPUT);
}

void EasyTripSensor::present()
{
  ::present(msgTripped.sensor, type);
}

void EasyTripSensor::process(unsigned long now)
{
  bool newTripped = (digitalRead(pin) == trippedState);

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

EasyMotionSensor::EasyMotionSensor(uint8_t sensorId, int sensorPin)
  : EasyTripSensor(sensorId, sensorPin, S_MOTION, HIGH, false, 0, "Motion: ") {}

EasySoundSensor::EasySoundSensor(uint8_t sensorId, int sensorPin, unsigned long debounce)
  : EasyTripSensor(sensorId, sensorPin, S_SOUND, LOW, false, debounce, "Sound: ") {}
