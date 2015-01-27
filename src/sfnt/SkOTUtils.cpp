/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkEndian.h"
#include "SkSFNTHeader.h"
#include "SkStream.h"
#include "SkOTTable_head.h"
#include "SkOTTable_name.h"
#include "SkOTTableTypes.h"
#include "SkOTUtils.h"

extern const uint8_t SK_OT_GlyphData_NoOutline[] = {
    0x0,0x0, //SkOTTableGlyphData::numberOfContours
    0x0,0x0, //SkOTTableGlyphData::xMin
    0x0,0x0, //SkOTTableGlyphData::yMin
    0x0,0x0, //SkOTTableGlyphData::xMax
    0x0,0x0, //SkOTTableGlyphData::yMax

    0x0,0x0, //SkOTTableGlyphDataInstructions::length
};

uint32_t SkOTUtils::CalcTableChecksum(SK_OT_ULONG *data, size_t length) {
    uint32_t sum = 0;
    SK_OT_ULONG *dataEnd = data + ((length + 3) & ~3) / sizeof(SK_OT_ULONG);
    for (; data < dataEnd; ++data) {
        sum += SkEndian_SwapBE32(*data);
    }
    return sum;
}

SkData* SkOTUtils::RenameFont(SkStreamAsset* fontData, const char* fontName, int fontNameLen) {

    // Get the sfnt header.
    SkSFNTHeader sfntHeader;
    if (fontData->read(&sfntHeader, sizeof(sfntHeader)) < sizeof(sfntHeader)) {
        return NULL;
    }

    // Find the existing 'name' table.
    int tableIndex;
    SkSFNTHeader::TableDirectoryEntry tableEntry;
    int numTables = SkEndian_SwapBE16(sfntHeader.numTables);
    for (tableIndex = 0; tableIndex < numTables; ++tableIndex) {
        if (fontData->read(&tableEntry, sizeof(tableEntry)) < sizeof(tableEntry)) {
            return NULL;
        }
        if (SkOTTableName::TAG == tableEntry.tag) {
            break;
        }
    }
    if (tableIndex == numTables) {
        return NULL;
    }

    if (!fontData->rewind()) {
        return NULL;
    }

    // The required 'name' record types: Family, Style, Unique, Full and PostScript.
    const SkOTTableName::Record::NameID::Predefined::Value namesToCreate[] = {
        SkOTTableName::Record::NameID::Predefined::FontFamilyName,
        SkOTTableName::Record::NameID::Predefined::FontSubfamilyName,
        SkOTTableName::Record::NameID::Predefined::UniqueFontIdentifier,
        SkOTTableName::Record::NameID::Predefined::FullFontName,
        SkOTTableName::Record::NameID::Predefined::PostscriptName,
    };
    const int namesCount = SK_ARRAY_COUNT(namesToCreate);

    // Copy the data, leaving out the old name table.
    // In theory, we could also remove the DSIG table if it exists.
    size_t nameTableLogicalSize = sizeof(SkOTTableName) + (namesCount * sizeof(SkOTTableName::Record)) + (fontNameLen * sizeof(wchar_t));
    size_t nameTablePhysicalSize = (nameTableLogicalSize + 3) & ~3; // Rounded up to a multiple of 4.

    size_t oldNameTablePhysicalSize = (SkEndian_SwapBE32(tableEntry.logicalLength) + 3) & ~3; // Rounded up to a multiple of 4.
    size_t oldNameTableOffset = SkEndian_SwapBE32(tableEntry.offset);

    //originalDataSize is the size of the original data without the name table.
    size_t originalDataSize = fontData->getLength() - oldNameTablePhysicalSize;
    size_t newDataSize = originalDataSize + nameTablePhysicalSize;

    SkAutoTUnref<SkData> rewrittenFontData(SkData::NewUninitialized(newDataSize));
    SK_OT_BYTE* data = static_cast<SK_OT_BYTE*>(rewrittenFontData->writable_data());

    if (fontData->read(data, oldNameTableOffset) < oldNameTableOffset) {
        return NULL;
    }
    if (fontData->skip(oldNameTablePhysicalSize) < oldNameTablePhysicalSize) {
        return NULL;
    }
    if (fontData->read(data + oldNameTableOffset, originalDataSize - oldNameTableOffset) < originalDataSize - oldNameTableOffset) {
        return NULL;
    }

    //Fix up the offsets of the directory entries after the old 'name' table entry.
    SkSFNTHeader::TableDirectoryEntry* currentEntry = reinterpret_cast<SkSFNTHeader::TableDirectoryEntry*>(data + sizeof(SkSFNTHeader));
    SkSFNTHeader::TableDirectoryEntry* endEntry = currentEntry + numTables;
    SkSFNTHeader::TableDirectoryEntry* headTableEntry = NULL;
    for (; currentEntry < endEntry; ++currentEntry) {
        uint32_t oldOffset = SkEndian_SwapBE32(currentEntry->offset);
        if (oldOffset > oldNameTableOffset) {
            currentEntry->offset = SkEndian_SwapBE32(SkToU32(oldOffset - oldNameTablePhysicalSize));
        }
        if (SkOTTableHead::TAG == currentEntry->tag) {
            headTableEntry = currentEntry;
        }
    }

    // Make the table directory entry point to the new 'name' table.
    SkSFNTHeader::TableDirectoryEntry* nameTableEntry = reinterpret_cast<SkSFNTHeader::TableDirectoryEntry*>(data + sizeof(SkSFNTHeader)) + tableIndex;
    nameTableEntry->logicalLength = SkEndian_SwapBE32(SkToU32(nameTableLogicalSize));
    nameTableEntry->offset = SkEndian_SwapBE32(SkToU32(originalDataSize));

    // Write the new 'name' table after the original font data.
    SkOTTableName* nameTable = reinterpret_cast<SkOTTableName*>(data + originalDataSize);
    unsigned short stringOffset = sizeof(SkOTTableName) + (namesCount * sizeof(SkOTTableName::Record));
    nameTable->format = SkOTTableName::format_0;
    nameTable->count = SkEndian_SwapBE16(namesCount);
    nameTable->stringOffset = SkEndian_SwapBE16(stringOffset);

    SkOTTableName::Record* nameRecords = reinterpret_cast<SkOTTableName::Record*>(data + originalDataSize + sizeof(SkOTTableName));
    for (int i = 0; i < namesCount; ++i) {
        nameRecords[i].platformID.value = SkOTTableName::Record::PlatformID::Windows;
        nameRecords[i].encodingID.windows.value = SkOTTableName::Record::EncodingID::Windows::UnicodeBMPUCS2;
        nameRecords[i].languageID.windows.value = SkOTTableName::Record::LanguageID::Windows::English_UnitedStates;
        nameRecords[i].nameID.predefined.value = namesToCreate[i];
        nameRecords[i].offset = SkEndian_SwapBE16(0);
        nameRecords[i].length = SkEndian_SwapBE16(SkToU16(fontNameLen * sizeof(wchar_t)));
    }

    SK_OT_USHORT* nameString = reinterpret_cast<SK_OT_USHORT*>(data + originalDataSize + stringOffset);
    for (int i = 0; i < fontNameLen; ++i) {
        nameString[i] = SkEndian_SwapBE16(fontName[i]);
    }

    unsigned char* logical = data + originalDataSize + nameTableLogicalSize;
    unsigned char* physical = data + originalDataSize + nameTablePhysicalSize;
    for (; logical < physical; ++logical) {
        *logical = 0;
    }

    // Update the table checksum in the directory entry.
    nameTableEntry->checksum = SkEndian_SwapBE32(SkOTUtils::CalcTableChecksum(reinterpret_cast<SK_OT_ULONG*>(nameTable), nameTableLogicalSize));

    // Update the checksum adjustment in the head table.
    if (headTableEntry) {
        size_t headTableOffset = SkEndian_SwapBE32(headTableEntry->offset);
        if (headTableOffset + sizeof(SkOTTableHead) < originalDataSize) {
            SkOTTableHead* headTable = reinterpret_cast<SkOTTableHead*>(data + headTableOffset);
            headTable->checksumAdjustment = SkEndian_SwapBE32(0);
            uint32_t unadjustedFontChecksum = SkOTUtils::CalcTableChecksum(reinterpret_cast<SK_OT_ULONG*>(data), originalDataSize + nameTablePhysicalSize);
            headTable->checksumAdjustment = SkEndian_SwapBE32(SkOTTableHead::fontChecksum - unadjustedFontChecksum);
        }
    }

    return rewrittenFontData.detach();
}


SkOTUtils::LocalizedStrings_NameTable*
SkOTUtils::LocalizedStrings_NameTable::CreateForFamilyNames(const SkTypeface& typeface) {
    static const SkFontTableTag nameTag = SkSetFourByteTag('n','a','m','e');
    size_t nameTableSize = typeface.getTableSize(nameTag);
    if (0 == nameTableSize) {
        return NULL;
    }
    SkAutoTDeleteArray<uint8_t> nameTableData(new uint8_t[nameTableSize]);
    size_t copied = typeface.getTableData(nameTag, 0, nameTableSize, nameTableData.get());
    if (copied != nameTableSize) {
        return NULL;
    }

    return new SkOTUtils::LocalizedStrings_NameTable((SkOTTableName*)nameTableData.detach(),
        SkOTUtils::LocalizedStrings_NameTable::familyNameTypes,
        SK_ARRAY_COUNT(SkOTUtils::LocalizedStrings_NameTable::familyNameTypes));
}

bool SkOTUtils::LocalizedStrings_NameTable::next(SkTypeface::LocalizedString* localizedString) {
    do {
        SkOTTableName::Iterator::Record record;
        if (fFamilyNameIter.next(record)) {
            localizedString->fString = record.name;
            localizedString->fLanguage = record.language;
            return true;
        }
        if (fTypesCount == fTypesIndex + 1) {
            return false;
        }
        ++fTypesIndex;
        fFamilyNameIter.reset(fTypes[fTypesIndex]);
    } while (true);
}

SkOTTableName::Record::NameID::Predefined::Value
SkOTUtils::LocalizedStrings_NameTable::familyNameTypes[3] = {
    SkOTTableName::Record::NameID::Predefined::FontFamilyName,
    SkOTTableName::Record::NameID::Predefined::PreferredFamily,
    SkOTTableName::Record::NameID::Predefined::WWSFamilyName,
};
