/*
(c) Panagiotis Karagiannis MIT Licenece
The original library taken from
https://github.com/simonmonk/CC1101_arduino (Released under MIT licence)
Original creator
http://www.elechouse.com/ thank you for this library
*/

/*	This library was originally copyright of Michael at elechouse.com but permision was
    granted by Wilson Shen on 2016-10-23 for me (Simon Monk) to uodate the code for Arduino 1.0+
    and release the code on github under the MIT license.

Wilson Shen <elechouse@elechouse.com>	23 October 2016 at 02:08
To: Simon Monk <srmonk@gmail.com>
Thanks for your email.
You are free to put it in github and to do and change.

On Oct 22, 2016 10:07 PM, "Simon Monk" <srmonk@gmail.com> wrote:
	Hi,

	I'm Simon Monk, I'm currently writing the Electronics Cookbook for O'Reilly. I use your 
	ELECHOUSE_CC1101 library in a 'recipe'. Your library is by far the easiest to use of 
	the libraries for this device, but the .h and .cpp file both reference WProgram.h which 
	as replaced by Arduino.h in Arduino 1.0.

	Rather than have to talk my readers through applying a fix to your library, I'd like 
	your permission to put the modified lib into Github and add an example from the book. 
	I would of course provide a link to your website in the book and mention that you can buy 
	the modules there. If its ok, I'd give the code an MIT OS license, to clarify its use.

	Thanks for a great library,

	Kind Regards,

	Simon Monk.
*/

#ifndef CC1101_h
#define CC1101_h

#include "Arduino.h"
#include <SPI.h>

//***************************************CC1101 define**************************************************//
// CC1101 CONFIG REGISTERS
#define CC1101_IOCFG2       0x00        // GDO2 output pin configuration
#define CC1101_IOCFG1       0x01        // GDO1 output pin configuration
#define CC1101_IOCFG0       0x02        // GDO0 output pin configuration
#define CC1101_FIFOTHR      0x03        // RX FIFO and TX FIFO thresholds
#define CC1101_SYNC1        0x04        // Sync word, high INT8U
#define CC1101_SYNC0        0x05        // Sync word, low INT8U
#define CC1101_PKTLEN       0x06        // Packet length
#define CC1101_PKTCTRL1     0x07        // Packet automation control
#define CC1101_PKTCTRL0     0x08        // Packet automation control
#define CC1101_ADDR         0x09        // Device address
#define CC1101_CHANNR       0x0A        // Channel number
#define CC1101_FSCTRL1      0x0B        // Frequency synthesizer control
#define CC1101_FSCTRL0      0x0C        // Frequency synthesizer control
#define CC1101_FREQ2        0x0D        // Frequency control word, high INT8U
#define CC1101_FREQ1        0x0E        // Frequency control word, middle INT8U
#define CC1101_FREQ0        0x0F        // Frequency control word, low INT8U
#define CC1101_MDMCFG4      0x10        // Modem configuration
#define CC1101_MDMCFG3      0x11        // Modem configuration
#define CC1101_MDMCFG2      0x12        // Modem configuration
#define CC1101_MDMCFG1      0x13        // Modem configuration
#define CC1101_MDMCFG0      0x14        // Modem configuration
#define CC1101_DEVIATN      0x15        // Modem deviation setting
#define CC1101_MCSM2        0x16        // Main Radio Control State Machine configuration
#define CC1101_MCSM1        0x17        // Main Radio Control State Machine configuration
#define CC1101_MCSM0        0x18        // Main Radio Control State Machine configuration
#define CC1101_FOCCFG       0x19        // Frequency Offset Compensation configuration
#define CC1101_BSCFG        0x1A        // Bit Synchronization configuration
#define CC1101_AGCCTRL2     0x1B        // AGC control
#define CC1101_AGCCTRL1     0x1C        // AGC control
#define CC1101_AGCCTRL0     0x1D        // AGC control
#define CC1101_WOREVT1      0x1E        // High INT8U Event 0 timeout
#define CC1101_WOREVT0      0x1F        // Low INT8U Event 0 timeout
#define CC1101_WORCTRL      0x20        // Wake On Radio control
#define CC1101_FREND1       0x21        // Front end RX configuration
#define CC1101_FREND0       0x22        // Front end TX configuration
#define CC1101_FSCAL3       0x23        // Frequency synthesizer calibration
#define CC1101_FSCAL2       0x24        // Frequency synthesizer calibration
#define CC1101_FSCAL1       0x25        // Frequency synthesizer calibration
#define CC1101_FSCAL0       0x26        // Frequency synthesizer calibration
#define CC1101_RCCTRL1      0x27        // RC oscillator configuration
#define CC1101_RCCTRL0      0x28        // RC oscillator configuration
#define CC1101_FSTEST       0x29        // Frequency synthesizer calibration control
#define CC1101_PTEST        0x2A        // Production test
#define CC1101_AGCTEST      0x2B        // AGC test
#define CC1101_TEST2        0x2C        // Various test settings
#define CC1101_TEST1        0x2D        // Various test settings
#define CC1101_TEST0        0x2E        // Various test settings

// CC1101 Strobe commands
#define CC1101_SRES         0x30        // Reset chip.
#define CC1101_SFSTXON      0x31        // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1).
                                        // If in RX/TX: Go to a wait state where only the synthesizer is
                                        // running (for quick RX / TX turnaround).
#define CC1101_SXOFF        0x32        // Turn off crystal oscillator.
#define CC1101_SCAL         0x33        // Calibrate frequency synthesizer and turn it off
                                        // (enables quick start).
#define CC1101_SRX          0x34        // Enable RX. Perform calibration first if coming from IDLE and
                                        // MCSM0.FS_AUTOCAL=1.
#define CC1101_STX          0x35        // In IDLE state: Enable TX. Perform calibration first if
                                        // MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled:
                                        // Only go to TX if channel is clear.
#define CC1101_SIDLE        0x36        // Exit RX / TX, turn off frequency synthesizer and exit
                                        // Wake-On-Radio mode if applicable.
#define CC1101_SAFC         0x37        // Perform AFC adjustment of the frequency synthesizer
#define CC1101_SWOR         0x38        // Start automatic RX polling sequence (Wake-on-Radio)
#define CC1101_SPWD         0x39        // Enter power down mode when CSn goes high.
#define CC1101_SFRX         0x3A        // Flush the RX FIFO buffer.
#define CC1101_SFTX         0x3B        // Flush the TX FIFO buffer.
#define CC1101_SWORRST      0x3C        // Reset real time clock.
#define CC1101_SNOP         0x3D        // No operation. May be used to pad strobe commands to two
                                        // INT8Us for simpler software.
// CC1101 STATUS REGISTERS
#define CC1101_PARTNUM      0x30
#define CC1101_VERSION      0x31
#define CC1101_FREQEST      0x32
#define CC1101_LQI          0x33
#define CC1101_RSSI         0x34
#define CC1101_MARCSTATE    0x35
#define CC1101_WORTIME1     0x36
#define CC1101_WORTIME0     0x37
#define CC1101_PKTSTATUS    0x38
#define CC1101_VCO_VC_DAC   0x39
#define CC1101_TXBYTES      0x3A
#define CC1101_RXBYTES      0x3B

//CC1101 PATABLE,TXFIFO,RXFIFO
#define CC1101_PATABLE      0x3E
#define CC1101_TXFIFO       0x3F
#define CC1101_RXFIFO       0x3F

// This define is the restriction the library enforces on
// maximum packet size
#define MAX_PACKET_LEN 61

#ifndef PLATFORM_GDO0
	#ifdef ARDUINO_ARCH_STM32
		#define PLATFORM_GDO0 PB0
	#elif defined(ARDUINO_ARCH_ESP8266)
		#define PLATFORM_GDO0 D2
	#elif defined(ARDUINO_ARCH_AVR)
		#define PLATFORM_GDO0 2
	#endif
#endif


//************************************* class **************************************************//

// An instance of the CC1101 represents a CC1101 chip, and we can configure it and send receive data by calling methods of this class.
class CC1101
{
	private:
		// Some of the functions have different name than the original library
		// The SPI functions have removed. Now the library uses
		// the platform's SPI stack and this in return allows to the
		// driver to work in any architecture spi works (all basically)
		void reset (void);
		void writeRegister(byte addr, byte value);
		void writeBurstRegister(byte addr, const byte *buffer, byte num);
		
		byte readRegister(byte addr);
		void readBurstRegister(byte addr, byte *buffer, byte num);
		byte readStatusRegister(byte addr);
		void setCommonRegisters();
		
		// Additions to the original Library
		const byte GDO0pin;
		const byte CSNpin;
		const byte MISOpin;
		SPIClass& spi;
		
		void waitMiso();
		void chipSelect();
        void chipDeselect();
		byte status[2]; // stores rssi and lqi values of the last getPacket() operation
		
	public:
		CC1101(const byte _gdo0=PLATFORM_GDO0, const byte _csn=SS,
		const byte _miso=MISO, SPIClass& _spi=SPI);
		void begin(const uint32_t freq);
		bool sendPacket(const byte *txBuffer, byte size);
		bool sendPacketOLD(const byte *txBuffer, byte size);
		void setRXstate(void);
		bool checkGDO0(void);
		byte getPacket(byte *rxBuffer);
		byte strobe(byte strobe);
		
		// Additions to the original Library

		bool sendPacket(const char* msg);

		// the default. Eats 1-2mA more and has ~2db better sensitivity. 
		void optimizeSensitivity();

		// the default is optimizeSensitivity();
		void optimizeCurrent();

		// All packets accepted. This is the default
		void disableAddressCheck();

		// Only packets with the first byte equal to addr are accepted.
		void enableAddressCheck(byte addr);

		// Only packets with the first byte equal to addr or 0 are accepted.
		void enableAddressCheckBcast(byte addr);

		// Set the baud rate to 4800bps. Note that the state becomes IDLE
		void setBaudrate4800bps();
		
		// Set the baud rate to 38000bps. Note that the state becomes IDLE
		void setBaudrate38000bps();
		
		// 10mW output power
		void setPower10dbm();
		
		// 3.2mW output power
		void setPower5dbm();;
		
		// 1mW output power
		void setPower0dbm();
		
		// Gets the signal strength og the last received packet in dbm.
		int16_t getRSSIdbm();

		// Express how easily the last packet demodulated from the signal.
		byte getLQI();

		// Report if the last received packet has correct CRC
		bool crcok();

		// Sends the IDLE strobe to chip and waits until the state becomes IDLE.
		void setIDLEstate();
		
		// Useful if we want the module to emulate remote controls (custom OOK modulation)
		// void setupSineWave();
		
		// Sends packets using printf formatting. Somewhat heavy for small microcontrollers.
		// but very flexible
		bool printf(const char* fmt, ...);
		
		// Sets the RF chip to power down state. Very low power consumption.
		void setPowerDownState();
		
		// getPacket sendPacket printf return to RX state. Other functions not affected by this setting.
		//void setRXdefault();
		
		// getPacket sendPacket printf return to IDLE state. Other functions not affected by this setting.
		// void setIDLEdefault();
		
		// Enable the buildin data whitener of the chip. This is the default.
		void enableWhitening();
		
		// Disable the buildin data whitener of the chip. The default is enable.
		void disableWhitening();
		
		// return the state of the chip SWRS061I page 31
		byte getState();
		
		// Sets the frequency of the carrier signal
		void setFrequency(const uint32_t freq);
		
		// OOK transmit only, suitable for implementing RF remotes
		// void beginRemote(uint32_t freq);

		// Do not use it unless for interoperability with an already installed system
		void setSyncWord(byte sync0, byte sync1);

		// If timout is greater than zero, waits up to timeout msec for clear channel otherwise fails.
		// void ccaTimeout(uint32_t timeout);
		// void disableCCA();

		// if an application needs only packets up to some size set this to let the
		// chip reject larger packets. Can be 1-61 bytes
		void setMaxPktSize(byte size);

		// Sends the previous packet stored in TXFIFO and failed to sent.
		// bool resendPaket();
};

#endif