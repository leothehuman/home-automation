// Enable debug prints
#define MY_DEBUG

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

int DHT11pin = 2;
int DHT11int = 0; //interrupt number (must be the one that use the previus defined pin (see table above)
int pirPin = 3;
int lightPin = A0;
int soundPin = A1;

const unsigned long SoundDelay = 1000;

EasyDHT11 easyDHT11("DHT11", CHILD_ID_TEMP, CHILD_ID_HUM, DHT11pin, DHT11int);
EasyLightSensor easyLightSensor("Light", CHILD_ID_LIGHT_LEVEL, lightPin);
EasyMotionSensor easyMotion("Motion", CHILD_ID_MOTION, pirPin);
EasySoundSensor easySound("Sound", CHILD_ID_SOUND, soundPin, SoundDelay);
EasyRGBLed easyRGBLed("Led", CHILD_ID_LIGHT, 5, 9, 6); // Must be a PWM pin -- 3, 5, 6, 9, 10, 11

void presentation()  
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("BedSensors", "1.0");
  EasySensor::presentAll();
}

void setup()
{
}

void loop()
{
  unsigned long now = millis();
  EasySensor::processAll();
}

void receive(const MyMessage& message)
{
  EasySensor::receiveAll(message);
}

