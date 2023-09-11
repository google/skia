/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef LatticeOp_DEFINED
#define LatticeOp_DEFINED

#include <memory>
#include "include/core/SkRefCnt.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/ops/GrOp.h"

class GrColorSpaceXform;
class GrPaint;
class SkLatticeIter;
class GrRecordingContext;
class GrTextureProxy;
class SkMatrix;
struct SkRect;

namespace skgpu::ganesh::LatticeOp {

GrOp::Owner MakeNonAA(GrRecordingContext*,
                      GrPaint&&,
                      const SkMatrix& viewMatrix,
                      GrSurfaceProxyView view,
                      SkAlphaType alphaType,
                      sk_sp<GrColorSpaceXform>,
                      GrSamplerState::Filter,
                      std::unique_ptr<SkLatticeIter>,
                      const SkRect& dst);

}  // namespace skgpu::ganesh::LatticeOp

#endif // LatticeOp_DEFINED
