/*
    Arduino CC1101 library demo
    This is an example of a CC1101 on a proMini or bare atmega328
    The GDO0 pin defaults to 2 for this platform but can be changed.
    This sketch can communicate with all other examples
    The examples are on the public domain
*/

#include <Arduino.h>
#include <SPI.h>
#include <CC1101.h>

//   Pinout. See the comments on the extended example  in order to change it
// 
//   CC1101       ProMini (3.3V only. The CC1101 chip is not 5V tolerant)
//    CSN           10
//    CSK           13
//    MISO          12
//    MOSI          11
//    GDO0          2
//    GND           GND
//    VCC           3.3V
//

// also ProMini needs a USB to TTL uart converter such as FTDI or CP2102
// the FTDI has the advantage that can directly connect to ProMini header
// and can reset the module automatically
 
// for a different GDO0 pin
// CC1101 radio(A0);
CC1101 radio;

void setup() {
    Serial.begin(9600);
    // Note we start the spi2 we declared above and not the SPI (which is the first bus)
    SPI.begin();
    radio.begin();
    Serial.println("ProMini here");
    radio.setRXdefault(); // every send and receive operation reenables RX
    radio.setRXstate(); // we start with RX
    
    // The onboard LED cannot be used because it is used by the SPI bus (Pin 13)
    pinMode(4, OUTPUT); // connect a LED with a resistor to PIN 4 and GND
    pinMode(5, OUTPUT); // you can also use it as Ground
}

// used for the periodic pings
uint32_t pingTimer=0;
// used for LED blinking when we receive a packet
uint32_t receiveTime;

void loop() {
    // Turn on the LED for 100ms without actually wait.
    digitalWrite(4, millis()-receiveTime<100);
    
    // Receive part. With th Setting of IOGd0 we get this only with a valid packet
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
            // or other hardware related problem or simply messing with the registers
            Serial.println("No/Invalid packet");
        }
    }

    // periodic pings.
    if ((millis()-pingTimer>5000)) { // ping every 5sec
        Serial.println("Sending ping");
        // Note : the printf is quite expensive in flash space but useful, especially in tests
        radio.printf("ProMini time : %lu",millis()/1000); // %lu = long unsigned
        pingTimer = millis();
    }
}
