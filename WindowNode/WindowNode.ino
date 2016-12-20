// Enable debug prints
#define MY_DEBUG

#include <MyConfig.h>
#include <MySensors.h>

#include <EasyDS18x20.h>
#include <EasyLightSensor.h>

//#define CHILD_ID_MOTION 1
#define CHILD_ID_LIGHT_LEVEL 2
#define CHILD_ID_TEMP 3
//#define CHILD_ID_HUM 4
//#define CHILD_ID_SOUND 5
//#define CHILD_ID_LIGHT 6
//#define CHILD_ID_DOOR 7
#define CHILD_ID_WINDOW 8

//int doorPin = 4;
//int pirPin = 5;
int tempPin = 6;
int lightPin = A0;

EasyLightSensor easyLight(CHILD_ID_LIGHT_LEVEL, lightPin);
EasyDS18x20 easyDS18x20(CHILD_ID_TEMP, tempPin);
void presentation()
{
  sendSketchInfo("WindowSensors", "1.0");
  EasySensor::presentAll();
}

void setup()
{
}

void loop()
{
  EasySensor::processAll();
}

void receive(const MyMessage& message)
{
  EasySensor::receiveAll(message);
}

