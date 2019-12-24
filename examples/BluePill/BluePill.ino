/*
    CC1101_RF library demo
    This is an example of a RF module on the first SPI bus of a
    bluePill / blackpill module. The GDO0 pin is selected to be near the other pins
    This sketch can communicate with all other examples on any platform and
    The examples are on the public domain
*/

#include <Arduino.h>
#include <SPI.h>
#include <CC1101_RF.h>

// this declaration only assigns the pins and the bus.
// all chip manipulation happens when we call radio.begin()
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

// Uncomment to use the Serial1 port
// #define Serial Serial1

void setup() {
    Serial.begin(9600); // This is the USB port
    SPI.begin(); // mandatory. The CC1101_RF does not do this automatically
    // set the pins and the basic registers of the chip. Freq=433.2Mhz
    radio.begin(433.2e6); 
    Serial.println("Radio begin");
    radio.setRXdefault(); // every send and receive operation reenables RX.
    radio.setRXstate(); // Set the current state to RX : listening for RF packets
    // You may prefer to use another pin and an external LED, the BUILDIN is too dim on bluepill
    // and not useful for outdoors tests
    pinMode(LED_BUILTIN, OUTPUT);
    // or
    // pinMode(PB9, OUTPUT); // A LED connected to PB9 - GND
}

// used for the periodic pings see below
uint32_t pingTimer=0;
// used for LED blinking when we receive a packet
uint32_t receiveTime;

void loop() {
    // Turn on the LED for 100ms. The Buildin LED on bluepill is ON when LOW
    digitalWrite(LED_BUILTIN, millis()-receiveTime>100);
    // or external LED. The "<" is because this LED is ON when HIGH
    // digitalWrite(PB9, millis()-receiveTime<100);

    // Receive part.
    if (radio.packetReceived()) {  //todo
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
            // with the default register settings should not see any
            // but we keep it here as may indicate loose pin connections
            // or other hardware related problem or simply messing with the CC1101 registers
            Serial.println("No/Invalid packet");
        }
    }

    // periodic pings.
    if ((millis()-pingTimer>5000)) { // ping every 5sec
        Serial.println("Sending ping");
        // change the string to know who is sending
        radio.sendPacket("Ping from BluePill");
        // printf is handy but enlarges the firmware a lot
        // radio.printf("time : %lu",millis()/1000); // %lu = long unsigned
        pingTimer = millis();
    }
}
