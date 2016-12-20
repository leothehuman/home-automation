#ifndef EASYDHT11_H
#define EASYDHT11_H

#include "EasySensor.h"

#include <idDHT11.h>

void dht11_wrapper();

class EasyDHT11 : public EasySensor
{
public:
  EasyDHT11(uint8_t tempId, uint8_t humId, int pin, int interrupt);
  virtual void present();
  virtual void process(unsigned long now);
  bool isOk();
  float getTemp();
  float getHum();

  static idDHT11& getInstance();
private:
    
  MyMessage msgTemp;
  MyMessage msgHum;
  float temp;
  float hum;
  idDHT11 dht11;
  int state; 
  unsigned long nextCheck;

  static idDHT11* instance;
};

#endif//EASYDHT11_H

