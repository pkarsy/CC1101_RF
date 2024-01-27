/*
CC1101_RF library demo

This sketch is used together with the "sleep" sketch and sends packets with long preamble
in order to wake the remote RF chip

The examples are on the public domain

*/

#include <Arduino.h>
#include <SPI.h>
#include <CC1101_RF.h>

//    PIN connections - Arduino PIN numbering

//   CC1101       Atmega328p (@3.3V only. The CC1101 chip is not 5V tolerant)
//    CSN           10
//    CSK           13
//    MISO          12
//    MOSI          11
//    GND           GND
//    VCC           3.3V
//    GDO0          NotConnected

const byte BUTTONPIN = 8;
CC1101 radio;
// or to use another pin for CSN (ChipSelect)
// CC1101 radio(A2);

bool debounceButton() {
    static byte buttonState; // debounced state LOW or HIGH (for 50ms)
    static byte lastButtonState;  // the previous reading
    static uint32_t lastChangeTime; // we need to have at least 50ms the same state to be reliable
    // current reading
    byte reading = digitalRead(BUTTONPIN);
    // If the switch changed, due to noise or pressing:
    if (reading != lastButtonState) {
        lastButtonState = reading;
        // reset the debouncing timer
        lastChangeTime = millis();
    }
    if (millis()-lastChangeTime>50) {
        // the reading is ok, it is same for the last 50 ms
        //
        // if the button state has changed:
        if (reading != buttonState) {
            buttonState = reading;
            // only when the state is changed to LOW
            // return true to the caller
            if (buttonState == LOW) {
                return true;
            }
        }
    }
    return false;
}

void setup() {
    Serial.begin(57600);
    Serial.println(F("#####################"));
    Serial.println(F("WakeUP packets emiter"));

    pinMode(BUTTONPIN, INPUT_PULLUP);

    SPI.begin();
    bool ok = radio.begin(433.2e6); // Freq=433.2Mhz
    if (!ok) {
        while(1) {
            Serial.print(F("Cannot find CC1101"));
            delay(1000);
        }
    }
    
    // JUST FOR TEST. See what happems when the receiver finds a packet without the correct syncword
    // Serial.println(F("Warning using different sync word, for testing the sleeping module, generally you do not want to change the syncword"));
    // Serial.println(F("Do not expect the peer to receive any packet, but with an ammeter(mA scale) you will see the receiving RF module drawing ~18mA current"));
    // When the (very long preamble) packet stops. The current consumption should return to 0.1mA
    // radio.setSyncWord(0xBA,0xAD); // Probably a bad SyncWord but the point here is exectly to test what happens when a syncword is not detected

    Serial.println(F("Press 0-9 to send normal packets with only 4 bytes of preamble.\r\nThe receiver will only occasionally receive them."));
    Serial.println(F("Press a-z to send packets with a very long preamble.\r\nThese packets can be detected every time even by a sleeping node."));
    radio.setRXstate(); // Set the current state to RX : listening for RF packets
}

void loop() {
    if (Serial.available()) {
        uint8_t c=Serial.read();
        while(Serial.read()!=-1){}; // reject any pending chars in serial buffer
        // we are sending ascii chars only
        // so the other end can print them
        if (c=='#') {
            const byte test[]="w234567812345678123456781234567812345678123456781234567812345";
            radio.sendPacket(test,sizeof(test)-1,1200);
        }
        else if ( (c>='a' && c<='z') || (c>='A' && c<='Z') ) {
            Serial.print("Got \""); Serial.write(c);Serial.print(F("\". Sending preamble+character ... "));
            bool ok = radio.sendPacket(&c,1,1200); // 1200 = preamble length in millisec
            if (ok) Serial.println(F("OK, packet transmitted"));
            else Serial.println(F("Failed to send packet, high RSSI"));
        } else if (c>='0' && c<='9') {
            bool ok = radio.sendPacket(&c,1,0);
            if (ok) Serial.println(F("1 byte packet with standard preamble transmitted"));
            else Serial.println(F("Failed to send packet, high RSSI"));
        }
    }
    // TODO use the debounce function
    if (digitalRead(BUTTONPIN)==LOW) {
        Serial.println(F("Button press, sending preamble + a single char \"$\""));
        const byte c='$';
        bool ok = radio.sendPacket(&c,1,1200);
        if (ok) Serial.println(F("OK, packet transmitted"));
        else Serial.println(F("Failed to send packet, high RSSI"));
    }

    byte packet[61];
    uint8_t pkt_size = radio.getPacket(packet);
    if (pkt_size>0 && radio.crcok()) {
        Serial.print(F("pkt=\""));
        Serial.write(packet,pkt_size);
        Serial.println(F("\""));
    }
}
