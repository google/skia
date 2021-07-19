/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlenderBase_DEFINED
#define SkBlenderBase_DEFINED

#include "include/core/SkBlender.h"
#include "include/core/SkColorSpace.h"
#include "include/private/SkTOptional.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkVM.h"

struct GrFPArgs;
class GrFragmentProcessor;
class SkRuntimeEffect;

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
    virtual skstd::optional<SkBlendMode> asBlendMode() const { return {}; }

    /** Creates the blend program in SkVM. */
    SK_WARN_UNUSED_RESULT
    skvm::Color program(skvm::Builder* p, skvm::Color src, skvm::Color dst,
                        const SkColorInfo& colorInfo, skvm::Uniforms* uniforms,
                        SkArenaAlloc* alloc) const {
        return this->onProgram(p, src, dst, colorInfo, uniforms, alloc);
    }

#if SK_SUPPORT_GPU
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
