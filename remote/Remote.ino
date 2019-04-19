/*

Demonstrates simple RX and TX operation.
Please read the notes in NRFLite.h for a description of all library features.

Radio    Arduino
CE    -> 9
CSN   -> 10 (Hardware SPI SS)
MOSI  -> 11 (Hardware SPI MOSI)
MISO  -> 12 (Hardware SPI MISO)
SCK   -> 13 (Hardware SPI SCK)
IRQ   -> No connection
VCC   -> No more than 3.6 volts
GND   -> GND

*/

#include <SPI.h>
#include <NRFLite.h>

int verticalPin = A0;
int verticalValue = 0;
int horizontalPin = A1;
int horizontalValue = 0;

const static uint8_t RADIO_ID = 1;             // Our radio's id.
const static uint8_t DESTINATION_RADIO_ID = 0; // Id of the radio we will transmit to.
const static uint8_t PIN_RADIO_CE = 9;
const static uint8_t PIN_RADIO_CSN = 10;

struct RadioPacketDebug // Any packet up to 32 bytes can be sent.
{
    uint8_t FromRadioId;
    uint32_t OnTimeMillis;
    uint32_t FailedTxCount;
};


struct RadioPacketJoystick
{
    int8_t vertical;
    int8_t horizontal;
};

NRFLite _radio;
RadioPacketJoystick _radioJoystick;
RadioPacketDebug _radioDebug;

void setup()
{
    Serial.begin(115200);
        
    if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN))
    {
        Serial.println("Cannot communicate with radio");
        while (1); // Wait here forever.
    }
    
    _radioDebug.FromRadioId = RADIO_ID;
}

void loop()
{
    _radioDebug.OnTimeMillis = millis();

    verticalValue = analogRead(verticalPin);
    horizontalValue = analogRead(horizontalPin);

    Serial.print("V: ");
    Serial.print(verticalValue);
    Serial.print(",H:");
    Serial.println(horizontalValue);

    verticalValue = map(verticalValue, 0, 1024, -100, 100);
    if(verticalValue > 0 && verticalValue < 10) {
      verticalValue = 0;
    }
    else if(verticalValue < 0 && verticalValue > -10) {
      verticalValue = 0;
    }

    horizontalValue = map(horizontalValue, 0, 1024, -100, 100);
    if(horizontalValue > 0 && horizontalValue < 10) {
      horizontalValue = 0;
    }
    else if(horizontalValue < 0 && horizontalValue > -10) {
      horizontalValue = 0;
    }
    
    // verticalValue = verticalValue - 512;
    // float angle = ((float)(horizontalValue-512)/512.0*3.1416);
    // int sign = (horizontalValue-512)/abs((horizontalValue-512));

    // _radioJoystick.leftWheel = verticalValue + (int)((float)(verticalValue)*sin(angle));
    // _radioJoystick.rightWheel = verticalValue - (int)((float)(verticalValue)*sin(angle));
    
    _radioJoystick.vertical = verticalValue;
    _radioJoystick.horizontal = horizontalValue;

    Serial.print("V: ");
    Serial.print(_radioJoystick.vertical);
    Serial.print(",H:");
    Serial.println(_radioJoystick.horizontal);
    //Serial.print();

    Serial.print("Sending ");
    Serial.print(_radioDebug.OnTimeMillis);
    Serial.print(" ms");

    if (_radio.send(DESTINATION_RADIO_ID, &_radioJoystick, sizeof(_radioJoystick))) // Note how '&' must be placed in front of the variable name.
    {
        Serial.println("...Success");
    }
    else
    {
        Serial.println("...Failed");
        _radioDebug.FailedTxCount++;
    }

    // delay(1000);
    delay(100);
    
    /*
    By default, 'send' transmits data and waits for an acknowledgement.
    You can also send without requesting an acknowledgement.
    _radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData), NRFLite::NO_ACK)
    _radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData), NRFLite::REQUIRE_ACK) // THE DEFAULT
    */
}
