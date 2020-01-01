/*
    CC1101_RF library demo
    This is an example of a RF module on the SECOND SPI bus of a
    bluePill / blackpill module. The GDO0 pin is selected to be near the other pins
    This sketch can communicate with all other examples.
    The examples are on the public domain
*/

#include <Arduino.h>
#include <SPI.h>
#include <CC1101_RF.h>

//    PIN connections
//
//   CC1101       Blue/BlackPill
//    CSN           PB12(SS2)
//    CSK           PB13(SCK2)
//    MISO          PB14(MISO2)
//    MOSI          PA15(MOSI2)
//    GDO0          PA8 // is near the other pins
//    GND           GND
//    VCC           3.3V


SPIClass spi2(2); // for STM32duino the second spibus

//CC1101 pins CSN, MISO, SPIbus
CC1101 radio( PB12, PB14, spi2   );

// Uncomment to use the Serial1 port instead of the USB
// #define Serial Serial1

void setup() {
    Serial.begin(9600);
    Serial.println("BluePill_SPI2 begin");
    SPI.begin(); // mandatory. CC1101_RF does not start SPI automatically
    radio.begin(433.2e6); // Freq=433.2Mhz
    //
    // LED setup. It is importand as we can use the module without serial terminal
    pinMode(LED_BUILTIN, OUTPUT);
    // or use an external more visible LED for outdoor tests
    // pinMode(PB9, OUTPUT); // A LED connected to PB9 - GND
    
    radio.setRXstate(); // Set the current state to RX : listening for RF packets
}

// used for the periodic pings see below
uint32_t pingTimer=0;
// used for LED blinking when we receive a packet
uint32_t receiveTime;

void loop() {
    // Turn on the LED for 100ms without loop block. The Buildin LED on bluepill is ON when LOW
    digitalWrite(LED_BUILTIN, millis()-receiveTime>100);
    // or external LED. The "<" is because this LED is ON when HIGH
    // digitalWrite(PB9, millis()-receiveTime<100);


    if ((millis()-pingTimer>5000)) { // ping every 5sec
        Serial.println("Sending ping");
        // change the string to know who is sending
        if (radio.sendPacket("Ping from BluePill_SPI2")) {
            Serial.println("Ping sent");
        } else {
            Serial.println("Ping failed due to high RSSI and/or incoming packet");
        }
        // printf is handy but enlarges the firmware a lot
        // bool success = radio.printf("time : %lu",millis()/1000); // %lu = long unsigned
        pingTimer = millis();
    }

    // Receive part. GDO0 is connected with PA8
    if (digitalRead(PA8)) {
        byte packet[64];
        byte pkt_size = radio.getPacket(packet);
        if (pkt_size>0 && radio.crcok()) { // We have a valid packet with some data
            Serial.print("Got packet \"");
            Serial.write(packet, pkt_size);
            Serial.print("\" len=");
            Serial.print(pkt_size);
            Serial.print(" Signal="); // for field tests to check the signal strength
            Serial.print(radio.getRSSIdbm());
            Serial.print(" LQI="); // for field tests to check the signal quality
            Serial.println(radio.getLQI());
        } else {
            Serial.println("No/Invalid packet");
        }
        receiveTime=millis();
    }
}



