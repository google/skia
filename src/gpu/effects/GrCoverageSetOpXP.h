/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCoverageSetOpXP_DEFINED
#define GrCoverageSetOpXP_DEFINED

#include "GrTypes.h"
#include "GrXferProcessor.h"
#include "SkRegion.h"

class GrProcOptInfo;

// See the comment above GrXPFactory's definition about this warning suppression.
#if defined(__GNUC__) || defined(__clang)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
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

    bool willReadsDst(const FragmentProcessorAnalysis&) const override {
        return fRegionOp != SkRegion::kReplace_Op;
    }

    GrXferProcessor* onCreateXferProcessor(const GrCaps&,
                                           const FragmentProcessorAnalysis&,
                                           bool hasMixedSamples,
                                           const DstTexture*) const override;

    bool onWillReadDstInShader(const GrCaps&, const FragmentProcessorAnalysis&) const override {
        return false;
    }

    GR_DECLARE_XP_FACTORY_TEST;

    SkRegion::Op fRegionOp;
    bool         fInvertCoverage;

    typedef GrXPFactory INHERITED;
};
#if defined(__GNUC__) || defined(__clang)
#pragma GCC diagnostic pop
#endif
#endif

