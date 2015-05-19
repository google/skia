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

class GlyphGenerator : public GrPathRange::PathGenerator {
public:
    GlyphGenerator(const SkTypeface& typeface, const SkDescriptor& desc)
        : fScalerContext(typeface.createScalerContext(&desc))
#ifdef SK_DEBUG
        , fDesc(desc.copy())
#endif
    {
        fFlipMatrix.setScale(1, -1);
    }

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
        out->transform(fFlipMatrix); // Load glyphs with the inverted y-direction.
    }
#ifdef SK_DEBUG
    bool isEqualTo(const SkDescriptor& desc) const override {
        return fDesc->equals(desc);
    }
#endif
private:
    const SkAutoTDelete<SkScalerContext> fScalerContext;
    SkMatrix fFlipMatrix;
#ifdef SK_DEBUG
    SkDescriptor* const fDesc;
#endif
};

GrPathRange* GrPathRendering::createGlyphs(const SkTypeface* typeface,
                                           const SkDescriptor* desc,
                                           const GrStrokeInfo& stroke) {
    if (NULL == typeface) {
        typeface = SkTypeface::GetDefaultTypeface();
        SkASSERT(NULL != typeface);
    }

    if (desc) {
        SkAutoTUnref<GlyphGenerator> generator(SkNEW_ARGS(GlyphGenerator, (*typeface, *desc)));
        return this->createPathRange(generator, stroke);
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

    SkAutoTUnref<GlyphGenerator> generator(SkNEW_ARGS(GlyphGenerator, (*typeface, *genericDesc)));
    return this->createPathRange(generator, stroke);
}
