#include "EasySensors.h"

void setup()
{
}

void presentation()
{
  EasySensor::presentAll();
}

void loop()
{
  EasySensor::processAll();
}

void receive(const MyMessage& message)
{
  EasySensor::receiveAll(message);
}
