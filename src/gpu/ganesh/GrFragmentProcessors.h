/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFragmentProcessors_DEFINED
#define GrFragmentProcessors_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/effects/SkRuntimeEffect.h"

#include <tuple>
#include <memory>

class GrColorInfo;
class GrFragmentProcessor;
class GrRecordingContext;
class SkBlenderBase;
class SkColorFilter;
class SkData;
class SkMaskFilter;
class SkMatrix;
class SkSurfaceProps;
class SkShader;
struct GrFPArgs;

using GrFPResult = std::tuple<bool, std::unique_ptr<GrFragmentProcessor>>;

namespace SkShaders {
class MatrixRec;
}

namespace GrFragmentProcessors {
/**
 * Returns a GrFragmentProcessor that implements this blend for the Ganesh GPU backend.
 * The GrFragmentProcessor expects premultiplied inputs and returns a premultiplied output.
 */
std::unique_ptr<GrFragmentProcessor> Make(const SkBlenderBase*,
                                          std::unique_ptr<GrFragmentProcessor> srcFP,
                                          std::unique_ptr<GrFragmentProcessor> dstFP,
                                          const GrFPArgs& fpArgs);

/**
 *  Returns a GrFragmentProcessor that implements the color filter in GPU shader code.
 *
 *  The fragment processor receives a input FP that generates a premultiplied input color, and
 *  produces a premultiplied output color.
 *
 *  A GrFPFailure indicates that the color filter isn't implemented for the GPU backend.
 */
GrFPResult Make(GrRecordingContext*,
                const SkColorFilter*,
                std::unique_ptr<GrFragmentProcessor> inputFP,
                const GrColorInfo& dstColorInfo,
                const SkSurfaceProps&);

std::unique_ptr<GrFragmentProcessor> Make(const SkMaskFilter*,
                                          const GrFPArgs&,
                                          const SkMatrix& ctm);

bool IsSupported(const SkMaskFilter*);

/**
 * Call on the root SkShader to produce a GrFragmentProcessor.
 *
 * The returned GrFragmentProcessor expects an unpremultiplied input color and produces a
 * premultiplied output.
 */
std::unique_ptr<GrFragmentProcessor> Make(const SkShader*, const GrFPArgs&, const SkMatrix& ctm);
std::unique_ptr<GrFragmentProcessor> Make(const SkShader*,
                                          const GrFPArgs&,
                                          const SkShaders::MatrixRec&);

// TODO(kjlubick, brianosman) remove this after all related effects have been migrated
GrFPResult make_effect_fp(sk_sp<SkRuntimeEffect> effect,
                          const char* name,
                          sk_sp<const SkData> uniforms,
                          std::unique_ptr<GrFragmentProcessor> inputFP,
                          std::unique_ptr<GrFragmentProcessor> destColorFP,
                          SkSpan<const SkRuntimeEffect::ChildPtr> children,
                          const GrFPArgs& childArgs);
}  // namespace GrFragmentProcessors

#endif
