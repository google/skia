/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestRectOp_DEFINED
#define TestRectOp_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/ops/GrMeshDrawOp.h"

class GrPaint;

namespace sk_gpu_test::test_ops {

/** Fully specified device space rect op. */
std::unique_ptr<GrDrawOp> MakeRect(GrRecordingContext*,
                                   GrPaint&&,
                                   const SkRect& drawRect,
                                   const SkRect& localRect,
                                   const SkMatrix& localM = SkMatrix::I());

/** Takes a single color FP instead of a full paint. Uses SkBlendMode::kSrcOver. */
std::unique_ptr<GrDrawOp> MakeRect(GrRecordingContext*,
                                   std::unique_ptr<GrFragmentProcessor>,
                                   const SkRect& drawRect,
                                   const SkRect& localRect,
                                   const SkMatrix& localM = SkMatrix::I());

/**
 * Uses the rect as the device space rect to draw as well as the local rect. The local matrix
 * is identity.
 */
std::unique_ptr<GrDrawOp> MakeRect(GrRecordingContext*, GrPaint&&, const SkRect& rect);

}  // namespace sk_gpu_test::test_ops

#endif
