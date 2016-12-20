#include "EasySensor.h"

EasySensor::EasySensor(const char* sensorName)
  : name(sensorName)
{
  if (numSensors == MAX_SENSORS)
  {
    Serial.println("MAX_SENSORS exceeded!");
    for (;;);
  }
  sensors[numSensors++] = this;
};

const char* EasySensor::getName() { return name; }

void EasySensor::presentAll()
{
  for (int i = 0; i < numSensors; ++i)
  {
    sensors[i]->present();
  }
}

namespace
{
  void time(const char* what, unsigned long& now)
  {
#ifdef EASY_DEBUG    
    unsigned long time = millis();
    unsigned long diff = time - now;
    if (time - now > 20)
    {
      Serial.print(what);
      Serial.print(" took ");
      Serial.print(time - now, DEC);
      Serial.println(" ms.");
    }
#endif    
    now = millis();
  }
}

void EasySensor::processAll()
{
  unsigned long now = millis();
  for (int i = 0; i < numSensors; ++i)
  {
    sensors[i]->process(now);
    time(sensors[i]->getName(), now);
  }
}

void EasySensor::receiveAll(const MyMessage& message)
{
  for (int i = 0; i < numSensors; ++i)
  {
    sensors[i]->receive(message);
  }
}

int EasySensor::numSensors = 0;
EasySensor* EasySensor::sensors[MAX_SENSORS];
