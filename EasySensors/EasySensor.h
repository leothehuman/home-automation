#ifndef EASYSENSOR_H
#define EASYSENSOR_H

#define MAX_SENSORS 10

#include <stdint.h>
#include <core/MySensorsCore.h> // All sensors use this

class EasySensor
{
public:
  EasySensor()
  {
    
  };
  virtual void present() = 0;
  virtual void process() = 0;
  virtual void receive(const MyMessage& message) {};
  virtual ~EasySensor() {};
  
  static void presentAll();
protected:
  void reg(uint8_t childId, uint8_t type);
private:
  struct PresentationInfo
  {
    uint8_t childId;
    uint8_t type;
  };
  static PresentationInfo sensors[MAX_SENSORS];
  static int numSensors;
};

#endif//EASYSENSOR_H
