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
    CSN           A0 (The library default is 10)
    CSK           13
    MISO          12
    MOSI          11
    GND           GND
    VCC           3.3V
    GDO0          A1 (The library needs this PIN only on WoR/MCU sleep, so no default)
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

// Use A0 for CSN instead of the default 10
CC1101 radio(A0);
// For WoR and/or Sleep we need the GDO0 pin
const uint8_t GDO0 = A1;
// we need an indicating LED
const byte LEDPIN=7;
// The button sends a predefined packet to the other nodes to wake them up
const byte BUTTONPIN = 8;

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

byte BUTTON_STATUS = HIGH;
// PCINT for D8 to D13
ISR (PCINT0_vect) 
{    
    if (BUTTON_STATUS==HIGH) BUTTON_STATUS=digitalRead(BUTTONPIN);
}

// PCINT for A0 to A5
volatile byte GDO0_STATUS=LOW;
ISR (PCINT1_vect)
{
    // We use this ISR because CC1101-GDO0 is connected to A0
    // As PCINT cannot distinguish between LOW->HIGH or HIGH->LOW changes
    // we assert the GDO0_STATUS to HIGH(1) only if the GDO0 is actually HIGH.
    if (GDO0_STATUS==LOW) GDO0_STATUS=digitalRead(GDO0);
    // Notice that this ISR never set the GDO0_STATUS to LOW so we are sure
    // that the loop() eventually will notice it set it to LOW
}  

// PCINT for D0 to D7
// ISR (PCINT2_vect) 
// {
// 
// }

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

// This function puts the MCU in a low power state to save battery energy
// and at the same time enables interrupts to detect a a GDO0 change
// Almost all the time MCU is sleeping inside this function.
void deepSleep() {
  // The smallest LED will draw a few mA destroying this LowPower project.
  digitalWrite(LEDPIN, LOW);
  Serial.println(F("MCU sleep. Waiting for GDO0 change"));
  Serial.flush(); // not sure if Serial.end is doing that
  Serial.end(); // Disable the Serial port to save power or interference with the programmer pins
  //radio.flush();
  radio.wor(); // TODO flush queues
  // not sure these are useful
  // Setting the following pins to LOW minimizes power usage
  // ACSR = (1<<ACD); //Disable the analog comparator
  // ADCSRA &= ~(1<<ADEN); //Disable Digital 
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  while (1) {
    // this is the recommended AVR code by the manufacturer
    cli();
    sleep_enable();
    sleep_bod_disable();
    sei();
    sleep_cpu(); // At this point the MCU is sitting idle whaiting for GDO0 change
    sleep_disable(); // at this point the MCU is waking from the interrupt
    // The settings of the library are such that whenever We have a SyncWord
    // the GDO0 pin goes HIGH
    if (GDO0_STATUS==HIGH) {
        GDO0_STATUS=LOW;
        break;
    }
  }
  digitalWrite(LEDPIN, HIGH);
  Serial.begin(57600);
}


void setup() {
    // This is the maximum a Atmega328@8MHz can do with HW serial
    Serial.begin(57600);
    Serial.println(F("##########################"));
    Serial.println(F("Sleeper begin. Send packets from wake sketch to wakeup this module"));
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
   
    // A printable character is better than a number ie 0 or 1 as we can print
    // the incoming packet and actually "see" the address as character #
    // Another good option is '1' '2' 'A' 'B' etc.
    
    //radio.enableAddressCheck('w');
    radio.setRXstate(); // Set the current state to RX : listening for RF packets
    //
    pciSetup(GDO0);
}

void loop() {
    // The only interrupt set to wake the MCU is GDO0
    // so we have incoming packet
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
        timer=millis();
    }
}