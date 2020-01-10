/*
CC1101_RF library demo
The examples are on the public domain

NOTICES:
- The sketch only tested with platformio
- To upload code the reset pin must be pressed (This is due to the sleeping MCU)
- The STM32LowPower requires to use the "stm32" core in platformio
and not the default STM32duino. So platformio.ini should be like this
- No power consumption mesurements yet

[env:bluepill]
platform = ststm32
board = genericSTM32F103C8
framework = arduino
upload_protocol = blackmagic
;upload_port = /dev/serial/by-id/usb........
;or
;upload_protocol = stlink
board_build.core = stm32


The sketch sets the RF chip to wor mode AND the STM32 to sleep mode.
The MCU can wake when receiving a packet from the "wake" sketch. Note that normal
packets cannot easily wake this sketch because the radio is on for very little time
(some ms or even lower every 0.5 sec in this sketch)
To be able to reliably wake the radio-MCU we need packets with a long preamble (010101010 for 0.5 sec)
to assure that the Radio module will eventually sense the signal

The use of GDo0 is mandatory in this sketch
When radio is in WOR mode (Wake on Radio) we cannot query it using the SPI
bus as this defeats the primary purpose which is to save power. In addition
there is no much point to enable WOR (saving about 15mA) without enabling
LowPower MCU sleep modes (saving 30~50mA on STM32F103 or 10-20mA on atmega328)
and going to uA range

    PIN connections

   CC1101       Blue/BlackPill
    CSN           PA4(SS1)
    CSK           PA5(SCK1)
    MISO          PA6(MISO1)
    MOSI          PA7(MOSI1)
    GDO0          PB0 - We need it this time
    GND           GND
    VCC           3.3V

Also:
Connect a bright LED to PB9 - GND (with a resistor as usual).
You can use the buildin LED but and it will not be very visible.
The led will turn on momentarily whenever the STM32 wakes up, 

*/

#include <Arduino.h>
#include <SPI.h>
#include <CC1101_RF.h>
#include <STM32LowPower.h>

CC1101 radio;

const uint8_t GDO0 = PB0;

// using the Serial1 port instead of the USB one
#define Serial Serial1

// doing nothing. We need it to set something in LowPower.attachInterruptWakeup
void emptyInterruptHandler() { 
}

void setup() {
    Serial.begin(9600);
    Serial.println("Sleeper begin");
    SPI.begin(); // mandatory. CC1101_RF does not start SPI automatically
    radio.begin(433.2e6); // Freq=433.2Mhz
    // LED setup. It is importand as we can use the module without serial terminal
    pinMode(PB9, OUTPUT);
    radio.setRXstate(); // Set the current state to RX : listening for RF packets
    LowPower.begin();
    // Attach a wakeup interrupt on pin
    LowPower.attachInterruptWakeup(GDO0, emptyInterruptHandler, RISING);
}

void loop() {
    radio.wor(500);
    Serial.println("Going for sleep");
    Serial.flush(); // wait to output the message, otherwise MCU may sleep before printing all the phrase
    digitalWrite(PB9, LOW);
    LowPower.deepSleep(); // we stay here in hibernation
    digitalWrite(PB9, HIGH); // a radio signal woke the MCU
    Serial.println("Out of sleep");
    Serial.flush(); // the same as above
    // The only interrupt set to wake the MCU is GDO0
    // so we have incoming packet
    byte packet[64];
    byte pkt_size = radio.getPacket(packet);
    radio.wor2rx(); // this function sets the registers to normal RX(receive) operation
    if (pkt_size>0 && radio.crcok()) { // We have a valid packet with some data
        Serial.print("Got packet \"");
        Serial.write(packet, pkt_size);
        Serial.print("\" len=");
        Serial.print(pkt_size);
        Serial.print(" Signal="); // for field tests to check the signal strength
        Serial.print(radio.getRSSIdbm());
        Serial.print(" LQI="); // for field tests to check the signal quality
        Serial.println(radio.getLQI());
    }
}
