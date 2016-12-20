// Enable debug prints
#define MY_DEBUG
#define EASY_DEBUG

#include <MyConfig.h>
#include <MySensors.h>

#include <EasyDHT11.h>
#include <EasyRGBLed.h>
#include <EasyLightSensor.h>
#include <EasyTripSensor.h>

#define CHILD_ID_MOTION 1
#define CHILD_ID_LIGHT_LEVEL 2
#define CHILD_ID_TEMP 3
#define CHILD_ID_HUM 4
#define CHILD_ID_SOUND 5
#define CHILD_ID_LIGHT 6

int idDHT11pin = 2;
int idDHT11intNumber = 0; //interrupt number (must be the one that use the previus defined pin (see table above)
int pirPin = 3;
int lightPin = A0;
int soundPin = A1;

const unsigned long SoundDelay = 1000;

EasyDHT11 easyDHT11(CHILD_ID_TEMP, CHILD_ID_HUM, idDHT11pin, idDHT11intNumber);
EasyLightSensor easyLightSensor(CHILD_ID_LIGHT_LEVEL, lightPin);
EasyMotionSensor easyMotion(CHILD_ID_MOTION, pirPin);
EasySoundSensor easySound(CHILD_ID_SOUND, soundPin, SoundDelay);
EasyRGBLed easyRGBLed(CHILD_ID_LIGHT, 5, 9, 6); // Must be a PWM pin -- 3, 5, 6, 9, 10, 11

void presentation()  
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("BedSensors", "1.0");
  EasySensor::present();
}

void setup()
{
}

void time(const char* what, unsigned long& now)
{
  unsigned long time = millis();
  unsigned long diff = time - now;
  if (time - now > 20)
  {
    Serial.print(what);
    Serial.print("Took ");
    Serial.print(time - now, DEC);
    Serial.println(" ms.");
  }
  now = millis();
}

void loop()
{
  unsigned long now = millis();

  easyDHT11.process(now);
  time("easyDHT11", now);

  easyMotion.process(now);
  time("easyMotion", now);

  easySound.process(now);
  time("easySound", now);

  easyLightSensor.process(now);
  time("easyLightSensor", now);

  easyRGBLed.process(now);
  time("easyRGBLed", now);
}

void receive(const MyMessage& message)
{
  easyRGBLed.receive(message);
}

