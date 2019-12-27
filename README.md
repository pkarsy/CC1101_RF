## CC1101_RF
Arduino library for Texas Instruments CC1101 chip. Implements a small but useful subset of the chip's functionality.
* Based on elechouse library, with many additions.
* Works with the latest Arduino IDE(1.8.10) and with Platformio.
* No need for interrupt handler. An empty interrupt handler may needed for wake from MCU sleep.
* Works with any SPI bus provided by the platform or with SoftwareSPI. You can even connect 2 CC1101 modules to the same ISP bus, only CSN and GDO0 need to be on different MCU pins.
* Tested to be working with Atmega328(3.3V variants), STM32f103(BluePill both SPI busses), esp8266(NodeMCU). I expect it to work after pin tweaking on almost any architecture arduino is ported.
* The developer chooses directly the exact frequency. This is in my opinion much better than choosing the base frequency and selecting channels. The ISM bands (especially outside US) are very narrow and choosing the right frequency is crusial. It is the duty of the developer however to comply with the national and international standrds about radio frequences.
* 4800 and 38400 baudrates. More can be added but these seem to be ok.

### Installation with Arduino IDE
The IDE does not like the -master suffix github generates so DO NOT Clone->Download ZIP. Instead :
* Get directly the file CC1101_RF.zip from the root of the repository
* Start the Arduino IDE and from the Sketch menu do Sketch->Include Library->Add ZIP Library and select the ZIP you just downloaded.

Alternatively (easiest for Linux but probably works everywere "git" command is available)
* cd to Arduino Libraries folder (~/Arduino/libraries on Linux)
* git clone https://github.com/pkarsy/CC1101_RF.git

### Installation with Platformio
TODO you can install it using the library manager

## Pin connections
The pins depend on platform and SPI bus. See the examples.

## Usage
Most of the functionality explained in the examples, especially in the extended example.

## Choosing frequency and data rate
Most projects do not require high data rate. for those projects the default (4800bps) is OK. The frequency selection usually needs more though however. The frequency must be inside one ISM band (If we talk about a project not requiring gevernment permission !).

* https://en.wikipedia.org/wiki/ISM_band
* https://www.thethingsnetwork.org/docs/lorawan/frequencies-by-country.html

Let's say we configure the module for 433.2Mhz. The CC1101 chip (all RF chips basically) use a crystal for precize carrier signal generation. If you are not very unlucky the crystall will have 30ppm error or less. The base frequency then, can be 433.187 - 433.213 MHz. Also the modulation of the signal (GFSK with ~25KHz deviation in this lib) needs a bandwith. Using "Carson banwith rule" for 4800bps we have 4.8+2*25=55KHz lets say 2*28KHz for 98% of the power.
. So our module can emit signals 433.187MHz-28Kz=433.159MHz up to 433.213MHz + 28 KHz = 433.241MHz which is well inside the ISM band. The story does not end here however, the receiver can have a crystal with the oposite ppm error than the transmitter. For example
* The transmitter sends 433.187MHz +/- 28KHz = 433.159-433.215 MHz (worst -30ppm)
* The receiver listens at 433.213MHz (worst +30ppm)
Finally the receiver needs a "window" of +/-(433.213-433.159) or +/- 54Mhz or 108KHz "BWchannel" as CC1101 documentation calls it. This is about the setting in this library (101KHz). Generally is not a good idea to use a large BWchannel setting as the chip then collects a lot of noise and signals from other ISM working devices.