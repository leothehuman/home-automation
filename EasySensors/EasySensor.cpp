#include "EasySensor.h"

void EasySensor::present()
{
  for (int i = 0; i < numSensors; ++i)
  {
    ::present(sensors[i].childId, sensors[i].type);
  }
}

void EasySensor::reg(uint8_t childId, uint8_t type)
{
  sensors[numSensors].childId = childId;
  sensors[numSensors].type = type;
  ++numSensors;
}

int EasySensor::numSensors = 0;
EasySensor::PresentationInfo EasySensor::sensors[MAX_SENSORS];
