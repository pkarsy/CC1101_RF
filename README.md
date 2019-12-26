## CC1101_RF
Arduino library for Texas Instruments CC1101 rf cip. Implements a small but useful subset of the chip's functionality.
* Based on elechouse library, with many additions.
* Works with the newest Arduino IDE(1.8.10) and with Platformio.
* No need for interrupt handler. An empty interrupt handler may needed for wake from sleep if the project uses low power modes.
* Works with any SPI bus provided by the platform or with SoftwareSPI. You can even connect 2 CC1101 modules to the same ISP bus, only CSN and GDO0 need to be on different MCU pins.
* Tested to be working with Atmega328(3.3V variants), STM32f103(BluePill both SPI busses), esp8266(NodeMCU). I expect it to work after pin tweaking on almost any architecture arduino is ported.
* The developer chooses directly the exact frequency he/she wants. This is in my opinion much better than choosing the base frequency and selecting channels. The ISM bands (especially outside US) are very narrow and choosing the right frequency is crusial. It is the duty of the developer however to comply with the national and international standrds about radio frequences.
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

