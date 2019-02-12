/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GLatticeOp_DEFINED
#define GLatticeOp_DEFINED

#include <memory>
#include "GrSamplerState.h"
#include "SkRefCnt.h"

class GrColorSpaceXform;
class GrDrawOp;
class GrPaint;
class SkLatticeIter;
class GrRecordingContext;
class GrTextureProxy;
class SkMatrix;
struct SkRect;

namespace GrLatticeOp {
std::unique_ptr<GrDrawOp> MakeNonAA(GrRecordingContext*,
                                    GrPaint&&,
                                    const SkMatrix& viewMatrix,
                                    sk_sp<GrTextureProxy>,
                                    sk_sp<GrColorSpaceXform>,
                                    GrSamplerState::Filter,
                                    std::unique_ptr<SkLatticeIter>,
                                    const SkRect& dst);
};

#endif
