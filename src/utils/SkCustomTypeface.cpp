/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFontMetrics.h"
#include "include/utils/SkCustomTypeface.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"

class SkUserTypeface : public SkTypeface {
    friend class SkCustomTypefaceBuilder;
    friend class SkUserScalerContext;

    SkUserTypeface(int count)
        : SkTypeface(SkFontStyle())
        , fGlyphCount(count)
    {}

    const int fGlyphCount;
    std::vector<SkPath> fPaths;
    std::vector<float>  fAdvances;
    SkRect              fBounds;

protected:
    SkScalerContext* onCreateScalerContext(const SkScalerContextEffects&,
                                           const SkDescriptor* desc) const override;
    void onFilterRec(SkScalerContextRec* rec) const override;
    void getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const override;

    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override;

    void onGetFamilyName(SkString* familyName) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;

    std::unique_ptr<SkStreamAsset> onOpenStream(int*) const override;

    // trivial

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        return sk_ref_sp(this);
    }
    int onCountGlyphs() const override { return fGlyphCount; }
    int onGetUPEM() const override { return 2048; /* ?? */ }
    bool onComputeBounds(SkRect* bounds) const override { *bounds = fBounds; return true; }

    // noops

    void getPostScriptGlyphNames(SkString*) const override {}
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate[],
                                     int) const override { return 0; }
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis[],
                                       int) const override { return 0; }
    int onGetTableTags(SkFontTableTag tags[]) const override { return 0; }
    size_t onGetTableData(SkFontTableTag, size_t, size_t, void*) const override { return 0; }
};

SkCustomTypefaceBuilder::SkCustomTypefaceBuilder(int numGlyphs) : fGlyphCount(numGlyphs) {
    fAdvances.resize(numGlyphs);
    fPaths.resize(numGlyphs);
}

void SkCustomTypefaceBuilder::setGlyph(SkGlyphID index, float advance, const SkPath& path) {
    if (index >= (unsigned)fGlyphCount) {
        return;
    }
    fAdvances[index] = advance;
    fPaths[index]    = path;
}

sk_sp<SkTypeface> SkCustomTypefaceBuilder::detach() {
    if (fGlyphCount <= 0) return nullptr;

    SkUserTypeface* tf = new SkUserTypeface(fGlyphCount);
    tf->fAdvances = std::move(fAdvances);
    tf->fPaths    = std::move(fPaths);

    // initially inverted, so that any "union" will overwrite the first time
    SkRect bounds = {SK_ScalarMax, SK_ScalarMax, -SK_ScalarMax, -SK_ScalarMax};

    for (const auto& path : tf->fPaths) {
        if (!path.isEmpty()) {
            bounds.join(path.getBounds());
        }
    }
    tf->fBounds = bounds;

    return sk_sp<SkTypeface>(tf);
}

/////////////

#include "src/core/SkScalerContext.h"

void SkUserTypeface::onFilterRec(SkScalerContextRec* rec) const {
    rec->setHinting(SkFontHinting::kNone);
}

void SkUserTypeface::getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const {
    for (int gid = 0; gid < fGlyphCount; ++gid) {
        glyphToUnicode[gid] = 0;
    }
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkUserTypeface::onGetAdvancedMetrics() const {
    return nullptr;
}

void SkUserTypeface::onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const {
    *isLocal = true;
}

void SkUserTypeface::onCharsToGlyphs(const SkUnichar uni[], int count, SkGlyphID glyphs[]) const {
    for (int i = 0; i < count; ++i) {
        glyphs[i] = 0;
    }
}

void SkUserTypeface::onGetFamilyName(SkString* familyName) const {
    *familyName = "";
}

SkTypeface::LocalizedStrings* SkUserTypeface::onCreateFamilyNameIterator() const {
    return nullptr;
}

//////////////

#include "src/core/SkScalerContext.h"

class SkUserScalerContext : public SkScalerContext {
public:
    SkUserScalerContext(sk_sp<SkUserTypeface>           face,
                        const SkScalerContextEffects& effects,
                        const SkDescriptor*           desc)
            : SkScalerContext(std::move(face), effects, desc) {
        fRec.getSingleMatrix(&fMatrix);
        this->forceGenerateImageFromPath();
    }

    const SkUserTypeface* userTF() const {
        return static_cast<SkUserTypeface*>(this->getTypeface());
    }

protected:
    unsigned generateGlyphCount() override {
        return this->userTF()->fGlyphCount;
    }

    bool generateAdvance(SkGlyph* glyph) override {
        const SkUserTypeface* tf = this->userTF();
        auto advance = fMatrix.mapXY(tf->fAdvances[glyph->getGlyphID()], 0);

        glyph->fAdvanceX = advance.fX;
        glyph->fAdvanceY = advance.fY;
        return true;
    }

    void generateMetrics(SkGlyph* glyph) override {
        glyph->zeroMetrics();
        this->generateAdvance(glyph);
        // Always generates from paths, so SkScalerContext::getMetrics will figure the bounds.
    }

    void generateImage(const SkGlyph&) override { SK_ABORT("Should have generated from path."); }

    bool generatePath(SkGlyphID glyph, SkPath* path) override {
        this->userTF()->fPaths[glyph].transform(fMatrix, path);
        return true;
    }

    void generateFontMetrics(SkFontMetrics* metrics) override {
        auto [_, sy] = fMatrix.mapXY(0, 1);

        sk_bzero(metrics, sizeof(*metrics));
        metrics->fTop    = this->userTF()->fBounds.fTop    * sy;
        metrics->fBottom = this->userTF()->fBounds.fBottom * sy;

        // todo: get these from the creator of the typeface?
        metrics->fAscent = metrics->fTop;
        metrics->fDescent = metrics->fBottom;
    }

private:
    SkMatrix fMatrix;
};

SkScalerContext* SkUserTypeface::onCreateScalerContext(const SkScalerContextEffects& effects,
                                                       const SkDescriptor*           desc) const {
    return new SkUserScalerContext(sk_ref_sp(const_cast<SkUserTypeface*>(this)), effects, desc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "include/private/SkFloatingPoint.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkPathPriv.h"

static void write_scaled_float_to_16(SkWStream* stream, float x, float scale) {
    stream->write16(SkToS16(sk_float_round2int(x * scale)) & 0xFFFF);
}

enum PVerb {
    kMove,
    kLine,
    kCurve,
    kClose,
};

static void compress_write(SkWStream* stream, const SkPath& path, int upem) {
    int pCount = 0;
    std::vector<PVerb> verbs;
    for (auto [v, p, w] : SkPathPriv::Iterate(path)) {
        switch (v) {
            default: break;
            case SkPathVerb::kMove: verbs.push_back(kMove); pCount += 1; break;
            case SkPathVerb::kQuad: verbs.push_back(kCurve); pCount += 2; break;
            case SkPathVerb::kLine: verbs.push_back(kLine); pCount += 1; break;
            case SkPathVerb::kClose: verbs.push_back(kClose); break;
        }
    }

    int vCount = verbs.size();

    stream->write16(upem);      // share w/ other paths?
    stream->write16(vCount);
    stream->write16(pCount);
    for (int i = 0; i < (vCount & ~3); i += 4) {
        stream->write8((verbs[i+0]<<6) | (verbs[i+1]<<4) | (verbs[i+2]<<2) | verbs[i+3]);
    }
    if (vCount & 3) {
        uint8_t b = 0;
        int shift = 6;
        for (int i = vCount & ~3; i < vCount; ++i) {
            b |= verbs[i] << shift;
            shift >>= 2;
        }
        stream->write8(b);
    }
    if (vCount & 1) {
        stream->write8(0);
    }

    const float scale = (float)upem;
    auto write_pts = [&](const SkPoint pts[], int count) {
        for (int i = 0; i < count; ++i) {
            write_scaled_float_to_16(stream, pts[i].fX, scale);
            write_scaled_float_to_16(stream, pts[i].fY, scale);
        }
    };

    for (auto [v, p, w] : SkPathPriv::Iterate(path)) {
        switch (v) {
            default: break;
            case SkPathVerb::kMove: write_pts(&p[0], 1); break;
            case SkPathVerb::kQuad: write_pts(&p[1], 2); break;
            case SkPathVerb::kLine: write_pts(&p[1], 1); break;
            case SkPathVerb::kClose:                     break;
        }
    }
}

static constexpr int kMaxGlyphCount = 65536;
static constexpr size_t kHeaderSize = 16;
static const char gHeaderString[] = "SkUserTypeface00";
static_assert(sizeof(gHeaderString) == 1 + kHeaderSize, "need header to be 16 bytes");

std::unique_ptr<SkStreamAsset> SkUserTypeface::onOpenStream(int* ttcIndex) const {
    SkDynamicMemoryWStream wstream;

    wstream.write(gHeaderString, kHeaderSize);

    SkASSERT(fAdvances.size() == (unsigned)fGlyphCount);
    SkASSERT(fPaths.size() == (unsigned)fGlyphCount);

    // just hacking around -- this makes the serialized font 1/2 size
    const bool use_compression = false;

    wstream.write32(fGlyphCount);

    if (use_compression) {
        for (float a : fAdvances) {
            write_scaled_float_to_16(&wstream, a, 2048);
        }
    } else {
        wstream.write(fAdvances.data(), fGlyphCount * sizeof(float));
    }

    for (const auto& p : fPaths) {
        if (use_compression) {
            compress_write(&wstream, p, 2048);
        } else {
            auto data = p.serialize();
            SkASSERT(SkIsAlign4(data->size()));
            wstream.write(data->data(), data->size());
        }
    }
//    SkDebugf("%d glyphs, %d bytes\n", fGlyphCount, wstream.bytesWritten());
    *ttcIndex = 0;
    return wstream.detachAsStream();
}

class AutoRestorePosition {
    SkStream* fStream;
    size_t fPosition;
public:
    AutoRestorePosition(SkStream* stream) : fStream(stream) {
        fPosition = stream->getPosition();
    }

    ~AutoRestorePosition() {
        if (fStream) {
            fStream->seek(fPosition);
        }
    }

    // So we don't restore the position
    void markDone() { fStream = nullptr; }
};

sk_sp<SkTypeface> SkCustomTypefaceBuilder::Deserialize(SkStream* stream) {
    AutoRestorePosition arp(stream);

    char header[kHeaderSize];
    if (stream->read(header, kHeaderSize) != kHeaderSize ||
        memcmp(header, gHeaderString, kHeaderSize) != 0)
    {
        return nullptr;
    }

    int glyphCount;
    if (!stream->readS32(&glyphCount) || glyphCount < 0 || glyphCount > kMaxGlyphCount) {
        return nullptr;
    }

    SkCustomTypefaceBuilder builder(glyphCount);

    std::vector<float> advances(glyphCount);
    if (stream->read(advances.data(), glyphCount * sizeof(float)) != glyphCount * sizeof(float)) {
        return nullptr;
    }

    // SkPath can read from a stream, so we have to page the rest into ram
    const size_t offset = stream->getPosition();
    const size_t length = stream->getLength() - offset;
    SkAutoMalloc ram(length);
    char* buffer = (char*)ram.get();

    if (stream->read(buffer, length) != length) {
        return nullptr;
    }

    size_t totalUsed = 0;
    for (int i = 0; i < glyphCount; ++i) {
        SkPath path;
        size_t used = path.readFromMemory(buffer + totalUsed, length - totalUsed);
        if (used == 0) {
            return nullptr;
        }
        builder.setGlyph(i, advances[i], path);
        totalUsed += used;
        SkASSERT(length >= totalUsed);
    }

    // all done, update the stream to only reflect the bytes we needed
    stream->seek(offset + totalUsed);

    arp.markDone();
    return builder.detach();
}
