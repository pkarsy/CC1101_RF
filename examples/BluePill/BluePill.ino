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
    radio.setRXdefault(); // every send and receive operation reenables RX.
    radio.setRXstate(); // Set the current state to RX : listening for RF packets
    // LED setup. It is importand as we can use the module without serial terminal
    // pinMode(LED_BUILTIN, OUTPUT);
    // or use an external more visible LED for outdoor tests
    pinMode(PB9, OUTPUT); // A LED connected to PB9 - GND
}

// used for the periodic pings see below
uint32_t pingTimer=0;
// used for LED blinking when we receive a packet
uint32_t receiveTime;

void loop() {
    // Turn on the LED for 100ms without loop block. The Buildin LED on bluepill is ON when LOW
    // digitalWrite(LED_BUILTIN, millis()-receiveTime>100);
    // or external LED. The "<" is because this LED is ON when HIGH
    digitalWrite(PB9, millis()-receiveTime<100);

    // Receive part.
    if (radio.packetReceived()) {
        byte packet[64];
        byte pkt_size = radio.getPacket(packet);
        receiveTime=millis();
        if (pkt_size>0) { // We have a valid packet with some data
            Serial.print("Got packet \"");
            Serial.write(packet,pkt_size);
            Serial.print("\" len=");
            Serial.print(pkt_size);
            Serial.print(" Signal="); // for field tests to check the signal strength
            Serial.print(radio.getSignalDbm());
            Serial.print(" LQI="); // for field tests to check the signal quality
            Serial.println(radio.LQI());
        } else {
            // with the default register settings should not see any invalid packet
            // but we keep it here as may indicate loose pin connections
            // or other hardware related problem or simply messing with the CC1101 registers
            Serial.println("No/Invalid packet");
        }
    }

    if ((millis()-pingTimer>5000)) { // ping every 5sec
        Serial.println("Sending ping");
        // change the string to know who is sending
        radio.sendPacket("Ping from BluePill");
        // printf is handy but enlarges the firmware a lot
        // radio.printf("time : %lu",millis()/1000); // %lu = long unsigned
        pingTimer = millis();
    }
}
