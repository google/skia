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

class GrFragmentProcessor;
class SkBlenderBase;
class SkData;
class SkMaskFilter;
class SkMatrix;
struct GrFPArgs;

using GrFPResult = std::tuple<bool, std::unique_ptr<GrFragmentProcessor>>;

namespace GrFragmentProcessors {
std::unique_ptr<GrFragmentProcessor> Make(const SkMaskFilter*,
                                          const GrFPArgs&,
                                          const SkMatrix& ctm);

/**
 * Returns a GrFragmentProcessor that implements this blend for the Ganesh GPU backend.
 * The GrFragmentProcessor expects premultiplied inputs and returns a premultiplied output.
 */
std::unique_ptr<GrFragmentProcessor> Make(const SkBlenderBase*,
                                          std::unique_ptr<GrFragmentProcessor> srcFP,
                                          std::unique_ptr<GrFragmentProcessor> dstFP,
                                          const GrFPArgs& fpArgs);

bool IsSupported(const SkMaskFilter*);

// TODO(kjlubick, brianosman) remove this after all related effects have been migrated
GrFPResult make_effect_fp(sk_sp<SkRuntimeEffect> effect,
                          const char* name,
                          sk_sp<const SkData> uniforms,
                          std::unique_ptr<GrFragmentProcessor> inputFP,
                          std::unique_ptr<GrFragmentProcessor> destColorFP,
                          SkSpan<const SkRuntimeEffect::ChildPtr> children,
                          const GrFPArgs& childArgs);
}

#endif
