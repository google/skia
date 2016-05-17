/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPathRendering.h"
#include "SkDescriptor.h"
#include "SkGlyph.h"
#include "SkMatrix.h"
#include "SkTypeface.h"
#include "GrPathRange.h"

const GrUserStencilSettings& GrPathRendering::GetStencilPassSettings(FillType fill) {
    switch (fill) {
        default:
            SkFAIL("Unexpected path fill.");
        case GrPathRendering::kWinding_FillType: {
            constexpr static GrUserStencilSettings kWindingStencilPass(
                GrUserStencilSettings::StaticInit<
                    0xffff,
                    GrUserStencilTest::kAlwaysIfInClip,
                    0xffff,
                    GrUserStencilOp::kIncWrap,
                    GrUserStencilOp::kIncWrap,
                    0xffff>()
            );
            return kWindingStencilPass;
        }
        case GrPathRendering::kEvenOdd_FillType: {
            constexpr static GrUserStencilSettings kEvenOddStencilPass(
                GrUserStencilSettings::StaticInit<
                    0xffff,
                    GrUserStencilTest::kAlwaysIfInClip,
                    0xffff,
                    GrUserStencilOp::kInvert,
                    GrUserStencilOp::kInvert,
                    0xffff>()
            );
            return kEvenOddStencilPass;
        }
    }
}

class GlyphGenerator : public GrPathRange::PathGenerator {
public:
    GlyphGenerator(const SkTypeface& typeface, const SkScalerContextEffects& effects,
                   const SkDescriptor& desc)
        : fScalerContext(typeface.createScalerContext(effects, &desc))
#ifdef SK_DEBUG
        , fDesc(desc.copy())
#endif
    {}

    virtual ~GlyphGenerator() {
#ifdef SK_DEBUG
        SkDescriptor::Free(fDesc);
#endif
    }

    int getNumPaths() override {
        return fScalerContext->getGlyphCount();
    }

    void generatePath(int glyphID, SkPath* out) override {
        SkGlyph skGlyph;
        skGlyph.initWithGlyphID(glyphID);
        fScalerContext->getMetrics(&skGlyph);

        fScalerContext->getPath(skGlyph, out);
    }
#ifdef SK_DEBUG
    bool isEqualTo(const SkDescriptor& desc) const override { return *fDesc == desc; }
#endif
private:
    const SkAutoTDelete<SkScalerContext> fScalerContext;
#ifdef SK_DEBUG
    SkDescriptor* const fDesc;
#endif
};

GrPathRange* GrPathRendering::createGlyphs(const SkTypeface* typeface,
                                           const SkScalerContextEffects& effects,
                                           const SkDescriptor* desc,
                                           const GrStyle& style) {
    if (nullptr == typeface) {
        typeface = SkTypeface::GetDefaultTypeface();
        SkASSERT(nullptr != typeface);
    }

    if (desc) {
        SkAutoTUnref<GlyphGenerator> generator(new GlyphGenerator(*typeface, effects, *desc));
        return this->createPathRange(generator, style);
    }

    SkScalerContextRec rec;
    memset(&rec, 0, sizeof(rec));
    rec.fFontID = typeface->uniqueID();
    rec.fTextSize = SkPaint::kCanonicalTextSizeForPaths;
    rec.fPreScaleX = rec.fPost2x2[0][0] = rec.fPost2x2[1][1] = SK_Scalar1;
    // Don't bake stroke information into the glyphs, we'll let the GPU do the stroking.

    SkAutoDescriptor ad(sizeof(rec) + SkDescriptor::ComputeOverhead(1));
    SkDescriptor*    genericDesc = ad.getDesc();

    genericDesc->init();
    genericDesc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);
    genericDesc->computeChecksum();
    
    // No effects, so we make a dummy struct
    SkScalerContextEffects noEffects;

    SkAutoTUnref<GlyphGenerator> generator(new GlyphGenerator(*typeface, noEffects, *genericDesc));
    return this->createPathRange(generator, style);
}
