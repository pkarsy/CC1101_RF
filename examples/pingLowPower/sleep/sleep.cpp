/*
CC1101_RF library demo
The examples are on the public domain

NOTES:
- The sketch is only tested with platformio

- A bare Atmega328p is an excellent choice for low power projects. 
Any external connection however, Serial, I2C, pullup resistors, LEDs etc can change the results dramatically.

- Some smaller AVR chips like the attiny family can be more
suitable due to smaller size but I have not tested them. The attiny13a (which is too popular) is
way too limited in flash RAM and pins to be useful with CC1101_RF. Attiny85 probably has enough flash
and ram but the pins are still very few for something useful.

- The sketch sets the RF chip to WakeOnRadio mode AND the avr chip to sleep mode.
The MCU can wake when receiving a packet from the "wake" sketch. Note that normal(short)
packets cannot easily(or at all) wake an RF chip in WOR mode, because the radio is turned on for a few milliseconds
or even lower, every 0.5 or 1 sec
To be able to reliably wake the radio we need packets with a long preamble (010101010......)
to ensure that the Radio module will eventually get the signal.

- The use of GDo0 is mandatory in WoR and/or MCU sleep mode
When radio is in WOR mode (Wake on Radio) we cannot query it using the SPI
bus as this defeats the primary purpose which is to save energy. In addition
there is no much point to enable WOR (saving about 15mA) without enabling
LowPower MCU sleep modes (saving 30~50mA on STM32F103 or 10-20mA on atmega328)
and going to uA range

PIN connections - Arduino PIN numbering

   CC1101       Atmega328p (@3.3V only. The CC1101 chip is not 5V tolerant)
    CSN           10
    CSK           13
    MISO          12
    MOSI          11
    GND           GND
    VCC           3.3V
    GDO0          9 (The library needs this PIN only on WoR/MCU sleep, so no default)
    GDO1-2        Not Connected (CC1101_RF never uses them but may be used with suitable register settings)

Also:
Connect a LED to Pin 7  (with a resistor as usual).
The led will turn on whenever the atmega wakes up.

If you use a UART programmer ensure the pins do not draw current when you are in hibernation
(unplug it in doubt)

If all is correct the curent draw should be ~0.1mA=100uA(mean value)
This 0.1mA is the voltage regulator (HT7333-A or MC1700) (5-7uA) The atmega328p (5-10 uA) and RF
(80uA with thesese specific WoR settings).
This is of course years on battery power, even a supercapacitor can power this module for days.
If we use longer WoR periods we can reduce the current even more but the response time will be larger.
Another option is to reduce the voltage to i.e 2.5V but at least atmega328p seems to be sold to be
used to 2.7V minimum. Older documents mention even 1.8V. but I did not test them. The RF module seems to work
OK at these voltages (or at least 2.2V)
*/

#include <Arduino.h>
#include <SPI.h>
#include <CC1101_RF.h>

// the CC1101 with the default pinout.
CC1101 radio;

// we need an indicating LED
const byte LEDPIN = A3;

// not used in the sleep module.
// const byte BUTTONPIN = 8;

// For WoR and/or Sleep we need the GDO0 pin
const uint8_t GDO0 = 9;

// Using PinChange Interrupts. Using the interrupts on Pin2 and Pin3 is very limiting
// Only 2 pins and you cannot use other pins. PCI on the other hand can use any pin

// Convenience function to enable PCINT(PinChangeInterrupt) on a PIN
// The important thing about PCINT is that it can wake a sleeping avr chip even from
// power down mode. Typically we set a Pin to INPUT(or INPUT_PULLUP if the pin is floating) mode
// and waiting for a change HIGH->LOW or LOW->HIGH
void pciSetup(const byte pin)
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // activate pin in PCMSK
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear interrupt flag in PCIFR
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group in PCICR
}

volatile byte GDO0_STATUS=LOW;
// PCINT for D8 to D13
ISR (PCINT0_vect) 
{
    // We use this ISR because CC1101-GDO0 is connected to A0
    // As PCINT cannot distinguish between LOW->HIGH or HIGH->LOW changes
    // we assert the GDO0_STATUS to HIGH(1) only if the GDO0 is actually HIGH.
    if (GDO0_STATUS==LOW) GDO0_STATUS=digitalRead(GDO0);
    // Notice that this ISR never set the GDO0_STATUS to LOW so we are sure
    // that the loop() eventually will notice it set it to LOW
}

// if unsure about which ISR to use you can enable all ISR's at the same time

// PCINT for A0 to A5
//ISR (PCINT1_vect)
//{
//    if (GDO0_STATUS==LOW) GDO0_STATUS=digitalRead(GDO0);
//}  

// PCINT for D0 to D7
//ISR (PCINT2_vect) 
//{
//    if (GDO0_STATUS==LOW) GDO0_STATUS=digitalRead(GDO0);
//}

// This function puts the MCU and the RF chip in a low power state to save battery energy
// and at the same time enables interrupts to detect a a GDO0 change
// Almost all the time MCU is sleeping inside this function.
// Every MCU unfortuantelly needs its own method
void deepSleep() {
  // The smallest LED will draw a few mA destroying a LowPower project.
  digitalWrite(LEDPIN, LOW); // some internet sources suggest to set as output
  Serial.println(F("MCU in sleep. CC1101 in WoR.\r\nWaiting for GDO0 to be asserted from RF chip."));
  Serial.flush(); // not sure if Serial.end() is doing that
  Serial.end(); // Disable the Serial port to save power or interference with the programmer pins
  radio.wor(); // TODO flush queues
  // not sure these are useful.
  // ACSR = (1<<ACD); //Disable the analog comparator
  // ADCSRA &= ~(1<<ADEN); //Disable Digital 
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // the lowest power consumption
  while (1) {
    // this is the recommended AVR sleep code by the manufacturer
    cli();
    sleep_enable();
    sleep_bod_disable();
    sei();
    sleep_cpu(); // Here the MCU is sitting idle whaiting for GDO0 change
    sleep_disable(); // at this point the MCU is waking from the interrupt
    // The settings of the library are such that whenever We have a SyncWord
    // the GDO0 pin goes HIGH
    if (GDO0_STATUS==HIGH) { // The interrupt handler did the change
        GDO0_STATUS=LOW; // we reset it to LOW to be ready for the next time
        break;
    }
  }
  digitalWrite(LEDPIN, HIGH);
  Serial.begin(57600);
}


void setup() {
    // This is the maximum an Atmega328@8MHz can do with HW serial
    Serial.begin(57600);
    Serial.println(F("##########################"));
    Serial.println(F("Sleep begin.\r\nSend packets from the \"wake\" sketch to wakeup this module"));
    Serial.flush();
    SPI.begin(); // mandatory. CC1101_RF does not start SPI automatically
    bool ok = radio.begin(433.2e6); // Freq=433.2Mhz
    if (!ok) {
        while(1) {
            Serial.print(F("Cannot find CC1101"));
            delay(1000);
        }
    }
    // LED setup. It is important (for testing) as we can use the module without serial terminal
    pinMode(LEDPIN, OUTPUT);
    // Serial.print(F("The module will wake up, but only packets starting with \'w\' will be accepted"));
    // radio.enableAddressCheck('w');
    radio.setRXstate(); // Set the current state to RX : listening for RF packets
    //
    pciSetup(GDO0);
}

void loop() {
    // We are looking for a packet for a while and then we are going to sleep
    static uint32_t timer;
    // 64 is the required buffer size for getPacket
    byte packet[64];
    byte pkt_size = radio.getPacket(packet);
    if (pkt_size>0 && radio.crcok()) { // We have a valid packet
        Serial.print("Pkt received after ");
        Serial.println(millis()-timer);
        timer = millis();
        Serial.print(F("Got packet \""));
        Serial.write(packet, pkt_size);
        Serial.print(F("\" len="));
        Serial.print(pkt_size);
        Serial.print(F(" Signal=")); // for field tests to check the signal strength
        Serial.print(radio.getRSSIdbm());
        Serial.print(F(" LQI=")); // for field tests to check the signal quality
        Serial.println(radio.getLQI());
    }
    // The MCU wakes when a SyncWord is received
    // The longest time to receive a 61 byte packet@4800baud is 110ms so we just need to wait
    // lets say 120ms. If you are using smaller packets you will need less than 120ms but generally
    // the energy is so little you do not want to be bothered with this
    if (millis()-timer>120) {
        deepSleep();
        // we reset the timer. Now the code will spend another 120ms inside loop()
        timer=millis();
    }
}
