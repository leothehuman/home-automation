#include "EasyDS18x20.h"

namespace {
  unsigned long pollInterval = 10000;
}

EasyDS18x20::EasyDS18x20(uint8_t sensorId, int sensorPin)
  : msgTemp(sensorId, V_TEMP)
  , ds(sensorPin)
  , type(0)
  , temp(0.0)
  , nextCheck(0)
{
}

void EasyDS18x20::present()
{
  ::present(msgTemp.sensor, S_TEMP);
}

void EasyDS18x20::process(unsigned long now)
{
  if (now < nextCheck)
  {
    return;
  }
  if (nextCheck == 0)
  {
    init(now);
    return;
  }
  nextCheck = now + pollInterval;

  uint8_t present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad

  uint8_t data[12];
  for (uint8_t i = 0; i < 9; i++) // we need 9 uint8_ts
  {
    data[i] = ds.read();
  }

#ifdef EASY_DEBUG_RAWDATA
  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for (uint8_t i = 0; i < 9; i++)
  {
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();
#endif

  uint8_t crc = OneWire::crc8(data, 8);
  if (crc != data[8])
  {
#ifdef EASY_DEBUG
    Serial.print(F("CRC error"));
#endif
    return;
  }

  /* Convert the data to actual temperature
   * because the result is a 16 bit signed integer, it should
   * be stored to an "int16_t" type, which is always 16 bits
   * even when compiled on a 32 bit processor.
   */
  int16_t raw = (data[1] << 8) | data[0];
  if (type)
  {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  }
  else
  {
    uint8_t cfg = (data[4] & 0x60);

    switch (cfg) // at lower res, the low bits are undefined, so let's zero them
    {
      case 0x00: // 9 bit resolution, 93.75 ms
        raw = raw & ~7;
        break;
      case 0x20: // 11 bit res, 375 ms
        raw = raw & ~3;
        break;
      case 0x40: // 10 bit res, 187.5 ms
        raw = raw & ~1;
        break;
      default: //// default is 12 bit resolution, 750 ms conversion time
        break;
    }
  }
  float newTemp = (float)raw / 16.0;
  if (abs(newTemp - temp) > 0.5)
  {
    temp = newTemp;
#ifdef EASY_DEBUG
    Serial.print("DS18x20: ");
    Serial.println(temp, 1);
#endif
    send(msgTemp.set(temp, 1));
  }
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
}

void EasyDS18x20::init(unsigned long now)
{
  bool badCRC = true;
  do
  {
    while (!ds.search(addr))
    {
#ifdef EASY_DEBUG
      Serial.println(F( "No more addresses."));
#endif
      ds.reset_search();
      wait(250);
    }
    badCRC = OneWire::crc8(addr, 7) != addr[7];
    if (badCRC)
    {
#ifdef EASY_DEBUG
      Serial.println(F("CRC is not valid!"));
#endif
      ds.reset_search();
      wait(250);
    }
    else
    {
      switch (addr[0])
      {
        case 0x10:
#ifdef EASY_DEBUG
          Serial.println(F("  Chip = DS18S20"));  // or old DS1820
#endif
          type = 1;
          break;
        case 0x28:
#ifdef EASY_DEBUG
          Serial.println(F("  Chip = DS18B20"));
#endif
          type = 0;
          break;
        case 0x22:
#ifdef EASY_DEBUG
          Serial.println(F("  Chip = DS1822"));
#endif
          type = 0;
          break;
        default:
          Serial.println(F("Device is not a DS18x20 family device."));
          badCRC = true;
      }
#ifdef EASY_DEBUG
      for (uint8_t i = 0; i < 7; i++) // we need 9 bytes
      {
        Serial.print(addr[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
#endif
    }
  }
  while (badCRC);
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  Serial.println("Requested conversion");
  nextCheck = now + pollInterval;
}

float EasyDS18x20::getTemp() const
{
  return temp;
}

