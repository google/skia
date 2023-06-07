/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilterBase_DEFINED
#define SkColorFilterBase_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkAttributes.h"

#include <cstddef>

class SkColorSpace;
class SkRuntimeEffect;
enum class SkBlendMode;
struct SkDeserialProcs;
struct SkStageRec;

#if defined(SK_GRAPHITE)
namespace skgpu::graphite {
class KeyContext;
class PaintParamsKeyBuilder;
class PipelineDataGatherer;
}
#endif

#if defined(SK_ENABLE_SKVM)
#include "include/core/SkImageInfo.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkVM_fwd.h"
#endif

#define SK_ALL_COLOR_FILTERS(M) \
    M(BlendMode)                \
    M(ColorSpaceXform)          \
    M(Compose)                  \
    M(Gaussian)                 \
    M(Matrix)                   \
    M(Runtime)                  \
    M(Table)                    \
    M(WorkingFormat)

class SkColorFilterBase : public SkColorFilter {
public:
    SK_WARN_UNUSED_RESULT
    virtual bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const = 0;

#if defined(SK_ENABLE_SKVM)
    SK_WARN_UNUSED_RESULT
    skvm::Color program(skvm::Builder*, skvm::Color,
                        const SkColorInfo& dst, skvm::Uniforms*, SkArenaAlloc*) const;
#endif

    /** Returns the flags for this filter. Override in subclasses to return custom flags.
    */
    virtual bool onIsAlphaUnchanged() const { return false; }

    enum class Type {
        // Used for stubs/tests
        kNoop,
#define M(type) k##type,
        SK_ALL_COLOR_FILTERS(M)
#undef M

    };

    virtual Type type() const = 0;

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
#if defined(SK_ENABLE_SKVM)
    virtual skvm::Color onProgram(skvm::Builder*, skvm::Color,
                                  const SkColorInfo& dst, skvm::Uniforms*, SkArenaAlloc*) const = 0;
#endif

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
void SkRegisterSkColorSpaceXformColorFilterFlattenable();
void SkRegisterTableColorFilterFlattenable();
void SkRegisterWorkingFormatColorFilterFlattenable();

#endif
