#include "EasySensor.h"

EasySensor::EasySensor()
{
  if (numSensors == MAX_SENSORS)
  {
    Serial.println("MAX_SENSORS exceeded!");
    for (;;);
  }
  sensors[numSensors++] = this;
};

void EasySensor::presentAll()
{
  for (int i = 0; i < numSensors; ++i)
  {
    sensors[i]->present();
  }
}

namespace
{
  void time(int i, unsigned long& now)
  {
#ifdef EASY_DEBUG    
    unsigned long time = millis();
    unsigned long diff = time - now;
    if (time - now > 20)
    {
      Serial.print("Sensor ");
      Serial.print(i);
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
    time(i, now);
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
