// NAME: PN5180.cpp
//
// DESC: Implementation of PN5180 class.
//
// Copyright (c) 2018 by Andreas Trappmann. All rights reserved.
//
// This file is part of the PN5180 library for the Arduino environment.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//


#include "PN5180.h"
#include "pn5180_trace.h"



// PN5180 1-Byte Direct Commands
// see 11.4.3.3 Host Interface Command List
#define PN5180_WRITE_REGISTER           (0x00)
#define PN5180_WRITE_REGISTER_OR_MASK   (0x01)
#define PN5180_WRITE_REGISTER_AND_MASK  (0x02)
#define PN5180_READ_REGISTER            (0x04)
#define PN5180_READ_EEPROM              (0x07)
#define PN5180_SEND_DATA                (0x09)
#define PN5180_READ_DATA                (0x0A)
#define PN5180_SWITCH_MODE              (0x0B)
#define PN5180_LOAD_RF_CONFIG           (0x11)
#define PN5180_RF_ON                    (0x16)
#define PN5180_RF_OFF                   (0x17)


#define LOW     false
#define HIGH    true


PN5180::PN5180(PinName mosi, PinName miso, PinName sck, PinName cs, PinName reset, PinName busy) :
    _spi(mosi, miso, sck),
    _cs(cs, 1), //don't select chip by default
    _reset(reset, 0), //keep in reset state by default
    _busy(busy)
{
}

void PN5180::powerUp(void)
{
  /*
   * 11.4.1 Physical Host Interface
   * The interface of the PN5180 to a host microcontroller is based on a SPI interface,
   * extended by signal line BUSY. The maximum SPI speed is 7 Mbps and fixed to CPOL
   * = 0 (idle low) and CPHA = 0 (sample rising edge).
   */
    _reset.write(1);
    _spi.frequency(5000000);
    _spi.format(8,0);
    wait_ms(0.1);
}

void PN5180::powerDown(void)
{
    _cs.write(1);
    _reset.write(0);
}


/*
 * WRITE_REGISTER - 0x00
 * This command is used to write a 32-bit value (little endian) to a configuration register.
 * The address of the register must exist. If the condition is not fulfilled, an exception is
 * raised.
 */
bool PN5180::writeRegister(uint8_t reg, uint32_t value) 
{
    uint8_t *p = (uint8_t*)&value;

#if DEBUG_PN5180
    tr_debug("Write Register 0x%s, value (LSB first)=0x", formatHex(reg));
    for (int i=0; i<4; i++) {
        tr_debug(formatHex(p[i]));
    }
    tr_debug("\n");
#endif

    /*
    For all 4 byte command parameter transfers (e.g. register values), the payload
    parameters passed follow the little endian approach (Least Significant Byte first).
    */
    uint8_t buf[6] = { PN5180_WRITE_REGISTER, reg, p[0], p[1], p[2], p[3] };

    bool success = transceiveCommand(buf, 6);

    return success;
}

/*
 * WRITE_REGISTER_OR_MASK - 0x01
 * This command modifies the content of a register using a logical OR operation. The
 * content of the register is read and a logical OR operation is performed with the provided
 * mask. The modified content is written back to the register.
 * The address of the register must exist. If the condition is not fulfilled, an exception is
 * raised.
 */
bool PN5180::writeRegisterWithOrMask(uint8_t reg, uint32_t mask) 
{
    uint8_t *p = (uint8_t*)&mask;

#if DEBUG_PN5180
    tr_debug("Write Register 0x%s with OR mask (LSB first)=0x", formatHex(reg));
    for (int i=0; i<4; i++) {
        tr_debug(formatHex(p[i]));
    }
    tr_debug("\n");
#endif

    uint8_t buf[6] = { PN5180_WRITE_REGISTER_OR_MASK, reg, p[0], p[1], p[2], p[3] };

    bool success = transceiveCommand(buf, 6);

    return success;
}

/*
 * WRITE _REGISTER_AND_MASK - 0x02
 * This command modifies the content of a register using a logical AND operation. The
 * content of the register is read and a logical AND operation is performed with the provided
 * mask. The modified content is written back to the register.
 * The address of the register must exist. If the condition is not fulfilled, an exception is
 * raised.
 */
bool PN5180::writeRegisterWithAndMask(uint8_t reg, uint32_t mask) 
{
    uint8_t *p = (uint8_t*)&mask;

#if DEBUG_PN5180
    tr_debug("Write Register 0x%s with AND mask (LSB first)=0x", formatHex(reg));
    for (int i=0; i<4; i++) {
        tr_debug(formatHex(p[i]));
    }
    tr_debug("\n");
#endif

    uint8_t buf[6] = { PN5180_WRITE_REGISTER_AND_MASK, reg, p[0], p[1], p[2], p[3] };

    bool success = transceiveCommand(buf, 6);

    return success;
}

/*
 * READ_REGISTER - 0x04
 * This command is used to read the content of a configuration register. The content of the
 * register is returned in the 4 byte response.
 * The address of the register must exist. If the condition is not fulfilled, an exception is
 * raised.
 */
bool PN5180::readRegister(uint8_t reg, uint32_t *value) 
{
    tr_debug("Reading register 0x%s...\n", formatHex(reg));

    uint8_t cmd[2] = { PN5180_READ_REGISTER, reg };

    bool success = transceiveCommand(cmd, 2, (uint8_t*)value, 4);

    tr_debug("Register value=0x%s\n", formatHex(*value));

    return success;
}

/*
 * READ_EEPROM - 0x07
 * This command is used to read data from EEPROM memory area. The field 'Address'
 * indicates the start address of the read operation. The field Length indicates the number
 * of bytes to read. The response contains the data read from EEPROM (content of the
 * EEPROM); The data is read in sequentially increasing order starting with the given
 * address.
 * EEPROM Address must be in the range from 0 to 254, inclusive. Read operation must
 * not go beyond EEPROM address 254. If the condition is not fulfilled, an exception is
 * raised.
 */
bool PN5180::readEEprom(uint8_t addr, uint8_t *buffer, uint8_t len) 
{
    if ((addr > 254) || ((addr+len) > 254)) {
        tr_error("ERROR: Reading beyond addr 254!\n");
        return false;
    }

    tr_debug("Reading EEPROM at 0x%s, size=%d...\n", formatHex(addr), len);

    uint8_t cmd[3] = { PN5180_READ_EEPROM, addr, len };

    bool success = transceiveCommand(cmd, 3, buffer, len);

#if DEBUG_PN5180
    tr_debug("EEPROM values: ");
    for (int i=0; i<len; i++) {
        tr_debug(formatHex(buffer[i]));
        tr_debug(" ");
    }
    tr_debug("\n");
#endif

    return success;
}

/*
 * SEND_DATA - 0x09
 * This command writes data to the RF transmission buffer and starts the RF transmission.
 * The parameter ‘Number of valid bits in last Byte’ indicates the exact number of bits to be
 * transmitted for the last byte (for non-byte aligned frames).
 * Precondition: Host shall configure the Transceiver by setting the register
 * SYSTEM_CONFIG.COMMAND to 0x3 before using the SEND_DATA command, as
 * the command SEND_DATA is only writing data to the transmission buffer and starts the
 * transmission but does not perform any configuration.
 * The size of ‘Tx Data’ field must be in the range from 0 to 260, inclusive (the 0 byte length
 * allows a symbol only transmission when the TX_DATA_ENABLE is cleared).‘Number of
 * valid bits in last Byte’ field must be in the range from 0 to 7. The command must not be
 * called during an ongoing RF transmission. Transceiver must be in ‘WaitTransmit’ state
 * with ‘Transceive’ command set. If the condition is not fulfilled, an exception is raised.
 */
bool PN5180::sendData(uint8_t *data, uint8_t len, uint8_t validBits) 
{
#if DEBUG_PN5180
    tr_debug("Send data (len=%d):", len);
    for (int i=0; i<len; i++) {
        tr_debug(" ");
        tr_debug(formatHex(data[i]));
    }
    tr_debug("\n");
#endif

    uint8_t buffer[len+2];
    buffer[0] = PN5180_SEND_DATA;
    buffer[1] = validBits; // number of valid bits of last byte are transmitted (0 = all bits are transmitted)
    for (uint8_t i=0; i<len; i++) {
        buffer[2+i] = data[i];
    }

    writeRegisterWithAndMask(SYSTEM_CONFIG, 0xfffffff8);  // Idle/StopCom Command
    writeRegisterWithOrMask(SYSTEM_CONFIG, 0x00000003);   // Transceive Command
    /*
    * Transceive command; initiates a transceive cycle.
    * Note: Depending on the value of the Initiator bit, a
    * transmission is started or the receiver is enabled
    * Note: The transceive command does not finish
    * automatically. It stays in the transceive cycle until
    * stopped via the IDLE/StopCom command
    */

    PN5180TransceiveStat transceiveState = getTransceiveState();
    if (PN5180_TS_WaitTransmit != transceiveState) {
        //tr_error("*** ERROR: Transceiver not in state WaitTransmit!?\n");
        return false;
    }

    bool success = transceiveCommand(buffer, len+2);

    return success;
}

/*
 * READ_DATA - 0x0A
 * This command reads data from the RF reception buffer, after a successful reception.
 * The RX_STATUS register contains the information to verify if the reception had been
 * successful. The data is available within the response of the command. The host controls
 * the number of bytes to be read via the SPI interface.
 * The RF data had been successfully received. In case the instruction is executed without
 * preceding an RF data reception, no exception is raised but the data read back from the
 * reception buffer is invalid. If the condition is not fulfilled, an exception is raised.
 */
uint8_t * PN5180::readData(uint16_t len) 
{
    if (len > 508) {
        error("*** FATAL: PN5180 does not support reading more than 508 bytes!");
        return 0L; //system halts here!
    }
    
    tr_debug("Reading Data (len=%d)...\n", len);

    uint8_t cmd[2] = { PN5180_READ_DATA, 0x00 };

    bool success = transceiveCommand(cmd, 2, readBuffer, len);
    if(!success)
        return 0L;

#if DEBUG_PN5180
    tr_debug("Data read: ");
    for (int i=0; i<len; i++) {
        tr_debug(formatHex(readBuffer[i]));
        tr_debug(" ");
    }
    tr_debug("\n");
#endif

    return readBuffer;
}

/*
 * LOAD_RF_CONFIG - 0x11
 * Parameter 'Transmitter Configuration' must be in the range from 0x0 - 0x1C, inclusive. If
 * the transmitter parameter is 0xFF, transmitter configuration is not changed.
 * Field 'Receiver Configuration' must be in the range from 0x80 - 0x9C, inclusive. If the
 * receiver parameter is 0xFF, the receiver configuration is not changed. If the condition is
 * not fulfilled, an exception is raised.
 * The transmitter and receiver configuration shall always be configured for the same
 * transmission/reception speed. No error is returned in case this condition is not taken into
 * account.
 *
 * Transmitter: RF   Protocol          Speed     Receiver: RF    Protocol    Speed
 * configuration                       (kbit/s)  configuration               (kbit/s)
 * byte (hex)                                    byte (hex)
 * ----------------------------------------------------------------------------------------------
 * ->0D              ISO 15693 ASK100  26        8D              ISO 15693   26
 *   0E              ISO 15693 ASK10   26        8E              ISO 15693   53
 */
bool PN5180::loadRFConfig(uint8_t txConf, uint8_t rxConf) 
{
    tr_debug("Load RF-Config: txConf=%s, rxConf=%s\n", formatHex(txConf), formatHex(rxConf));

    uint8_t cmd[3] = { PN5180_LOAD_RF_CONFIG, txConf, rxConf };

    bool success = transceiveCommand(cmd, 3);

    return success;
}

/*
 * RF_ON - 0x16
 * This command is used to switch on the internal RF field. If enabled the TX_RFON_IRQ is
 * set after the field is switched on.
 */
bool PN5180::setRF_on() 
{
    tr_debug("Set RF ON\n");

    uint8_t cmd[2] = { PN5180_RF_ON, 0x00 };

    bool success = transceiveCommand(cmd, 2);

    while (0 == (TX_RFON_IRQ_STAT & getIRQStatus())); // wait for RF field to set up
    clearIRQStatus(TX_RFON_IRQ_STAT);
    return success;
}

/*
 * RF_OFF - 0x17
 * This command is used to switch off the internal RF field. If enabled, the TX_RFOFF_IRQ
 * is set after the field is switched off.
 */
bool PN5180::setRF_off() 
{
    tr_debug("Set RF OFF\n");

    uint8_t cmd[2] = { PN5180_RF_OFF, 0x00 };

    bool success = transceiveCommand(cmd, 2);

    while (0 == (TX_RFOFF_IRQ_STAT & getIRQStatus())); // wait for RF field to shut down
    clearIRQStatus(TX_RFOFF_IRQ_STAT);
    return success;
}

//---------------------------------------------------------------------------------------------

/*
11.4.3.1 A Host Interface Command consists of either 1 or 2 SPI frames depending whether the
host wants to write or read data from the PN5180. An SPI Frame consists of multiple
bytes.

All commands are packed into one SPI Frame. An SPI Frame consists of multiple bytes.
No NSS toggles allowed during sending of an SPI frame.

For all 4 byte command parameter transfers (e.g. register values), the payload
parameters passed follow the little endian approach (Least Significant Byte first).

Direct Instructions are built of a command code (1 Byte) and the instruction parameters
(max. 260 bytes). The actual payload size depends on the instruction used.
Responses to direct instructions contain only a payload field (no header).
All instructions are bound to conditions. If at least one of the conditions is not fulfilled, an exception is
raised. In case of an exception, the IRQ line of PN5180 is asserted and corresponding interrupt
status register contain information on the exception.
*/

/*
 * A Host Interface Command consists of either 1 or 2 SPI frames depending whether the
 * host wants to write or read data from the PN5180. An SPI Frame consists of multiple
 * bytes.
 * All commands are packed into one SPI Frame. An SPI Frame consists of multiple bytes.
 * No NSS toggles allowed during sending of an SPI frame.
 * For all 4 byte command parameter transfers (e.g. register values), the payload
 * parameters passed follow the little endian approach (Least Significant Byte first).
 * The BUSY line is used to indicate that the system is BUSY and cannot receive any data
 * from a host. Recommendation for the BUSY line handling by the host:
 * 1. Assert NSS to Low
 * 2. Perform Data Exchange
 * 3. Wait until BUSY is high
 * 4. Deassert NSS
 * 5. Wait until BUSY is low
 * If there is a parameter error, the IRQ is set to ACTIVE and a GENERAL_ERROR_IRQ is set.
 */
bool PN5180::transceiveCommand(uint8_t *sendBuffer, size_t sendBufferLen, uint8_t *recvBuffer, size_t recvBufferLen) 
{
#if DEBUG_PN5180
    tr_debug("Sending SPI frame: '");
    for (uint8_t i=0; i<sendBufferLen; i++) {
        if (i>0) {
            tr_debug(" ");
        }
        tr_debug(formatHex(sendBuffer[i]));
    }
    tr_debug("'\n");
#endif

    // Wait until busy is low
    if(waitForBusyState(LOW) == false)
        return false;
    // 1. Assert NSS to Low
    _cs = 0; 
    wait_ms(2);
    // 2. Perform Data Exchange
    for (uint8_t i=0; i<sendBufferLen; i++) {
        _spi.write(sendBuffer[i]);
    }
    // 3. Wait until BUSY is high
    if(waitForBusyState(HIGH) == false)
        return false;
    // 4. Deassert NSS
    _cs = 1; 
    wait_ms(1);
    // 5. Wait until BUSY is low
    if(waitForBusyState(LOW) == false)
        return false;

    // stop here if we only want to send data
    if ((0 == recvBuffer) || (0 == recvBufferLen)) 
        return true;

    tr_debug("Receiving SPI frame...\n");
    // 1. Assert NSS to Low
    _cs = 0; 
    wait_ms(2);
    // 2. Perform Data Exchange
    for (uint8_t i=0; i<recvBufferLen; i++) {
        recvBuffer[i] = _spi.write(0xff);
    }
    // 3. Wait until BUSY is high
    if(waitForBusyState(HIGH) == false)
        return false;
    // 4. Deassert NSS
    _cs = 1; 
    wait_ms(1);
    // 5. Wait until BUSY is low
    if(waitForBusyState(LOW) == false)
        return false;

#if DEBUG_PN5180
    tr_debug("Received: ");
    for (uint8_t i=0; i<recvBufferLen; i++) {
        if (i > 0) { 
            tr_debug(" "); 
        }
        tr_debug(formatHex(recvBuffer[i]));
    }
    tr_debug("'\n");
#endif

    return true;
}

bool PN5180::waitForBusyState(bool stateToWaitFor)
{
    uint16_t ctr=0;
    while(_busy != stateToWaitFor) {
        wait_ms(0.05);
        ctr++;
        if(ctr > 2000) { //100ms = timeout
            tr_error("Busy pin timeout\n");
            return false;
        }
    }
    return true;
}


/*
 * Reset NFC device
 */
void PN5180::reset() 
{
    _reset = 0;  // at least 10us required
    wait_ms(10);
    _reset = 1; // 2ms to ramp up required
    wait_ms(10);
    
    while (0 == (IDLE_IRQ_STAT & getIRQStatus())); // wait for system to start up
    
    clearIRQStatus(0xffffffff); // clear all flags
}

/**
 * @name  getInterrrupt
 * @desc  read interrupt status register and clear interrupt status
 */
uint32_t PN5180::getIRQStatus() 
{
    tr_debug("Read IRQ-Status register...\n");

    uint32_t irqStatus;
    readRegister(IRQ_STATUS, &irqStatus);

    tr_debug("IRQ-Status=0x%s\n", formatHex(irqStatus));

    return irqStatus;
}

bool PN5180::clearIRQStatus(uint32_t irqMask) 
{
    tr_debug("Clear IRQ-Status with mask=x%s\n", formatHex(irqMask));

    return writeRegister(IRQ_CLEAR, irqMask);
}

/*
 * Get TRANSCEIVE_STATE from RF_STATUS register
 */
#if DEBUG_PN5180
extern void showIRQStatus(uint32_t);
#endif

PN5180TransceiveStat PN5180::getTransceiveState() 
{
    tr_debug("Get Transceive state...\n");

    uint32_t rfStatus;
    if (!readRegister(RF_STATUS, &rfStatus)) {
#if DEBUG_PN5180
        showIRQStatus(getIRQStatus());
#endif    
        tr_debug("ERROR reading RF_STATUS register.\n");
        return PN5180TransceiveStat(0);
    }

    /*
    * TRANSCEIVE_STATEs:
    *  0 - idle
    *  1 - wait transmit
    *  2 - transmitting
    *  3 - wait receive
    *  4 - wait for data
    *  5 - receiving
    *  6 - loopback
    *  7 - reserved
    */
    uint8_t state = ((rfStatus >> 24) & 0x07);
    tr_debug("TRANSCEIVE_STATE=0x%s\n", formatHex(state));

    return PN5180TransceiveStat(state);
}
