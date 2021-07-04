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

#ifndef CC1101_RF_h
#define CC1101_RF_h

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

// the restriction the library enforces on maximum packet size
#define MAX_PACKET_LEN 61
// Most modules come with 26Mhz crystal
#ifndef CC1101_CRYSTAL_FREQUENCY
#define  CC1101_CRYSTAL_FREQUENCY 26000000ul
#endif

//************************************* class **************************************************//

// An instance of the CC1101 represents a CC1101 chip
// we can configure it and send receive packets by calling methods of this class.
class CC1101 {
	private:
		// Some of the functions have different name than the original library
		// The SPI functions have removed. Now the library uses
		// the platform's SPI stack and this in return allows the
		// library to work in any architecture spi works (all basically)

		// Reset the chip. It is called automatically by begin()
		void reset (void);

		void writeRegister(byte addr, byte value);
		void writeBurstRegister(byte addr, const byte *buffer, byte num);
		void readBurstRegister(byte addr, byte *buffer, byte num);
		byte readStatusRegister(byte addr);

		// Sets the registers used by this library. Called automatically by begin()
		void setCommonRegisters();
		
		// Additions to the original Library

		// The SlaveSelect Pin. By default is the SS pin, but but can be any pin.
		const byte CSNpin;

		// In most architectures it is the MISO pin. On esp8266 however the MCU
		// cannot digitalRead(MISO). In that case we set this to another pin and
		// connect it with MISO with a cable. See the nodeMCU example
		const byte MISOpin;

		// Usually the default SPI bus of the target architecture. It can be other spi bus
		// however or SoftwareSPI. See the BluePill_SPI2 for a different configuration
		SPIClass& spi;
		
		void waitMiso();
		void chipSelect();
        void chipDeselect();

		// Only for debugging
		void printRegs();

		// The 2 bytes appended by the hardware to a received packet.
		// contains rssi and lqi values of the last getPacket() operation.
		byte status[2];

	public:
		CC1101(const byte _csn=SS,
		const byte _miso=MISO, SPIClass& _spi=SPI);

		byte readRegister(byte addr);
		
		void begin(const uint32_t freq);

		// this is a sendPacket variant that should work with very low MCU clock rates and/or SPI bus speed.
		// Fills the TX buffer before actually start the transmission.
		// It cannot send packet with long preamble (to wake a remote WakeOnRadio chip)
		bool sendPacketSlowMCU(const byte *txBuffer, byte size);

		// Sets the chip to RX. Actually waits until the state is RX.
		void setRXstate(void);

		// read data received from CC1101 RXFIFO. Stores the data to packet and returns the packet size.
		// reurns 0 if no data is pending.
		// The packet must be checked for size>0 && crcok() before used.
		// Sets the state to RX
		byte getPacket(byte *packet);

		// Sends a strobe (1 byte command) to the CC1101 chip.
		byte strobe(byte strobe);
		
		// Uses a null terminated char array.
		bool sendPacket(const char* msg);

		// the default. Eats 1-2mA more and has ~2db better sensitivity.
		// Sets the chip to IDLE state.
		void optimizeSensitivity();

		// the default is optimizeSensitivity(). Not sure if it is useful.
		// Sets the chip to IDLE state
		void optimizeCurrent();

		// All packets accepted. This is the default.
		// Sets the chip to IDLE state.
		void disableAddressCheck();

		// Only packets with the first byte equal to addr are accepted.
		// Sets the chip to IDLE state.
		void enableAddressCheck(byte addr);

		// Only packets with the first byte equal to addr or 0 are accepted.
		// Sets the chip to IDLE state.
		void enableAddressCheckBcast(byte addr);

		// Set the baud rate to 4800bps.
		// this is the default due to superior sensitivity, and there is no need to
		// set it explicity.
		// Sets the chip to IDLE state.
		void setBaudrate4800bps();
		
		// Set the baud rate to 38000bps.
		// Should be used after begin(freq) and before setRXstate()
		// Sets the chip to IDLE state.
		void setBaudrate38000bps();
		
		// 10mW output power
		// this is the default
		void setPower10dbm();
		
		// 3.2mW output power
		void setPower5dbm();
		
		// 1mW output power
		void setPower0dbm();
		
		// return the signal strength of the last received packet in dbm.
		int16_t getRSSIdbm();

		// Express how easily the last packet demodulated from the signal.
		byte getLQI();

		// Reports if the last received packet has correct CRC.
		bool crcok();

		// Sends the IDLE strobe to chip and waits until the state becomes IDLE.
		void setIDLEstate();
		
		// Sends packets using printf formatting. Somewhat heavy for small microcontrollers.
		// but very flexible. Sets the chip to RX state
		// uses sprintf internally and then calls sendPacket(packet, size)
		bool printf(const char* fmt, ...);
		
		// Sets the RF chip to power down state. Very low power consumption.
		void setPowerDownState();
		
		// Enable the buildin data whitening of the chip. Sets the chip to IDLE state
		// This is the default.
		void enableWhitening();
		
		// Disable the buildin data whitener of the chip. The default is enable. Sets the chip to IDLE state
		// Should be used after begin(freq) and before setRXstate()
		void disableWhitening();

		// Enables/disables whitening according to flag
		// Sometimes is easier to use than enableWhitening() and disableWhitening()
		void whitening(const bool w);
		
		// return the state of the chip CC1101 manual SWRS061I page 31
		// we read 2 times because of errata notes.
		byte getState();
		
		// Sets the frequency of the carrier signal. Sets the chip to IDLE state.
		// No need to use it in setup as begin calls it internally
		void setFrequency(const uint32_t freq);
		
		// Do not use it unless for interoperability with an already installed system
		// the default syncWord has the best charasterics for packet detection
		// Never use syncWord for packet filtering, use adresses instead
		// Should be used after begin(freq) and before setRXstate()
		// another consideration is the order of sync0, sync1.
		// It is easy to set them in reverse. This library sets (sync0, sync1)
		// but a lot of libraries and code found on internet sets (sync1, sync0)
		// If no communication is possible instead of
		// setSyncWord(0x45,0x77) try
		// setSyncWord(0x77,0x45)
		// This is in fact another good reason to never change the syncWord.
		// sets the chip to IDLE state
		//[[deprecated]]
		__attribute__((deprecated)) void setSyncWord(byte sync0, byte sync1);

		// The sync1, sync0 order is clarified here
		void setSyncWord10(byte sync1, byte sync0);

		// if an application needs only packets up to some size set this to let the
		// chip reject larger packets. Can be 1-61 bytes.
		// Should be used after begin(freq) and before setRXstate()
		// Sets the chip to IDLE state.
		void setMaxPktSize(byte size);

		// txBuffer: byte array to send.
		// size: number of bytes to send, no more than 61 bytes.
		// duration: used ONLY with WOR applications and it is the duration of
		// the wake preamble before the packet. see the "wor" folder in examples
		// returns true if the packet is transmitted, false if there are
		// other devices talking.
		// Note that in the case of very slow MCU or SPI bus you may encounter TXFIFO underflow
		// in that case try using the
		// sendPacketSlowMCU(const byte *txBuffer, byte size) function
		//
		// sets the state to RX. 
		bool sendPacket(const byte *txBuffer,const byte size, const uint32_t duration=0);

		// the same as the previous function but adds the addres to the start of the packet
		//bool sendPacket(const byte addr, const byte *txBuffer, byte size, const uint32_t duration=0);

		// Sets the chip to WakeOnRadio state. The chip sleeps for "timeout" milliseconds
		// and briefly wakes up to check for incoming message/preamble. If no message is
		// present go to sleep again.
		void wor(uint16_t timeout=1000); //  1000ms=1sec cycle

		// Should be used immediatelly after WOR -> WakeUp -> getPacket() see the WOR example
		void wor2rx();

		static const byte BUFFER_SIZE = 64;
};

#endif