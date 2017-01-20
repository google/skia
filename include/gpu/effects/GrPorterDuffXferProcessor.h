/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPorterDuffXferProcessor_DEFINED
#define GrPorterDuffXferProcessor_DEFINED

#include "GrTypes.h"
#include "GrXferProcessor.h"
#include "SkBlendMode.h"

class GrProcOptInfo;

// See the comment above GrXPFactory's definition about this warning suppression.
#if defined(__GNUC__) || defined(__clang)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif
class GrPorterDuffXPFactory : public GrXPFactory {
public:
    static const GrXPFactory* Get(SkBlendMode blendMode);

    void getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                  GrXPFactory::InvariantBlendedColor*) const override;


    /** Because src-over is so common we special case it for performance reasons. If this returns
        null then the SimpleSrcOverXP() below should be used. */
    static GrXferProcessor* CreateSrcOverXferProcessor(const GrCaps& caps,
                                                       const GrPipelineAnalysis&,
                                                       bool hasMixedSamples,
                                                       const GrXferProcessor::DstTexture*);
    /** This XP implements non-LCD src-over using hw blend with no optimizations. It is returned
        by reference because it is global and its ref-cnting methods are not thread safe. */
    static const GrXferProcessor& SimpleSrcOverXP();

    static inline void SrcOverInvariantBlendedColor(
                                                GrColor inputColor,
                                                GrColorComponentFlags validColorFlags,
                                                bool isOpaque,
                                                GrXPFactory::InvariantBlendedColor* blendedColor) {
        if (!isOpaque) {
            blendedColor->fWillBlendWithDst = true;
            blendedColor->fKnownColorFlags = kNone_GrColorComponentFlags;
            return;
        }
        blendedColor->fWillBlendWithDst = false;

        blendedColor->fKnownColor = inputColor;
        blendedColor->fKnownColorFlags = validColorFlags;
    }

    static bool SrcOverWillNeedDstTexture(const GrCaps&, const GrPipelineAnalysis&);

private:
    constexpr GrPorterDuffXPFactory(SkBlendMode);

    GrXferProcessor* onCreateXferProcessor(const GrCaps& caps,
                                           const GrPipelineAnalysis&,
                                           bool hasMixedSamples,
                                           const DstTexture*) const override;

    bool willReadDstColor(const GrCaps&, ColorType, CoverageType) const override;

    GR_DECLARE_XP_FACTORY_TEST;
    static void TestGetXPOutputTypes(const GrXferProcessor*, int* outPrimary, int* outSecondary);

    SkBlendMode fBlendMode;

    friend class GrPorterDuffTest; // for TestGetXPOutputTypes()
    typedef GrXPFactory INHERITED;
};
#if defined(__GNUC__) || defined(__clang)
#pragma GCC diagnostic pop
#endif

#endif
