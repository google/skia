/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAdvancedTypefaceMetrics.h"
#include "SkEndian.h"
#include "SkFontDescriptor.h"
#include "SkFontMgr.h"
#include "SkLazyPtr.h"
#include "SkOTTable_OS_2.h"
#include "SkStream.h"
#include "SkTypeface.h"

//#define TRACE_LIFECYCLE

#ifdef TRACE_LIFECYCLE
    static int32_t gTypefaceCounter;
#endif

SkTypeface::SkTypeface(Style style, SkFontID fontID, bool isFixedPitch)
    : fUniqueID(fontID), fStyle(style), fIsFixedPitch(isFixedPitch) {
#ifdef TRACE_LIFECYCLE
    SkDebugf("SkTypeface: create  %p fontID %d total %d\n",
             this, fontID, ++gTypefaceCounter);
#endif
}

SkTypeface::~SkTypeface() {
#ifdef TRACE_LIFECYCLE
    SkDebugf("SkTypeface: destroy %p fontID %d total %d\n",
             this, fUniqueID, --gTypefaceCounter);
#endif
}

///////////////////////////////////////////////////////////////////////////////

class SkEmptyTypeface : public SkTypeface {
public:
    static SkEmptyTypeface* Create() {
        return SkNEW(SkEmptyTypeface);
    }
protected:
    SkEmptyTypeface() : SkTypeface(SkTypeface::kNormal, 0, true) { }

    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE { return NULL; }
    virtual SkScalerContext* onCreateScalerContext(const SkDescriptor*) const SK_OVERRIDE {
        return NULL;
    }
    virtual void onFilterRec(SkScalerContextRec*) const SK_OVERRIDE { }
    virtual SkAdvancedTypefaceMetrics* onGetAdvancedTypefaceMetrics(
                                SkAdvancedTypefaceMetrics::PerGlyphInfo,
                                const uint32_t*, uint32_t) const SK_OVERRIDE { return NULL; }
    virtual void onGetFontDescriptor(SkFontDescriptor*, bool*) const SK_OVERRIDE { }
    virtual int onCharsToGlyphs(const void* chars, Encoding encoding,
                                uint16_t glyphs[], int glyphCount) const SK_OVERRIDE {
        if (glyphs && glyphCount > 0) {
            sk_bzero(glyphs, glyphCount * sizeof(glyphs[0]));
        }
        return 0;
    }
    virtual int onCountGlyphs() const SK_OVERRIDE { return 0; };
    virtual int onGetUPEM() const SK_OVERRIDE { return 0; };
    class EmptyLocalizedStrings : public SkTypeface::LocalizedStrings {
    public:
        virtual bool next(SkTypeface::LocalizedString*) SK_OVERRIDE { return false; }
    };
    virtual void onGetFamilyName(SkString* familyName) const SK_OVERRIDE {
        familyName->reset();
    }
    virtual SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const SK_OVERRIDE {
        return SkNEW(EmptyLocalizedStrings);
    };
    virtual int onGetTableTags(SkFontTableTag tags[]) const SK_OVERRIDE { return 0; }
    virtual size_t onGetTableData(SkFontTableTag, size_t, size_t, void*) const SK_OVERRIDE {
        return 0;
    }
};

SK_DECLARE_STATIC_MUTEX(gCreateDefaultMutex);
SkTypeface* SkTypeface::CreateDefault(int style) {
    // If backed by fontconfig, it's not safe to call SkFontHost::CreateTypeface concurrently.
    // To be safe, we serialize here with a mutex so only one call to
    // CreateTypeface is happening at any given time.
    // TODO(bungeman, mtklein): This is sad.  Make our fontconfig code safe?
    SkAutoMutexAcquire lock(&gCreateDefaultMutex);

    SkAutoTUnref<SkFontMgr> fm(SkFontMgr::RefDefault());
    SkTypeface* t = fm->legacyCreateTypeface(NULL, style);;
    return t ? t : SkEmptyTypeface::Create();
}

void SkTypeface::DeleteDefault(SkTypeface* t) {
    // The SkTypeface returned by SkFontHost::CreateTypeface may _itself_ be a
    // cleverly-shared singleton.  This is less than ideal.  This means we
    // cannot just assert our ownership and SkDELETE(t) like we'd want to.
    SkSafeUnref(t);
}

SkTypeface* SkTypeface::GetDefaultTypeface(Style style) {
    SK_DECLARE_STATIC_LAZY_PTR_ARRAY(SkTypeface, defaults, 4, CreateDefault, DeleteDefault);

    SkASSERT((int)style < 4);
    return defaults[style];
}

SkTypeface* SkTypeface::RefDefault(Style style) {
    return SkRef(GetDefaultTypeface(style));
}

uint32_t SkTypeface::UniqueID(const SkTypeface* face) {
    if (NULL == face) {
        face = GetDefaultTypeface();
    }
    return face->uniqueID();
}

bool SkTypeface::Equal(const SkTypeface* facea, const SkTypeface* faceb) {
    return SkTypeface::UniqueID(facea) == SkTypeface::UniqueID(faceb);
}

///////////////////////////////////////////////////////////////////////////////

SkTypeface* SkTypeface::CreateFromName(const char name[], Style style) {
    if (NULL == name) {
        return RefDefault(style);
    }
    SkAutoTUnref<SkFontMgr> fm(SkFontMgr::RefDefault());
    return fm->legacyCreateTypeface(name, style);
}

SkTypeface* SkTypeface::CreateFromTypeface(const SkTypeface* family, Style s) {
    if (!family) {
        return SkTypeface::RefDefault(s);
    }

    if (family->style() == s) {
        family->ref();
        return const_cast<SkTypeface*>(family);
    }

    SkAutoTUnref<SkFontMgr> fm(SkFontMgr::RefDefault());
    bool bold = s & SkTypeface::kBold;
    bool italic = s & SkTypeface::kItalic;
    SkFontStyle newStyle = SkFontStyle(bold ? SkFontStyle::kBold_Weight
                                            : SkFontStyle::kNormal_Weight,
                                       SkFontStyle::kNormal_Width,
                                       italic ? SkFontStyle::kItalic_Slant
                                              : SkFontStyle::kUpright_Slant);
    return fm->matchFaceStyle(family, newStyle);
}

SkTypeface* SkTypeface::CreateFromStream(SkStream* stream, int index) {
    SkAutoTUnref<SkFontMgr> fm(SkFontMgr::RefDefault());
    return fm->createFromStream(stream, index);
}

SkTypeface* SkTypeface::CreateFromFile(const char path[], int index) {
    SkAutoTUnref<SkFontMgr> fm(SkFontMgr::RefDefault());
    return fm->createFromFile(path, index);
}

///////////////////////////////////////////////////////////////////////////////

void SkTypeface::serialize(SkWStream* wstream) const {
    bool isLocal = false;
    SkFontDescriptor desc(this->style());
    this->onGetFontDescriptor(&desc, &isLocal);

    if (isLocal && NULL == desc.getFontData()) {
        int ttcIndex;
        desc.setFontData(this->onOpenStream(&ttcIndex));
        desc.setFontIndex(ttcIndex);
    }

    desc.serialize(wstream);
}

SkTypeface* SkTypeface::Deserialize(SkStream* stream) {
    SkFontDescriptor desc(stream);
    SkStream* data = desc.getFontData();
    if (data) {
        SkTypeface* typeface = SkTypeface::CreateFromStream(data, desc.getFontIndex());
        if (typeface) {
            return typeface;
        }
    }
    return SkTypeface::CreateFromName(desc.getFamilyName(), desc.getStyle());
}

///////////////////////////////////////////////////////////////////////////////

int SkTypeface::countTables() const {
    return this->onGetTableTags(NULL);
}

int SkTypeface::getTableTags(SkFontTableTag tags[]) const {
    return this->onGetTableTags(tags);
}

size_t SkTypeface::getTableSize(SkFontTableTag tag) const {
    return this->onGetTableData(tag, 0, ~0U, NULL);
}

size_t SkTypeface::getTableData(SkFontTableTag tag, size_t offset, size_t length,
                                void* data) const {
    return this->onGetTableData(tag, offset, length, data);
}

SkStream* SkTypeface::openStream(int* ttcIndex) const {
    int ttcIndexStorage;
    if (NULL == ttcIndex) {
        // So our subclasses don't need to check for null param
        ttcIndex = &ttcIndexStorage;
    }
    return this->onOpenStream(ttcIndex);
}

int SkTypeface::charsToGlyphs(const void* chars, Encoding encoding,
                              uint16_t glyphs[], int glyphCount) const {
    if (glyphCount <= 0) {
        return 0;
    }
    if (NULL == chars || (unsigned)encoding > kUTF32_Encoding) {
        if (glyphs) {
            sk_bzero(glyphs, glyphCount * sizeof(glyphs[0]));
        }
        return 0;
    }
    return this->onCharsToGlyphs(chars, encoding, glyphs, glyphCount);
}

int SkTypeface::countGlyphs() const {
    return this->onCountGlyphs();
}

int SkTypeface::getUnitsPerEm() const {
    // should we try to cache this in the base-class?
    return this->onGetUPEM();
}

bool SkTypeface::getKerningPairAdjustments(const uint16_t glyphs[], int count,
                                           int32_t adjustments[]) const {
    SkASSERT(count >= 0);
    // check for the only legal way to pass a NULL.. everything is 0
    // in which case they just want to know if this face can possibly support
    // kerning (true) or never (false).
    if (NULL == glyphs || NULL == adjustments) {
        SkASSERT(NULL == glyphs);
        SkASSERT(0 == count);
        SkASSERT(NULL == adjustments);
    }
    return this->onGetKerningPairAdjustments(glyphs, count, adjustments);
}

SkTypeface::LocalizedStrings* SkTypeface::createFamilyNameIterator() const {
    return this->onCreateFamilyNameIterator();
}

void SkTypeface::getFamilyName(SkString* name) const {
    SkASSERT(name);
    this->onGetFamilyName(name);
}

SkAdvancedTypefaceMetrics* SkTypeface::getAdvancedTypefaceMetrics(
                                SkAdvancedTypefaceMetrics::PerGlyphInfo info,
                                const uint32_t* glyphIDs,
                                uint32_t glyphIDsCount) const {
    SkAdvancedTypefaceMetrics* result =
            this->onGetAdvancedTypefaceMetrics(info, glyphIDs, glyphIDsCount);
    if (result && result->fType == SkAdvancedTypefaceMetrics::kTrueType_Font) {
        struct SkOTTableOS2 os2table;
        if (this->getTableData(SkTEndian_SwapBE32(SkOTTableOS2::TAG), 0,
                               sizeof(os2table), &os2table) > 0) {
            if (os2table.version.v2.fsType.field.Bitmap ||
                (os2table.version.v2.fsType.field.Restricted &&
                 !(os2table.version.v2.fsType.field.PreviewPrint ||
                   os2table.version.v2.fsType.field.Editable))) {
                result->fFlags = SkTBitOr<SkAdvancedTypefaceMetrics::FontFlags>(
                        result->fFlags,
                        SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag);
            }
            if (os2table.version.v2.fsType.field.NoSubsetting) {
                result->fFlags = SkTBitOr<SkAdvancedTypefaceMetrics::FontFlags>(
                        result->fFlags,
                        SkAdvancedTypefaceMetrics::kNotSubsettable_FontFlag);
            }
        }
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool SkTypeface::onGetKerningPairAdjustments(const uint16_t glyphs[], int count,
                                             int32_t adjustments[]) const {
    return false;
}
