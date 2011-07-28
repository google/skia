
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkEndian.h"
#include "SkSfntUtils.h"

static uint16_t parse_be16(const uint8_t*& p) {
    uint16_t value = (p[0] << 8) | p[1];
    p += 2;
    return value;
}

static uint32_t parse_be32(const uint8_t*& p) {
    uint32_t value = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
    p += 4;
    return value;
}

static Sk64 parse_be64(const uint8_t*& p) {
    Sk64 value;
    value.fHi = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
    value.fLo = (p[4] << 24) | (p[5] << 16) | (p[6] << 8) | p[7];
    p += 8;
    return value;
}

///////////////////////////////////////////////////////////////////////////////

bool SkSfntUtils::ReadTable_head(SkFontID fontID, SkSfntTable_head* head) {
    static const uint32_t gTag = SkSetFourByteTag('h', 'e', 'a', 'd');
    static const size_t gSize = 54;
    
    uint8_t storage[gSize];
    size_t size = SkFontHost::GetTableData(fontID, gTag, 0, gSize, storage);
    if (size != gSize) {
        return false;
    }
    
    const uint8_t* p = storage;
    head->fVersion = parse_be32(p);
    head->fRevision = parse_be32(p);
    head->fCheckSumAdjustment = parse_be32(p);
    head->fMagicNumber = parse_be32(p);
    head->fFlags = parse_be16(p);
    head->fUnitsPerEm = parse_be16(p);
    head->fDateCreated = parse_be64(p);
    head->fDateModified = parse_be64(p);
    head->fXMin = parse_be16(p);
    head->fXMin = parse_be16(p);
    head->fXMin = parse_be16(p);
    head->fXMin = parse_be16(p);
    head->fMacStyle = parse_be16(p);
    head->fLowestPPEM = parse_be16(p);
    head->fFontDirectionHint = parse_be16(p);
    head->fIndexToLocFormat = parse_be16(p);
    head->fGlyphDataFormat = parse_be16(p);
    SkASSERT(p - storage == (long)size);
    return true;
}

bool SkSfntUtils::ReadTable_maxp(SkFontID fontID, SkSfntTable_maxp* maxp) {
    static const uint32_t gTag = SkSetFourByteTag('m', 'a', 'x', 'p');
    static const size_t gSize = 32;
    
    uint8_t storage[gSize];
    size_t size = SkFontHost::GetTableData(fontID, gTag, 0, gSize, storage);
    if (size != gSize) {
        return false;
    }
    
    const uint8_t* p = storage;
    maxp->fVersion = parse_be32(p);
    maxp->fNumGlyphs = parse_be16(p);
    maxp->fMaxPoints = parse_be16(p);
    maxp->fMaxContours = parse_be16(p);
    maxp->fMaxComponentPoints = parse_be16(p);
    maxp->fMaxComponentContours = parse_be16(p);
    maxp->fMaxZones = parse_be16(p);
    maxp->fMaxTwilightPoints = parse_be16(p);
    maxp->fMaxStorage = parse_be16(p);
    maxp->fMaxFunctionDefs = parse_be16(p);
    maxp->fMaxInstructionDefs = parse_be16(p);
    maxp->fMaxStackElements = parse_be16(p);
    maxp->fMaxSizeOfInstructions = parse_be16(p);
    maxp->fMaxComponentElements = parse_be16(p);
    maxp->fMaxComponentDepth = parse_be16(p);
    SkASSERT(p - storage == (long)size);
    return true;
}

