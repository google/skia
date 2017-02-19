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

namespace GrPorterDuffXPFactory {
// See the comment above GrXPFactory's definition about this warning suppression.
#if defined(__GNUC__) || defined(__clang)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif
class XPF : public GrXPFactory {
public:
    constexpr XPF(SkBlendMode blendMode) : fBlendMode(blendMode) {}
    ~XPF() = default;

    /** Returns a simple non-LCD porter duff blend XP with no optimizations or coverage. */
    static sk_sp<GrXferProcessor> CreateNoCoverageXP(SkBlendMode);

private:
    bool isPreCoverageBlendedColorConstant(const GrProcOptInfo&, GrColor*) const override;
    bool willReadsDst(const GrProcOptInfo&, const GrProcOptInfo&) const override;

    GrXferProcessor* onCreateXferProcessor(const GrCaps& caps,
                                           const GrPipelineAnalysis&,
                                           bool hasMixedSamples,
                                           const DstTexture*) const override;

    bool willReadDstInShader(const GrCaps&, ColorType, CoverageType) const override;

    GR_DECLARE_XP_FACTORY_TEST;

    SkBlendMode fBlendMode;
    typedef GrXPFactory INHERITED;
};

static constexpr const XPF gClear(SkBlendMode::kClear);
static constexpr const XPF gSrc(SkBlendMode::kSrc);
static constexpr const XPF gDst(SkBlendMode::kDst);
static constexpr const XPF gSrcOver(SkBlendMode::kSrcOver);
static constexpr const XPF gDstOver(SkBlendMode::kDstOver);
static constexpr const XPF gSrcIn(SkBlendMode::kSrcIn);
static constexpr const XPF gDstIn(SkBlendMode::kDstIn);
static constexpr const XPF gSrcOut(SkBlendMode::kSrcOut);
static constexpr const XPF gDstOut(SkBlendMode::kDstOut);
static constexpr const XPF gSrcATop(SkBlendMode::kSrcATop);
static constexpr const XPF gDstATop(SkBlendMode::kDstATop);
static constexpr const XPF gXor(SkBlendMode::kXor);
static constexpr const XPF gPlus(SkBlendMode::kPlus);
static constexpr const XPF gModulate(SkBlendMode::kModulate);
static constexpr const XPF gScreen(SkBlendMode::kScreen);

const GrXferProcessor& SimpleSrcOverXP();
bool SrcOverWillNeedDstTexture(const GrCaps& caps, const GrPipelineAnalysis& analysis);
bool WillSrcOverReadDst(const GrProcOptInfo& colorInput, const GrProcOptInfo& coverageInput);
bool IsSrcOverPreCoverageBlendedColorConstant(const GrProcOptInfo& colorInput, GrColor* color);
GrXferProcessor* CreateSrcOverXferProcessor(const GrCaps& caps, const GrPipelineAnalysis& analysis, bool hasMixedSamples, const GrXferProcessor::DstTexture* dstTexture);
sk_sp<GrXferProcessor> CreateNoCoverageXP(SkBlendMode blendmode);
void TestGetXPOutputTypes(const GrXferProcessor* xp, int* outPrimary, int* outSecondary);

constexpr const GrXPFactory* Get(SkBlendMode blendMode) {
    return blendMode == SkBlendMode::kSrcOver ? &gSrcOver :
           blendMode == SkBlendMode::kClear ? &gClear :
           blendMode == SkBlendMode::kSrc ? &gSrc :
           blendMode == SkBlendMode::kDst ? &gDst :
           blendMode == SkBlendMode::kDstOver ? &gDstOver :
           blendMode == SkBlendMode::kSrcIn ? &gSrcIn :
           blendMode == SkBlendMode::kDstIn ? &gDstIn :
           blendMode == SkBlendMode::kSrcOut ? &gSrcOut :
           blendMode == SkBlendMode::kDstOut ? &gDstOut :
           blendMode == SkBlendMode::kSrcATop ? &gSrcATop :
           blendMode == SkBlendMode::kDstATop ? &gDstATop :
           blendMode == SkBlendMode::kXor ? &gXor :
           blendMode == SkBlendMode::kPlus ? &gPlus :
           blendMode == SkBlendMode::kModulate ? &gModulate :
           blendMode == SkBlendMode::kScreen ? &gScreen : nullptr;
}
}


#if defined(__GNUC__) || defined(__clang)
#pragma GCC diagnostic pop
#endif

#endif
