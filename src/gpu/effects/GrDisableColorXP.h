/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDisableColorXP_DEFINED
#define GrDisableColorXP_DEFINED

#include "GrTypes.h"
#include "GrXferProcessor.h"
#include "SkRefCnt.h"

// See the comment above GrXPFactory's definition about this warning suppression.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
class GrDisableColorXPFactory : public GrXPFactory {
public:
    static const GrDisableColorXPFactory* Get() {
        static const GrDisableColorXPFactory gDisableColorXPFactory;
        return &gDisableColorXPFactory;
    }

    sk_sp<const GrXferProcessor> makeXferProcessor() const { return fXferProcessor; }

private:
    GrDisableColorXPFactory();

    AnalysisProperties analysisProperties(
            const GrProcessorAnalysisColor&, const GrProcessorAnalysisCoverage&, const GrCaps&,
            GrClampType) const override {
        return AnalysisProperties::kCompatibleWithCoverageAsAlpha |
               AnalysisProperties::kIgnoresInputColor;
    }

    sk_sp<const GrXferProcessor> makeXferProcessor(
            const GrProcessorAnalysisColor&, GrProcessorAnalysisCoverage, bool hasMixedSamples,
            const GrCaps&, GrClampType) const override {
        return this->makeXferProcessor();
    }

    const sk_sp<const GrXferProcessor> fXferProcessor;

    GR_DECLARE_XP_FACTORY_TEST

    typedef GrXPFactory INHERITED;
};
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif
