/*
    Arduino CC1101 library demo
    Generally speaking, there are a million things that can go wrong, so be patient and read
    the comments. This sketch has a lot and you probably you want to try the other examples first.
    This sketch must be uploaded to 2(or more) targets, you may want to have at least one connected
    to a pc so you can view the serial terminal. Of course you can rely on the blinking let to
    check connectivity
    The targets can be of different architectures :
    * proMini 3.3V (avr atmega328@8MHz) NOTE : Do not use a 5V arduino like UNO
    * one BluePill (stm32f1x)
    * Esp8266 board like NodeMCU (Esp8266)
    Unfortunatelly these are all the boards I have so the library is tested only on these.
    For start probably use 2 identical boards. 2 blue pills I would say using a
    SWD(2$ for a ST-link clone) programmer
    or even better a BlackMagicProbe (or another BluePill converted to BlackMagic)
    NOTE ! The radios must have 1 meter OR MORE distance between them, otherwise the
    receiver circuits will be saturated.
*/

#include <Arduino.h>
#include <SPI.h>
#include <CC1101_RF.h>

// using the second SPI bus (STM32)
// SPIClass spi(2);

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
// to change the default GDO0 pin (avr=2,stm32f10x=PB0,esp8266=TODO)
// CC1101(OtherGDO0 pin)

// the declaration only assigns the pins.
// all chip manipulation happens when we call radio.begin()
CC1101 radio;

// probably you dont need this
// mCC1101 does not need interrupts to operate due to GDo0 conf
// however interrupts are necessary for low power projects when
// the mcu is in sleep. In that case uncomment this empty function
// and uncomment the attachInterrupt line inside Setup() so 
// the mcu can wake up when a valid packet is received. The function
// does not need to have anything inside.
//void interruptHandler(void) {
//}

// Programming with the USB serial port on stm32 is really a pain, as dissapears and reapears
// on every reset. So this setting just uses Serial1 wich can be connected to a USB-to-Serial adapter
// such as CP2102 or if you have blackmagic(or created one yourself) to connect it to the
// blackmagick's auxiliary serial port(9600bps)
#ifdef ARDUINO_ARCH_STM32
#define Serial Serial1
#endif

void setup() {
    Serial.begin(9600);

    #ifdef ARDUINO_ARCH_STM32
    Serial.println("STM32 here");
    #elif defined(ARDUINO_ARCH_AVR)
    Serial.println("AVR here");
    #elif defined(ARDUINO_ARCH_ESP8266)
    Serial.println("ESP8266 here");
    #elif defined(ARDUINO_ARCH_ESP32)
    Serial.println("ESP32 here")
    #else
    Serial.print("OTHER arch here")
    #endif

    
    // The library does not do this automatically, so you can use another
    // spi or software-spi bus
    SPI.begin();
    // 433.2 MHz Note the argument is uint32_t and not float
    radio.begin(433.2e6);

    // Next commands communicate with the chip (via SPI bus) and cannot be used before begin

    // The defaultis 4800, lower baudrate but longer distance and obstacle penetration
    // radio.setBaudrate4800bps();
    // or
    // radio.setBaudrate38000bps();

    // 99.9% Do not set it
    // Do not use sync word for packet filtering. The default syncword has the best reception capability
    // Use differnet frequences to isolate different projects and addresses for fine tuning
    // TODO se allo module
    // If you want to communicate with modules using the panstamp library you have to use the same
    // syncword and preferably the default one which is 0xD3(sync1),0x91(sync0)
    // byte sync1=0xD3;
    // byte sync0=0x91;
    // radio.writeReg(CC1101_SYNC0, 0x91);
    // radio.writeReg(CC1101_SYNC1, 0xD3);
    // We print it here, mainly to ensure we can communicate with the module
    //Serial.print("Syncword : sync0=0x");
    //Serial.print(radio.readReg(CC1101_SYNC0,CC1101_CONFIG_REGISTER),HEX);
    //Serial.print(" sync1=0x");
    //Serial.println(radio.readReg(CC1101_SYNC1,CC1101_CONFIG_REGISTER),HEX);

    
    // If enabled the cc1101 chip
    // rejects packets with different addresses(the first byte of the packet)
    // In this demo you can enable it at runtime with '=' keypress
    // the default is radio.disableAddressCheck()
    // it is responsibility of the developer to construct a packet with the address as
    // the first byte, and also to strip the first byte from incoming packets.
    //
    // the radio will reject all packets with packet[0]!=212
    // radio.setDevAddress(212);
    // radio.enableAddressCheck();
    //
    // Similar but also accepts packets beginning with 0x00 (broadcast address)
    // radio.enableAddressCheckBcast();
    // radio.enableAddressCheck();

    // Usually do not enable this. See notes in the global section
    // attachInterrupt(digitalPinToInterrupt(CC1101_GDO0), interruptHandler, FALLING);

    // all send.recv operations turn RX at the end
    // the default is return to IDLE
    radio.setRXdefault();

    // Start listening
    radio.setRXstate();
    // ********** radio config END ****************

    Serial.println("Press a few keys to send 1-byte RF packets");
    Serial.println("Special commands are \"=\" and \".\"");
    // The LED is important because is flashing on incoming packets
    // and we can test the module without serial terminal. The buildin is
    // quite dim for outdoor tests
    pinMode(PB9, OUTPUT);
}


// used for tests enabling/disabling address
bool addrCheck = false;
// used for the periodic pings see below
uint32_t pingTimer=0;
// enable/disable the pings
bool ping = true;
uint32_t receiveTime;

void loop() {
    // Turn on the LED connected to PB9 (for 0.1sec) at start, and when a packet arrives
    digitalWrite(PB9, millis()-receiveTime<100);
    
    // Receive part. With the Setting of IOGd0 we get this only with a valid packet
    if (radio.packetReceived()) {  //todo
        byte packet[64];
        byte pkt_size = radio.getPacket(packet);
        // no need to set RX state because of setRXdefault() on setup()
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
            // or other hardware related problem
            Serial.println("No/Invalid packet");
        }
    }
    
    // keyboard handling
    if (Serial.available()>0) {
        byte c = Serial.read();
        switch (c) {
            case '=':
                addrCheck = !addrCheck;
                if (addrCheck) {
                    // Note on real life projects, almost certainly you want to enable/disable address check
                    // at setup() and never touch the setting again
                    Serial.println("Addr check enabled addr=\'a\'. Now only packets with first byte \'a\' will be accepted");
                    // normally we put a number here like 0x61 but 'a'
                    // is printable and suitable for this demo
                    radio.enableAddressCheck('a'); // sets the state to IDLE
                    radio.setRXstate();
                    // or using addrss 0x00 as broadcast
                    // radio.enableAddressCheckBcast('a');
                } else {
                    Serial.println("Addr check disabled. Every packet will be accepted");
                    radio.disableAddressCheck(); // sets the state to IDLE
                    radio.setRXstate();
                }
            break;

            // more conditions here
            case '.':
                ping = not ping;
                if (ping) {
                    Serial.println("Sending pings enabled");
                } else {
                    Serial.println("Sending pings disabled");
                }
            break;

            default:
                if (c<32 || c>126) c='*';
                byte packet[1]={c};
                // using sendPacket with a byte array and a size
                radio.sendPacket(packet,1);
                Serial.print("Sending \"");
                Serial.print((char)c);
                Serial.println("\"");
                // we postpone the ping
                pingTimer = millis();
            break; 
        }
    }

    // periodic pings. Can be used for field tests
    // Comment all the block to disable or set the delay to a big value
    if (ping && (millis()-pingTimer>10000)) { // ping every 10sec
        // for sending packet we have 3 options
        // sendPacket needs a buffer and a size
        // todo

        Serial.println("Sending ping");
        // printf uses char arrays and uses the normal printf formatting
        // max packet size is 60-61 bytes. However is adding to the
        // firmware size significantly due to internal use of sprintf
        //radio.printf("time=%lu",millis()/1000); // %lu = long unsigned
        //
        // using sendPacket with a char[]
        radio.sendPacket("ping");
        // we could do the same with
        // radio.sendPacket((byte*)"ping",4);
        //
        // next ping in 10sec
        pingTimer = millis();
    }
}

