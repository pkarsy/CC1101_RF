/*
Based ypon the elchouse CC1101 library
Licenced under MIT licence
Panagiotis Karagiannis <pkarsy@gmail.com>
uses the usal SPI pins plus the GDO0 pin of the CC1101.
GDO2 in not used
Due to register settings particularly TODO interrupt flag is not needed
bacause GDO0 is asserted (and stays high) as long as a valid packet is
buffered in the RX fifo. You need interrupt only if there is need to wake from sleep
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

// #define CC1101_DEBUG

#ifdef CC1101_DEBUG
  #define PRINTLN(x) Serial1.println(x)
  #define PRINT(x) Serial1.print(x)
#else
  #define PRINTLN(x)
  #define PRINT(x)
#endif

#include <stdarg.h>
#include <Arduino.h>
#include <CC1101_RF.h>

#define     WRITE_BURST         0x40                        //write burst
#define     READ_SINGLE         0x80                        //read single
#define     READ_BURST          0xC0                        //read burst
#define     BYTES_IN_RXFIFO     0x7F                        //byte number in RXfifo

CC1101::CC1101(const byte _gdo0, const byte _csn, byte wiredToMisoPin, SPIClass& _spi)
: GDO0pin(_gdo0),CSNpin(_csn),MISOpin(wiredToMisoPin), spi(_spi) {
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
// modes this library supports. For all other registers there are
// specific commands
void CC1101::setCommonRegisters()
{
    setIDLEstate();
    writeRegister(CC1101_IOCFG0, 0x01); // Rx report only. This is different than openelec and panstamp lib
    //
    writeRegister(CC1101_FIFOTHR, 0x4F); // The "F" 0b1111 ensures that GDO0 assrets only If a full packet is received
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
    writeRegister(CC1101_PKTLEN, 0x3D);
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
void CC1101::begin(const uint32_t freq) {
    pinMode(MISOpin, INPUT);
    pinMode(GDO0pin, INPUT);
    pinMode(CSNpin, OUTPUT);
    reset();
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
}

// txBuffer: byte array to send; size: number of data to send, no more than 61
// relay on TX return to IDLE
bool CC1101::sendPacket(const byte *txBuffer,byte size) {
    if (size==0 || size>61) return false;
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
    // by default CC1101_RF lib has register IOCFG0==0x01 which is good for RX
    // but does not give TX info. So we poll the state of the chip
    // until state=IDLE_STATE=0 according to SWRS061I doc page 31
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
            //PRINT(state);
        }
    }
    setIDLEstate();
    strobe(CC1101_SFTX);
    setRXstate();
    PRINTLN("true");
    return true;
}

//bool resendPacket() {
//}

/* bool CC1101::sendPacketOLD(const byte *txBuffer,byte size) {
    if (size==0 || size>61) return false;
    //byte state = getState();
    //if (state==0b110) strobe(CC1101_SFRX);
    if (byte state = getState() != 1) { // we are not in RX state
        if (state==0b110) {
            strobe(CC1101_SFRX); // we cannot do any better
        }
        //else if (state==0b111) strobe(CC1101_SFTX);
        //else 
        strobe(CC1101_SRX);
        delayMicroseconds(200);
    }
    //setIDLEstate();
    //strobe(CC1101_SFTX);
    writeRegister(CC1101_TXFIFO,size);
    writeBurstRegister(CC1101_TXFIFO, txBuffer, size);   //write data to send
    if (ccaMillis>0) {
        //strobe(CC1101_SFRX);
        //strobe(CC1101_SRX);
        //setRXstate(); // not only the RX strobe but actually waits for RX
        //delayMicroseconds(250); // the time needed for a correct CCA
        uint32_t startTime = millis();
        while(1) {
            if (millis()-startTime>ccaMillis) { // timeout
                setIDLEstate();
                strobe(CC1101_SFTX);
                //strobe(CC1101_SFRX);
                if (rxDefault) setRXstate();
                //Serial1.println("false");
                return false;
            }
            strobe(CC1101_STX);
            byte state = getState();
            if (state==2) break; // TX
            Serial1.print("state=");
            Serial1.println(state);
            delayMicroseconds(200);
        }
    } else {
        //writeRegister(CC1101_TXFIFO, size);
        //writeBurstRegister(CC1101_TXFIFO, txBuffer, size);   //write data to send
        strobe(CC1101_STX);                                 //start send
        // by default CC1101 lib has CC1101_IOCFG0==0x07 which is good for RX
        // but does not give TX info. So we poll the state of the chip
        // until IDLE_STATE=0 according to SWRS061I doc page 31
        // note that due to library setting the chip return to IDLE
        // after packet sending
        //
    }
    // by default CC1101 lib has CC1101_IOCFG0==0x07 which is good for RX
    // but does not give TX info. So we poll the state of the chip
    // until IDLE_STATE=0 according to SWRS061I doc page 31
    // note that due to library setting the chip return to IDLE
    // after packet sending
    while(getState()!=0) { // wait to go to idle
        delayMicroseconds(250);
    }
    setIDLEstate();
    //strobe(CC1101_SFRX);
    strobe(CC1101_SFTX); // to be sure
    if (rxDefault) setRXstate();
    //else setIDLEstate();
    Serial1.println("true");
    return true;
} */

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

// with CC1101_IOCFG0 set to 0x07 GDO0 stays HIGH when RX fifo holds a
// packet with correct crc
bool CC1101::checkGDO0(void) {
    return digitalRead(GDO0pin);
}


// read data received from RXfifo. Assumes 1 byte PacketSize + payload + 2bytes (CRCok, RSSI, LQI)
byte CC1101::getPacket(byte *rxBuffer) {
    byte state = getState();
    if (state==1) { // RX
        PRINTLN("getPacket:RX");
        return 0;
    }
    byte rxbytes = readStatusRegister(CC1101_RXBYTES);
    rxbytes = rxbytes & BYTES_IN_RXFIFO;
    byte size=0;
    if(rxbytes) {
        //PRINT("FIFO=");
        //PRINTLN(rxbytes);
        size=readRegister(CC1101_RXFIFO);
        if (size>0 && size<=61) {
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
            PRINT("Wrong rx pkar size=");
            PRINTLN(size);
            size=0;
        }
    }
    setIDLEstate();
    strobe(CC1101_SFRX);
    strobe(CC1101_SRX);
    return size;
    

    /*        
    byte rx1, rx2, pktLen;
    // Get length byte in packet (safely)
    
    rx1 = readStatusRegister(CC1101_RXBYTES); // & BYTES_IN_RXFIFO;
    while(1) {
        rx2 = rx1;
        rx1 = readStatusRegister(CC1101_RXBYTES); // & BYTES_IN_RXFIFO;
        if (rx1==rx2) break;
    }
    bool overflow = rx1>>7;
    PRINTLN(rx1);
    rx1 = rx1 & BYTES_IN_RXFIFO;
    if (rx1==0) { // No RX bytes
        if (overflow) {
            PRINTLN("Ovf without data");
            strobe(CC1101_SFRX);
            strobe(CC1101_SRX);
        }
        return 0;
    }
    pktLen = readRegister(CC1101_RXFIFO); // TODO check if >0 
    if (overflow && pktLen+3>rx1) { //1+pktlen+2status
        PRINTLN("Ovf+not enough data");
        strobe(CC1101_SFRX);
        strobe(CC1101_SRX);
        return 0;
    }
    //PRINTLN(pktLen);
    //if (pktLen==0) {
    //    readBurstRegister(CC1101_RXFIFO, status, 2);
    //    return 0;
    //} 
    byte bytesToWrite = pktLen;
    // Copy rest of packet (safely)
    while (bytesToWrite>0) {
        //PRINTLN(bytesToWrite);
        rx1 = readStatusRegister(CC1101_RXBYTES) & BYTES_IN_RXFIFO;
        while(1) {
            rx2 = rx1;
            rx1 = readStatusRegister(CC1101_RXBYTES) & BYTES_IN_RXFIFO;
            if (rx1==rx2) break; // && rx1>1
        } //while (rx1<2 && rx1!=rx2);
        //PRINTLN(rx1);
        while (rx1>2 && bytesToWrite>0) {
            *rxBuffer = readRegister(CC1101_RXFIFO); // = pktLen
            rxBuffer++;
            bytesToWrite--;
            rx1--;
        }
    }
    readBurstRegister(CC1101_RXFIFO, status, 2);
    rx1-=2;
    PRINT("RXFIFO=");
    PRINTLN(rx1);
    if (overflow && rx1<4) {
        PRINTLN("Ovf+No more data");
        strobe(CC1101_SFRX);
        strobe(CC1101_SRX);
    }
    delay(10);
    strobe(CC1101_SFRX);
    strobe(CC1101_SFRX);
    strobe(CC1101_STX);
    delay(10);
    setIDLEstate();
    setRXstate();
    return pktLen;
    */

}

// additions to elechouse lib

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

// settings from RF studio
void CC1101::optimizeSensitivity() {
    setIDLEstate();
    writeRegister(CC1101_FSCTRL1, 0x06);
    writeRegister(CC1101_MDMCFG2, 0x13);
    setRXstate();
}

void CC1101::optimizeCurrent() {
    setIDLEstate();
    writeRegister(CC1101_FSCTRL1, 0x08);
    writeRegister(CC1101_MDMCFG2, 0x93);
}

void CC1101::disableAddressCheck() {
    setIDLEstate();
    // also all functions have
    // PKTCTRL1.CRC_AUTOFLUSH=1
    // which is also mandatory with GDO0 setting (Assert only on packet with CRC-ok)
    writeRegister(CC1101_PKTCTRL1, 4+0); // TODO oxi 8+ means CRC_AUTOFLUSH
}

void CC1101::enableAddressCheck(byte addr) {
    setIDLEstate();
    writeRegister(CC1101_ADDR, addr);
    // CRC_AUTOFLUSH=1 plus TODO, enable Addr(no bcast)
    writeRegister(CC1101_PKTCTRL1, 4+1);
}

void CC1101::enableAddressCheckBcast(byte addr) {
    setIDLEstate();
    writeRegister(CC1101_ADDR, addr);
    // CRC_AUTOFLUSH=1 plus TODO plus enable Addr and 0x00 is broadcast
    writeRegister(CC1101_PKTCTRL1, 4+2);
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
// use baudrate4800 if you need to be able to receive with weak signals.
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

bool CC1101::crcok() {
    return status[1]>>7;
}

// reports how easily a packet is demodulated (is read)
uint8_t CC1101::getLQI() {
    return status[1]&0b01111111;;
    // return 0x3F - status[1]&0b01111111;;
}

void CC1101::setIDLEstate() {
    strobe(CC1101_SIDLE);
    while (getState()!=0); // wait until state is IDLE(=0)
}

// sive wave output for OOK etc.
// call setBaudrate4800bps or setBaudrate38000bps to cancel this mode
// CC1101_SFTX starts the signal IDLE stops it
/* void CC1101::setupSineWave() {
    setIDLEstate(); // needed for tx fifo flush
    writeRegister(CC1101_DEVIATN,    0x00); // deviation = 0(~1KHz) so the GFSK signal will be almost sine wave
    //{CC1101_MDMCFG2,    0x30},
    //{CC1101_FREND0,     0x11}, // what rfstudio suggests
    strobe(CC1101_SFTX); // flush tx fifo
    //strobe(CC1101_STX);  //
} */

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


// Put CC1101 into power-down state, for battery powered projects
void CC1101::setPowerDownState() {
    setIDLEstate();
    // For sure
    strobe(CC1101_SFRX); // Flush RX buffer
    strobe(CC1101_SFTX); // Flush TX buffer
    // Enter Power-down state
    strobe(CC1101_SPWD);
}

// if enabled sendPacket getPacket printf return to RX mode automatically
// useful if the application is mostly in receive mode. Note that The frequency
// power etc functions all return to IDLE independent of this setting
// However it does not turn RX mode by itself. Run enableRX if you want this
//void CC1101::setRXdefault() {
//    rxDefault = true;
//}

// this is the default, no need to run explicity, and does not turn the
// chip to IDLE state by itself
//void CC1101::setIDLEdefault() {
//    rxDefault = false;
//}

void CC1101::enableWhitening() {
    setIDLEstate();
    writeRegister(CC1101_PKTCTRL0, 0x45); // WHITE_DATA=1 PKT_FORMAT=0(normal) CRC_EN=1 LENGTH_CONFIG=1(var len)
}

void CC1101::disableWhitening() {
    setIDLEstate();
    writeRegister(CC1101_PKTCTRL0, 0x05); // WHITE_DATA=0 PKT_FORMAT=0(normal) CRC_EN=1 LENGTH_CONFIG=1(var len)
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
    //byte s1=strobe(CC1101_SNOP);
    //s1 = (s1>>4)&0b00111;
    //byte s2=strobe(CC1101_SNOP);
    //if (s1)
    //s2 = (s2>>4)&0b00111;
    //return s1;
}

/* calculate the value that is written to the register for settings the base frequency
that the CC1101 should use for sending/receiving over the air.
*/
void CC1101::setFrequency(const uint32_t freq) {
    const uint32_t CRYSTAL_FREQUENCY = 26000000; // 26MHz crystal
    //
    // We use uint64_t as the <<16 overflows uint32_t 
    uint32_t reg_freq = ((uint64_t)freq<<16) / CRYSTAL_FREQUENCY;
    //
    // this is split into 3 bytes that are written to 3 different registers on the CC1101
    uint8_t FREQ2 = (reg_freq>>16) & 0xFF;   // high byte, bits 7..6 are always 0 for this register
    uint8_t FREQ1 = (reg_freq>>8) & 0xFF;    // middle byte
    uint8_t FREQ0 = reg_freq & 0xFF;         // low byte
    //uint32_t realfreq=(uint32_t)FREQ2*(1<<16)+(uint32_t)FREQ1*(1<<8)+(uint32_t)FREQ0;
    //realfreq=(uint64_t)realfreq*CCXXX1_CRYSTAL_FREQUENCY/(1<<16);
    //printf("freq=%d\n",realfreq);
    setIDLEstate();
    writeRegister(CC1101_CHANNR, 0);
    writeRegister(CC1101_FREQ2, FREQ2);
    writeRegister(CC1101_FREQ1, FREQ1);
    writeRegister(CC1101_FREQ0, FREQ0);
    //Serial.println(FREQ2,HEX);
    //Serial.println(FREQ1,HEX);
    //Serial.println(FREQ0,HEX);
}

void CC1101::setSyncWord(byte sync0, byte sync1) {
    setIDLEstate();
    writeRegister(CC1101_SYNC0, sync0);
    writeRegister(CC1101_SYNC1, sync1);
}

//void CC1101::ccaTimeout(uint32_t timeout) {
//    ccaMillis = timeout;
//}

//void CC1101::disableCCA() {
//    ccaMillis = 0;
//}

void CC1101::setMaxPktSize(byte size) {
    if (size<1) size=1;
    if (size>61) size=61;
    writeRegister(CC1101_PKTLEN, size);
}

/* void CC1101::beginRemote(uint32_t freq) {
    pinMode(MISOpin, INPUT);
    pinMode(CSNpin, OUTPUT);
    reset();
    setFreq(freq);
    // setPower10dbm();
    //writeRegister(CC1101_PATABLE, 0xC5);
    uint8_t paTable[] = {0x03, 0xC5}; 
    writeBurstRegister(CC1101_PATABLE, paTable, sizeof(paTable));
    // unmodulated
    writeRegister(CC1101_PKTCTRL0, 0x75); // asynchronous serial mode
    writeRegister(CC1101_MDMCFG2, 0x32);
    writeRegister(CC1101_FREND0, 0x11);
    
    // modulated
    //writeRegister(CC1101_FIFOTHR, 0x47);
    //writeRegister(CC1101_PKTCTRL0, 0x75);
    //writeRegister(CC1101_FSCAL3, 0xEA);
    //writeRegister(CC1101_FSCAL2, 0x2A);
    //writeRegister(CC1101_FSCAL1, 0x00);
    //writeRegister(CC1101_FSCAL0, 0x1F);
    //writeRegister(CC1101_TEST2, 0x81);
    //writeRegister(CC1101_TEST1, 0x35);
    //
    writeRegister(CC1101_IOCFG2,   0x0D); // GDO2 -> Serial Data Output. Used for asynchronous serial mode.
    strobe(CC1101_SCAL);
    strobe(CC1101_SFTX);
    strobe(CC1101_STX);
} */