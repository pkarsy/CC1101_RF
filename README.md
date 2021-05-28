### CC1101_RF
Arduino library for Texas Instruments CC1101 chip.
* Based on [elechouse library](https://github.com/simonmonk/CC1101_arduino), with many additions.
* Works with the latest Arduino IDE and with Platformio IDE (2021-).
* Works with any SPI bus provided by the platform or with SoftwareSPI. 2 CC1101 modules can be connected to the same ISP bus, only CSN and GDO0(optional) need to be on different MCU pins.
* Tested to be working with Atmega328(3.3V variants), STM32f103(BluePill both SPI busses) and both arduino cores, esp8266(NodeMCU). It is not using any MCU specific code. It is expected to work after pin tweaking on almost any architecture arduino is ported.
* The developer chooses directly the exact frequency. This is better than choosing the base frequency and selecting channels. The ISM bands (especially outside US) are very narrow and choosing the right frequency is crusial. It is the duty of the developer however to use the available bandwith efficiently and to comply with the national and international standards about radio transmission.
* 4800 and 38000 baudrates.
* Support for WakeOnRadio. Th RF chip (CC1101 in this case) goes to sleep and wakes up periodically to check for incoming message. The use of WakeOnRadio(WOR) together with MCU sleep can dramatically reduce power consumption, allowing RF projects to run literally for years using only battery power, and still be able to receive messages. See the wor folder in the examples.
* Optional GDO0 pin connection. sendPacket and getPacket functions work without relying on the state of the GDO0 pin. However the use of this CC1101 pin is easy (all breakout CC1101 boards populate it) and is needed if we use microcontroller sleep mode or/and WakeOnRadio.
* Even if one is using the GDO0 pin there is no need for interrupt handler(the exception again is Sleep/WakeOnRadio). The reason is that the GDO0 is asserted when a packet is received and stays high until the packet is read. This is a big plus. Interrupt driven logic can be tricky and error prone. 
* Permissive MIT licence.

### Using with Arduino IDE
* cd to Arduino Libraries folder (~/Arduino/libraries on Linux)
```bash
* git clone https://github.com/pkarsy/CC1101_RF.git .
```

### Using with Platformio
In platformio.ini add :
```bash
lib_deps =
    https://github.com/pkarsy/CC1101_RF.git
```

### Pin connections
The pins depend on platform and SPI bus. See the examples.

### Usage
Most of the functionality explained in the examples but here is some code:

```cpp
#include <CC1101_RF.h>

CC1101 radio;

setup() {
    SPI.begin(); // mandatory
    radio.begin(433.4e6); // 433.4 MHz
    // other radio setup like enableAdress etc
    radio.setRXstate();
}

loop() {
    if (some_condition) {
        if (radio.sendPacket(packet,size)) Serial.println("packet sent");
        else Serial.println("fail to send packet"); // high RSSI or currently receiving a packet
    }
    // it is OK to call it continuously, even when no packet is waiting in the CC1101 buffer.
    uint8_t pkt_size=radio.getPacket(buffer);
    if (pkt_size>0 && radio.crcok()) {
        // do something with the packet
    }
}
```
or if GDO0 is connected, use getPacket selectively

```cpp
    if (digitalRead(GDO0pin)) {
        // get the packet, check again for size and crcok.
        // This allows to know when a packet is arrived without quering the
        // chip, offloading the SPI bus and allowing for sleep mode to work
        // see the examples for this
    }
```

Some things to keep in mind :
* Usually most of the time the module must be in RX. This however depends on the communication schema used.
* When a packet is received the module goes to IDLE state and we must do a getPacket(..) as soon as possible in order to be adle to receive more packets. So delay() must be avoided in loop(). The communication is half duplex, so a protocol must be implemented, and every module should know when to transmit and when not. The chip's CCA(Clear Channel Assessment) is enabled of course, but this alone does not guarantee reliable communication.
* In short distances (less than 1m) the modules seem to be "deafened". If for some specific reason the modules are in proximity, the functions setPower5dbm() and setPower0dbm() can be used to reduce the transmit power (the default is 10dbm). Another valid reason to reduce the transmit power is to reduce interference to nearby RF modules.

### Fixing bugs, adding features
* If you found a bug, and want to report it use the [Github Issues](https://github.com/pkarsy/CC1101_RF/issues)

If however you prefer to code:
* If you code with platformio, you need to adjust the serial ports inside platformio.ini
* Even some seemingly innocent changes in register CC1101 settings can break the library. It is recommended to use a target with debugging support. This can be :  
  * A blackmagic probe(or clone) with a STM32 BluePill + vscode + platformio IDE. BMP supports also JTAG so can be used with a lot more platforms than STM32. BMP also supports an auxiliary serial port which can be connected to the target. 
  * A ST-link v2 (or clone) can also be used if the target is STM32.

* there is a var CC1101_DEBUG which can be enabled by editing the CC1101_RF.cpp file or preferably in platformio.ini
* even if you can find multiple unrelated things to fix/improve, please focus on one each time.
* Follow some tutorial about contributing to a github project like [this one](https://akrabat.com/the-beginners-guide-to-contributing-to-a-github-project/), or any other you may prefer.


### API
At the moment look at the source code. The extended example contains comments for the most useful functions.

### Capabilities of the chip
CC1101 at 4800bps and 10dbm can penetrate easily 3-4 reinforced concrete floors, or a few hundred meters without obstacles. This is more than enough for many projects. Of course LoRa devices can do better, but given the lower price, the easier pin connections (at least for the modules found on ebay), and the capability of the same chip to use all sub-GHz ISM bands, means the chip is quite good despite being more than 10 years old.

### Choosing data rate and frequency
Most projects do not require high data rate. for those projects the default (4800bps) is OK.

The frequency selection usually needs more attention however. The frequency must be inside one ISM band, if we talk about a project not requiring gevernment permission !

* https://en.wikipedia.org/wiki/ISM_band
* https://www.thethingsnetwork.org/docs/lorawan/frequencies-by-country.html

Let's say we configure the module for 433.2Mhz. The CC1101 chip (all RF chips basically) use a crystal for precize carrier signal generation. If you are not very unlucky the crystall will have 30ppm error or less. The base frequency then, can be 433.187 - 433.213 MHz. Also the modulation of the signal (GFSK with ~25KHz deviation in this lib) needs a bandwith. Using the "Carson Bandwith Rule" for 4800bps we have 4.8+2*25=55KHz lets say +/- 28KHz for 98% of the power. So our module can emit signals
* from 433.187MHz-28KHz=433.159MHz (worst -30ppm sending "0")
* up to 433.213MHz + 28KHz = 433.241MHz (worst +30ppm sending "1")
Both extremes are well inside the ISM band.

The story does not end here however, the receiver also has a crystal ! And this crystal can have the oposite ppm error than the transmitter. For example :

* The transmitter sends 433.187MHz - 28KHz = 433.159 (worst -30ppm sending "0")
* The receiver listens at 433.213MHz (worst +30ppm).

Consequently the receiver needs a "window" of +/-(433.213-433.159) or +/-54Khz or 108KHz "BWchannel" as CC1101 documentation calls it. This is about the setting in this library (101KHz). Generally is not a good idea to use a larger than needed BWchannel setting, as the chip then collects a lot of noise, and signals from other ISM working devices. For 868 band the required "BWchannel" is somewhat higher but at the moment this lib does not change the setting. RFstudio (TI's software) has some preconfigured settings with this BWchannel, and they know better. A function tweaking the BWchannel may be added if a such a need(reception problems) arises.

The above calculations show that we have to isolate nearby projects with at least ~100 to ~150KHz difference in frequency. Probably even 200KHz as RFStudio suggest(channel spacing). For example:
* one project with 433.2Mhz : radio.begin(433.2e6)
* another nearby project and isolated (no need for communication) to the first at 433.35MHz : radio.begin(433.35e6)

Even then, expect some disturbance from unrelated nearby devices. For example some garage doors use 433.42MHz +/- unknown ppm

Another consideration is which ISM band to use: Should I choose 433 or 868MHz ? (And there are more) Both seem to be allowed in Europe. Some 868 sub-bands allow 25mW or even 500mW. This is actually not good for us, as CC1101 can transmit only 10mW and the module will compete with higher power modules. Others say that 868 is in fact better, and that 433 is more crowded. Note that the antennas do not perform the same on every frequency. And finally the rules for frequency and power/time allocation are somewhat complex. You have to do your tests to be sure, and read the rules for your country.
