/*
CC1101_RF library demo

This sketch is used together with the "sleep" sketch and sends packets with long preamble
in order to wake the remote RF chip

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
//    GND           GND
//    VCC           3.3V

CC1101 radio;

// use the Serial1 port instead of the USB
#define Serial Serial1

void setup() {
    Serial.begin(9600);
    Serial.println("Wake pulses emiter");
    Serial.println("Press 0-9 to send normal packets with only 4 bytes of preamble");
    Serial.println("Press a-z to send packets with a very long 0.5sec preamble");
    SPI.begin();
    radio.begin(433.2e6); // Freq=433.2Mhz
    radio.setRXstate(); // Set the current state to RX : listening for RF packets
}

void loop() {
    if (Serial.available()) {
        uint8_t c=Serial.read();
        while(Serial.read()!=-1){}; // reject any pending chars in serial buffer
        // we accept ascii chars only
        // because the other end can print them
        if ( (c>='a' && c<='z') || (c>='A' && c<='Z') ) {
            //byte packet[]={c};
            bool success = radio.sendPacket(&c,1,525); // a little more than 0.5s
            if (success) Serial.println("0.5s preamble + 1 byte packet transmitted");
            else Serial.println("Failed to send packet, high RSSI");
        /* } else if (c=='0') {
            byte p[]="ena megalo test me polla bytes alla kai pali mikro einai";
            bool success = radio.sendPacketP(p,57-1,0);
            Serial.println(success);
        } else if (c=='1') {
            uint32_t time=micros();
            bool success = radio.sendPacket(&c,1);
            time=micros()-time;
            if (success) Serial.println("1 byte using sendPacket()");
            else Serial.println("Failed to send packet, high RSSI");
            Serial.println(time); */
        } else if (c>='0' && c<='9') {
            uint32_t time=micros();
            bool success = radio.sendPacket(&c,1,0);
            //time=micros()-time;
            Serial.println(success);
            if (success) Serial.println("1 byte packet with standard preamble transmitted");
            else Serial.println("Failed to send packet, high RSSI");
            //Serial.println(time);
        }
    }   

    byte packet[61];
    uint8_t pkt_size = radio.getPacket(packet);
    if (pkt_size>0 && radio.crcok()) {
        Serial.print("pkt=\"");
        Serial.write(packet,pkt_size);
        Serial.println("\"");
    }
}
