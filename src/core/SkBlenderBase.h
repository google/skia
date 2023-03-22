/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlenderBase_DEFINED
#define SkBlenderBase_DEFINED

#include "include/core/SkBlender.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkVM.h"

#include <optional>

struct GrFPArgs;
class GrFragmentProcessor;
class SkColorInfo;
class SkRuntimeEffect;
struct SkStageRec;

namespace skgpu::graphite {
enum class DstColorType;
class KeyContext;
class PaintParamsKeyBuilder;
class PipelineDataGatherer;
}

/**
 * Encapsulates a blend function, including non-public APIs.
 * Blends combine a source color (the result of our paint) and destination color (from the canvas)
 * into a final color.
 */
class SkBlenderBase : public SkBlender {
public:
    /**
     * Returns true if this SkBlender represents any SkBlendMode, and returns the blender's
     * SkBlendMode in `mode`. Returns false for other types of blends.
     */
    virtual std::optional<SkBlendMode> asBlendMode() const { return {}; }

    SK_WARN_UNUSED_RESULT bool appendStages(const SkStageRec& rec) const {
        return this->onAppendStages(rec);
    }

    SK_WARN_UNUSED_RESULT
    virtual bool onAppendStages(const SkStageRec& rec) const = 0;

    /** Creates the blend program in SkVM. */
    SK_WARN_UNUSED_RESULT
    skvm::Color program(skvm::Builder* p, skvm::Color src, skvm::Color dst,
                        const SkColorInfo& colorInfo, skvm::Uniforms* uniforms,
                        SkArenaAlloc* alloc) const {
        return this->onProgram(p, src, dst, colorInfo, uniforms, alloc);
    }

#if defined(SK_GANESH)
    /**
     * Returns a GrFragmentProcessor that implements this blend for the GPU backend.
     * The GrFragmentProcessor expects premultiplied inputs and returns a premultiplied output.
     */
    virtual std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            std::unique_ptr<GrFragmentProcessor> srcFP,
            std::unique_ptr<GrFragmentProcessor> dstFP,
            const GrFPArgs& fpArgs) const = 0;
#endif

    virtual SkRuntimeEffect* asRuntimeEffect() const { return nullptr; }

#if defined(SK_GRAPHITE)
    /**
     * TODO: Make pure virtual.
     * dstColorType = kPrimitive when blending the result of the paint evaluation with a primitive
     * color (which is supplied by certain geometries). dstColorType = kSurface when blending the
     * result of the paint evaluation with the back buffer.
     */
    virtual void addToKey(const skgpu::graphite::KeyContext&,
                          skgpu::graphite::PaintParamsKeyBuilder*,
                          skgpu::graphite::PipelineDataGatherer*,
                          skgpu::graphite::DstColorType dstColorType) const;
#endif

    static SkFlattenable::Type GetFlattenableType() { return kSkBlender_Type; }
    Type getFlattenableType() const override { return GetFlattenableType(); }

private:
    virtual skvm::Color onProgram(skvm::Builder* p, skvm::Color src, skvm::Color dst,
                                  const SkColorInfo& colorInfo, skvm::Uniforms* uniforms,
                                  SkArenaAlloc* alloc) const = 0;

    using INHERITED = SkFlattenable;
};

inline SkBlenderBase* as_BB(SkBlender* blend) {
    return static_cast<SkBlenderBase*>(blend);
}

inline const SkBlenderBase* as_BB(const SkBlender* blend) {
    return static_cast<const SkBlenderBase*>(blend);
}

inline const SkBlenderBase* as_BB(const sk_sp<SkBlender>& blend) {
    return static_cast<SkBlenderBase*>(blend.get());
}

#endif  // SkBlenderBase_DEFINED
