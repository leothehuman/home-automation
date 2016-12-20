#ifndef EASYSENSOR_H
#define EASYSENSOR_H

#define MAX_SENSORS 10

#define EASY_DEBUG

#include <stdint.h>
#include <core/MySensorsCore.h> // All sensors use this

class EasySensor
{
public:
  EasySensor();

  virtual void present() = 0;
  virtual void process(unsigned long now) = 0;
  virtual void receive(const MyMessage& message) {};
  virtual ~EasySensor() {};
  
  static void presentAll();
  static void processAll();
  static void receiveAll(const MyMessage& message);
private:
  static EasySensor* sensors[MAX_SENSORS];
  static int numSensors;
};

#endif//EASYSENSOR_H
