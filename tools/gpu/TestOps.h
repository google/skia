/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestRectOp_DEFINED
#define TestRectOp_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/ops/GrMeshDrawOp.h"

class GrPaint;

namespace sk_gpu_test::test_ops {

/**
 * Fully specified device space rect op. The test Op draws a rectangle with local coords and a
 * local matrix. It is important to test effects in the presence of GP local matrices. Our standard
 * rect drawing code doesn't exercise this because it applies any local matrix to pre-transformed
 * local coord vertex attributes.
 */
GrOp::Owner MakeRect(GrRecordingContext*,
                     GrPaint&&,
                     const SkRect& drawRect,
                     const SkRect& localRect,
                     const SkMatrix& localM = SkMatrix::I());

/**
 * A simpler version of MakeRect that takes a single color FP instead of a full paint. Uses
 * SkBlendMode::kSrcOver.
 */
GrOp::Owner MakeRect(GrRecordingContext*,
                     std::unique_ptr<GrFragmentProcessor>,
                     const SkRect& drawRect,
                     const SkRect& localRect,
                     const SkMatrix& localM = SkMatrix::I());

/**
 * A simpler version of MakeRect that uses the same rect as the device space rect to draw as well as
 * the local rect. The local matrix is identity.
 */
GrOp::Owner MakeRect(GrRecordingContext*, GrPaint&&, const SkRect& rect);

}  // namespace sk_gpu_test::test_ops

#endif
