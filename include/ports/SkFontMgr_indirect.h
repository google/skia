/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_indirect_DEFINED
#define SkFontMgr_indirect_DEFINED

#include "SkDataTable.h"
#include "SkFontMgr.h"
#include "SkFontStyle.h"
#include "SkRemotableFontMgr.h"
#include "SkTArray.h"
#include "SkTypeface.h"

class SkData;
class SkStream;
class SkString;
class SkTypeface;

class SK_API SkFontMgr_Indirect : public SkFontMgr {
public:
    // TODO: The SkFontMgr is only used for createFromStream/File/Data.
    // In the future these calls should be broken out into their own interface
    // with a name like SkFontRenderer.
    SkFontMgr_Indirect(SkFontMgr* impl, SkRemotableFontMgr* proxy)
        : fImpl(SkRef(impl)), fProxy(SkRef(proxy)), fFamilyNamesInited(false)
    { }

protected:
    virtual int onCountFamilies() const SK_OVERRIDE;
    virtual void onGetFamilyName(int index, SkString* familyName) const SK_OVERRIDE;
    virtual SkFontStyleSet* onCreateStyleSet(int index) const SK_OVERRIDE;

    virtual SkFontStyleSet* onMatchFamily(const char familyName[]) const SK_OVERRIDE;

    virtual SkTypeface* onMatchFamilyStyle(const char familyName[],
                                           const SkFontStyle& fontStyle) const SK_OVERRIDE;

    virtual SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                                    const SkFontStyle&,
                                                    const char bpc47[],
                                                    uint32_t character) const SK_OVERRIDE;

    virtual SkTypeface* onMatchFaceStyle(const SkTypeface* familyMember,
                                         const SkFontStyle& fontStyle) const SK_OVERRIDE;

    virtual SkTypeface* onCreateFromStream(SkStream* stream, int ttcIndex) const SK_OVERRIDE;
    virtual SkTypeface* onCreateFromFile(const char path[], int ttcIndex) const SK_OVERRIDE;
    virtual SkTypeface* onCreateFromData(SkData* data, int ttcIndex) const SK_OVERRIDE;

    virtual SkTypeface* onLegacyCreateTypeface(const char familyName[],
                                               unsigned styleBits) const SK_OVERRIDE;

private:
    SkTypeface* createTypefaceFromFontId(const SkFontIdentity& fontId) const;

    SkAutoTUnref<SkFontMgr> fImpl;
    SkAutoTUnref<SkRemotableFontMgr> fProxy;

    struct DataEntry {
        uint32_t fDataId;  // key1
        uint32_t fTtcIndex;  // key2
        SkTypeface* fTypeface;  // value: weak ref to typeface

        DataEntry() { }

        // This is a move!!!
        DataEntry(DataEntry& that)
            : fDataId(that.fDataId)
            , fTtcIndex(that.fTtcIndex)
            , fTypeface(that.fTypeface)
        {
            SkDEBUGCODE(that.fDataId = SkFontIdentity::kInvalidDataId;)
            SkDEBUGCODE(that.fTtcIndex = 0xbbadbeef;)
            that.fTypeface = NULL;
        }

        ~DataEntry() {
            if (fTypeface) {
                fTypeface->weak_unref();
            }
        }
    };
    /**
     *  This cache is essentially { dataId: { ttcIndex: typeface } }
     *  For data caching we want a mapping from data id to weak references to
     *  typefaces with that data id. By storing the index next to the typeface,
     *  this data cache also acts as a typeface cache.
     */
    mutable SkTArray<DataEntry> fDataCache;
    mutable SkMutex fDataCacheMutex;

    mutable SkAutoTUnref<SkDataTable> fFamilyNames;
    mutable bool fFamilyNamesInited;
    mutable SkMutex fFamilyNamesMutex;
    static void set_up_family_names(const SkFontMgr_Indirect* self);

    friend class SkStyleSet_Indirect;
};

#endif
