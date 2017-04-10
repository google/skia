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
    static sk_sp<const GrXferProcessor> MakeSrcOverXferProcessor(const GrProcessorAnalysisColor&,
                                                                 GrProcessorAnalysisCoverage,
                                                                 bool hasMixedSamples,
                                                                 const GrCaps&);

    /** Returns a simple non-LCD porter duff blend XP with no optimizations or coverage. */
    static sk_sp<const GrXferProcessor> MakeNoCoverageXP(SkBlendMode);

    /** This XP implements non-LCD src-over using hw blend with no optimizations. It is returned
        by reference because it is global and its ref-cnting methods are not thread safe. */
    static const GrXferProcessor& SimpleSrcOverXP();

    static AnalysisProperties SrcOverAnalysisProperties(const GrProcessorAnalysisColor&,
                                                        const GrProcessorAnalysisCoverage&,
                                                        const GrCaps&);

private:
    constexpr GrPorterDuffXPFactory(SkBlendMode);

    sk_sp<const GrXferProcessor> makeXferProcessor(const GrProcessorAnalysisColor&,
                                                   GrProcessorAnalysisCoverage,
                                                   bool hasMixedSamples,
                                                   const GrCaps&) const override;

    AnalysisProperties analysisProperties(const GrProcessorAnalysisColor&,
                                          const GrProcessorAnalysisCoverage&,
                                          const GrCaps&) const override;

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
