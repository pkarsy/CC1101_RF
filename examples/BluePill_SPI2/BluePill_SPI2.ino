/*
    Arduino CC1101 library demo
    Generally speaking, there are a million things that can go wrong, so be patient and read
    the comments. And this sketch has a lot !
    This sketch must be uploaded to 2(or more) targets, and at least one connected
    to a pc so you can view the serial terminal. You can connect all serial ports to the same PC but it will
    be a little bit confusing, you will not sure who is sending and who is receiving
    The targets can be of different architectures for example:
    one proMini 3.3V (avr atmega328@8MHz)
    one BluePill (stm32f1x)
    and one Esp8266 board like NodeMCU (Esp8266)
    Unfortunatelly these are all the boards I have so the library is tested only on these.
    For start probably use 2 identical boards. 2 blue pills I would say using a
    SWD(2$ for a ST-link clone) programmer
    or even better a BlackMagicProbe (or another BluePill converted to BlackMagic)
    NOTE ! The radios must have 1m OR MORE distance between them, to be able to receive signals
*/


#include <Arduino.h>

#include <SPI.h>
#include <ArduinoCC1101.h>

SPIClass spi2(2); // for STM32 the second spibus

    // only set this if you are not using the first HW SPI bus
    // of course the begin in the previous line must be the same
    // radio.setSPIbus(&spi2);
    // in that case you also have to set all other settings below

    // if for some reason you need to connect the CC1101-CSN(chip select)
    // pin to a pin other than the platform's SS pin
    // atmega -> 10? stm32->? etc
    // uncomment the directive
    // radio.setCSn(OTHER_PIN_THAN_SS);

    // mCC1101 library uses a pin connected with CC1101 GDO0 as status flag
    // Wire the GDO0 pin with:
    // PB0 for stm32 (it is near to the SPI pins on BluePill)
    // 2 for AVR (proMini etc)
    // todo for esp8266
    // edit and uncomment if the defaults are not ok. If the MCU goes to sleep
    // (for low power projects), the PIN must also be an interrupt capable PIN
    // radio.setGDO0pin(OTHER_PIN);

    // for esp8266 you have to set this
    // for esp8266/esp32 there is an additional problem. The MCU cannot do
    // digitalRead(MISO) while spi is active. The solution here is to connect
    // externally a spare pin with miso, and read this instead of MISO
    // Do not #define for avr and stm
    // the default is TODO
    // WARNING again you need an external wire from MISO to this pin
    // radio.setWiredToMisoPIN(ANOTHER_PIN); //

// the declaration only assigns the pins.
// all chip manipulation happens when we call radio.begin()
CC1101 radio(PA8, PB12, PB14, spi2);

// probably you dont need this
// mCC1101 does not need interrupts to operate due to GDo0 conf
// however interrupts are necessary for low power projects when
// the mcu is in sleep. In that case uncomment this empty function
// and uncomment the attachInterrupt line inside Setup() so
// the mcu can wake up when a valid packet is received. The function
// does not need to have anything inside.
//void interruptHandler(void) {
//}

// Working with the USB serial port on stm32 is really a pain, as dissapears and reapears
// on every reset. So this setting just uses Serial1 wich can be connected to a USB-to-Serial adapter
// such as CP2102 or if you have blackmagic(or created one yourself) to connect it to the
// blackmagick's auxiliary serial port(9600bps)
#ifdef ARDUINO_ARCH_STM32
#define Serial Serial1
#endif

void setup() {
    Serial.begin(9600);
    spi2.begin();
    radio.begin();
    Serial.println("Radio on SPI2 begin");
    radio.setRXdefault();
    radio.setRXstate();
    // the external LED
    pinMode(PB8,OUTPUT);
}


// used for the periodic pings see below
uint32_t pingTimer=0;
// used for LED blinking
uint32_t receiveTime;

void loop() {
    // Blink the LED on PB8
    digitalWrite(PB8, millis()-receiveTime<100);

    // Receive part. With the Setting of IOGd0 we get this only with a valid packet
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
        radio.printf("time=%lu",millis()/1000); // %lu = long unsigned
        pingTimer = millis();
    }
}
