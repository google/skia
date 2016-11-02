/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTUtils_DEFINED
#define SkOTUtils_DEFINED

#include "SkOTTableTypes.h"
#include "SkOTTable_name.h"
#include "SkTypeface.h"

class SkData;
class SkStream;

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
        LocalizedStrings_NameTable(SkOTTableName* nameTableData,
                                   SkOTTableName::Record::NameID::Predefined::Value types[],
                                   int typesCount)
            : fTypes(types), fTypesCount(typesCount), fTypesIndex(0)
            , fNameTableData(nameTableData), fFamilyNameIter(*nameTableData, fTypes[fTypesIndex])
        { }

        /** Creates an iterator over all the family names in the 'name' table of a typeface.
         *  If no valid 'name' table can be found, returns nullptr.
         */
        static LocalizedStrings_NameTable* CreateForFamilyNames(const SkTypeface& typeface);

        bool next(SkTypeface::LocalizedString* localizedString) override;
    private:
        static SkOTTableName::Record::NameID::Predefined::Value familyNameTypes[3];

        SkOTTableName::Record::NameID::Predefined::Value* fTypes;
        int fTypesCount;
        int fTypesIndex;
        std::unique_ptr<SkOTTableName[]> fNameTableData;
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
};

#endif
