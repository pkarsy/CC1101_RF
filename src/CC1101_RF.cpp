/*
Based ypon the elchouse CC1101 library
Licenced under MIT licence
Panagiotis Karagiannis <pkarsy@gmail.com>

The GDO0 pin is set (IOCFG0=0x01) and can be used as a flag that a packet is received
or as an interrupt source, but the library functions do not use it.
GDO2 in not used at all (The default function is CHIP_RDy)

GDO0 is asserted (and stays high) as long as a full packet is
buffered in the RX fifo. Given that, interrupts needed only for sleep MCU modes
or WakeOnRadio.
*/

/*

    This library was originally copyright of Michael at elechouse.com but permision was
    granted by Wilson Shen on 2016-10-23 for me (Simon Monk) to uodate the code for Arduino 1.0+
    and release the code on github under the MIT license.


Wilson Shen <elechouse@elechouse.com>   23 October 2016 at 02:08
To: Simon Monk
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

// set the CC1101_DEBUG_PORT inside platformio.ini to have
// debug output on CC1101_DEBUG_PORT
#ifdef CC1101_DEBUG_PORT
    #define PRINTLN(x, ...) CC1101_DEBUG_PORT.println(x, ##__VA_ARGS__)
    #define PRINT(x, ...) CC1101_DEBUG_PORT.print(x, ##__VA_ARGS__)
#else
    #define PRINTLN(x, ...)
    #define PRINT(x, ...)
#endif

#include <stdarg.h>
#include <Arduino.h>
#include <CC1101_RF.h>

#define     WRITE_BURST         0x40                        //write burst
#define     READ_SINGLE         0x80                        //read single
#define     READ_BURST          0xC0                        //read burst
#define     BYTES_IN_RXFIFO     0x7F                        //byte number in RXfifo

CC1101::CC1101(const byte _csn, byte wiredToMisoPin, SPIClass& _spi)
: CSNpin(_csn),MISOpin(wiredToMisoPin), spi(_spi) {
}

// writes a byte to a register address
void CC1101::writeRegister(byte addr, byte value) {
    chipSelect();
    waitMiso();
    spi.transfer(addr);
    spi.transfer(value);
    chipDeselect();
}

// writes a buffer to a register address
void CC1101::writeBurstRegister(byte addr, const byte *buffer, byte num) {
    byte i, temp;
    temp = addr | WRITE_BURST;
    chipSelect();
    waitMiso();
    spi.transfer(temp);
    for (i = 0; i < num; i++) {
        spi.transfer(buffer[i]);
    }
    chipDeselect();
}

// sends a strobe(a command) to CC1101
byte CC1101::strobe(byte strobe) {
    chipSelect();
    waitMiso();
    byte reply = spi.transfer(strobe);
    chipDeselect();
    return reply;
}


// readRegister reads data from register address
byte CC1101::readRegister(byte addr) {
    byte temp, value;
    temp = addr|READ_SINGLE; // bit 7 is set for signe register read
    chipSelect();
    waitMiso();
    spi.transfer(temp);
    value=spi.transfer(0);
    chipDeselect();
    return value;
}


// readBurstRegister reads burst data from register address
// and stores the data to buffer
void CC1101::readBurstRegister(byte addr, byte *buffer, byte num) {
    byte i,temp;
    temp = addr | READ_BURST;
    chipSelect();
    waitMiso();
    spi.transfer(temp);
    for(i=0;i<num;i++) {
        buffer[i]=spi.transfer(0);
    }
    chipDeselect();
}

// readStatus : read status register
byte CC1101::readStatusRegister(byte addr) {
    byte value,temp;
    temp = addr | READ_BURST;
    chipSelect();
    waitMiso();
    spi.transfer(temp);
    value=spi.transfer(0);
    chipDeselect();
    return value;
}


// writes the register settings which are common to all
// modes this library supports. For the other registers there are
// specific commands
void CC1101::setCommonRegisters()
{
    setIDLEstate();
    //writeRegister(CC1101_IOCFG0, 0x01); // Rx report only. This is different than openelec and panstamp lib
    writeRegister(CC1101_IOCFG0, 0x06); // Asserts when SyncWord is sent/received
    //
    writeRegister(CC1101_FIFOTHR, 0x4F); // The "F" 0b1111 ensures that GDO0 assrets only if a full packet is received
    //
    writeRegister(CC1101_MDMCFG3, 0x83);
    writeRegister(CC1101_MCSM0, 0x18);
    writeRegister(CC1101_FOCCFG, 0x16);
    writeRegister(CC1101_AGCCTRL2, 0x43);
    writeRegister(CC1101_WORCTRL, 0xFB);
    writeRegister(CC1101_FSCAL3, 0xE9);
    writeRegister(CC1101_FSCAL2, 0x2A);
    writeRegister(CC1101_FSCAL1, 0x00);
    writeRegister(CC1101_FSCAL0, 0x1F);
    writeRegister(CC1101_TEST2, 0x81);
    writeRegister(CC1101_TEST1, 0x35);
    writeRegister(CC1101_TEST0, 0x09);
    //
    // max pkt size = 61. Dealing with larger packets is hard
    // and given the higher possibility of crc errors
    // probably not worth the effort. Generally the packets should be as
    // short as possible
    writeRegister(CC1101_PKTLEN, MAX_PACKET_LEN); // 0x3D
    writeRegister(CC1101_MCSM1,0x30); // CCA enabled TX->IDLE RX->IDLE
}

void CC1101::reset (void) {
    chipDeselect();
    delayMicroseconds(50);
    chipSelect();
    delayMicroseconds(50);
    chipDeselect();
    delayMicroseconds(50);
    chipSelect();
    waitMiso();
    spi.transfer(CC1101_SRES);
    waitMiso();
    chipDeselect();
}

// CC1101 pin & registers initialization
bool CC1101::begin(const uint32_t freq) {
    pinMode(MISOpin, INPUT);
    //pinMode(GDO0pin, INPUT);
    pinMode(CSNpin, OUTPUT);
    reset();
    // Check the version of the Chip as reported by the chip itself
    // Should be 20 and this guves us a way to check if the CC1101 is 
    // indeeed wireed corretly
    byte version = readStatusRegister(CC1101_VERSION);
    // CC1101 is not present or the wiring/pins is wrong
    if (version<20) return false;
    //
    // do not comment the following function calls.
    // Every function sets multipurpose registers. some registers
    // will not be set and the library will not work.
    setCommonRegisters();
    enableWhitening();
    setFrequency(freq);
    setBaudrate4800bps();
    optimizeSensitivity();
    setPower10dbm();
    disableAddressCheck();
    return true;
}


bool CC1101::sendPacketSlowMCU(const byte *txBuffer,byte size) {
    if (txBuffer==NULL || size==0) {
        PRINTLN("sendPacket called with wrong arguments");
        return false;
    }
    if (size>MAX_PACKET_LEN) {
        PRINTLN("Warning, packet truncated to max packet length");
        size=MAX_PACKET_LEN;
    }
    byte txbytes = readStatusRegister(CC1101_TXBYTES); // contains Bit:8 FIFO_UNDERFLOW + other bytes FIFO bytes
    if (txbytes!=0 || getState()!=1 ) {
        if (txbytes) PRINTLN("BYTES IN TX");
        setIDLEstate();
        strobe(CC1101_SFTX);
        strobe(CC1101_SFRX);
        setRXstate();
    }
    writeRegister(CC1101_TXFIFO, size);
    writeBurstRegister(CC1101_TXFIFO, txBuffer, size); //write data to send
    delayMicroseconds(500);
    strobe(CC1101_STX);
    byte state = getState();
    // We poll the state of the chip (state byte)
    // until state==IDLE_STATE==0
    // note that due to library setting the chip return to IDLE after TX
    if (state==1) {
        // high RSSI
        // NOTE leaves the payload in the packet
        // No IDLE strobe here, we have potentially an incoming packet.
        PRINTLN("send=false");
        return false;
    } else  {
        while(1) {
            state = getState();
            if (state==0) break;
        }
    }
    setIDLEstate();
    strobe(CC1101_SFTX);
    setRXstate();
    PRINTLN("true");
    return true;
}


// Expects a char buffer terminated with 0
bool CC1101::sendPacket(const char* msg) {
    size_t msglen = strlen(msg);
    return sendPacket((const byte*)msg, (byte)msglen);
}

// Sends the SRX strobe (if needed) and waits until the state actually goes RX
// flushes FIFOs if needed
void CC1101::setRXstate(void) {
    while(1) {
        byte state=getState();
        if      (state==0b001) break; // RX state = 1 SWRS061I doc page 31
        else if (state==0b110) strobe(CC1101_SFRX);
        else if (state==0b111) strobe(CC1101_SFTX);
        strobe(CC1101_SRX);
    }
}

// getPacket read sdata received from RXfifo. Assumes (1 byte PacketLength) + (payload) + (2bytes CRCok, RSSI, LQI)
// requires a buffer with 64 bytes to store the data (max payload = 61)
byte CC1101::getPacket(byte *rxBuffer) {
    byte state = getState();
    if (state==1) { // RX
        return 0;
    }
    byte rxbytes = readStatusRegister(CC1101_RXBYTES);
    rxbytes = rxbytes & BYTES_IN_RXFIFO;
    byte size=0;
    if(rxbytes) {
        size=readRegister(CC1101_RXFIFO);
        if (size>0 && size<=MAX_PACKET_LEN) {
            if ( (size+3)<=rxbytes ) { // TODO
                readBurstRegister(CC1101_RXFIFO, rxBuffer, size);
                readBurstRegister(CC1101_RXFIFO, status, 2);
                byte rem=rxbytes-(size+3);
                if (rem>0) {
                    PRINT("FIFO STILL HAS BYTES :");
                    PRINTLN(rem);
                }
            } else {
                PRINTLN("size+3<=rxbytes");
                size=0;
            }
        } else { 
            PRINT("Wrong rx size=");
            PRINTLN(size);
            size=0;
        }
    }
    setIDLEstate();
    strobe(CC1101_SFRX);
    setRXstate();
    if (size==0) memset(status,0,2); // sets the crc to be wrong and clears old LQI RSSI values
    return size;
}

// The pin is the actual MISO pin EXCEPT when the MCU cannot digitalRead(MISO)
// if SPI is active (esp8266). In this case we connect another pin with MISO
// and we digitalRead this instead
void CC1101::waitMiso() {
    while (digitalRead(MISOpin)>0);
}

// Drives CSN to LOW and according to the SPI standard,
// CC1101 starts listening to SPI bus
void CC1101::chipSelect() {
    digitalWrite(CSNpin, LOW);
}

// Drives CSN HIGH and CC1101 ignores the SPI bus
// TODO not quite drives MISO
void CC1101::chipDeselect() {
    digitalWrite(CSNpin, HIGH);
}

// settings from RF studio. This is the defauklt
void CC1101::optimizeSensitivity() {
    setIDLEstate();
    writeRegister(CC1101_FSCTRL1, 0x06);
    writeRegister(CC1101_MDMCFG2, 0x17); // 0b0-001-0-111 OptSensit-GFSK-MATCHESTER-32bitSyncWord+CarrSense
    setRXstate();
}

// the examples do not use this setting, sensitivity is more importand than 1-2mA
void CC1101::optimizeCurrent() {
    setIDLEstate();
    writeRegister(CC1101_FSCTRL1, 0x08);
    writeRegister(CC1101_MDMCFG2, 0x97); // 0b1-001-0-111  OptCurrent-GFSK-MATCHESTER-32bitSyncWord+CarrSense
}

void CC1101::disableAddressCheck() {
    setIDLEstate();
    // two status bytes will be appended to the payload + no address check
    writeRegister(CC1101_PKTCTRL1,CC1101_PKTCTRL1_DEFAULT_VAL+0);
}

void CC1101::enableAddressCheck(byte addr) {
    setIDLEstate();
    writeRegister(CC1101_ADDR, addr);
    // two status bytes will be appended to the payload + address check
    writeRegister(CC1101_PKTCTRL1, CC1101_PKTCTRL1_DEFAULT_VAL+1);
}

void CC1101::enableAddressCheckBcast(byte addr) {
    setIDLEstate();
    writeRegister(CC1101_ADDR, addr);
    // two status bytes will be appended to the payload + address check + accept 0 address
    writeRegister(CC1101_PKTCTRL1, CC1101_PKTCTRL1_DEFAULT_VAL+2);
}

void CC1101::setBaudrate4800bps() {
    setIDLEstate();
    writeRegister(CC1101_MDMCFG4, 0xC7);
    writeRegister(CC1101_DEVIATN, 0x40);
}

void CC1101::setBaudrate38000bps() {
    setIDLEstate();
    writeRegister(CC1101_MDMCFG4, 0xCA);
    writeRegister(CC1101_DEVIATN, 0x35);
}


void CC1101::setBaudrate(const uint16_t baudrate) {
    if (baudrate >= 10000) setBaudrate38000bps();
    else setBaudrate4800bps();
}

// 10mW
void CC1101::setPower10dbm() {
    //setIDLEstate();
    writeRegister(CC1101_PATABLE, 0xC5);
}

// 3.2mW
void CC1101::setPower5dbm() {
    //setIDLEstate();
    writeRegister(CC1101_PATABLE, 0x86);
}

// 1mW
void CC1101::setPower0dbm() {
    //setIDLEstate();
    writeRegister(CC1101_PATABLE, 0x50);
}

// reports the signal strength of the last received packet in dBm
// it is always a negative number and can be -30 to -100 dbm sometimes even less.
int16_t CC1101::getRSSIdbm() {
    // from TI app note
    uint8_t rssi_dec = status[0];
    int16_t rssi_dBm;
    // uint8_t rssi_offset = 74;
    const int16_t rssi_offset = 74;
    if (rssi_dec >= 128) {
        rssi_dBm = (int16_t)((int16_t)(rssi_dec - 256) / 2) - rssi_offset;
    } else {
        rssi_dBm = (rssi_dec / 2) - rssi_offset;
    }
    return rssi_dBm;
}

// reports if the last packet has correct CRC
bool CC1101::crcok() {
    return status[1]>>7;
}

// reports how easily the last packet is demodulated (is read)
uint8_t CC1101::getLQI() {
    return status[1]&0b01111111;;
    // return 0x3F - status[1]&0b01111111;;
}

void CC1101::setIDLEstate() {
    strobe(CC1101_SIDLE);
    while (getState()!=0); // wait until state is IDLE(=0)
}

bool CC1101::printf(const char* fmt, ...) {
    byte pkt[MAX_PACKET_LEN+1];
    va_list args;
    va_start(args, fmt);
    // TODO vsnprintf_P gia avr
    byte length = vsnprintf( (char*)pkt,MAX_PACKET_LEN+1, (const char*)fmt, args );
    va_end(args);
    if (length>MAX_PACKET_LEN) length=MAX_PACKET_LEN;
    return sendPacket(pkt, length);
}


// Put CC1101 into power-down state.
void CC1101::setPowerDownState() {
    setIDLEstate();
    strobe(CC1101_SFRX); // Flush RX buffer
    strobe(CC1101_SFTX); // Flush TX buffer
    // Enter Power-down state
    strobe(CC1101_SPWD);
}

void CC1101::enableWhitening() {
    setIDLEstate();
    writeRegister(CC1101_PKTCTRL0, 0x45); // WHITE_DATA=1 PKT_FORMAT=0(normal) CRC_EN=1 LENGTH_CONFIG=1(var len)
}

void CC1101::disableWhitening() {
    setIDLEstate();
    writeRegister(CC1101_PKTCTRL0, 0x05); // WHITE_DATA=0 PKT_FORMAT=0(normal) CRC_EN=1 LENGTH_CONFIG=1(var len)
}

void CC1101::whitening(const bool w) {
    if (w) enableWhitening();
    else enableWhitening();
}

// return the state of the chip SWRS061I page 31
byte CC1101::getState() { // we read 2 times due to errata note
    byte old_state=strobe(CC1101_SNOP);
    while(1) {
        byte state = strobe(CC1101_SNOP);
        if (state==old_state) {
            return (state>>4)&0b00111;
        }
        old_state=state;
    }
}

// calculate the value that is written to the register for settings the base frequency
// that the CC1101 should use for sending/receiving over the air.
void CC1101::setFrequency(const uint32_t freq) {
    // We use uint64_t as the <<16 overflows uint32_t
    // however the division with 26000000 allows the final
    // result to be uint32 again
    uint32_t reg_freq = ((uint64_t)freq<<16) / CC1101_CRYSTAL_FREQUENCY;
    //
    // this is split into 3 bytes that are written to 3 different registers on the CC1101
    uint8_t FREQ2 = (reg_freq>>16) & 0xFF;   // high byte, bits 7..6 are always 0 for this register
    uint8_t FREQ1 = (reg_freq>>8) & 0xFF;    // middle byte
    uint8_t FREQ0 = reg_freq & 0xFF;         // low byte
    setIDLEstate();
    writeRegister(CC1101_CHANNR, 0);
    writeRegister(CC1101_FREQ2, FREQ2);
    writeRegister(CC1101_FREQ1, FREQ1);
    writeRegister(CC1101_FREQ0, FREQ0);
    #ifdef CC1101_DEBUG
        PRINT("FREQ2=");
        PRINTLN(FREQ2, HEX);
        PRINT("FREQ1=");
        PRINTLN(FREQ1, HEX);
        PRINT("FREQ0=");
        PRINTLN(FREQ0,HEX);
        uint32_t realfreq=((uint32_t)FREQ2<<16)+((uint32_t)FREQ1<<8)+(uint32_t)FREQ0;
        realfreq=((uint64_t)realfreq*CC1101_CRYSTAL_FREQUENCY)>>16;
        PRINT("Real frequency = ");
        PRINTLN(realfreq);
    #endif
}

void CC1101::setSyncWord(byte sync0, byte sync1) {
    setIDLEstate();
    writeRegister(CC1101_SYNC0, sync0);
    writeRegister(CC1101_SYNC1, sync1);
}

void CC1101::setSyncWord10(byte sync1, byte sync0) {
    setIDLEstate();
    writeRegister(CC1101_SYNC1, sync1);
    writeRegister(CC1101_SYNC0, sync0);
}

void CC1101::setMaxPktSize(byte size) {
    setIDLEstate();
    if (size<1) size=1;
    if (size>MAX_PACKET_LEN) size=MAX_PACKET_LEN;
    writeRegister(CC1101_PKTLEN, size);
}


#ifdef CC1101_DEBUG
void CC1101::printRegs() {
    PRINT("WORCTRL=0x"); PRINTLN(readRegister(CC1101_WORCTRL),HEX);
    PRINT("MCSM2=0x");PRINTLN(readRegister(CC1101_MCSM2),HEX);
    PRINT("MCSM0=0x");PRINTLN(readRegister(CC1101_MCSM0),HEX);
    PRINT("WOREVT0=0x");PRINTLN(readRegister(CC1101_WOREVT0),HEX);
    PRINT("WOREVT1=0x");PRINTLN(readRegister(CC1101_WOREVT1),HEX);
}
#endif

void CC1101::wor(uint16_t timeout) {
    PRINTLN("WOR");
    if (timeout<15) timeout=15; // CC1101 has an ERRATA note we should not WOR for less than 15ms
    constexpr const uint16_t maxtimeout=750ul*0xffff/(CC1101_CRYSTAL_FREQUENCY/1000);
    // timeout<=1890msec for 26Mhz crystal.
    if (timeout>maxtimeout) timeout=maxtimeout;
    //
    // RC_CAL=1 probably is the RC counting event0 event1
    // 0x78 EVENT1=7 ((1.333ms) 0x38-> EVENT1=3(346.15us) for 1sec WoR mean current difference is 2-4uA
    // which is very small so 7 is the safest. TI APP NOTE gives example
    // with event1=3 however the crystal must is known brand with known startup time ?
    // 0x58 is probably very good 0.667 – 0.692 ms. I suppose most crustals can do this ?
    // manual says that CHP_RDYn asserts in 150us but this depends on crystal type (or quality ?)
    // we choose 7 to be sure
    writeRegister(CC1101_WORCTRL,  0x78); // wor_res=0 EVENT1=7 (1.333ms)
    //
    // 12.5% duty cycle but with LOW RSSI just reuturn to SLEEP (because RX_TIME_RSSI=1)
    // so the actual power consumption will be very small unless of course the peer
    // activates the module constantly
    writeRegister(CC1101_MCSM2,   0b11000);
    //
    writeRegister(CC1101_MCSM0,  0x38); // autocal every 4th time from rx/tx to idle
    //
    uint16_t evt01=timeout*(CC1101_CRYSTAL_FREQUENCY/1000)/750;
    PRINT("WOREVT0=");
    PRINTLN(evt01 & 0xff, HEX);
    PRINT("WOREVT1=");
    PRINTLN(evt01>>8, HEX);
    writeRegister(CC1101_WOREVT0, evt01 & 0xff);
    writeRegister(CC1101_WOREVT1, evt01>>8);
    // 750*0x876A/26000000.0 =~ 1.0000 sec
    strobe(CC1101_SWOR);
}

void CC1101::wor2rx() {
    writeRegister(CC1101_WORCTRL,0xFB);
    writeRegister(CC1101_MCSM2, 0x07);
    writeRegister(CC1101_MCSM0, 0x18);
    //writeRegister(CC1101_IOCFG0, 0x01); // Rx report only. This is different than openelec and panstamp lib
    writeRegister(CC1101_WOREVT0, 0x6B); // probably not needed
    writeRegister(CC1101_WOREVT1, 0x87); // probably not needed
}


bool CC1101::sendPacket(const byte *txBuffer, byte size, const uint32_t duration) {
    if (txBuffer==NULL || size==0) {
        PRINTLN("sendPacket called with wrong arguments");
        return false;
    }
    if (size>MAX_PACKET_LEN) {
        PRINTLN("Warning, packet truncated");
        size=MAX_PACKET_LEN;
    }
    byte txbytes = readStatusRegister(CC1101_TXBYTES); // contains Bit:8 FIFO_UNDERFLOW + other bytes FIFO bytes
    if (txbytes!=0 || getState()!=1 ) {
        if (txbytes) PRINTLN("BYTES IN TX");
        else PRINTLN("getState()!=RX");
        setIDLEstate();
        strobe(CC1101_SFTX);
        strobe(CC1101_SFRX);
        setRXstate();
    }
    delayMicroseconds(500); // it helps ?
    strobe(CC1101_STX);
    byte state = getState();
    // CC1101_RF lib has register IOCFG0==0x01 which is good for RX
    // but does not give TX info. So we poll the state of the chip (state byte)
    // until state=IDLE_STATE=0
    // note that due to library setting the chip return to IDLE after TX
    if (state==1) {
        // high RSSI
        // No IDLE strobe here, we have potentially an incoming packet.
        PRINTLN("send=false");
        return false;
    } else  {
        uint32_t t = millis();
        while(millis()-t<duration){};
        writeRegister(CC1101_TXFIFO, size); // write the size of the packet
        writeBurstRegister(CC1101_TXFIFO, txBuffer, size); // write the packet data to txbuffer
        delayMicroseconds(500); // it helps ?
        //
        while(1) {
            state = getState();
            if (state==0) break; // we wait for IDLE state
        }
    }
    setIDLEstate();
    strobe(CC1101_SFTX);
    setRXstate();
    PRINTLN("true");
    return true;
}

// END //
