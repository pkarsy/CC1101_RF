## CC1101_RF
Arduino library for Texas Instruments CC1101 rf cip.
* Based on elechouse library, with many additions.
* Works with the newest Arduino IDE and with Platformio. 
* Works with any spi bus provided by the platform or even with SoftwareSPI.
* Tested to be working with Atmega328(3.3V variants), STM32f103(BluePill both SPI busses), esp8266(NodeMCU) I expect it to work after pin tweaking on almost any architecture arduino is ported.
* The developer chooses directly the exact frequency he wants. This is in my opinion much better than choosing the base frequency and selecting channels. The ISM bands (especially outside US) are very narrow and choosing the right frequency is crusial. The developer however is responsible for the obligation with the law and the rules of Radio Frequencies
* optionally the packets can be made to be air compatibe with the panstamp library for interoperability with other RF projects.
* 4800 and 38400 baudrates. More can be added but these seem to work perfect.

### Installation with Arduino IDE
The IDE does not like the -master suffix so DO NOT Clone->Download ZIP. Instead :
* Get directly the file CC1101_RF.zip from the root of the repository
* Start the Arduino IDE and from the Sketch menu do Sketch->Include Library->Add ZIP Library and select the ZIP you just downloaded.

Alternatively (easiest for Linux but works everywere)
* cd to Arduino Libraries folder (usually ~/Arduino/libraries on Linux)
* git clone https://TODO

### Platformio
TODO you can install it using the library manager

## Pin connections
The pins depend on platform and SPI bus. See the examples.

## Usage
Most of the functionality explained in the examples, especially in the extended example. 

