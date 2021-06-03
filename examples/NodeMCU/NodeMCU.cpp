/*
    CC1101_RF library demo
    This is an example of a RF module on the SPI(HSPI) bus of a Nodemcu

    WARNING  : MISO is connected with both D6 and D2. The MCU cannot do digitalRead with MISO(D6)
    when SPI is active, so we digitalRead(D2) instead
    
    This sketch can communicate with all other examples on any platform.
    The examples are on the public domain

      PIN connections
   CC1101         NodeMCU

    CSN           D8(CS)
    CSK(CLK)      D5()
    MISO          D6(MISO) + D2(for digitalRead) (IMPORTANT CC1101-MISO + D6 + D2 are connected)
    MOSI          D7(MOSI)
    GND           GND
    VCC           3.3V
*/

#include <Arduino.h>
#include <SPI.h>
#include <CC1101_RF.h>

//        CSN-PIN  Connected-with-MISO-PIN
CC1101 radio(D8,  D2);

void setup() {
    Serial.begin(9600);
    Serial.println("NodeMCU begin");
    SPI.begin(); // mandatory. CC1101_RF does not start SPI automatically
    radio.begin(433.2e6); // Freq=433.2Mhz. Do not forget the "e6"
    radio.setRXstate(); // Set the current state to RX : listening for RF packets
    // LED setup. It is importand as we can use the module without serial terminal
    pinMode(LED_BUILTIN, OUTPUT);
}

// used for the periodic pings see below
uint32_t pingTimer;
// used for LED blinking when we receive a packet
uint32_t ledTimer;

void loop() {
    // Turn on the LED for 200ms without blocking the loop.
    // The Buildin LED on NodeMCU is ON when LOW
    digitalWrite(LED_BUILTIN, millis()-ledTimer>200);

    if ((millis()-pingTimer>5000)) { // ping every 5sec
        Serial.println("Sending ping");
        // change the string to know who is sending
        bool success = radio.sendPacket("Ping from NodeMCU");
        if (success) {
            Serial.println("Ping sent");
        } else {
            Serial.println("Ping failed due to high RSSI and/or incoming packet");
        }
        // printf is handy but enlarges the firmware a lot
        // radio.printf("time : %lu",millis()/1000); // %lu = long unsigned
        pingTimer = millis();
    }

    // Receive part. if GDO0 is connected with D1 you can use it to detect incoming packets
    //if (digitalRead(D1)) {
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
        ledTimer=millis();
    }
    //}
}
