// Enable debug prints
//#define MY_DEBUG

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
//#define CHILD_ID_DOOR 7

int DHT11pin = 2;
int DHT11int = 0; //interrupt number (must be the one that use the previus defined pin (see table above)
int pirPin = 3;
int lightPin = A0;
int soundPin = A1;

const unsigned long SoundDelay = 1000;

EasyDHT11 easyDHT11(CHILD_ID_TEMP, CHILD_ID_HUM, DHT11pin, DHT11int);
EasyLightSensor easyLightSensor(CHILD_ID_LIGHT_LEVEL, lightPin);
EasyMotionSensor easyMotion(CHILD_ID_MOTION, pirPin);
//EasySoundSensor easySound(CHILD_ID_SOUND, soundPin, SoundDelay);
EasyRGBLed easyRGBLed(CHILD_ID_LIGHT, 5, 9, 6); // Must be a PWM pin -- 3, 5, 6, 9, 10, 11

void presentation()  
{
  sendSketchInfo("BedSensors", "1.0");
  EasySensor::presentAll();
}

void setup()
{
  pinMode(soundPin, INPUT);
}

void loop()
{
  EasySensor::processAll();
}

void receive(const MyMessage& message)
{
  EasySensor::receiveAll(message);
}
