/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAdvancedTypefaceMetrics.h"
#include "SkEndian.h"
#include "SkFontDescriptor.h"
#include "SkFontHost.h"
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

    SkTypeface* t = SkFontHost::CreateTypeface(NULL, NULL, (Style)style);
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
    return SkFontHost::CreateTypeface(NULL, name, style);
}

SkTypeface* SkTypeface::CreateFromTypeface(const SkTypeface* family, Style s) {
    if (family && family->style() == s) {
        family->ref();
        return const_cast<SkTypeface*>(family);
    }
    return SkFontHost::CreateTypeface(family, NULL, s);
}

SkTypeface* SkTypeface::CreateFromStream(SkStream* stream) {
    return SkFontHost::CreateTypefaceFromStream(stream);
}

SkTypeface* SkTypeface::CreateFromFile(const char path[]) {
    return SkFontHost::CreateTypefaceFromFile(path);
}

///////////////////////////////////////////////////////////////////////////////

void SkTypeface::serialize(SkWStream* wstream) const {
    bool isLocal = false;
    SkFontDescriptor desc(this->style());
    this->onGetFontDescriptor(&desc, &isLocal);

    desc.serialize(wstream);
    if (isLocal) {
        int ttcIndex;   // TODO: write this to the stream?
        SkAutoTUnref<SkStream> rstream(this->openStream(&ttcIndex));
        if (rstream.get()) {
            size_t length = rstream->getLength();
            wstream->writePackedUInt(length);
            wstream->writeStream(rstream, length);
        } else {
            wstream->writePackedUInt(0);
        }
    } else {
        wstream->writePackedUInt(0);
    }
}

SkTypeface* SkTypeface::Deserialize(SkStream* stream) {
    SkFontDescriptor desc(stream);
    size_t length = stream->readPackedUInt();
    if (length > 0) {
        void* addr = sk_malloc_flags(length, 0);
        if (addr) {
            SkAutoTUnref<SkMemoryStream> localStream(SkNEW(SkMemoryStream));
            localStream->setMemoryOwned(addr, length);

            if (stream->read(addr, length) == length) {
                return SkTypeface::CreateFromStream(localStream.get());
            } else {
                // Failed to read the full font data, so fall through and try to create from name.
                // If this is because of EOF, all subsequent reads from the stream will be EOF.
                // If this is because of a stream error, the stream is in an error state,
                // do not attempt to skip any remaining bytes.
            }
        } else {
            // failed to allocate, so just skip and create-from-name
            stream->skip(length);
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
    bool isLocal = false;
    SkFontDescriptor desc(this->style());
    this->onGetFontDescriptor(&desc, &isLocal);
    name->set(desc.getFamilyName());
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
