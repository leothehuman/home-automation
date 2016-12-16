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
                Serial.println(F( "No more addresses."));
                Serial.println();
                ds_.reset_search();
                delay(250);
            }
            badCRC = OneWire::crc8(addr_, 7) != addr_[7];
            if (badCRC)
            {
                Serial.println(F("CRC is not valid!"));
                ds_.reset_search();
                delay(250);
            }
            else
            {
                switch (addr_[0])
                {
                case 0x10:
                    Serial.println(F("  Chip = DS18S20"));  // or old DS1820
                    type_ = 1;
                    break;
                case 0x28:
                    Serial.println(F("  Chip = DS18B20"));
                    type_ = 0;
                    break;
                case 0x22:
                    Serial.println(F("  Chip = DS1822"));
                    type_ = 0;
                    break;
                default:
                    Serial.println(F("Device is not a DS18x20 family device."));
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
        nextCheck_ = now + 1000;

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
            Serial.print(F("CRC error"));
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

        ds_.reset();
        ds_.select(addr_);
        ds_.write(0x44, 1);        // start conversion, with parasite power on at the end
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

