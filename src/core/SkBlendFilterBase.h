/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlendFilterBase_DEFINED
#define SkBlendFilterBase_DEFINED

#include "include/core/SkBlendFilter.h"
#include "include/core/SkColorSpace.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkVM.h"

/**
 * Encapsulates a custom blend function for Runtime Effects. These combine a source color (the
 * result of our paint) and destination color (from the canvas) into a final color.
 */
class SkBlendFilterBase : public SkBlendFilter {
public:
    SK_WARN_UNUSED_RESULT
    skvm::Color program(skvm::Builder* p, skvm::Color src, skvm::Color dst,
                        const SkColorInfo& colorInfo, skvm::Uniforms* uniforms,
                        SkArenaAlloc* alloc) const {
        return this->onProgram(p, src, dst, colorInfo, uniforms, alloc);
    }

    static SkFlattenable::Type GetFlattenableType() { return kSkBlendFilter_Type; }
    Type getFlattenableType() const override { return GetFlattenableType(); }

private:
    virtual skvm::Color onProgram(skvm::Builder* p, skvm::Color src, skvm::Color dst,
                                  const SkColorInfo& colorInfo, skvm::Uniforms* uniforms,
                                  SkArenaAlloc* alloc) const = 0;

    using INHERITED = SkFlattenable;
};

inline SkBlendFilterBase* as_BFB(SkBlendFilter* blend) {
    return static_cast<SkBlendFilterBase*>(blend);
}

inline const SkBlendFilterBase* as_BFB(const SkBlendFilter* blend) {
    return static_cast<const SkBlendFilterBase*>(blend);
}

inline const SkBlendFilterBase* as_BFB(const sk_sp<SkBlendFilter>& blend) {
    return static_cast<SkBlendFilterBase*>(blend.get());
}

#endif  // SkBlendFilterBase_DEFINED
