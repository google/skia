/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_indirect_DEFINED
#define SkFontMgr_indirect_DEFINED

#include "../private/SkMutex.h"
#include "../private/SkOnce.h"
#include "../private/SkTArray.h"
#include "SkFontMgr.h"
#include "SkRefCnt.h"
#include "SkRemotableFontMgr.h"
#include "SkTypeface.h"
#include "SkTypes.h"

class SkData;
class SkFontStyle;
class SkStreamAsset;
class SkString;

class SK_API SkFontMgr_Indirect : public SkFontMgr {
public:
    // TODO: The SkFontMgr is only used for createFromStream/File/Data.
    // In the future these calls should be broken out into their own interface
    // with a name like SkFontRenderer.
    SkFontMgr_Indirect(sk_sp<SkFontMgr> impl, sk_sp<SkRemotableFontMgr> proxy)
        : fImpl(std::move(impl)), fProxy(std::move(proxy))
    { }

protected:
    int onCountFamilies() const override;
    void onGetFamilyName(int index, SkString* familyName) const override;
    SkFontStyleSet* onCreateStyleSet(int index) const override;

    SkFontStyleSet* onMatchFamily(const char familyName[]) const override;

    SkTypeface* onMatchFamilyStyle(const char familyName[],
                                   const SkFontStyle& fontStyle) const override;

    SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                            const SkFontStyle&,
                                            const char* bcp47[],
                                            int bcp47Count,
                                            SkUnichar character) const override;

    SkTypeface* onMatchFaceStyle(const SkTypeface* familyMember,
                                 const SkFontStyle& fontStyle) const override;

    SkTypeface* onCreateFromStream(SkStreamAsset* stream, int ttcIndex) const override;
    SkTypeface* onCreateFromFile(const char path[], int ttcIndex) const override;
    SkTypeface* onCreateFromData(SkData* data, int ttcIndex) const override;

    SkTypeface* onLegacyCreateTypeface(const char familyName[], SkFontStyle) const override;

private:
    SkTypeface* createTypefaceFromFontId(const SkFontIdentity& fontId) const;

    sk_sp<SkFontMgr> fImpl;
    sk_sp<SkRemotableFontMgr> fProxy;

    struct DataEntry {
        uint32_t fDataId;  // key1
        uint32_t fTtcIndex;  // key2
        SkTypeface* fTypeface;  // value: weak ref to typeface

        DataEntry() { }

        DataEntry(DataEntry&& that)
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

    friend class SkStyleSet_Indirect;
};

#endif
