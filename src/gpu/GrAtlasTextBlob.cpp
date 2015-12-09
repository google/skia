/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlasTextBlob.h"

void GrAtlasTextBlob::appendGlyph(int runIndex,
                                  const SkRect& positions,
                                  GrColor color,
                                  GrBatchTextStrike* strike,
                                  GrGlyph* glyph) {
    Run& run = fRuns[runIndex];
    GrMaskFormat format = glyph->fMaskFormat;

    Run::SubRunInfo* subRun = &run.fSubRunInfo.back();
    if (run.fInitialized && subRun->maskFormat() != format) {
        subRun = &run.push_back();
        subRun->setStrike(strike);
    } else if (!run.fInitialized) {
        subRun->setStrike(strike);
    }

    run.fInitialized = true;

    size_t vertexStride = GetVertexStride(format);

    subRun->setMaskFormat(format);

    run.fVertexBounds.joinNonEmptyArg(positions);
    subRun->setColor(color);

    intptr_t vertex = reinterpret_cast<intptr_t>(this->fVertices + subRun->vertexEndIndex());

    if (kARGB_GrMaskFormat != glyph->fMaskFormat) {
        // V0
        SkPoint* position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fLeft, positions.fTop);
        SkColor* colorPtr = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *colorPtr = color;
        vertex += vertexStride;

        // V1
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fLeft, positions.fBottom);
        colorPtr = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *colorPtr = color;
        vertex += vertexStride;

        // V2
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fRight, positions.fBottom);
        colorPtr = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *colorPtr = color;
        vertex += vertexStride;

        // V3
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fRight, positions.fTop);
        colorPtr = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *colorPtr = color;
    } else {
        // V0
        SkPoint* position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fLeft, positions.fTop);
        vertex += vertexStride;

        // V1
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fLeft, positions.fBottom);
        vertex += vertexStride;

        // V2
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fRight, positions.fBottom);
        vertex += vertexStride;

        // V3
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fRight, positions.fTop);
    }
    subRun->appendVertices(vertexStride);
    fGlyphs[subRun->glyphEndIndex()] = glyph;
    subRun->glyphAppended();
}

// TODO get this code building again
#ifdef CACHE_SANITY_CHECK
void GrAtlasTextBlob::AssertEqual(const GrAtlasTextBlob& l, const GrAtlasTextBlob& r) {
    SkASSERT(l.fSize == r.fSize);
    SkASSERT(l.fPool == r.fPool);

    SkASSERT(l.fBlurRec.fSigma == r.fBlurRec.fSigma);
    SkASSERT(l.fBlurRec.fStyle == r.fBlurRec.fStyle);
    SkASSERT(l.fBlurRec.fQuality == r.fBlurRec.fQuality);

    SkASSERT(l.fStrokeInfo.fFrameWidth == r.fStrokeInfo.fFrameWidth);
    SkASSERT(l.fStrokeInfo.fMiterLimit == r.fStrokeInfo.fMiterLimit);
    SkASSERT(l.fStrokeInfo.fJoin == r.fStrokeInfo.fJoin);

    SkASSERT(l.fBigGlyphs.count() == r.fBigGlyphs.count());
    for (int i = 0; i < l.fBigGlyphs.count(); i++) {
        const BigGlyph& lBigGlyph = l.fBigGlyphs[i];
        const BigGlyph& rBigGlyph = r.fBigGlyphs[i];

        SkASSERT(lBigGlyph.fPath == rBigGlyph.fPath);
        // We can't assert that these have the same translations
    }

    SkASSERT(l.fKey == r.fKey);
    SkASSERT(l.fViewMatrix.cheapEqualTo(r.fViewMatrix));
    SkASSERT(l.fPaintColor == r.fPaintColor);
    SkASSERT(l.fMaxMinScale == r.fMaxMinScale);
    SkASSERT(l.fMinMaxScale == r.fMinMaxScale);
    SkASSERT(l.fTextType == r.fTextType);

    SkASSERT(l.fRunCount == r.fRunCount);
    for (int i = 0; i < l.fRunCount; i++) {
        const Run& lRun = l.fRuns[i];
        const Run& rRun = r.fRuns[i];

        if (lRun.fStrike.get()) {
            SkASSERT(rRun.fStrike.get());
            SkASSERT(GrBatchTextStrike::GetKey(*lRun.fStrike) ==
                     GrBatchTextStrike::GetKey(*rRun.fStrike));

        } else {
            SkASSERT(!rRun.fStrike.get());
        }

        if (lRun.fTypeface.get()) {
            SkASSERT(rRun.fTypeface.get());
            SkASSERT(SkTypeface::Equal(lRun.fTypeface, rRun.fTypeface));
        } else {
            SkASSERT(!rRun.fTypeface.get());
        }

        // We offset bounds right before flush time so they will not be correct here
        //SkASSERT(lRun.fVertexBounds == rRun.fVertexBounds);

        SkASSERT(lRun.fDescriptor.getDesc());
        SkASSERT(rRun.fDescriptor.getDesc());
        SkASSERT(lRun.fDescriptor.getDesc()->equals(*rRun.fDescriptor.getDesc()));

        if (lRun.fOverrideDescriptor.get()) {
            SkASSERT(lRun.fOverrideDescriptor->getDesc());
            SkASSERT(rRun.fOverrideDescriptor.get() && rRun.fOverrideDescriptor->getDesc());;
            SkASSERT(lRun.fOverrideDescriptor->getDesc()->equals(
                    *rRun.fOverrideDescriptor->getDesc()));
        } else {
            SkASSERT(!rRun.fOverrideDescriptor.get());
        }

        // color can be changed
        //SkASSERT(lRun.fColor == rRun.fColor);
        SkASSERT(lRun.fInitialized == rRun.fInitialized);
        SkASSERT(lRun.fDrawAsPaths == rRun.fDrawAsPaths);

        SkASSERT(lRun.fSubRunInfo.count() == rRun.fSubRunInfo.count());
        for(int j = 0; j < lRun.fSubRunInfo.count(); j++) {
            const Run::SubRunInfo& lSubRun = lRun.fSubRunInfo[j];
            const Run::SubRunInfo& rSubRun = rRun.fSubRunInfo[j];

            SkASSERT(lSubRun.fVertexStartIndex == rSubRun.fVertexStartIndex);
            SkASSERT(lSubRun.fVertexEndIndex == rSubRun.fVertexEndIndex);
            SkASSERT(lSubRun.fGlyphStartIndex == rSubRun.fGlyphStartIndex);
            SkASSERT(lSubRun.fGlyphEndIndex == rSubRun.fGlyphEndIndex);
            SkASSERT(lSubRun.fTextRatio == rSubRun.fTextRatio);
            SkASSERT(lSubRun.fMaskFormat == rSubRun.fMaskFormat);
            SkASSERT(lSubRun.fDrawAsDistanceFields == rSubRun.fDrawAsDistanceFields);
            SkASSERT(lSubRun.fUseLCDText == rSubRun.fUseLCDText);

            //We can't compare the bulk use tokens with this method
            /*
            SkASSERT(lSubRun.fBulkUseToken.fPlotsToUpdate.count() ==
                     rSubRun.fBulkUseToken.fPlotsToUpdate.count());
            SkASSERT(lSubRun.fBulkUseToken.fPlotAlreadyUpdated ==
                     rSubRun.fBulkUseToken.fPlotAlreadyUpdated);
            for (int k = 0; k < lSubRun.fBulkUseToken.fPlotsToUpdate.count(); k++) {
                SkASSERT(lSubRun.fBulkUseToken.fPlotsToUpdate[k] ==
                         rSubRun.fBulkUseToken.fPlotsToUpdate[k]);
            }*/
        }
    }
}

#endif
