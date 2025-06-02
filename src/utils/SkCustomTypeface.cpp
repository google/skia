/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkCustomTypeface.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontParameters.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkAdvancedTypefaceMetrics.h" // IWYU pragma: keep
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStreamPriv.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

class SkArenaAlloc;
class SkDescriptor;

namespace {
static inline const constexpr bool kSkShowTextBlitCoverage = false;
}

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

    explicit SkUserTypeface(SkFontStyle style, const SkFontMetrics& metrics,
                            std::vector<SkCustomTypefaceBuilder::GlyphRec>&& recs)
        : SkTypeface(style)
        , fGlyphRecs(std::move(recs))
        , fMetrics(metrics)
    {}

    const std::vector<SkCustomTypefaceBuilder::GlyphRec> fGlyphRecs;
    const SkFontMetrics                                  fMetrics;

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

    std::unique_ptr<SkStreamAsset> onOpenExistingStream(int*) const override { return nullptr; }

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
        return SkToInt(fGlyphRecs.size());
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

SkCustomTypefaceBuilder::GlyphRec& SkCustomTypefaceBuilder::ensureStorage(SkGlyphID index) {
    if (index >= fGlyphRecs.size()) {
           fGlyphRecs.resize(SkToSizeT(index) + 1);
    }

    return fGlyphRecs[index];
}

void SkCustomTypefaceBuilder::setGlyph(SkGlyphID index, float advance, const SkPath& path) {
    auto& rec = this->ensureStorage(index);
    rec.fAdvance  = advance;
    rec.fPath     = path;
    rec.fDrawable = nullptr;
}

void SkCustomTypefaceBuilder::setGlyph(SkGlyphID index, float advance,
                                       sk_sp<SkDrawable> drawable, const SkRect& bounds) {
    auto& rec = this->ensureStorage(index);
    rec.fAdvance  = advance;
    rec.fDrawable = std::move(drawable);
    rec.fBounds   = bounds;
    rec.fPath.reset();
}

sk_sp<SkTypeface> SkCustomTypefaceBuilder::detach() {
    if (fGlyphRecs.empty()) return nullptr;

    // initially inverted, so that any "union" will overwrite the first time
    SkRect bounds = {SK_ScalarMax, SK_ScalarMax, -SK_ScalarMax, -SK_ScalarMax};

    for (const auto& rec : fGlyphRecs) {
        bounds.join(rec.isDrawable()
                        ? rec.fBounds
                        : rec.fPath.getBounds());
    }

    fMetrics.fTop    = bounds.top();
    fMetrics.fBottom = bounds.bottom();
    fMetrics.fXMin   = bounds.left();
    fMetrics.fXMax   = bounds.right();

    return sk_sp<SkUserTypeface>(new SkUserTypeface(fStyle, fMetrics, std::move(fGlyphRecs)));
}

/////////////

void SkUserTypeface::onFilterRec(SkScalerContextRec* rec) const {
    rec->useStrokeForFakeBold();
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
    desc->setFactoryId(SkCustomTypefaceBuilder::FactoryId);
    *isLocal = true;
}

void SkUserTypeface::onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const {
    for (int i = 0; i < count; ++i) {
        glyphs[i] = chars[i] < this->glyphCount() ? SkTo<SkGlyphID>(chars[i]) : 0;
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

class SkUserScalerContext : public SkScalerContext {
public:
    SkUserScalerContext(SkUserTypeface& face,
                        const SkScalerContextEffects& effects,
                        const SkDescriptor* desc)
            : SkScalerContext(face, effects, desc) {
        fRec.getSingleMatrix(&fMatrix);
    }

    const SkUserTypeface* userTF() const {
        return static_cast<SkUserTypeface*>(this->getTypeface());
    }

protected:
    GlyphMetrics generateMetrics(const SkGlyph& glyph, SkArenaAlloc*) override {
        GlyphMetrics mx(glyph.maskFormat());

        const SkUserTypeface* tf = this->userTF();
        const SkGlyphID gid = glyph.getGlyphID();
        if (gid >= tf->fGlyphRecs.size()) {
            mx.neverRequestPath = true;
            return mx;
        }

        const auto& rec = tf->fGlyphRecs[gid];
        mx.advance = fMatrix.mapPoint({rec.fAdvance, 0});

        if (rec.isDrawable()) {
            mx.maskFormat = SkMask::kARGB32_Format;

            SkRect bounds = fMatrix.mapRect(rec.fBounds);
            bounds.offset(SkFixedToScalar(glyph.getSubXFixed()),
                          SkFixedToScalar(glyph.getSubYFixed()));
            bounds.roundOut(&mx.bounds);

            // These do not have an outline path.
            mx.neverRequestPath = true;
        } else {
            mx.computeFromPath = true;
        }
        return mx;
    }

    void generateImage(const SkGlyph& glyph, void* imageBuffer) override {
        const auto& rec = this->userTF()->fGlyphRecs[glyph.getGlyphID()];
        if (!rec.isDrawable()) {
            this->generateImageFromPath(glyph, imageBuffer);
            return;
        }

        auto canvas = SkCanvas::MakeRasterDirectN32(glyph.width(), glyph.height(),
                                                    static_cast<SkPMColor*>(imageBuffer),
                                                    glyph.rowBytes());
        if constexpr (kSkShowTextBlitCoverage) {
            canvas->clear(0x33FF0000);
        } else {
            canvas->clear(SK_ColorTRANSPARENT);
        }

        canvas->translate(-glyph.left(), -glyph.top());
        canvas->translate(SkFixedToScalar(glyph.getSubXFixed()),
                          SkFixedToScalar(glyph.getSubYFixed()));
        canvas->drawDrawable(rec.fDrawable.get(), &fMatrix);
    }

    bool generatePath(const SkGlyph& glyph, SkPath* path, bool* modified) override {
        const auto& rec = this->userTF()->fGlyphRecs[glyph.getGlyphID()];

        SkASSERT(!rec.isDrawable());

        rec.fPath.transform(fMatrix, path);

        return true;
    }

    sk_sp<SkDrawable> generateDrawable(const SkGlyph& glyph) override {
        class DrawableMatrixWrapper final : public SkDrawable {
        public:
            DrawableMatrixWrapper(sk_sp<SkDrawable> drawable, const SkMatrix& m)
                : fDrawable(std::move(drawable))
                , fMatrix(m)
            {}

            SkRect onGetBounds() override {
                return fMatrix.mapRect(fDrawable->getBounds());
            }

            size_t onApproximateBytesUsed() override {
                return fDrawable->approximateBytesUsed() + sizeof(DrawableMatrixWrapper);
            }

            void onDraw(SkCanvas* canvas) override {
                if constexpr (kSkShowTextBlitCoverage) {
                    SkPaint paint;
                    paint.setColor(0x3300FF00);
                    paint.setStyle(SkPaint::kFill_Style);
                    canvas->drawRect(this->onGetBounds(), paint);
                }
                canvas->drawDrawable(fDrawable.get(), &fMatrix);
            }
        private:
            const sk_sp<SkDrawable> fDrawable;
            const SkMatrix          fMatrix;
        };

        const auto& rec = this->userTF()->fGlyphRecs[glyph.getGlyphID()];

        return rec.fDrawable
            ? sk_make_sp<DrawableMatrixWrapper>(rec.fDrawable, fMatrix)
            : nullptr;
    }

    void generateFontMetrics(SkFontMetrics* metrics) override {
        auto [sx, sy] = fMatrix.mapPoint({1, 1});
        *metrics = scale_fontmetrics(this->userTF()->fMetrics, sx, sy);
    }

private:
    SkMatrix fMatrix;
};

std::unique_ptr<SkScalerContext> SkUserTypeface::onCreateScalerContext(
    const SkScalerContextEffects& effects, const SkDescriptor* desc) const
{
    return std::make_unique<SkUserScalerContext>(*const_cast<SkUserTypeface*>(this), effects, desc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static constexpr int kMaxGlyphCount = 65536;
static constexpr size_t kHeaderSize = 16;
static const char gHeaderString[] = "SkUserTypeface01";
static_assert(sizeof(gHeaderString) == 1 + kHeaderSize, "need header to be 16 bytes");

enum GlyphType : uint32_t { kPath, kDrawable };

std::unique_ptr<SkStreamAsset> SkUserTypeface::onOpenStream(int* ttcIndex) const {
    SkDynamicMemoryWStream wstream;

    wstream.write(gHeaderString, kHeaderSize);

    wstream.write(&fMetrics, sizeof(fMetrics));

    SkFontStyle style = this->fontStyle();
    wstream.write(&style, sizeof(style));

    wstream.write32(this->glyphCount());

    for (const auto& rec : fGlyphRecs) {
        wstream.write32(rec.isDrawable() ? GlyphType::kDrawable : GlyphType::kPath);

        wstream.writeScalar(rec.fAdvance);

        wstream.write(&rec.fBounds, sizeof(rec.fBounds));

        auto data = rec.isDrawable()
                ? rec.fDrawable->serialize()
                : rec.fPath.serialize();

        const size_t sz = data->size();
        SkASSERT(SkIsAlign4(sz));
        wstream.write(&sz, sizeof(sz));
        wstream.write(data->data(), sz);
    }

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

    for (int i = 0; i < glyphCount; ++i) {
        uint32_t gtype;
        if (!stream->readU32(&gtype) ||
            (gtype != GlyphType::kDrawable && gtype != GlyphType::kPath)) {
            return nullptr;
        }

        float advance;
        if (!stream->readScalar(&advance)) {
            return nullptr;
        }

        SkRect bounds;
        if (stream->read(&bounds, sizeof(bounds)) != sizeof(bounds) || !bounds.isFinite()) {
            return nullptr;
        }

        // SkPath and SkDrawable cannot read from a stream, so we have to page them into ram
        size_t sz;
        if (stream->read(&sz, sizeof(sz)) != sizeof(sz)) {
            return nullptr;
        }

        // The amount of bytes in the stream must be at least as big as sz, otherwise
        // sz is invalid.
        if (StreamRemainingLengthIsBelow(stream, sz)) {
            return nullptr;
        }

        auto data = SkData::MakeUninitialized(sz);
        if (stream->read(data->writable_data(), sz) != sz) {
            return nullptr;
        }

        switch (gtype) {
        case GlyphType::kDrawable: {
            SkDeserialProcs procs;
            procs.fAllowSkSL = false;
            auto drawable = SkDrawable::Deserialize(data->data(), data->size(), &procs);
            if (!drawable) {
                return nullptr;
            }
            builder.setGlyph(i, advance, std::move(drawable), bounds);
        } break;
        case GlyphType::kPath: {
            SkPath path;
            if (path.readFromMemory(data->data(), data->size()) != data->size()) {
                return nullptr;
            }

            builder.setGlyph(i, advance, path);
        } break;
        default:
            return nullptr;
        }
    }

    arp.markDone();
    return builder.detach();
}

sk_sp<SkTypeface> SkCustomTypefaceBuilder::MakeFromStream(std::unique_ptr<SkStreamAsset> stream,
                                                          const SkFontArguments&) {
    return Deserialize(stream.get());
}
