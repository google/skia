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

#include <memory>
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

#define SK_ALL_BLENDERS(M) \
    M(BlendMode)           \
    M(Runtime)

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

    bool affectsTransparentBlack() const;

    [[nodiscard]] bool appendStages(const SkStageRec& rec) const {
        return this->onAppendStages(rec);
    }

    [[nodiscard]] virtual bool onAppendStages(const SkStageRec& rec) const = 0;

    virtual SkRuntimeEffect* asRuntimeEffect() const { return nullptr; }

    static SkFlattenable::Type GetFlattenableType() { return kSkBlender_Type; }
    SkFlattenable::Type getFlattenableType() const override { return GetFlattenableType(); }

    enum class BlenderType {
    #define M(type) k ## type,
        SK_ALL_BLENDERS(M)
    #undef M
    };

    virtual BlenderType type() const = 0;
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
