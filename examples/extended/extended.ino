/*
    Arduino CC1101 library demo. This one targets BluePill but easily can adapted to other
    targets. This example can communicate with all other examples

    PIN connections. 6 pins are necessary

   CC1101       Blue/BlackPill

    CSN           PA4(SS1)
    CSK           PA5(SCK1)
    MISO          PA6(MISO1)
    MOSI          PA7(MOSI1)
    GND           GND
    VCC           3.3V

    GDo0 is optional see getPacket section

*/

#include <Arduino.h>
#include <SPI.h>
#include <CC1101_RF.h>

CC1101 radio;

// CC1101_RF does not need interrupts to operate.
// however interrupts are necessary for low power projects when
// the mcu is in sleep. In that case uncomment this empty function
// and uncomment the attachInterrupt line inside Setup() so 
// the mcu can wake up when a valid packet is received. The function
// does not need to have anything inside. Sleep is minimally tested
//void interruptHandler(void) {
//}

// Programming with the USB serial port on stm32 is really a pain, as dissapears and reapears
// on every reset. So this setting just uses Serial1 wich can be connected to a USB-to-Serial adapter
// such as CP2102 or if you have blackmagic(or created one yourself) to connect it to the
// blackmagick's auxiliary serial port(9600bps)
// #define Serial Serial1

void setup() {
    Serial.begin(9600);
    Serial.println("BluePill extended example");

    // The library does not start SPI automatically. If your program stucks at radio.begin()
    // it is probably because you forgot this. The reason such a simple thing is not handled by
    // radio.begin is that we can start SPI with special options if such a need exists. Another
    // one is there may be 2 modules sharing the same SPI bus and of course 1 SPI.begin is needed.
    SPI.begin();
    // 433.2 MHz . The argument is uint32_t (433200000) , not float
    radio.begin(433.2e6);

    // Next commands communicate with the chip (via SPI bus) and should be used after begin
    // and before setRXstate()

    // The default is 4800, lower baudrate but longer distance and obstacle penetration
    // radio.setBaudrate4800bps(); // no need to set, is the default
    // or
    // radio.setBaudrate38000bps(); // higher baudrate but shorter distance.

    // radio.setPower5dbm();
    // radio.setPower0dbm();  // probably only good for tests at very short distances
    // radio.setPower10dbm(); // this is the default

    // 99.9% Do not set it
    // Do not use sync word for packet filtering. The default syncword has the best reception capability
    // Use differnet frequences to isolate different projects and addresses for fine tuning
    // If you want to communicate with modules using other syncwords, you have to use the same
    // syncword however. (But also same symbolrate frequency whitening modulation etc.)
    // the default one which is 0x91(sync0), 0xD3(sync1)
    // radio.setSyncWord(0x91, 0xD3); // this is the default, no need to set
    
    // If enabled the cc1101 chip
    // rejects packets with different addresses(the first byte of the packet)
    // In this demo you can enable it at runtime with '=' keypress
    // the default is radio.disableAddressCheck()
    // it is responsibility of the developer to construct a packet with the address as
    // the first byte, and also to strip the first byte from incoming packets.
    //
    // the radio will reject all packets with packet[0]!=212
    // radio.enableAddressCheck(212);
    //
    // Similar but also accepts packets beginning with 0x00 (broadcast address)
    // radio.enableAddressCheckBcast(212);
    //
    // the default is
    // radio.disableAddressCheck();

    // See notes in the global section
    // attachInterrupt(digitalPinToInterrupt(CC1101_GDO0), interruptHandler, FALLING);

    // Start listening. Should be the last radio related command inside setup()
    radio.setRXstate();

    Serial.println("Press a few keys to send 1-byte RF packets");
    Serial.println("Special commands are \"=\" and \".\"");

    // LED setup. It is importand as we can use the module without serial terminal
    pinMode(LED_BUILTIN, OUTPUT);
    // or use an external more visible LED for outdoor tests
    // pinMode(PB9, OUTPUT); // A LED connected to PB9 - GND
}


// used for tests enabling/disabling address
bool addrCheck = false;
// used for the periodic pings see below
uint32_t pingTimer=0;
// enable/disable the pings
bool ping = true;
uint32_t ledTimer;

void loop() {
    // Turn on the LED for 200ms without block. The Buildin LED on bluepill is ON when LOW
    digitalWrite(LED_BUILTIN, millis()-ledTimer>200);
    // or external LED. The "<" is because this LED is ON when HIGH
    // digitalWrite(PB9, millis()-ledTimer<200);
    
    // Receive part. You can uncomment the digitalRead if you have GDO0 connected to PB0
    // if (digitalRead(PB0)) {
        byte packet[64];
        byte pkt_size = radio.getPacket(packet);
        // no need to set RX state because of setRXdefault() on setup()
        
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
        } else {
            // without GDo0 is noisy
            // Serial.println("No/Invalid packet");
        }
    //}
    
    // keyboard handling
    if (Serial.available()>0) {
        byte c = Serial.read();
        while (Serial.read()!=-1); // discard excessive keypresses
        switch (c) {
            case '=':
                addrCheck = !addrCheck;
                if (addrCheck) {
                    // Note on real life projects, almost certainly you want to enable/disable address check
                    // at setup() and never touch the setting again
                    Serial.println("Addr check enabled addr=\'a\'. Now only packets with first byte \'a\' will be accepted");
                    // normally we put a number here like 0x61 but 'a'
                    // is printable and suitable for this demo
                    radio.enableAddressCheck('a'); // sets the state to IDLE.
                    radio.setRXstate();
                    // or using addrss 0x00 as broadcast
                    // radio.enableAddressCheckBcast('a');
                } else {
                    Serial.println("Address check disabled.");
                    radio.disableAddressCheck(); // sets the state to IDLE
                    radio.setRXstate();
                }
            break;

            case '.':
                ping = not ping;
                if (ping) {
                    Serial.println("Sending pings enabled");
                } else {
                    Serial.println("Sending pings disabled");
                }
            break;

            default:
                if (c<32 || c>126) break;
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

    if (ping && (millis()-pingTimer>5000)) { // ping every 5sec
        //
        // using sendPacket with a char[]
        bool success = radio.sendPacket("ping");
        if (success) {
            Serial.println("Ping sent");
        } else {
            Serial.println("Ping failed due to high RSSI and/or incoming packet");
        }
        //
        // we could do the same with this one. This is the most importand function
        // for machine to machine communication.
        // bool success = radio.sendPacket((byte*)"ping",4);
        //
        // printf uses char arrays and uses the normal printf formatting
        // max packet size is 60-61 bytes. However is adding to the
        // firmware size significantly due to internal use of sprintf
        // bool success = radio.printf("time=%lu",millis()/1000); // %lu = long unsigned
        //
        // next ping in 10sec
        pingTimer = millis();
    }
}

