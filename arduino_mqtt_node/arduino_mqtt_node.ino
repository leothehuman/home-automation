
#include <idDHT11.h>
#include <OneWire.h>

#include "DS.h"

int idDHT11pin = 2; //Digital pin for comunications
int idDHT11intNumber = 0; //interrupt number (must be the one that use the previus defined pin (see table above)
void dht11_wrapper(); // must be declared before the lib initialization
idDHT11 DHT11(idDHT11pin,idDHT11intNumber,dht11_wrapper);
void dht11_wrapper()
{
    DHT11.isrCallback();
}
unsigned long nextDHTcheck = 0;
#define DHT_IDLE 0
#define DHT_WAIT 1
int state = DHT_IDLE;
float temperature = 0.0;
float humidity = 0.0;
unsigned long lastChecked = 0;

int pirPin = 8;
int ledPin = 9;
unsigned long pir = 0;

OneWire oneWire(4);
DS dsSensor(oneWire);

int lightPin = A0;  //define a pin for Photo resistor
int light = 0;

unsigned char lightsBrightness = 0;

int soundPin = 7;
unsigned long sound = 0;

const unsigned long SoundDelay = 1000;
const unsigned long PirDelay = 1000;

int ledControlPin[3] = {5, 3, 6}; // Must be a PWM pin -- 3, 5, 6, 9, 10, 11

void setup()
{
    delay(1000);
    Serial.begin(9600);

    dsSensor.init();

    pinMode(pirPin, INPUT);
    pinMode(lightPin, INPUT);
    pinMode(soundPin, INPUT);
    pinMode(ledPin, OUTPUT);
    for (int j = 0; j < 3; ++j)
    {
        pinMode(ledControlPin[j], OUTPUT);
    }
}

void time(char* what, unsigned long& now)
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

void handleDHTstate(unsigned long now)
{
    if (nextDHTcheck < now)
    {
        DHT11.acquire();
        nextDHTcheck = now + 10000;
        state = DHT_WAIT;
    }

    if (state == DHT_WAIT && !DHT11.acquiring())
    {
        int result = DHT11.getStatus();
        if (result == IDDHTLIB_OK)
        {
            Serial.print("DHT11:   ");
            temperature = DHT11.getCelsius();
            Serial.print(temperature, 2);
            Serial.print("oC, ");
            humidity = DHT11.getHumidity();
            Serial.print(humidity, 2);
            Serial.println("%");
            lastChecked = now;
        }
        else
        {
            Serial.println("Error");
        }
        state = DHT_IDLE;
    }
}

void handlePresenceSensor(unsigned long now)
{
    if (digitalRead(pirPin) == HIGH)
    {
        pir = now + PirDelay;
    }
}

void handleLightSensor(unsigned long now)
{
    light = analogRead(lightPin);
}


void handleSoundSensor(unsigned long now)
{
    if (digitalRead(soundPin) == LOW)
    {
        sound = now + SoundDelay;
    }
}

unsigned char rgb[3] = {0, 0, 0};
void recolor(unsigned char brightness)
{
    rgb[0] = brightness;
//    rgb[1] = brightness/4;
    rgb[2] = brightness/2;
    for (int j = 0; j < 3; ++j)
    {
        analogWrite(ledControlPin[j], rgb[j]);
    }
}

unsigned long nextLightsUpdate = 0;
void handleLights(unsigned long now)
{
    if (now < nextLightsUpdate)
    {
        return;
    }
    nextLightsUpdate = now + 25;
    if (pir > now && light < 200)
    {
        if (lightsBrightness < 255)
        {
            lightsBrightness += 1;
            recolor(lightsBrightness);
        }
    }
    else
    {
        if (lightsBrightness > 0)
        {
            lightsBrightness -= 1;
            recolor(lightsBrightness);
        }
    }
}

void loop()
{
    unsigned long now = millis();

    dsSensor.process(now);
    time("dsSensor.process", now);
    handleDHTstate(now);
    time("handleDHTstate", now);

    handlePresenceSensor(now);
    time("handlePresenceSensor", now);
    handleSoundSensor(now);
    time("handleSoundSensor", now);

    handleLightSensor(now);
    time("handleLightSensor", now);

    digitalWrite(ledPin, sound > now | pir > now);

    handleLights(now);
    time("handleLights", now);
}
