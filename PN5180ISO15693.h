// NAME: PN5180ISO15693.h
//
// DESC: ISO15693 protocol on NXP Semiconductors PN5180 module for Arduino.
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
#ifndef PN5180ISO15693_H
#define PN5180ISO15693_H

#include "PN5180.h"

enum ISO15693ErrorCode {
    EC_NO_CARD                          = -1,
    ISO15693_EC_OK                      = 0,
    ISO15693_EC_NOT_SUPPORTED           = 0x01,
    ISO15693_EC_NOT_RECOGNIZED          = 0x02,
    ISO15693_EC_OPTION_NOT_SUPPORTED    = 0x03,
    ISO15693_EC_UNKNOWN_ERROR           = 0x0f,
    ISO15693_EC_BLOCK_NOT_AVAILABLE     = 0x10,
    ISO15693_EC_BLOCK_ALREADY_LOCKED    = 0x11,
    ISO15693_EC_BLOCK_IS_LOCKED         = 0x12,
    ISO15693_EC_BLOCK_NOT_PROGRAMMED    = 0x13,
    ISO15693_EC_BLOCK_NOT_LOCKED        = 0x14,
    ISO15693_EC_CUSTOM_CMD_ERROR        = 0xA0
};

enum ISO15693CommandFlags {
    ISO15693_CF_SINGLESUBCARRIER_UNADDRESSED                = 0x02,
    ISO15693_CF_DUALSUBCARRIER_UNADDRESSED                  = 0x03,
    ISO15693_CF_SINGLESUBCARRIER_ADDRESSED                  = 0x22,
    ISO15693_CF_SINGLESUBCARRIER_UNADDRESSED_WITHOPTIONS    = 0x42,
    ISO15693_CF_SINGLESUBCARRIER_ADDRESSED_WITHOPTIONS      = 0x62
};

enum ISO15693Command {
    ISO15693_CMD_INVENTORY                          = 0x01,
    ISO15693_CMD_STAYQUIET                          = 0x02,
    ISO15693_CMD_READSINGLEBLOCK                    = 0x20,
    ISO15693_CMD_WRITESINGLEBLOCK                   = 0x21,
    ISO15693_CMD_LOCKBLOCK                          = 0x22,
    ISO15693_CMD_READMULTIPLEBLOCKS                 = 0x23,
    ISO15693_CMD_WRITEMULTIPLEBLOCK                 = 0x24,
    ISO15693_CMD_SELECT                             = 0x25,
    ISO15693_CMD_RESETTOREADY                       = 0x26,
    ISO15693_CMD_WRITE_AFI                          = 0x27,
    ISO15693_CMD_LOCK_AFI                           = 0x28,
    ISO15693_CMD_WRITE_DSFID                        = 0x29,
    ISO15693_CMD_LOCK_DSFID                         = 0x2A,
    ISO15693_CMD_GETSYSTEMINFO                      = 0x2B,
    ISO15693_CMD_GETMULTIPLEBLOCKSECURITYSTATUS     = 0x2C
};

class PN5180ISO15693 : public PN5180 
{
public:
    PN5180ISO15693(PinName mosi, PinName miso, PinName sck, PinName cs, PinName reset, PinName busy);
  
    ISO15693ErrorCode getInventory(uint8_t *uid);

    ISO15693ErrorCode readSingleBlock(uint8_t *uid, uint8_t blockNo, uint8_t *blockData, uint8_t blockSize);
    ISO15693ErrorCode writeSingleBlock(uint8_t *uid, uint8_t blockNo, uint8_t *blockData, uint8_t blockSize);

    ISO15693ErrorCode getSystemInfo(uint8_t *uid, uint8_t *blockSize, uint8_t *numBlocks);

 
    bool setupRF();

    const char* errorToString(int err);
  
private:
    ISO15693ErrorCode issueISO15693Command(uint8_t *cmd, uint8_t cmdLen, uint8_t **resultPtr);
};

#endif // PN5180ISO15693_H 
