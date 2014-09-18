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
        : fDesc(desc.copy()),
          fScalerContext(typeface.createScalerContext(fDesc)) {
        fFlipMatrix.setScale(1, -1);
    }

    virtual ~GlyphGenerator() {
        SkDescriptor::Free(fDesc);
    }

    virtual int getNumPaths() {
        return fScalerContext->getGlyphCount();
    }

    virtual void generatePath(int glyphID, SkPath* out) {
        SkGlyph skGlyph;
        skGlyph.init(SkGlyph::MakeID(glyphID));
        fScalerContext->getMetrics(&skGlyph);

        fScalerContext->getPath(skGlyph, out);
        out->transform(fFlipMatrix); // Load glyphs with the inverted y-direction.
    }

    virtual bool isEqualTo(const SkDescriptor& desc) const {
        return fDesc->equals(desc);
    }

private:
    SkDescriptor* const fDesc;
    const SkAutoTDelete<SkScalerContext> fScalerContext;
    SkMatrix fFlipMatrix;
};

GrPathRange* GrPathRendering::createGlyphs(const SkTypeface* typeface,
                                           const SkDescriptor* desc,
                                           const SkStrokeRec& stroke) {
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
