/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCoverageSetOpXP_DEFINED
#define GrCoverageSetOpXP_DEFINED

#include "include/core/SkRegion.h"
#include "include/gpu/GrTypes.h"
#include "src/gpu/GrXferProcessor.h"

// See the comment above GrXPFactory's definition about this warning suppression.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

/**
 * This xfer processor directly blends the the src coverage with the dst using a set operator. It is
 * useful for rendering coverage masks using CSG. It can optionally invert the src coverage before
 * applying the set operator.
 */
class GrCoverageSetOpXPFactory : public GrXPFactory {
public:
    static const GrXPFactory* Get(SkRegion::Op regionOp, bool invertCoverage = false);

private:
    constexpr GrCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage);

    sk_sp<const GrXferProcessor> makeXferProcessor(const GrProcessorAnalysisColor&,
                                                   GrProcessorAnalysisCoverage,
                                                   bool hasMixedSamples,
                                                   const GrCaps&,
                                                   GrClampType) const override;

    AnalysisProperties analysisProperties(const GrProcessorAnalysisColor& color,
                                          const GrProcessorAnalysisCoverage& coverage,
                                          bool hasMixedSamples,
                                          const GrCaps&,
                                          GrClampType) const override {
        auto props = AnalysisProperties::kIgnoresInputColor;
        switch (fRegionOp) {
            case SkRegion::kReplace_Op:
                props |= AnalysisProperties::kUnaffectedByDstValue;
                break;
            case SkRegion::kUnion_Op:
            case SkRegion::kDifference_Op:
                // FIXME: If we can formalize the fact that this op only operates on alpha, we can
                // set AnalysisProperties::kUnaffectedByDstValue if color/coverage/hasMixedSamples
                // are all opaque.
                break;
            case SkRegion::kIntersect_Op:
            case SkRegion::kXOR_Op:
            case SkRegion::kReverseDifference_Op:
                break;
        }
        return props;
    }


    GR_DECLARE_XP_FACTORY_TEST

    SkRegion::Op fRegionOp;
    bool         fInvertCoverage;

    using INHERITED = GrXPFactory;
};
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif
