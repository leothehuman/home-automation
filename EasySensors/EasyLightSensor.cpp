#include "EasyLightSensor.h"

namespace
{
  unsigned long lightDelay = 1000;
}

EasyLightSensor::EasyLightSensor(uint8_t sensorId, int pin)
  : msgLightLevel(sensorId, V_LIGHT_LEVEL)
  , sensorPin(pin)
  , level(0)
  , nextCheck(0)
{
  pinMode(sensorPin, INPUT);
}

void EasyLightSensor::present()
{
  ::present(msgLightLevel.sensor, S_LIGHT_LEVEL);
}

void EasyLightSensor::process(unsigned long now)
{
  if (now < nextCheck)
  {
    return;
  }
  nextCheck = now + lightDelay;
  uint8_t newLevel = map(analogRead(sensorPin), 0, 1023, 0, 100);
  if (abs(newLevel - level) > 1)
  {
    level = newLevel;
#ifdef EASY_DEBUG
    Serial.print(F("Light: "));
    Serial.println(level);
#endif
    send(msgLightLevel.set(level));
  }
}

uint8_t EasyLightSensor::getLevel() { return level; }
