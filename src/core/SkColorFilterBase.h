/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilterBase_DEFINED
#define SkColorFilterBase_DEFINED

#include "include/core/SkColorFilter.h"
#include "include/private/SkColorData.h"
#include "src/core/SkVM_fwd.h"

#include <memory>
#include <tuple>

class GrColorInfo;
class GrFragmentProcessor;
class GrRecordingContext;
class SkArenaAlloc;
class SkBitmap;
class SkColorInfo;
class SkColorSpace;
class SkRuntimeEffect;
class SkSurfaceProps;
struct SkStageRec;
using GrFPResult = std::tuple<bool, std::unique_ptr<GrFragmentProcessor>>;

namespace skgpu::graphite {
class KeyContext;
class PaintParamsKeyBuilder;
class PipelineDataGatherer;
}

class SkColorFilterBase : public SkColorFilter {
public:
    SK_WARN_UNUSED_RESULT
    virtual bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const = 0;

    SK_WARN_UNUSED_RESULT
    skvm::Color program(skvm::Builder*, skvm::Color,
                        const SkColorInfo& dst, skvm::Uniforms*, SkArenaAlloc*) const;

    /** Returns the flags for this filter. Override in subclasses to return custom flags.
    */
    virtual bool onIsAlphaUnchanged() const { return false; }

#if defined(SK_GANESH)
    /**
     *  A subclass may implement this factory function to work with the GPU backend. It returns
     *  a GrFragmentProcessor that implements the color filter in GPU shader code.
     *
     *  The fragment processor receives a input FP that generates a premultiplied input color, and
     *  produces a premultiplied output color.
     *
     *  A GrFPFailure indicates that the color filter isn't implemented for the GPU backend.
     */
    virtual GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                           GrRecordingContext* context,
                                           const GrColorInfo& dstColorInfo,
                                           const SkSurfaceProps& props) const;
#endif

    bool affectsTransparentBlack() const {
        return this->filterColor(SK_ColorTRANSPARENT) != SK_ColorTRANSPARENT;
    }

    virtual SkRuntimeEffect* asRuntimeEffect() const { return nullptr; }

    static SkFlattenable::Type GetFlattenableType() {
        return kSkColorFilter_Type;
    }

    SkFlattenable::Type getFlattenableType() const override {
        return kSkColorFilter_Type;
    }

    static sk_sp<SkColorFilter> Deserialize(const void* data, size_t size,
                                          const SkDeserialProcs* procs = nullptr) {
        return sk_sp<SkColorFilter>(static_cast<SkColorFilter*>(
                                  SkFlattenable::Deserialize(
                                  kSkColorFilter_Type, data, size, procs).release()));
    }

    virtual SkPMColor4f onFilterColor4f(const SkPMColor4f& color, SkColorSpace* dstCS) const;

#if defined(SK_GRAPHITE)
    /**
        Add implementation details, for the specified backend, of this SkColorFilter to the
        provided key.

        @param keyContext backend context for key creation
        @param builder    builder for creating the key for this SkShader
        @param gatherer   if non-null, storage for this colorFilter's data
    */
    virtual void addToKey(const skgpu::graphite::KeyContext& keyContext,
                          skgpu::graphite::PaintParamsKeyBuilder* builder,
                          skgpu::graphite::PipelineDataGatherer* gatherer) const;
#endif

protected:
    SkColorFilterBase() {}

    virtual bool onAsAColorMatrix(float[20]) const;
    virtual bool onAsAColorMode(SkColor* color, SkBlendMode* bmode) const;

private:
    virtual skvm::Color onProgram(skvm::Builder*, skvm::Color,
                                  const SkColorInfo& dst, skvm::Uniforms*, SkArenaAlloc*) const = 0;

    friend class SkColorFilter;

    using INHERITED = SkFlattenable;
};

static inline SkColorFilterBase* as_CFB(SkColorFilter* filter) {
    return static_cast<SkColorFilterBase*>(filter);
}

static inline const SkColorFilterBase* as_CFB(const SkColorFilter* filter) {
    return static_cast<const SkColorFilterBase*>(filter);
}

static inline const SkColorFilterBase* as_CFB(const sk_sp<SkColorFilter>& filter) {
    return static_cast<SkColorFilterBase*>(filter.get());
}

static inline sk_sp<SkColorFilterBase> as_CFB_sp(sk_sp<SkColorFilter> filter) {
    return sk_sp<SkColorFilterBase>(static_cast<SkColorFilterBase*>(filter.release()));
}


void SkRegisterComposeColorFilterFlattenable();
void SkRegisterMatrixColorFilterFlattenable();
void SkRegisterModeColorFilterFlattenable();
void SkRegisterColorSpaceXformColorFilterFlattenable();
void SkRegisterTableColorFilterFlattenable();
void SkRegisterWorkingFormatColorFilterFlattenable();

#endif
