// Enable debug prints
#define MY_DEBUG
#define EASY_DEBUG

#include <MyConfig.h>
#include <MySensors.h>

#include <OneWire.h>

#include <EasyDHT11.h>
#include <EasyRGBLed.h>

#include "DS.h"

#define CHILD_ID_MOTION 1
#define CHILD_ID_LIGHT_LEVEL 2
#define CHILD_ID_TEMP 3
#define CHILD_ID_HUM 4
#define CHILD_ID_SOUND 5
#define CHILD_ID_LIGHT 6

MyMessage msgMotion(CHILD_ID_MOTION, V_TRIPPED);
MyMessage msgLightLevel(CHILD_ID_LIGHT_LEVEL, V_LIGHT_LEVEL);
MyMessage msgSound(CHILD_ID_SOUND, V_TRIPPED);

void presentation()  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("BedSensors", "1.0");

  present(CHILD_ID_MOTION, S_MOTION);
  present(CHILD_ID_LIGHT_LEVEL, S_LIGHT_LEVEL);
  present(CHILD_ID_SOUND, S_SOUND);
  EasySensor::present();
}

int idDHT11pin = 2; //Digital pin for comunications
int idDHT11intNumber = 0; //interrupt number (must be the one that use the previus defined pin (see table above)
int pirPin = 3;
//int ledPin = 9;
unsigned long pir = 0;

OneWire oneWire(4);
DS dsSensor(oneWire);

EasyDHT11 easyDHT11(CHILD_ID_TEMP, CHILD_ID_HUM, idDHT11pin, idDHT11intNumber);
EasyRGBLed easyRGBLed(CHILD_ID_LIGHT, 5, 9, 6); // Must be a PWM pin -- 3, 5, 6, 9, 10, 11

int lightPin = A0;  //define a pin for Photo resistor

int soundPin = A1;

const unsigned long PirDelay = 1000;

void setup()
{
    pinMode(pirPin, INPUT);
    pinMode(lightPin, INPUT);
    pinMode(soundPin, INPUT);
//    dsSensor.init();
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

bool tripped = 0;
void handlePresenceSensor(unsigned long now)
{
    bool newTripped = digitalRead(pirPin) == HIGH;
    if (newTripped != tripped)
    {
        tripped = newTripped;
        Serial.print(F("Tripped: "));
        Serial.println(tripped ? "1" : "0");
        send(msgMotion.set(tripped ? "1" : "0"));  // Send tripped value to gw 
    }
    if (newTripped) 
    {
        pir = now + PirDelay;
    }
}

int light = 0; // Light level (0-100%)
const unsigned lightDelay = 1000;
unsigned long nextLightUpdate = 0;
void handleLightSensor(unsigned long now)
{
    if (now < nextLightUpdate)
    {
        return;
    }
    int newLight = map(analogRead(lightPin), 0, 1023, 0, 100);
    if (abs(newLight - light) > 1)
    {
        light = newLight;
        Serial.print(F("Light: "));
        Serial.println(light);
        send(msgLightLevel.set(light));
    }
    nextLightUpdate = now + lightDelay;
}

const unsigned long SoundDelay = 1000;
unsigned long sound = 0;
bool soundTripped = 0;
void handleSoundSensor(unsigned long now)
{
    bool newSoundTripped = (digitalRead(soundPin) == LOW);
    // We have sent the message already and still have sound
    if (newSoundTripped && soundTripped)
    {
        sound = now + SoundDelay;
        return;
    }
    if ((newSoundTripped != soundTripped) && (now > sound))
    {
        soundTripped = newSoundTripped;
        Serial.print(F("Sound: "));
        Serial.println(soundTripped ? "1" : "0");
        send(msgSound.set(soundTripped));
        if (soundTripped)
        {
            sound = now + SoundDelay;
        }
    }
}

void loop()
{
    unsigned long now = millis();
//    dsSensor.process(now);
//    time("dsSensor.process", now);
    easyDHT11.process(now);
    time("easyDHT11", now);

    handlePresenceSensor(now);
    time("handlePresenceSensor", now);
    handleSoundSensor(now);
    time("handleSoundSensor", now);

    handleLightSensor(now);
    time("handleLightSensor", now);

    easyRGBLed.process(now);
    time("easyRGBLed", now);
}

void receive(const MyMessage& message)
{
  easyRGBLed.receive(message);
}

