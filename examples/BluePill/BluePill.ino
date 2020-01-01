/*
    CC1101_RF library demo
    This is an example of a RF module on the first SPI bus of a
    bluePill / blackpill module. The GDO0 pin is selected to be near the other pins
    This sketch can communicate with all other examples on any platform.
    The examples are on the public domain
*/

#include <Arduino.h>
#include <SPI.h>
#include <CC1101_RF.h>

//    PIN connections
//
//   CC1101       Blue/BlackPill
//    CSN           PA4(SS1)
//    CSK           PA5(SCK1)
//    MISO          PA6(MISO1)
//    MOSI          PA7(MOSI1)
//    GDO0          PB0 // is near the other pins
//    GND           GND
//    VCC           3.3V

// without parameters defaults to GDO0=PB0 CSN=SS(ChipSelect).
// See BluePill_SPI2 example for other configurations
CC1101 radio;

// Uncomment to use the Serial1 port instead of the USB
// #define Serial Serial1

void setup() {
    Serial.begin(9600);
    Serial.println("BluePill begin");
    SPI.begin(); // mandatory. CC1101_RF does not start SPI automatically
    radio.begin(433.2e6); // Freq=433.2Mhz
    
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

    // Receive part. GDO0 is connected to PB0
    // convert it to if(true) or remove the if clause entirely to disable the use of GDo0
    // and of course o need to connect GDo0 then.
    // However if you plan to use MCU sleep modes the GDo0 is mandatory
    if (digitalRead(PB0)) {
        byte packet[64];
        byte pkt_size = radio.getPacket(packet);
        receiveTime=millis();
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
            // very noisy without GDo0
            // Serial.println("No/Invalid packet");
        }
    }

    if ((millis()-pingTimer>5000)) { // ping every 5sec
        // change the string to know who is sending
        if (radio.sendPacket("Ping from BluePill")) {
            Serial.println("Ping sent");
        } else {
            Serial.println("Ping failed due to high RSSI and/or incoming packet");
        }
        // printf is handy but enlarges the firmware a lot
        // radio.printf("time : %lu",millis()/1000); // %lu = long unsigned
        pingTimer = millis();
    }
}
