#include "EasyTripSensor.h"

EasyTripSensor::EasyTripSensor(uint8_t sensorId, int pin, uint8_t type, int trippedValue, bool pullUp, unsigned long debounce, const char* debugMsg)
  : msgTripped(sensorId, V_TRIPPED)
  , sensorPin(pin)
  , trippedState(trippedValue)
  , debounceDelay(debounce)
  , debugMessage(debugMsg)
  , nextDebounce(0)
{
  reg(sensorId, type);
  pinMode(sensorPin, pullUp ? INPUT_PULLUP : INPUT);
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
