/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GLatticeOp_DEFINED
#define GLatticeOp_DEFINED

#include <memory>
#include "include/core/SkRefCnt.h"
#include "src/gpu/GrSamplerState.h"

class GrColorSpaceXform;
class GrDrawOp;
class GrPaint;
class SkLatticeIter;
class GrRecordingContext;
class GrTextureProxy;
class SkMatrix;
struct SkRect;

namespace GrLatticeOp {
GrOp::Owner MakeNonAA(GrRecordingContext*,
                      GrPaint&&,
                      const SkMatrix& viewMatrix,
                      GrSurfaceProxyView view,
                      SkAlphaType alphaType,
                      sk_sp<GrColorSpaceXform>,
                      GrSamplerState::Filter,
                      std::unique_ptr<SkLatticeIter>,
                      const SkRect& dst);
}  // namespace GrLatticeOp

#endif
