#include "EasyRGBLed.h"

EasyRGBLed::EasyRGBLed(const char* name, uint8_t sensorId, int rPin, int gPin, int bPin)
  : EasySensor(name)
  , msgLight(sensorId, V_LIGHT)
  , msgDimmer(sensorId, V_DIMMER)
  , msgRgb(sensorId, V_RGB)
  , controlPin{rPin, gPin, bPin}

  , setOn(false)
  , setBrightness(100)
  , setColor{'f','f','f','f','f', 0}

  , setRgb{255, 255, 255}
  , rgb{0, 0, 0}

  , brightness(0)
  , nextUpdate(0)
  , initializedState(false)
  , initializedBrightness(false)
  , initializedColor(false)
  , stateSent(true)
{
  for (int j = 0; j < 3; ++j)
  {
    pinMode(controlPin[j], OUTPUT);
  }
}

void EasyRGBLed::present()
{
  ::present(msgLight.sensor, S_RGB_LIGHT);
}

void EasyRGBLed::recolor(int brightness)
{
  for (int i = 0; i < 3; ++i)
  {
    rgb[i] = (int)setRgb[i] * brightness / 100;
    analogWrite(controlPin[i], rgb[i]);
  }
}

void EasyRGBLed::process(unsigned long now)
{
  if (nextUpdate == 0)
  {
    request(msgRgb.sensor, V_RGB);
    request(msgDimmer.sensor, V_DIMMER);
    request(msgLight.sensor, V_LIGHT);
  }
  if (!(initializedState && initializedBrightness && initializedColor) && (now > 10000))
  {
    initializedState = true;
    initializedBrightness = true;
    initializedColor = true;
    stateSent = false;
  }

  if (!stateSent)
  {
    if (initializedColor)
    {
      send(msgRgb.set(setColor));
    }
    if (initializedState)
    {
      send(msgLight.set(setOn ? 1 : 0));
    }
    if (initializedBrightness)
    {
      send(msgDimmer.set(setBrightness));
    }
    stateSent = true;
  }

  if (now < nextUpdate)
  {
    return;
  }
  nextUpdate = now + 10;
  if (setBrightness * setOn > brightness)
  {
    brightness += 1;
    recolor(brightness);
  }
  else if (setBrightness * setOn < brightness)
  {
    brightness -= 1;
    recolor(brightness);
  }
}

void EasyRGBLed::receive(const MyMessage& message)
{
  if (msgLight.sensor != message.sensor)
  {
    return;
  }
  if (message.type == V_LIGHT)
  {
    int requestedLevel = atoi( message.data );
    if (requestedLevel != setOn)
    {
      setOn = requestedLevel;
#ifdef EASY_DEBUG
      Serial.print(F("Turning lights "));
      Serial.println(setOn ? "on" : "off");
#endif
    }
    initializedState = true;
    stateSent = false;
  } 
  else if (message.type == V_DIMMER)
  {
    int requestedLevel = atoi( message.data );
    requestedLevel = requestedLevel > 100 ? 100 : requestedLevel;
    requestedLevel = requestedLevel < 0   ? 0   : requestedLevel;

    if (requestedLevel != setBrightness || (bool)requestedLevel != setOn)
    {
      setBrightness = requestedLevel;
#ifdef EASY_DEBUG
      Serial.print( F("Changing brightness to ") );
      Serial.print( setBrightness );
#endif
      if (setBrightness == 0)
      {
        setBrightness = 1;
      }
      setOn = (bool)requestedLevel;
      initializedBrightness = true;
      stateSent = false;
    }
  }
  else if (message.type == V_RGB)
  {
    String hexstring = message.getString();
    hexstring.toCharArray(setColor, sizeof(setColor));
    long int color = strtol( &hexstring[0], NULL, 16);
    for (int i = 2; i >= 0; --i)
    {
      setRgb[i] = color % 256;
      color = color / 256;
    }
    recolor(brightness);
#ifdef EASY_DEBUG
    Serial.print(F("Changing color to "));
    Serial.println(setColor);
#endif
    initializedColor = true;
    stateSent = false;
  }
}
