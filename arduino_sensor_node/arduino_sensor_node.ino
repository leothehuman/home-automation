#include <UIPEthernet.h>
// The connection_data struct needs to be defined in an external file.
#include <UIPServer.h>
#include <UIPClient.h>
#include <idDHT11.h>

#include <OneWire.h>

class DS
{
public:
    DS(OneWire& ds) : ds_(ds), temperature_(0), nextCheck_(0) {}
    void init()
    {
        bool badCRC = true;
        do
        {
            while (!ds_.search(addr_))
            {
                Serial.println("No more addresses.");
                Serial.println();
                ds_.reset_search();
                delay(250);
            }
            badCRC = OneWire::crc8(addr_, 7) != addr_[7];
            if (badCRC)
            {
                Serial.println("CRC is not valid!");
                ds_.reset_search();
                delay(250);
            }
            else
            {
                switch (addr_[0])
                {
                case 0x10:
                    Serial.println("  Chip = DS18S20");  // or old DS1820
                    type_ = 1;
                    break;
                case 0x28:
                    Serial.println("  Chip = DS18B20");
                    type_ = 0;
                    break;
                case 0x22:
                    Serial.println("  Chip = DS1822");
                    type_ = 0;
                    break;
                default:
                    Serial.println("Device is not a DS18x20 family device.");
                    badCRC = true;
                }
                for (byte i = 0; i < 7; i++) // we need 9 bytes
                {
                    Serial.print(addr_[i], HEX);
                    Serial.print(" ");
                }
                Serial.println();
            }
        }
        while (badCRC);
        ds_.reset();
        ds_.select(addr_);
        ds_.write(0x44, 1);        // start conversion, with parasite power on at the end
        nextCheck_ = millis() + 1000;
    }
    float getTemperature() const
    {
        return temperature_;
    }
    void process(unsigned long now)
    {
        if (nextCheck_ > now)
        {
            return;
        }
        nextCheck_ = now + 10000;

        byte present = ds_.reset();
        ds_.select(addr_);
        ds_.write(0xBE); // Read Scratchpad

        byte data[12];
        for (byte i = 0; i < 9; i++) // we need 9 bytes
        {
            data[i] = ds_.read();
        }

        //Serial.print("  Data = ");
        //Serial.print(present, HEX);
        //Serial.print(" ");
        //for (byte i = 0; i < 9; i++)
        //{
        //    Serial.print(data[i], HEX);
        //    Serial.print(" ");
        //}
        //Serial.print(" CRC=");
        //Serial.print(OneWire::crc8(data, 8), HEX);
        //Serial.println();

        byte crc = OneWire::crc8(data, 8);
        if (crc != data[8])
        {
            Serial.print("CRC error");
            return;
        }

        // Convert the data to actual temperature
        // because the result is a 16 bit signed integer, it should
        // be stored to an "int16_t" type, which is always 16 bits
        // even when compiled on a 32 bit processor.
        int16_t raw = (data[1] << 8) | data[0];
        if (type_)
        {
            raw = raw << 3; // 9 bit resolution default
            if (data[7] == 0x10) {
                raw = (raw & 0xFFF0) + 12 - data[6];
            }
        }
        else
        {
            byte cfg = (data[4] & 0x60);
            // at lower res, the low bits are undefined, so let's zero them
            switch (cfg)
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
        temperature_ = (float)raw / 16.0;
        Serial.print("DS18x20: ");
        Serial.print(temperature_);
        Serial.println("oC");
    }
    const byte* getAddress() const
    {
        return addr_;
    }
private:
    OneWire& ds_;
    byte addr_[8];
    byte type_;
    float temperature_;
    unsigned long nextCheck_;
};

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

EthernetServer server = EthernetServer(80);

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

    IPAddress myIP(192,168,1,5);

    Ethernet.begin(dsSensor.getAddress() + 1, myIP);

    server.begin();
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

void handleHttpServer(unsigned long now)
{
    EthernetClient client = server.available();
    if (!client)
    {
        return;
    }
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected())
    {
        if (client.available())
        {
            char c = client.read();
            Serial.write(c);
            if (c == '\n' && currentLineIsBlank)
            {
                client.println("HTTP/1.1 200 OK");
                client.println("Content-Type: application/json");
                client.println("Connection: close");
                client.println("Refresh: 5");
                client.println();
                client.print("{ temperature: \"");
                client.print(temperature, 2);
                client.print("\", humidity: \"");
                client.print(humidity, 2);
                client.print("\", temp: \"");
                client.print(dsSensor.getTemperature(), 2);
                client.print("\", light: \"");
                client.print(light, DEC);
                client.print("\", brightness: \"");
                client.print(lightsBrightness, DEC);
                client.print("\", sound: \"");
                client.print(sound > now ? "true" : "false");
                client.print("\", pir: \"");
                client.print(pir > now ? "true" : "false");
                client.println("\" }");
                break;
            }
            if (c == '\n')
            {
                currentLineIsBlank = true;
            }
            else if (c != '\r')
            {
                currentLineIsBlank = false;
            }
        }
    }
    delay(100);
    client.stop();
    Serial.println("client disconnected");
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

    handleHttpServer(now);
    time("handleHttpServer", now);
}
