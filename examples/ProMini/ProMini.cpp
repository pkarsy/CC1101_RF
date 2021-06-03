/*
    Arduino CC1101 library demo
    From your IDE select ProMini 3.3V
    This sketch can communicate with all other examples
    The examples are on the public domain
*/

#include <Arduino.h>
#include <SPI.h>
#include <CC1101_RF.h>

//   Pinout
//
//   CC1101       ProMini (3.3V only. The CC1101 chip is not 5V tolerant)
//    CSN           10
//    CSK           13
//    MISO          12
//    MOSI          11
//    GND           GND
//    VCC           3.3V

// also ProMini needs a USB to TTL uart converter such as FTDI or CP2102
// the FTDI has the advantage that can directly connect to ProMini header
// and can reset the module automatically hoever I am not sure it can provide
// promini+cc1101 with enough power

CC1101 radio;

void setup() {
    Serial.begin(9600);
    Serial.println("ProMini begin");
    SPI.begin(); // mandatory. CC1101_RF does not start SPI automatically
    radio.begin(433.2e6); // Freq=433.2Mhz
    
    // LED setup. It is important as we can use the module without serial terminal
    // The onboard LED cannot be used because it is used by the SPI bus (Pin 13)
    pinMode(4, OUTPUT); // connect a LED with a resistor to PIN 4 and GND
    pinMode(5, OUTPUT); // By default is LOW(0 Volt). Use it as an extra Ground PIN.

    radio.setRXstate(); // Set the current state to RX : listening for RF packets
}

// used for the periodic pings
uint32_t pingTimer=0;
// used for LED blinking when we receive a packet
uint32_t ledTimer;

void loop() {
    // Turn on the LED for 200ms without actually wait.
    digitalWrite(4, millis()-ledTimer>200);

    // periodic pings.
    if ((millis()-pingTimer>5000)) { // ping every 5sec
        bool sucess = radio.sendPacket("Ping from ProMini");
        if (sucess) {
            Serial.println("Ping sent");
        } else {
            Serial.println("Ping failed due to high RSSI and/or incoming packet");
        }
        // printf is handy but enlarges the firmware. On atmega328 is surprisingly light
        // however, about 2 Kbytes
        // bool sucess = radio.printf("time : %lu",millis()/1000); // %lu = long unsigned
        pingTimer = millis();
    }

    // Receive part. Use the digitalRead(2) if GDo0 is connected to Arduino Pin 2
    // if (digitalRead(2)) {
    byte packet[64];
    byte pkt_size = radio.getPacket(packet);
    if (pkt_size>0 && radio.crcok()) { // We have a valid packet with some data
        Serial.print("Got packet \"");
        Serial.write(packet,pkt_size);
        Serial.print("\" len=");
        Serial.print(pkt_size);
        Serial.print(" Signal="); // for field tests to check the signal strength
        Serial.print(radio.getRSSIdbm());
        Serial.print(" LQI="); // for field tests to check the signal quality
        Serial.println(radio.getLQI());
        ledTimer=millis(); // we turn the led on when a packet arrives
    }
    //}
}
