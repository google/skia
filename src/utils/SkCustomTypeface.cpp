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

static SkFontMetrics scale_fontmetrics(const SkFontMetrics& src, float sx, float sy) {
    SkFontMetrics dst = src;

    #define SCALE_X(field)  dst.field *= sx
    #define SCALE_Y(field)  dst.field *= sy

    SCALE_X(fAvgCharWidth);
    SCALE_X(fMaxCharWidth);
    SCALE_X(fXMin);
    SCALE_X(fXMax);

    SCALE_Y(fTop);
    SCALE_Y(fAscent);
    SCALE_Y(fDescent);
    SCALE_Y(fBottom);
    SCALE_Y(fLeading);
    SCALE_Y(fXHeight);
    SCALE_Y(fCapHeight);
    SCALE_Y(fUnderlineThickness);
    SCALE_Y(fUnderlinePosition);
    SCALE_Y(fStrikeoutThickness);
    SCALE_Y(fStrikeoutPosition);

    #undef SCALE_X
    #undef SCALE_Y

    return dst;
}

class SkUserTypeface final : public SkTypeface {
private:
    friend class SkCustomTypefaceBuilder;
    friend class SkUserScalerContext;

    explicit SkUserTypeface(SkFontStyle style) : SkTypeface(style) {}

    std::vector<SkPath> fPaths;
    std::vector<float>  fAdvances;
    SkFontMetrics       fMetrics;

    std::unique_ptr<SkScalerContext> onCreateScalerContext(const SkScalerContextEffects&,
                                                           const SkDescriptor* desc) const override;
    void onFilterRec(SkScalerContextRec* rec) const override;
    void getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const override;

    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override;

    void onGetFamilyName(SkString* familyName) const override;
    bool onGetPostScriptName(SkString*) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;

    std::unique_ptr<SkStreamAsset> onOpenStream(int*) const override;

    // trivial

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        return sk_ref_sp(this);
    }
    int onCountGlyphs() const override { return this->glyphCount(); }
    int onGetUPEM() const override { return 2048; /* ?? */ }
    bool onComputeBounds(SkRect* bounds) const override {
        bounds->setLTRB(fMetrics.fXMin, fMetrics.fTop, fMetrics.fXMax, fMetrics.fBottom);
        return true;
    }

    // noops

    void getPostScriptGlyphNames(SkString*) const override {}
    bool onGlyphMaskNeedsCurrentColor() const override { return false; }
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate[],
                                     int) const override { return 0; }
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis[],
                                       int) const override { return 0; }
    int onGetTableTags(SkFontTableTag tags[]) const override { return 0; }
    size_t onGetTableData(SkFontTableTag, size_t, size_t, void*) const override { return 0; }

    int glyphCount() const {
        SkASSERT(fPaths.size() == fAdvances.size());
        return SkToInt(fPaths.size());
    }
};

SkCustomTypefaceBuilder::SkCustomTypefaceBuilder() {
    sk_bzero(&fMetrics, sizeof(fMetrics));
}

void SkCustomTypefaceBuilder::setMetrics(const SkFontMetrics& fm, float scale) {
    fMetrics = scale_fontmetrics(fm, scale, scale);
}

void SkCustomTypefaceBuilder::setFontStyle(SkFontStyle style) {
    fStyle = style;
}

void SkCustomTypefaceBuilder::setGlyph(SkGlyphID index, float advance, const SkPath& path) {
    SkASSERT(fPaths.size() == fAdvances.size());
    if (index >= fPaths.size()) {
           fPaths.resize(SkToSizeT(index) + 1);
        fAdvances.resize(SkToSizeT(index) + 1);
    }
    fAdvances[index] = advance;
    fPaths[index]    = path;
}

sk_sp<SkTypeface> SkCustomTypefaceBuilder::detach() {
    SkASSERT(fPaths.size() == fAdvances.size());
    if (fPaths.empty()) return nullptr;

    sk_sp<SkUserTypeface> tf(new SkUserTypeface(fStyle));
    tf->fAdvances = std::move(fAdvances);
    tf->fPaths    = std::move(fPaths);
    tf->fMetrics  = fMetrics;

    // initially inverted, so that any "union" will overwrite the first time
    SkRect bounds = {SK_ScalarMax, SK_ScalarMax, -SK_ScalarMax, -SK_ScalarMax};

    for (const auto& path : tf->fPaths) {
        if (!path.isEmpty()) {
            bounds.join(path.getBounds());
        }
    }
    tf->fMetrics.fTop    = bounds.top();
    tf->fMetrics.fBottom = bounds.bottom();
    tf->fMetrics.fXMin   = bounds.left();
    tf->fMetrics.fXMax   = bounds.right();

    return std::move(tf);
}

/////////////

#include "src/core/SkScalerContext.h"

void SkUserTypeface::onFilterRec(SkScalerContextRec* rec) const {
    rec->setHinting(SkFontHinting::kNone);
}

void SkUserTypeface::getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const {
    for (int gid = 0; gid < this->glyphCount(); ++gid) {
        glyphToUnicode[gid] = SkTo<SkUnichar>(gid);
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
        glyphs[i] = uni[i] < this->glyphCount() ? SkTo<SkGlyphID>(uni[i]) : 0;
    }
}

void SkUserTypeface::onGetFamilyName(SkString* familyName) const {
    *familyName = "";
}

bool SkUserTypeface::onGetPostScriptName(SkString*) const {
    return false;
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
        // Always generates from paths, so SkScalerContext::makeGlyph will figure the bounds.
    }

    void generateImage(const SkGlyph&) override { SK_ABORT("Should have generated from path."); }

    bool generatePath(SkGlyphID glyph, SkPath* path) override {
        this->userTF()->fPaths[glyph].transform(fMatrix, path);
        return true;
    }

    void generateFontMetrics(SkFontMetrics* metrics) override {
        auto [sx, sy] = fMatrix.mapXY(1, 1);
        *metrics = scale_fontmetrics(this->userTF()->fMetrics, sx, sy);
    }

private:
    SkMatrix fMatrix;
};

std::unique_ptr<SkScalerContext> SkUserTypeface::onCreateScalerContext(
    const SkScalerContextEffects& effects, const SkDescriptor* desc) const
{
    return std::make_unique<SkUserScalerContext>(
            sk_ref_sp(const_cast<SkUserTypeface*>(this)), effects, desc);
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
static const char gHeaderString[] = "SkUserTypeface01";
static_assert(sizeof(gHeaderString) == 1 + kHeaderSize, "need header to be 16 bytes");

std::unique_ptr<SkStreamAsset> SkUserTypeface::onOpenStream(int* ttcIndex) const {
    SkDynamicMemoryWStream wstream;

    wstream.write(gHeaderString, kHeaderSize);

    wstream.write(&fMetrics, sizeof(fMetrics));

    SkFontStyle style = this->fontStyle();
    wstream.write(&style, sizeof(style));

    // just hacking around -- this makes the serialized font 1/2 size
    const bool use_compression = false;

    wstream.write32(this->glyphCount());

    if (use_compression) {
        for (float a : fAdvances) {
            write_scaled_float_to_16(&wstream, a, 2048);
        }
    } else {
        wstream.write(fAdvances.data(), this->glyphCount() * sizeof(float));
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
        0 != memcmp(header, gHeaderString, kHeaderSize))
    {
        return nullptr;
    }

    SkFontMetrics metrics;
    if (stream->read(&metrics, sizeof(metrics)) != sizeof(metrics)) {
        return nullptr;
    }

    SkFontStyle style;
    if (stream->read(&style, sizeof(style)) != sizeof(style)) {
        return nullptr;
    }

    int glyphCount;
    if (!stream->readS32(&glyphCount) || glyphCount < 0 || glyphCount > kMaxGlyphCount) {
        return nullptr;
    }

    SkCustomTypefaceBuilder builder;

    builder.setMetrics(metrics);
    builder.setFontStyle(style);

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
