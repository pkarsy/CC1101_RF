/*
    CC1101_RF library demo
    This is an example of a RF module on the SPI(HSPI) bus of a Nodemcu
    The GDO0 pin is connected to D1
    WARNING  : MISO is connected with both D6 and D2. The MCU cannot do digitalRead with MISO(D6)
    when SPI is active, so we digitalRead(D2) instead
    This sketch can communicate with all other examples on any platform.
    The examples are on the public domain
*/

#include <Arduino.h>
#include <SPI.h>
#include <CC1101_RF.h>

//    PIN connections
//
//   CC1101         NodeMCU
//    CSN           D8(CS)
//    CSK(CLK)      D5()
//    MISO          D6(MISO) + D2(for digitalRead) (both)
//    MOSI          D7(MOSI)
//    GDO0          D1
//    GND           GND
//    VCC           3.3V


//          GDO0  CSN  Connected-with-MISO
CC1101 radio(D1,  D8,  D2);

void setup() {
    Serial.begin(115200);
    Serial.println("NodeMCU begin");
    SPI.begin(); // mandatory. CC1101_RF does not start SPI automatically
    radio.begin(433.2e6); // Freq=433.2Mhz. Do not forget the e6
    radio.setRXdefault(); // every send and receive operation reenables RX.
    radio.setRXstate(); // Set the current state to RX : listening for RF packets
    // LED setup. It is importand as we can use the module without serial terminal
    pinMode(LED_BUILTIN, OUTPUT);
}

// used for the periodic pings see below
uint32_t pingTimer=0;
// used for LED blinking when we receive a packet
uint32_t receiveTime;

void loop() {
    // Turn on the LED for 100ms without blocking the loop.
    // The Buildin LED on NodeMCU is ON when LOW
    digitalWrite(LED_BUILTIN, millis()-receiveTime>100);

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
            Serial.println(radio.getLQI());
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
        radio.sendPacket("Ping from NodeMCU");
        // printf is handy but enlarges the firmware a lot
        // radio.printf("time : %lu",millis()/1000); // %lu = long unsigned
        pingTimer = millis();
    }
}
