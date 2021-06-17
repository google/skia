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
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkVM.h"

class SkRuntimeEffect;

/**
 * Encapsulates a custom blend function for Runtime Effects. These combine a source color (the
 * result of our paint) and destination color (from the canvas) into a final color.
 */
class SkBlenderBase : public SkBlender {
public:
    SK_WARN_UNUSED_RESULT
    skvm::Color program(skvm::Builder* p, skvm::Color src, skvm::Color dst,
                        const SkColorInfo& colorInfo, skvm::Uniforms* uniforms,
                        SkArenaAlloc* alloc) const {
        return this->onProgram(p, src, dst, colorInfo, uniforms, alloc);
    }

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
