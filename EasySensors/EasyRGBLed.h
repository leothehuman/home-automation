#ifndef EASYRGBLED_H
#define EASYRGBLED_H

#include "EasySensor.h"

class EasyRGBLed : public EasySensor
{
public:
  EasyRGBLed(uint8_t sensorId, int rPin, int gPin, int bPin);
  virtual void present();
  virtual void process(unsigned long now);
  virtual void receive(const MyMessage& message);
private:
  MyMessage msgLight;
  MyMessage msgDimmer;
  MyMessage msgRgb;
  int controlPin[3]; // Must be a PWM pin -- 3, 5, 6, 9, 10, 11

  bool setOn;
  int setBrightness;
  char setColor[7];

  unsigned char setRgb[3]; // Another representation of the above

  unsigned char rgb[3]; // Current state (combination of on, brightness and rgb)

  unsigned char brightness;
  unsigned long nextUpdate;

  bool initializedState;
  bool initializedBrightness;
  bool initializedColor;
  bool stateSent;

  void recolor(int brightness);
};

#endif//EASYRGBLED_H
