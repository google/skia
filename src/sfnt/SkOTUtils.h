/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTUtils_DEFINED
#define SkOTUtils_DEFINED

#include "include/core/SkTypeface.h"
#include "src/sfnt/SkOTTableTypes.h"
#include "src/sfnt/SkOTTable_OS_2_V4.h"
#include "src/sfnt/SkOTTable_name.h"

class SkData;
class SkStream;
struct SkAdvancedTypefaceMetrics;

struct SkOTUtils {
    /**
      *  Calculates the OpenType checksum for data.
      */
    static uint32_t CalcTableChecksum(SK_OT_ULONG *data, size_t length);

    /**
      *  Renames an sfnt font. On failure (invalid data or not an sfnt font)
      *  returns nullptr.
      *
      *  Essentially, this removes any existing 'name' table and replaces it
      *  with a new one in which FontFamilyName, FontSubfamilyName,
      *  UniqueFontIdentifier, FullFontName, and PostscriptName are fontName.
      *
      *  The new 'name' table records will be written with the Windows,
      *  UnicodeBMPUCS2, and English_UnitedStates settings.
      *
      *  fontName and fontNameLen must be specified in terms of ASCII chars.
      *
      *  Does not affect fontData's ownership.
      */
    static SkData* RenameFont(SkStreamAsset* fontData, const char* fontName, int fontNameLen);

    /** An implementation of LocalizedStrings which obtains it's data from a 'name' table. */
    class LocalizedStrings_NameTable : public SkTypeface::LocalizedStrings {
    public:
        /** Takes ownership of the nameTableData and will free it with SK_DELETE. */
        LocalizedStrings_NameTable(std::unique_ptr<uint8_t[]> nameTableData, size_t size,
                                   SK_OT_USHORT types[],
                                   int typesCount)
            : fTypes(types), fTypesCount(typesCount), fTypesIndex(0)
            , fNameTableData(std::move(nameTableData))
            , fFamilyNameIter(fNameTableData.get(), size, fTypes[fTypesIndex])
        { }

        /** Creates an iterator over all data in the 'name' table of a typeface.
         *  If no valid 'name' table can be found, returns nullptr.
         */
        static sk_sp<LocalizedStrings_NameTable> Make(
            const SkTypeface& typeface,
            SK_OT_USHORT types[],
            int typesCount);

        /** Creates an iterator over all the family names in the 'name' table of a typeface.
         *  If no valid 'name' table can be found, returns nullptr.
         */
        static sk_sp<LocalizedStrings_NameTable> MakeForFamilyNames(const SkTypeface& typeface);

        bool next(SkTypeface::LocalizedString* localizedString) override;
    private:
        static SK_OT_USHORT familyNameTypes[3];

        SK_OT_USHORT* fTypes;
        int fTypesCount;
        int fTypesIndex;
        std::unique_ptr<uint8_t[]> fNameTableData;
        SkOTTableName::Iterator fFamilyNameIter;
    };

    /** An implementation of LocalizedStrings which has one name. */
    class LocalizedStrings_SingleName : public SkTypeface::LocalizedStrings {
    public:
        LocalizedStrings_SingleName(SkString name, SkString language)
            : fName(name), fLanguage(language), fHasNext(true)
        { }

        bool next(SkTypeface::LocalizedString* localizedString) override {
            localizedString->fString = fName;
            localizedString->fLanguage = fLanguage;

            bool hadNext = fHasNext;
            fHasNext = false;
            return hadNext;
        }

    private:
        SkString fName;
        SkString fLanguage;
        bool fHasNext;
    };

    static void SetAdvancedTypefaceFlags(SkOTTableOS2_V4::Type fsType,
                                         SkAdvancedTypefaceMetrics* info);
};

#endif
