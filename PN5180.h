// NAME: PN5180.h
//
// DESC: NFC Communication with NXP Semiconductors PN5180 module for Arduino.
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
#ifndef PN5180_H
#define PN5180_H

#if defined (DEVICE_SPI)

#include "mbed.h"

// PN5180 Registers
#define SYSTEM_CONFIG       (0x00)
#define IRQ_ENABLE          (0x01)
#define IRQ_STATUS          (0x02)
#define IRQ_CLEAR           (0x03)
#define TRANSCEIVE_CONTROL  (0x04)
#define TIMER1_RELOAD       (0x0c)
#define TIMER1_CONFIG       (0x0f)
#define RX_WAIT_CONFIG      (0x11)
#define CRC_RX_CONFIG       (0x12)
#define RX_STATUS           (0x13)
#define RF_STATUS           (0x1d)
#define SYSTEM_STATUS       (0x24)
#define TEMP_CONTROL        (0x25)

// PN5180 EEPROM Addresses
#define DIE_IDENTIFIER      (0x00)
#define PRODUCT_VERSION     (0x10)
#define FIRMWARE_VERSION    (0x12)
#define EEPROM_VERSION      (0x14)
#define IRQ_PIN_CONFIG      (0x1A)

enum PN5180TransceiveStat {
    PN5180_TS_Idle = 0,
    PN5180_TS_WaitTransmit = 1,
    PN5180_TS_Transmitting = 2,
    PN5180_TS_WaitReceive = 3,
    PN5180_TS_WaitForData = 4,
    PN5180_TS_Receiving = 5,
    PN5180_TS_LoopBack = 6,
    PN5180_TS_RESERVED = 7
};

enum PN5180RFTXConfig {
    PN5180_RF_TX_CFG_ISO14443A_NFCPI106_106KBIT     = 0x00,
    PN5180_RF_TX_CFG_ISO14443A_212KBIT              = 0x01,
    PN5180_RF_TX_CFG_ISO14443A_424KBIT              = 0x02,
    PN5180_RF_TX_CFG_ISO14443A_848KBIT              = 0x03,
    PN5180_RF_TX_CFG_ISO14443B_106KBIT              = 0x04,
    PN5180_RF_TX_CFG_ISO14443B_212KBIT              = 0x05,
    PN5180_RF_TX_CFG_ISO14443B_424KBIT              = 0x06,
    PN5180_RF_TX_CFG_ISO14443B_848KBIT              = 0x07,
    PN5180_RF_TX_CFG_FELICA_NFCPI212_212KBIT        = 0x08,
    PN5180_RF_TX_CFG_FELICA_NFCPI424_424KBIT        = 0x09,
    PN5180_RF_TX_CFG_NFC_ACTIVEINITIATOR_106KBIT    = 0x0A,
    PN5180_RF_TX_CFG_NFC_ACTIVEINITIATOR_212KBIT    = 0x0B,
    PN5180_RF_TX_CFG_NFC_ACTIVEINITIATOR_424KBIT    = 0x0C,
    PN5180_RF_TX_CFG_ISO15693_ASK100_26KBIT         = 0x0D,
    PN5180_RF_TX_CFG_ISO15693_ASK10_26KBIT          = 0x0E,
    PN5180_RF_TX_CFG_ISO18003M3MANCH_424_4_18KBIT   = 0x0F,
    PN5180_RF_TX_CFG_ISO18003M3MANCH_424_2_9KBIT    = 0x10,
    PN5180_RF_TX_CFG_ISO18003M3MANCH_848_4_18KBIT   = 0x11,
    PN5180_RF_TX_CFG_ISO18003M3MANCH_848_2_9KBIT    = 0x12,
    PN5180_RF_TX_CFG_ISO18003M3MANCH_424_4_106KBIT  = 0x13,
    PN5180_RF_TX_CFG_ISO14443A_PICC_212KBIT         = 0x14,
    PN5180_RF_TX_CFG_ISO14443A_PICC_424KBIT         = 0x15,
    PN5180_RF_TX_CFG_ISO14443A_PICC_848KBIT         = 0x16,
    PN5180_RF_TX_CFG_NFC_PASSIVETARGET_212KBIT      = 0x17,
    PN5180_RF_TX_CFG_NFC_PASSIVETARGET_424KBIT      = 0x18,
    PN5180_RF_TX_CFG_NFC_ACTIVETARGET_106KBIT       = 0x19,
    PN5180_RF_TX_CFG_NFC_ACTIVETARGET_212KBIT       = 0x1A,
    PN5180_RF_TX_CFG_NFC_ACTIVETARGET_424KBIT       = 0x1B,
    PN5180_RF_TX_CFG_GTM                            = 0x1C
};
enum PN5180RFRXConfig {
    PN5180_RF_RX_CFG_ISO14443A_NFCPI106_106KBIT     = 0x80 + PN5180_RF_TX_CFG_ISO14443A_NFCPI106_106KBIT,
    PN5180_RF_RX_CFG_ISO14443A_212KBIT              = 0x80 + PN5180_RF_TX_CFG_ISO14443A_212KBIT,
    PN5180_RF_RX_CFG_ISO14443A_424KBIT              = 0x80 + PN5180_RF_TX_CFG_ISO14443A_424KBIT,
    PN5180_RF_RX_CFG_ISO14443A_848KBIT              = 0x80 + PN5180_RF_TX_CFG_ISO14443A_848KBIT,
    PN5180_RF_RX_CFG_ISO14443B_106KBIT              = 0x80 + PN5180_RF_TX_CFG_ISO14443B_106KBIT,
    PN5180_RF_RX_CFG_ISO14443B_212KBIT              = 0x80 + PN5180_RF_TX_CFG_ISO14443B_212KBIT,
    PN5180_RF_RX_CFG_ISO14443B_424KBIT              = 0x80 + PN5180_RF_TX_CFG_ISO14443B_424KBIT,
    PN5180_RF_RX_CFG_ISO14443B_848KBIT              = 0x80 + PN5180_RF_TX_CFG_ISO14443B_848KBIT,
    PN5180_RF_RX_CFG_FELICA_NFCPI212_212KBIT        = 0x80 + PN5180_RF_TX_CFG_FELICA_NFCPI212_212KBIT,
    PN5180_RF_RX_CFG_FELICA_NFCPI424_424KBIT        = 0x80 + PN5180_RF_TX_CFG_FELICA_NFCPI424_424KBIT,
    PN5180_RF_RX_CFG_NFC_ACTIVEINITIATOR_106KBIT    = 0x80 + PN5180_RF_TX_CFG_NFC_ACTIVEINITIATOR_106KBIT,
    PN5180_RF_RX_CFG_NFC_ACTIVEINITIATOR_212KBIT    = 0x80 + PN5180_RF_TX_CFG_NFC_ACTIVEINITIATOR_212KBIT,
    PN5180_RF_RX_CFG_NFC_ACTIVEINITIATOR_424KBIT    = 0x80 + PN5180_RF_TX_CFG_NFC_ACTIVEINITIATOR_424KBIT,
    PN5180_RF_RX_CFG_ISO15693_ASK100_26KBIT         = 0x80 + PN5180_RF_TX_CFG_ISO15693_ASK100_26KBIT,
    PN5180_RF_RX_CFG_ISO15693_ASK10_26KBIT          = 0x80 + PN5180_RF_TX_CFG_ISO15693_ASK10_26KBIT,
    PN5180_RF_RX_CFG_ISO18003M3MANCH_424_4_18KBIT   = 0x80 + PN5180_RF_TX_CFG_ISO18003M3MANCH_424_4_18KBIT,
    PN5180_RF_RX_CFG_ISO18003M3MANCH_424_2_9KBIT    = 0x80 + PN5180_RF_TX_CFG_ISO18003M3MANCH_424_2_9KBIT,
    PN5180_RF_RX_CFG_ISO18003M3MANCH_848_4_18KBIT   = 0x80 + PN5180_RF_TX_CFG_ISO18003M3MANCH_848_4_18KBIT,
    PN5180_RF_RX_CFG_ISO18003M3MANCH_848_2_9KBIT    = 0x80 + PN5180_RF_TX_CFG_ISO18003M3MANCH_848_2_9KBIT,
    PN5180_RF_RX_CFG_ISO18003M3MANCH_424_4_106KBIT  = 0x80 + PN5180_RF_TX_CFG_ISO18003M3MANCH_424_4_106KBIT,
    PN5180_RF_RX_CFG_ISO14443A_PICC_212KBIT         = 0x80 + PN5180_RF_TX_CFG_ISO14443A_PICC_212KBIT,
    PN5180_RF_RX_CFG_ISO14443A_PICC_424KBIT         = 0x80 + PN5180_RF_TX_CFG_ISO14443A_PICC_424KBIT,
    PN5180_RF_RX_CFG_ISO14443A_PICC_848KBIT         = 0x80 + PN5180_RF_TX_CFG_ISO14443A_PICC_848KBIT,
    PN5180_RF_RX_CFG_NFC_PASSIVETARGET_212KBIT      = 0x80 + PN5180_RF_TX_CFG_NFC_PASSIVETARGET_212KBIT,
    PN5180_RF_RX_CFG_NFC_PASSIVETARGET_424KBIT      = 0x80 + PN5180_RF_TX_CFG_NFC_PASSIVETARGET_424KBIT,
    PN5180_RF_RX_CFG_NFC_ACTIVETARGET_106KBIT       = 0x80 + PN5180_RF_TX_CFG_NFC_ACTIVETARGET_106KBIT,
    PN5180_RF_RX_CFG_NFC_ACTIVETARGET_212KBIT       = 0x80 + PN5180_RF_TX_CFG_NFC_ACTIVETARGET_212KBIT,
    PN5180_RF_RX_CFG_NFC_ACTIVETARGET_424KBIT       = 0x80 + PN5180_RF_TX_CFG_NFC_ACTIVETARGET_424KBIT,
    PN5180_RF_RX_CFG_GTM                            = 0x80 + PN5180_RF_TX_CFG_GTM
};

// PN5180 IRQ_STATUS
#define RX_IRQ_STAT         (1<<0)  // End of RF rececption IRQ
#define TX_IRQ_STAT         (1<<1)  // End of RF transmission IRQ
#define IDLE_IRQ_STAT       (1<<2)  // IDLE IRQ
#define RFOFF_DET_IRQ_STAT  (1<<6)  // RF Field OFF detection IRQ
#define RFON_DET_IRQ_STAT   (1<<7)  // RF Field ON detection IRQ
#define TX_RFOFF_IRQ_STAT   (1<<8)  // RF Field OFF in PCD IRQ
#define TX_RFON_IRQ_STAT    (1<<9)  // RF Field ON in PCD IRQ
#define RX_SOF_DET_IRQ_STAT (1<<14) // RF SOF Detection IRQ

class PN5180 
{
public:
    PN5180(PinName mosi, PinName miso, PinName sck, PinName cs, PinName reset, PinName busy); 

    void powerUp();
    void powerDown();
    void reset();

    // cmd 0x00 
    bool writeRegister(uint8_t reg, uint32_t value);
    // cmd 0x01
    bool writeRegisterWithOrMask(uint8_t addr, uint32_t mask);
    // cmd 0x02
    bool writeRegisterWithAndMask(uint8_t addr, uint32_t mask);
    //cmd 0x04
    bool readRegister(uint8_t reg, uint32_t *value);
    //cmd 0x07
    bool readEEprom(uint8_t addr, uint8_t *buffer, uint8_t len);
    //cmd 0x09
    bool sendData(uint8_t *data, uint8_t len, uint8_t validBits = 0);
    //cmd 0x0a
    uint8_t * readData(uint16_t len);
    //cmd 0x11
    bool loadRFConfig(uint8_t txConf, uint8_t rxConf);
    //cmd 0x16
    bool setRF_on();
    //cmd 0x17
    bool setRF_off();


    uint32_t getIRQStatus();
    bool clearIRQStatus(uint32_t irqMask);

    PN5180TransceiveStat getTransceiveState();

private:
    SPI _spi;
    DigitalOut _cs;
    DigitalOut _reset;
    DigitalIn _busy;

    uint8_t readBuffer[508];

    bool transceiveCommand(uint8_t *sendBuffer, size_t sendBufferLen, uint8_t *recvBuffer = 0, size_t recvBufferLen = 0);
    bool waitForBusyState(bool stateToWaitFor);
};

#endif // DEVICE_SPI
#endif // PN5180_H
