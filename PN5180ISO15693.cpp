/// NAME: PN5180ISO15693.h
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

#include "PN5180ISO15693.h"
#include "pn5180_trace.h"
#include <locale>

PN5180ISO15693::PN5180ISO15693(PinName mosi, PinName miso, PinName sck, PinName cs, PinName reset, PinName busy) 
    : PN5180(mosi, miso, sck, cs, reset, busy) 
{
}

/*
 * Inventory, code=01
 *
 * Request format: SOF, Req.Flags, Inventory, AFI (opt.), Mask len, Mask value, CRC16, EOF
 * Response format: SOF, Resp.Flags, DSFID, UID, CRC16, EOF
 *
 */
ISO15693ErrorCode PN5180ISO15693::getInventory(uint8_t *uid) 
{
    //                     Flags,  CMD, maskLen
    uint8_t inventory[] = { 0x26, 0x01, 0x00 };
    //                        |\- inventory flag + high data rate
    //                        \-- 1 slot: only one card, no AFI field present
    tr_debug("Get Inventory...\n");

    for (int i=0; i<8; i++) {
        uid[i] = 0;  
    }
    
    uint8_t *readBuffer;
    ISO15693ErrorCode rc = issueISO15693Command(inventory, sizeof(inventory), &readBuffer);
    if (ISO15693_EC_OK != rc) {
        return rc;
    }

    tr_debug("Response flags: %s, Data Storage Format ID: %s, UID: ", formatHex(readBuffer[0]), formatHex(readBuffer[1]));
    
    for (int i=0; i<8; i++) {
        uid[i] = readBuffer[2+i];
#if DEBUG_PN5180
        tr_debug(formatHex(uid[7-i])); // LSB comes first
        if (i<2) tr_debug(":");
#endif
    }
    
    tr_debug("\n");

    return ISO15693_EC_OK;
}

/*
 * Read single block, code=20
 *
 * Request format: SOF, Req.Flags, ReadSingleBlock, UID (opt.), BlockNumber, CRC16, EOF
 * Response format:
 *  when ERROR flag is set:
 *    SOF, Resp.Flags, ErrorCode, CRC16, EOF
 *
 *     Response Flags:
  *    xxxx.3xx0
  *         |||\_ Error flag: 0=no error, 1=error detected, see error field
  *         \____ Extension flag: 0=no extension, 1=protocol format is extended
  *
  *  If Error flag is set, the following error codes are defined:
  *    01 = The command is not supported, i.e. the request code is not recognized.
  *    02 = The command is not recognized, i.e. a format error occurred.
  *    03 = The option is not supported.
  *    0F = Unknown error.
  *    10 = The specific block is not available.
  *    11 = The specific block is already locked and cannot be locked again.
  *    12 = The specific block is locked and cannot be changed.
  *    13 = The specific block was not successfully programmed.
  *    14 = The specific block was not successfully locked.
  *    A0-DF = Custom command error codes
 *
 *  when ERROR flag is NOT set:
 *    SOF, Flags, BlockData (len=blockLength), CRC16, EOF
 */
ISO15693ErrorCode PN5180ISO15693::readSingleBlock(uint8_t *uid, uint8_t blockNo, uint8_t *blockData, uint8_t blockSize) 
{
    //                            flags, cmd, uid,             blockNo
    uint8_t readSingleBlock[] = { 0x22, 0x20, 1,2,3,4,5,6,7,8, blockNo }; // UID has LSB first!
    //                              |\- high data rate
    //                              \-- no options, addressed by UID
    for (int i=0; i<8; i++) {
        readSingleBlock[2+i] = uid[i];
    }

#if DEBUG_PN5180
    tr_debug("Read Single Block #%d, size=%d: ", blockNo, blockSize);
    for (uint16_t i=0; i<sizeof(readSingleBlock); i++) {
        tr_debug("%s ", formatHex(readSingleBlock[i]));
    }
    tr_debug("\n");
#endif

    uint8_t *resultPtr;
    ISO15693ErrorCode rc = issueISO15693Command(readSingleBlock, sizeof(readSingleBlock), &resultPtr);
    if (ISO15693_EC_OK != rc) {
      return rc;
    }

    tr_debug("Value=");
    
    for (int i=0; i<blockSize; i++) {
        blockData[i] = resultPtr[2+i];   
        tr_debug("%s ", formatHex(blockData[i]));  
    }

#if DEBUG_PN5180
    tr_debug(" ");
    for (int i=0; i<blockSize; i++) {
        char c = blockData[i];
        if (isprint(c)) {
            tr_debug("%c", c);
        }
        else tr_debug(".");
    }
    tr_debug("\n");
#endif

    return ISO15693_EC_OK;
}

/*
 * Write single block, code=21
 *
 * Request format: SOF, Requ.Flags, WriteSingleBlock, UID (opt.), BlockNumber, BlockData (len=blcokLength), CRC16, EOF
 * Response format:
 *  when ERROR flag is set:
 *    SOF, Resp.Flags, ErrorCode, CRC16, EOF
 *
 *     Response Flags:
  *    xxxx.3xx0
  *         |||\_ Error flag: 0=no error, 1=error detected, see error field
  *         \____ Extension flag: 0=no extension, 1=protocol format is extended
  *
  *  If Error flag is set, the following error codes are defined:
  *    01 = The command is not supported, i.e. the request code is not recognized.
  *    02 = The command is not recognized, i.e. a format error occurred.
  *    03 = The option is not supported.
  *    0F = Unknown error.
  *    10 = The specific block is not available.
  *    11 = The specific block is already locked and cannot be locked again.
  *    12 = The specific block is locked and cannot be changed.
  *    13 = The specific block was not successfully programmed.
  *    14 = The specific block was not successfully locked.
  *    A0-DF = Custom command error codes
 *
 *  when ERROR flag is NOT set:
 *    SOF, Resp.Flags, CRC16, EOF
 */
ISO15693ErrorCode PN5180ISO15693::writeSingleBlock(uint8_t *uid, uint8_t blockNo, uint8_t *blockData, uint8_t blockSize) 
{
    //                            flags, cmd, uid,             blockNo
    uint8_t writeSingleBlock[] = { 0x22, 0x21, 1,2,3,4,5,6,7,8, blockNo }; // UID has LSB first!
    //                               |\- high data rate
    //                               \-- no options, addressed by UID

    uint8_t writeCmdSize = sizeof(writeSingleBlock) + blockSize;
    uint8_t *writeCmd = (uint8_t*)malloc(writeCmdSize);
    uint8_t pos = 0;
    writeCmd[pos++] = writeSingleBlock[0];
    writeCmd[pos++] = writeSingleBlock[1];
    for (int i=0; i<8; i++) {
        writeCmd[pos++] = uid[i];
    }
    writeCmd[pos++] = blockNo;
    for (int i=0; i<blockSize; i++) {
        writeCmd[pos++] = blockData[i];
    }

#if DEBUG_PN5180
    tr_debug("Write Single Block #%d, size=%d:", blockNo, blockSize);
    for (int i=0; i<writeCmdSize; i++) {
        tr_debug(" %s", formatHex(writeCmd[i]));
    }
    tr_debug("\n");
#endif

    uint8_t *resultPtr;
    ISO15693ErrorCode rc = issueISO15693Command(writeCmd, writeCmdSize, &resultPtr);
    if (ISO15693_EC_OK != rc) {
        free(writeCmd);
        return rc;
    }

    free(writeCmd);
    return ISO15693_EC_OK;
}

/*
 * Get System Information, code=2B
 *
 * Request format: SOF, Req.Flags, GetSysInfo, UID (opt.), CRC16, EOF
 * Response format:
 *  when ERROR flag is set:
 *    SOF, Resp.Flags, ErrorCode, CRC16, EOF
 *
 *     Response Flags:
  *    xxxx.3xx0
  *         |||\_ Error flag: 0=no error, 1=error detected, see error field
  *         \____ Extension flag: 0=no extension, 1=protocol format is extended
  *
  *  If Error flag is set, the following error codes are defined:
  *    01 = The command is not supported, i.e. the request code is not recognized.
  *    02 = The command is not recognized, i.e. a format error occurred.
  *    03 = The option is not supported.
  *    0F = Unknown error.
  *    10 = The specific block is not available.
  *    11 = The specific block is already locked and cannot be locked again.
  *    12 = The specific block is locked and cannot be changed.
  *    13 = The specific block was not successfully programmed.
  *    14 = The specific block was not successfully locked.
  *    A0-DF = Custom command error codes
  *
 *  when ERROR flag is NOT set:
 *    SOF, Flags, InfoFlags, UID, DSFID (opt.), AFI (opt.), Other fields (opt.), CRC16, EOF
 *
 *    InfoFlags:
 *    xxxx.3210
 *         |||\_ DSFID: 0=DSFID not supported, DSFID field NOT present; 1=DSFID supported, DSFID field present
 *         ||\__ AFI: 0=AFI not supported, AFI field not present; 1=AFI supported, AFI field present
 *         |\___ VICC memory size:
 *         |        0=Information on VICC memory size is not supported. Memory size field is present. ???
 *         |        1=Information on VICC memory size is supported. Memory size field is present.
 *         \____ IC reference:
 *                  0=Information on IC reference is not supported. IC reference field is not present.
 *                  1=Information on IC reference is supported. IC reference field is not present.
 *
 *    VICC memory size:
 *      xxxb.bbbb nnnn.nnnn
 *        bbbbb - Block size is expressed in number of bytes, on 5 bits, allowing to specify up to 32 bytes i.e. 256 bits.
 *        nnnn.nnnn - Number of blocks is on 8 bits, allowing to specify up to 256 blocks.
 *
 *    IC reference: The IC reference is on 8 bits and its meaning is defined by the IC manufacturer.
 */
ISO15693ErrorCode PN5180ISO15693::getSystemInfo(uint8_t *uid, uint8_t *blockSize, uint8_t *numBlocks) 
{
    uint8_t sysInfo[] = { 0x22, 0x2b, 1,2,3,4,5,6,7,8 };  // UID has LSB first!
    for (int i=0; i<8; i++) {
        sysInfo[2+i] = uid[i];
    }

#if DEBUG_PN5180
    tr_debug("Get System Information");
    for (uint16_t i=0; i<sizeof(sysInfo); i++) {
        tr_debug(" %s", formatHex(sysInfo[i]));
    }
    tr_debug("\n");
#endif

    uint8_t *readBuffer;
    ISO15693ErrorCode rc = issueISO15693Command(sysInfo, sizeof(sysInfo), &readBuffer);
    if (ISO15693_EC_OK != rc) {
        return rc;
    }

    for (int i=0; i<8; i++) {
        uid[i] = readBuffer[2+i];
    }
    
#if DEBUG_PN5180
    tr_debug("UID=");
    for (int i=0; i<8; i++) {
        tr_debug(formatHex(readBuffer[9-i]));  // UID has LSB first!
        if (i<2) tr_debug(":");
    }
    tr_debug("\n");
#endif
  
    uint8_t *p = &readBuffer[10];

    uint8_t infoFlags = readBuffer[1];
    if (infoFlags & 0x01) { // DSFID flag
        uint8_t dsfid = *p++;
        tr_debug("DSFID=%s\n", formatHex(dsfid));  // Data storage format identifier
    }
    else {
        tr_debug("No DSFID\n");
    }
  
    if (infoFlags & 0x02) { // AFI flag
        uint8_t afi = *p++;
        tr_debug("AFI=%s - ", formatHex(afi));  // Application family identifier
        switch (afi >> 4) 
        {
            case 0: tr_debug("All families"); break;
            case 1: tr_debug("Transport"); break;
            case 2: tr_debug("Financial"); break;
            case 3: tr_debug("Identification"); break;
            case 4: tr_debug("Telecommunication"); break;
            case 5: tr_debug("Medical"); break;
            case 6: tr_debug("Multimedia"); break;
            case 7: tr_debug("Gaming"); break;
            case 8: tr_debug("Data storage"); break;
            case 9: tr_debug("Item management"); break;
            case 10: tr_debug("Express parcels"); break;
            case 11: tr_debug("Postal services"); break;
            case 12: tr_debug("Airline bags"); break;
            default: tr_debug("Unknown"); break;
        }
        tr_debug("\n");
    }
    else {
        tr_debug("No AFI\n");
    }

    if (infoFlags & 0x04) { // VICC Memory size
        *numBlocks = *p++;
        *blockSize = *p++;
        *blockSize = (*blockSize) & 0x1f;

        *blockSize = *blockSize + 1; // range: 1-32
        *numBlocks = *numBlocks + 1; // range: 1-256
        uint16_t viccMemSize = (*blockSize) * (*numBlocks);

        tr_debug("VICC MemSize=%d BlockSize=%d NumBlocks=%d\n", viccMemSize, *blockSize, *numBlocks);
    }
    else {
        tr_debug("No VICC memory size\n");
    }
   
    if (infoFlags & 0x08) { // IC reference
        uint8_t icRef = *p++;
        tr_debug("IC Ref=%s\n", formatHex(icRef));
    }
    else {
        tr_debug("No IC ref\n");
    }

    return ISO15693_EC_OK;
}

/*
 * ISO 15693 - Protocol
 *
 * General Request Format:
 *  SOF, Req.Flags, Command code, Parameters, Data, CRC16, EOF
 *
 *  Request Flags:
 *    xxxx.3210
 *         |||\_ Subcarrier flag: 0=single sub-carrier, 1=two sub-carrier
 *         ||\__ Datarate flag: 0=low data rate, 1=high data rate
 *         |\___ Inventory flag: 0=no inventory, 1=inventory
 *         \____ Protocol extension flag: 0=no extension, 1=protocol format is extended
 *
 *  If Inventory flag is set:
 *    7654.xxxx
 *     ||\_ AFI flag: 0=no AFI field present, 1=AFI field is present
 *     |\__ Number of slots flag: 0=16 slots, 1=1 slot
 *     \___ Option flag: 0=default, 1=meaning is defined by command description
 *
 *  If Inventory flag is NOT set:
 *    7654.xxxx
 *     ||\_ Select flag: 0=request shall be executed by any VICC according to Address_flag
 *     ||                1=request shall be executed only by VICC in selected state
 *     |\__ Address flag: 0=request is not addressed. UID field is not present.
 *     |                  1=request is addressed. UID field is present. Only VICC with UID shall answer
 *     \___ Option flag: 0=default, 1=meaning is defined by command description
 *
 * General Response Format:
 *  SOF, Resp.Flags, Parameters, Data, CRC16, EOF
 *
 *  Response Flags:
 *    xxxx.3210
 *         |||\_ Error flag: 0=no error, 1=error detected, see error field
 *         ||\__ RFU: 0
 *         |\___ RFU: 0
 *         \____ Extension flag: 0=no extension, 1=protocol format is extended
 *
 *  If Error flag is set, the following error codes are defined:
 *    01 = The command is not supported, i.e. the request code is not recognized.
 *    02 = The command is not recognized, i.e. a format error occurred.
 *    03 = The option is not supported.
 *    0F = Unknown error.
 *    10 = The specific block is not available.
 *    11 = The specific block is already locked and cannot be locked again.
 *    12 = The specific block is locked and cannot be changed.
 *    13 = The specific block was not successfully programmed.
 *    14 = The specific block was not successfully locked.
 *    A0-DF = Custom command error codes
 *
 *  Function return values:
 *    0 = OK
 *   -1 = No card detected
 *   >0 = Error code
 */
ISO15693ErrorCode PN5180ISO15693::issueISO15693Command(uint8_t *cmd, uint8_t cmdLen, uint8_t **resultPtr) 
{
    tr_debug("Issue Command 0x%s...\n", formatHex(cmd[1]));

    sendData(cmd, cmdLen);
    wait_ms(10);

    if (0 == (getIRQStatus() & RX_SOF_DET_IRQ_STAT)) {
        return EC_NO_CARD;
    }
    
    uint32_t rxStatus;
    readRegister(RX_STATUS, &rxStatus);
    
    uint16_t len = (uint16_t)(rxStatus & 0x000001ff);
    tr_debug("RX-Status=%s, len=%d\n", formatHex(rxStatus), len);

    *resultPtr = readData(len);
    if (0L == *resultPtr) {
        tr_debug("*** ERROR in readData!\n");
        return ISO15693_EC_UNKNOWN_ERROR;
    }
  
#if DEBUG_PN5180
    tr_info("Read=");
    for (int i=0; i<len; i++) {
        tr_info(formatHex((*resultPtr)[i]));
        if (i<len-1) tr_info(":");
    }
    tr_info("\n");
#endif

    uint32_t irqStatus = getIRQStatus();
    if (0 == (RX_SOF_DET_IRQ_STAT & irqStatus)) { // no card detected
        clearIRQStatus(TX_IRQ_STAT | IDLE_IRQ_STAT);
        return EC_NO_CARD;
    }

    uint8_t responseFlags = (*resultPtr)[0];
    if (responseFlags & (1<<0)) { // error flag
        uint8_t errorCode = (*resultPtr)[1];
        
        tr_debug("ERROR code=%s - %s\n", formatHex(errorCode), errorToString((int)errorCode));

        if (errorCode >= 0xA0) { // custom command error codes
            return ISO15693_EC_CUSTOM_CMD_ERROR;
        }
        else return (ISO15693ErrorCode)errorCode;
    }

    if (responseFlags & (1<<3)) { // extendsion flag
        tr_debug("Extension flag is set!\n");
    }

    clearIRQStatus(RX_SOF_DET_IRQ_STAT | IDLE_IRQ_STAT | TX_IRQ_STAT | RX_IRQ_STAT);
    return ISO15693_EC_OK;
}


bool PN5180ISO15693::setupRF() 
{
    tr_debug("Loading RF-Configuration...\n");
    if (loadRFConfig(0x0d, 0x8d)) {  // ISO15693 parameters
        tr_debug("done.\n");
    }
    else return false;

    tr_debug("Turning ON RF field...\n");
    if (setRF_on()) {
        tr_debug("done.\n");
    }
    else return false;

    writeRegisterWithAndMask(SYSTEM_CONFIG, 0xfffffff8);  // Idle/StopCom Command
    writeRegisterWithOrMask(SYSTEM_CONFIG, 0x00000003);   // Transceive Command

    return true;
}

const char* PN5180ISO15693::errorToString(int err) 
{
    switch ((ISO15693ErrorCode)err) 
    {
        case EC_NO_CARD: return "No card detected!";
        case ISO15693_EC_OK: return "OK!";
        case ISO15693_EC_NOT_SUPPORTED: return "Command is not supported!";
        case ISO15693_EC_NOT_RECOGNIZED: return "Command is not recognized!";
        case ISO15693_EC_OPTION_NOT_SUPPORTED: return "Option is not supported!";
        case ISO15693_EC_UNKNOWN_ERROR: return "Unknown error!";
        case ISO15693_EC_BLOCK_NOT_AVAILABLE: return "Specified block is not available!";
        case ISO15693_EC_BLOCK_ALREADY_LOCKED: return "Specified block is already locked!";
        case ISO15693_EC_BLOCK_IS_LOCKED: return "Specified block is locked and cannot be changed!";
        case ISO15693_EC_BLOCK_NOT_PROGRAMMED: return "Specified block was not successfully programmed!";
        case ISO15693_EC_BLOCK_NOT_LOCKED: return "Specified block was not successfully locked!";
        default:
        {
            if ( (err >= 0xA0) && (err <= 0xDF) ) {
                return "Custom command error code!";
            }
            else return "Undefined error code in ISO15693!";
        }
    }
}
