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

    /** Because src-over is so common we special case it for performance reasons. If this returns
        null then the SimpleSrcOverXP() below should be used. */
    static GrXferProcessor* CreateSrcOverXferProcessor(const GrCaps& caps,
                                                       const FragmentProcessorAnalysis&,
                                                       bool hasMixedSamples,
                                                       const GrXferProcessor::DstTexture*);

    /** Returns a simple non-LCD porter duff blend XP with no optimizations or coverage. */
    static sk_sp<GrXferProcessor> CreateNoCoverageXP(SkBlendMode);

    /** This XP implements non-LCD src-over using hw blend with no optimizations. It is returned
        by reference because it is global and its ref-cnting methods are not thread safe. */
    static const GrXferProcessor& SimpleSrcOverXP();

    static bool WillSrcOverReadDst(const FragmentProcessorAnalysis& analysis);
    static bool IsSrcOverPreCoverageBlendedColorConstant(const GrProcOptInfo& colorInput,
                                                         GrColor* color);
    static bool WillSrcOverNeedDstTexture(const GrCaps&, const FragmentProcessorAnalysis&);

private:
    constexpr GrPorterDuffXPFactory(SkBlendMode);

    bool willReadsDst(const FragmentProcessorAnalysis&) const override;

    GrXferProcessor* onCreateXferProcessor(const GrCaps& caps,
                                           const FragmentProcessorAnalysis&,
                                           bool hasMixedSamples,
                                           const DstTexture*) const override;

    bool onWillReadDstInShader(const GrCaps&, const FragmentProcessorAnalysis&) const override;

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
