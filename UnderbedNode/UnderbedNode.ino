// Enable debug prints
#define MY_DEBUG

#include <MyConfig.h>
#include <MySensors.h>
#include <idDHT11.h>
#include <OneWire.h>

#include "DS.h"

#define CHILD_ID_MOTION 1
#define CHILD_ID_LIGHT_LEVEL 2
#define CHILD_ID_TEMP 3
#define CHILD_ID_HUM 4
#define CHILD_ID_SOUND 5
#define CHILD_ID_LIGHT 6

MyMessage msgMotion(CHILD_ID_MOTION, V_TRIPPED);
MyMessage msgLightLevel(CHILD_ID_LIGHT_LEVEL, V_LIGHT_LEVEL);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgSound(CHILD_ID_SOUND, V_TRIPPED);

MyMessage msgLight(CHILD_ID_LIGHT, V_LIGHT);
MyMessage msgDimmer(CHILD_ID_LIGHT, V_DIMMER);
MyMessage msgRgb(CHILD_ID_LIGHT, V_RGB);

void presentation()  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("BedSensors", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_MOTION, S_MOTION);
  present(CHILD_ID_LIGHT_LEVEL, S_LIGHT_LEVEL);
  present(CHILD_ID_TEMP, S_TEMP);
  present(CHILD_ID_HUM, S_HUM);
  present(CHILD_ID_SOUND, S_SOUND);
  present(CHILD_ID_LIGHT, S_RGB_LIGHT);
}

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

int pirPin = 3;
//int ledPin = 9;
unsigned long pir = 0;

OneWire oneWire(4);
DS dsSensor(oneWire);

int lightPin = A0;  //define a pin for Photo resistor

int soundPin = A1;

const unsigned long PirDelay = 1000;


int ledControlPin[3] = {5, 9, 6}; // Must be a PWM pin -- 3, 5, 6, 9, 10, 11
void setup()
{
    pinMode(pirPin, INPUT);
    pinMode(lightPin, INPUT);
    pinMode(soundPin, INPUT);
    
    for (int j = 0; j < 3; ++j)
    {
        pinMode(ledControlPin[j], OUTPUT);
    }
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
            float newTemperature = DHT11.getCelsius();
            if (abs(newTemperature - temperature) > 1)
            {
                temperature = newTemperature;
                Serial.print(temperature, 2);
                Serial.print("oC, ");
                send(msgTemp.set((int)(temperature)));
            }
            float newHumidity = DHT11.getHumidity();
            if (abs(newHumidity - humidity) > 1)
            {
                humidity = newHumidity;
                Serial.print(humidity, 2);
                Serial.println("%");
                send(msgHum.set((int)(humidity)));
            }
        }
        else
        {
            Serial.println("Error");
        }
        state = DHT_IDLE;
    }
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
    if (newLight != light)
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

char setColor[7] = "ffffff";
int setBrightness = 100;
bool setOn = false;

unsigned char rgb[3] = {0, 0, 0};
unsigned char setRgb[3] = {255, 255, 255};
void recolor(int brightness)
{
    for (int i = 0; i < 3; ++i)
    {
      rgb[i] = (int)setRgb[i] * brightness / 100;
      analogWrite(ledControlPin[i], rgb[i]);
    }
}

unsigned char lightsBrightness = 0;
unsigned long nextLightsUpdate = 0;
void handleLights(unsigned long now)
{
    if (now < nextLightsUpdate)
    {
        return;
    }
    nextLightsUpdate = now + 10;
    if (setBrightness * setOn > lightsBrightness)
    {
        lightsBrightness += 1;
        recolor(lightsBrightness);
    }
    else if (setBrightness * setOn < lightsBrightness)
    {
        lightsBrightness -= 1;
        recolor(lightsBrightness);
    }
}

bool initialStateSent = false;
bool lightStateSent = false;

void loop()
{
    if (!initialStateSent)
    {
        send(msgTemp.set((int)(temperature)));
        send(msgHum.set((int)(humidity)));
        send(msgMotion.set(tripped ? "1" : "0"));  // Send tripped value to gw 
        send(msgSound.set(soundTripped));
        send(msgLightLevel.set(light));
        initialStateSent = true;
    }
    if (!lightStateSent)
    {
        send(msgRgb.set(setColor));
        send(msgLight.set(setOn ? 1 : 0));
        send(msgDimmer.set(setBrightness * setOn));
        lightStateSent = true;
    }
    unsigned long now = millis();

//    dsSensor.process(now);
//    time("dsSensor.process", now);
    handleDHTstate(now);
    time("handleDHTstate", now);

    handlePresenceSensor(now);
    time("handlePresenceSensor", now);
    handleSoundSensor(now);
    time("handleSoundSensor", now);

    handleLightSensor(now);
    time("handleLightSensor", now);

    handleLights(now);
    time("handleLights", now);
}

void receive(const MyMessage& message)
{
    if (message.type == V_LIGHT)
    {
        int requestedLevel = atoi( message.data );
        if (requestedLevel != setOn)
        {
            setOn = requestedLevel;
            Serial.print(F("Turning lights "));
            Serial.println(setOn ? "on" : "off");
        }
        lightStateSent = false;
    } 
    else if (message.type == V_DIMMER)
    {
        int requestedLevel = atoi( message.data );
        // Clip incoming level to valid range of 0 to 100
        requestedLevel = requestedLevel > 100 ? 100 : requestedLevel;
        requestedLevel = requestedLevel < 0   ? 0   : requestedLevel;

        if (requestedLevel != setBrightness || (bool)requestedLevel != setOn)
        {
          Serial.print( "Changing level to " );
          Serial.print( requestedLevel );
          Serial.print( ", from " ); 
          Serial.println( setBrightness );
          setBrightness = requestedLevel;
          if (setBrightness == 0)
          {
            setBrightness = 1;
          }
          setOn = (bool)requestedLevel;
          lightStateSent = false;
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
        recolor(lightsBrightness);
        Serial.print("Changing color to ");
        Serial.println(setColor);
        lightStateSent = false;
    }
}

