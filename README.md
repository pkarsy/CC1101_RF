### CC1101_RF

Arduino library for Texas Instruments CC1101 chip. It is based on [elechouse library](https://github.com/simonmonk/CC1101_arduino). The scope is to transfer packets between 2 nodes and not to implement the various modes of operation(OOK for example) and special characteristics CC1101 has.

### WARNING: If you are using the GDO0 pin on your project, as of 26/1/2024 there are some incompatible changes
Even if you are not using this feature, it is recommended to use the updated library which has much better packet detection, to avoid false packets.
 Some characteristics :

* Works with Arduino IDE and with Platformio.
* Works with hardware SPI, or with Software SPI.
* Tested with Atmega328(3.3V variants), STM32f103(BluePill etc), ESP-8266. It does not use any MCU-specific code. It is expected to work after pin tweaking on any architecture Arduino is ported.
* The developer chooses directly the exact carrier frequency. This is better than choosing the base frequency and selecting channels. The ISM bands (especially outside the US) are very narrow and choosing the right frequency is crucial. It is the duty of the developer however to use the available bandwidth efficiently and to comply with the national and international standards of radio transmission.
* 4800(the default) and 38000 baudrates. As it happens with radio transmission, 4800 is slower but with much better reception capability.
* Simple interface. Sending packets is a synchronous operation. No callback function (to signal the end of transmission). Of course, an asynchronous send will free the chip earlier to do other jobs but adds to the complexity, so it is not implemented.
* Most of the time also no need for interrupts/callbacks for packet receive (see below).
* The maximum packet size is 61 bytes(library limitation). This implementation choice again simplifies the programming interface. With 61 bytes the internal CC1101 buffer never overflows. And if desired, there is a command to further limit the maximum packet size.
* **Support for WakeOnRadio**. CC1101 goes to sleep and wakes up periodically to check for incoming messages. The use of WakeOnRadio(WOR) together with MCU sleep can dramatically reduce power consumption, allowing projects to run for years using only battery power, and still be able to receive RF messages. You can check the pingLowPower example.
* sendPacket and getPacket functions work without relying on the state of the GDO0 pin. The use of this CC1101 pin is needed only if we use microcontroller sleep mode and/or WakeOnRadio.
* Interoperability with other CC1101 libraries is not implemented as it adds complexity.
* Permissive MIT license.

### Using with Arduino IDE
cd to Arduino Libraries folder (~/Arduino/libraries on Linux)
```bash
git clone https://github.com/pkarsy/CC1101_RF.git .
```

### Using with Platformio
No need to install anything. In platformio.ini just add :
```bash
lib_deps =
    https://github.com/pkarsy/CC1101_RF.git
```

### Pin connections
The pins depend on the platform and SPI bus. See the examples.

### Usage
Here is some code (Platformio) :

```cpp
#include <Arduino.h>
#include <SPI.h>
#include <CC1101_RF.h>

// See the examples how to use diffrent pinout/spi interface
CC1101 radio;

void setup() {
    SPI.begin(); // mandatory
    bool ok = radio.begin(433.4e6); // 433.4 MHz
    if (!ok) {
        Serial.println("CC1101 is not detected");
        while(1);
    }
    // here you can change other radio
    // settings like enableAdress etc
    radio.setRXstate();
}

void loop() {
    if (some_condition) {
        // This is the largest packet size,  you can use a smaller buffer in you app
        byte packet[61];
        // fill the packet with data up to 61 bytes
        if (radio.sendPacket(packet, size)) Serial.println("packet sent");
        else Serial.println("fail to send packet"); // high RSSI or currently receiving a packet
        // No need to setup a buffer, maximum strlen = 61. If the size is larger the message will
        // be truncated
        bool ok = radio.sendPacket("Hello world!");
        if (ok) ....
        // or
        bool ok = radio.printf("millis()=%lu", millis());
        if (ok) ....
    }
    // Receing part
    // As the incoming packet can have any size it is recommended to use 64 bytes buffer
    // even if you limit the packet size with setMaxPktSize(), even if you know the peer
    // is sending shorter packets. The use of an unsufficient buffer size can easily crash
    // the application
    byte packet[64];
    // it is OK to call it continuously, even when no packet is waiting in the CC1101 buffer.
    uint8_t pkt_size=radio.getPacket(packet);
    if (pkt_size>0 && radio.crcok()) {
        // do something with the packet
    }
}
```
### Low Power mode
If you are going to use WakeOnRadio and/or MCU sleep you will need to connect the CC1101 GDO0 pin
to some MCU pin capable of interrupts. See the examples/pingLowPower project.
WARNING: After 0.7.4 brach, the GDO0 behavior is changed. The old library had a bug that could cause the RF chip to exit RX or WoR state without the MCU ever getting a GDO0 interrupt, making the module unable to receive other packets and unable to send the CC1101 chip again in low power mode.

### Examples
First, you need to download the library locally. Then the examples can be opened as separate platformio projects, but also by opening the main library using platformio and selecting a platformio.ini target.

### Some things to keep in mind :
* Usually most of the time the module must be in RX. This however depends on the communication schema used.
* When a packet is received the module goes to IDLE state and we must do a getPacket(buf) as soon as possible to be able to receive more packets. So delay(msec) and generally blocking operations must be avoided in loop(). The communication is half-duplex, so a protocol must be implemented, and every module should know when to transmit and when to listen. The chip's CCA(Clear Channel Assessment) is enabled of course, but this alone does not guarantee reliable communication.
* It is very tempting to use SyncWord to isolate nearby projects but this is a very bad practice. The role of SyncWord is for packet detection, NOT FOR PACKET FILTERING. Use setFrequency(freq) and/or setAddress(addr) for filtering and leave the SyncWord as is.
* To reduce interference to nearby RF modules the functions setPower5dbm() and setPower0dbm() can be used. This also allows communication in short distances (less than 1m) where the signal is very strong.

### Fixing bugs, adding features
* If you found a bug, and want to report it use the [Github Issues](https://github.com/pkarsy/CC1101_RF/issues)

### API
Look at the source code. The examples contain comments for the most useful functions.

### Capabilities of the chip
CC1101 at 4800bps and 10dbm can easily penetrate 3-4 reinforced concrete floors, or a few hundred meters without obstacles using simple modules with pre-soldered spring antennas. This is more than enough for many projects. Simple copper wire 17cm antennas usually perform much better. And using directional antennas can go even further.  Of course, LoRa devices can do better but there is no compelling reason to use them if the project does not need such capabilities. Given the lower price, the easier pin connections (at least for the modules found on Ebay/Aliexpress), and the capability of the same chip to use all sub-GHz ISM bands, means the CC1101 chip is quite useful, despite being more than 10 years old.
The WakeOnRadio capabilities are very good enabling the chip to be suitable for low-power projects.

### Choosing data rate and frequency
Most projects do not require a high data rate. for those projects the default (4800bps) is OK.

The frequency selection usually needs more attention. The frequency must be inside an ISM band, otherwise government permission is required!

* https://en.wikipedia.org/wiki/ISM_band
* https://www.thethingsnetwork.org/docs/lorawan/frequencies-by-country.html

Let's say we configure the module for 433.2Mhz. The CC1101 chip (all RF chips basically) uses a crystal for precise carrier signal generation. If you are not very unlucky the crystal will have a 30ppm error or less. The base frequency then, can be 433.187 - 433.213 MHz. Also, the modulation of the signal (GFSK with ~25KHz deviation in this lib) needs a bandwidth. Using the "Carson Bandwith Rule" for 4800bps we have 4.8+2*25=55KHz let's say +/- 28KHz for 98% of the power. So our module can emit signals
* from 433.187MHz-28KHz=433.159MHz (worst -30ppm sending "0")
* up to 433.213MHz + 28KHz = 433.241MHz (worst +30ppm sending "1")
Both extremes are well inside the ISM band.

The story does not end here however, the receiver also has a crystal! This crystal can have the opposite ppm error than the transmitter. For example :

* The transmitter sends 433.187MHz - 28KHz = 433.159 (worst -30ppm sending "0")
* The receiver listens at 433.213MHz (worst +30ppm).

Consequently the receiver needs a "window" of +/-(433.213-433.159) or +/-54Khz or 108KHz "BWchannel" as CC1101 documentation calls it. This is about the setting in this library (101KHz). Generally is not a good idea to use a larger-than-needed BWchannel setting, as the chip then collects a lot of noise, and signals from other ISM working devices. For the 868Mhz band the required "BWchannel" is somewhat higher but at the moment this lib does not change the setting. RFstudio (TI's software) has some preconfigured settings with this BWchannel, and they know better. A function tweaking the BWchannel may be added if such a need(reception problems) arises.

The above calculations show that we have to isolate nearby projects with at least ~100 to ~150KHz gap. Probably even 200KHz as RFStudio suggests (channel spacing). For example:
* one project with 433.2Mhz : radio.begin(433.2e6)
* another nearby project and isolated (no need for communication) to the first at 433.35MHz: radio.begin(433.35e6)

Even then, expect some disturbance from unrelated nearby devices. For example, some garage doors use 433.42MHz (+/- what is needed for modulation and crystal error)

Another consideration is which ISM band to use: Should we choose 433 or 868MHz? (And there are more) Both seem to be allowed in Europe. Some 868MHz sub-bands allow 25mW or even 500mW. This is not good for us, as CC1101 can transmit only 10mW and the module will compete with higher power modules. Others say that 868 is better and that 433 is more crowded. Note that the antennas and the modules do not perform the same on every frequency. And finally, the rules for frequency and power/time allocation are somewhat complex. You have to do your tests to be sure and read the rules for your country.
