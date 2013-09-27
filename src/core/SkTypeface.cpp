/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAdvancedTypefaceMetrics.h"
#include "SkFontDescriptor.h"
#include "SkFontHost.h"
#include "SkStream.h"
#include "SkTypeface.h"

SK_DEFINE_INST_COUNT(SkTypeface)

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
    SkEmptyTypeface() : SkTypeface(SkTypeface::kNormal, 0, true) { }
protected:
    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE { return NULL; }
    virtual SkScalerContext* onCreateScalerContext(const SkDescriptor*) const SK_OVERRIDE {
        return NULL;
    }
    virtual void onFilterRec(SkScalerContextRec*) const SK_OVERRIDE { }
    virtual SkAdvancedTypefaceMetrics* onGetAdvancedTypefaceMetrics(
                                SkAdvancedTypefaceMetrics::PerGlyphInfo,
                                const uint32_t*, uint32_t) const SK_OVERRIDE { return NULL; }
    virtual void onGetFontDescriptor(SkFontDescriptor*, bool*) const SK_OVERRIDE { }
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
    virtual SkTypeface* onRefMatchingStyle(Style) const SK_OVERRIDE { return NULL; }
};

SkTypeface* SkTypeface::GetDefaultTypeface(Style style) {
    // we keep a reference to this guy for all time, since if we return its
    // fontID, the font cache may later on ask to resolve that back into a
    // typeface object.
    static const uint32_t FONT_STYLE_COUNT = 4;
    static SkTypeface* gDefaultTypefaces[FONT_STYLE_COUNT];
    SkASSERT((unsigned)style < FONT_STYLE_COUNT);

    // mask off any other bits to avoid a crash in SK_RELEASE
    style = (Style)(style & 0x03);

    if (NULL == gDefaultTypefaces[style]) {
        gDefaultTypefaces[style] = SkFontHost::CreateTypeface(NULL, NULL, style);
    }
    if (NULL == gDefaultTypefaces[style]) {
        gDefaultTypefaces[style] = SkNEW(SkEmptyTypeface);
    }

    return gDefaultTypefaces[style];
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
    return this->onGetAdvancedTypefaceMetrics(info, glyphIDs, glyphIDsCount);
}

SkTypeface* SkTypeface::refMatchingStyle(Style style) const {
    return this->onRefMatchingStyle(style);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int SkTypeface::onCharsToGlyphs(const void* chars, Encoding encoding,
                                uint16_t glyphs[], int glyphCount) const {
    static bool printed = false;
    if (!printed) {
        // Only want to see this message once
        SkDebugf("\n *** onCharsToGlyphs unimplemented ***\n");
        printed = true;
    }
    if (glyphs && glyphCount > 0) {
        sk_bzero(glyphs, glyphCount * sizeof(glyphs[0]));
    }
    return 0;
}
