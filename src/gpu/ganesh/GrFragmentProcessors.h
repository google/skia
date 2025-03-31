/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFragmentProcessors_DEFINED
#define GrFragmentProcessors_DEFINED

#include "include/effects/SkRuntimeEffect.h"

#include <tuple>
#include <memory>

class GrColorInfo;
class GrFragmentProcessor;
class SkBlenderBase;
class SkColorFilter;
class SkMaskFilter;
class SkMatrix;
class SkSurfaceProps;
class SkShader;
struct GrFPArgs;
namespace skgpu::ganesh { class SurfaceDrawContext; }

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
GrFPResult Make(skgpu::ganesh::SurfaceDrawContext*,
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

/**
 * Returns a GrFragmentProcessor for the passed-in runtime effect child. The processor will be
 * created with generic/null inputs, since the runtime effect is responsible for filling in the
 * arguments to the function.
 */
GrFPResult MakeChildFP(const SkRuntimeEffect::ChildPtr& child, const GrFPArgs& childArgs);

}  // namespace GrFragmentProcessors

#endif
