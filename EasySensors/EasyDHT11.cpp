#include "EasyDHT11.h"

namespace {
  int DHT_IDLE = 0;
  int DHT_WAIT = 1;
  int DHT_ERR = 2;
  unsigned long pollInterval = 10000;
}

void dht11_wrapper()
{
  EasyDHT11::getInstance().isrCallback();
}

EasyDHT11::EasyDHT11(uint8_t tempId, uint8_t humId, int pin, int interrupt)
  : msgTemp(tempId, V_HUM)
  , msgHum(humId, V_TEMP)
  , temp(0.0)
  , hum(0.0)
  , dht11(pin, interrupt, dht11_wrapper)
  , state(DHT_ERR)
  , nextCheck(0)
{
  reg(tempId, S_TEMP);
  reg(humId, S_HUM);
  instance = &dht11;
}

void EasyDHT11::process(unsigned long now)
{
  if (nextCheck < now)
  {
    dht11.acquire();
    nextCheck = now + pollInterval;
    state = DHT_WAIT;
  }

  if (state == DHT_WAIT && !dht11.acquiring())
  {
    int result = dht11.getStatus();
    if (result == IDDHTLIB_OK)
    {
      float newTemp = dht11.getCelsius();
      if (abs(newTemp - temp) > 1)
      {
        temp = newTemp;
#ifdef EASY_DEBUG
        Serial.print(F("Temp: "));
        Serial.println(temp, 1);
#endif
        send(msgTemp.set(temp, 1));
      }
      float newHum = dht11.getHumidity();
      if (abs(newHum - hum) > 1)
      {
        hum = newHum;
#ifdef EASY_DEBUG
        Serial.print(F("Hum: "));
        Serial.println(hum, 1);
#endif
        send(msgHum.set(hum, 1));
      }
      state = DHT_IDLE;
    }
    else
    {
      state = DHT_ERR;
    }
  }
}

bool EasyDHT11::isOk() { return state != DHT_ERR; }
float EasyDHT11::getTemp() { return temp; }
float EasyDHT11::getHum() { return hum; }
idDHT11& EasyDHT11::getInstance() { return *instance; }

idDHT11* EasyDHT11::instance = 0;
