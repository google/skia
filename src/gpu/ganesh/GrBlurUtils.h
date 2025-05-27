/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBlurUtils_DEFINED
#define GrBlurUtils_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "src/gpu/SkBackingFit.h"

#include <memory>

class GrClip;
class GrFragmentProcessor;
class GrPaint;
class GrRecordingContext;
class GrStyledShape;
class GrSurfaceProxyView;
class SkColorSpace;
class SkMaskFilter;
class SkMatrix;
class SkPaint;
class SkRRect;
enum SkAlphaType : int;
enum class GrColorType;
enum class SkTileMode;
namespace skgpu { namespace ganesh { class SurfaceDrawContext; } }
struct GrShaderCaps;
struct SkIRect;
struct SkISize;
struct SkRect;

/**
 *  Blur utilities.
 */
namespace GrBlurUtils {

static constexpr int kBlurRRectMaxDivisions = 6;

/**
 * This method computes all the parameters for drawing a partially occluded nine-patched
 * blurred rrect mask:
 *   rrectToDraw - the integerized rrect to draw in the mask
 *   widthHeight - how large to make the mask (rrectToDraw will be centered in this coord system).
 *   rectXs, rectYs - the x & y coordinates of the covering geometry lattice
 *   texXs, texYs - the texture coordinate at each point in rectXs & rectYs
 * It returns true if 'devRRect' is nine-patchable
 */
bool ComputeBlurredRRectParams(const SkRRect& srcRRect,
                               const SkRRect& devRRect,
                               SkScalar sigma,
                               SkScalar xformedSigma,
                               SkRRect* rrectToDraw,
                               SkISize* widthHeight,
                               SkScalar rectXs[kBlurRRectMaxDivisions],
                               SkScalar rectYs[kBlurRRectMaxDivisions],
                               SkScalar texXs[kBlurRRectMaxDivisions],
                               SkScalar texYs[kBlurRRectMaxDivisions]);

/**
 * Draw a shape handling the mask filter if present.
 */
void DrawShapeWithMaskFilter(GrRecordingContext*,
                             skgpu::ganesh::SurfaceDrawContext*,
                             const GrClip*,
                             const SkPaint&,
                             const SkMatrix&,
                             const GrStyledShape&);

/**
 * Draw a shape handling the mask filter. The mask filter is not optional.
 * The GrPaint will be modified after return.
 */
void DrawShapeWithMaskFilter(GrRecordingContext*,
                             skgpu::ganesh::SurfaceDrawContext*,
                             const GrClip*,
                             const GrStyledShape&,
                             GrPaint&&,
                             const SkMatrix& viewMatrix,
                             const SkMaskFilter*);

/**
 * Applies a 2D Gaussian blur to a given texture. The blurred result is returned
 * as a surfaceDrawContext in case the caller wishes to draw into the result.
 * The GrSurfaceOrigin of the result will watch the GrSurfaceOrigin of srcView. The output
 * color type, color space, and alpha type will be the same as the src.
 *
 * Note: one of sigmaX and sigmaY should be non-zero!
 * @param context         The GPU context
 * @param srcView         The source to be blurred.
 * @param srcColorType    The colorType of srcProxy
 * @param srcAlphaType    The alphaType of srcProxy
 * @param srcColorSpace   Color space of the source.
 * @param dstBounds       The destination bounds, relative to the source texture.
 * @param srcBounds       The source bounds, relative to the source texture's offset. No pixels
 *                        will be sampled outside of this rectangle.
 * @param sigmaX          The blur's standard deviation in X.
 * @param sigmaY          The blur's standard deviation in Y.
 * @param tileMode        The mode to handle samples outside bounds.
 * @param fit             backing fit for the returned render target context
 * @return                The surfaceDrawContext containing the blurred result.
 */
std::unique_ptr<skgpu::ganesh::SurfaceDrawContext> GaussianBlur(
        GrRecordingContext*,
        GrSurfaceProxyView srcView,
        GrColorType srcColorType,
        SkAlphaType srcAlphaType,
        sk_sp<SkColorSpace> srcColorSpace,
        SkIRect dstBounds,
        SkIRect srcBounds,
        float sigmaX,
        float sigmaY,
        SkTileMode mode,
        SkBackingFit fit = SkBackingFit::kApprox);

std::unique_ptr<GrFragmentProcessor> MakeCircleBlur(GrRecordingContext* context,
                                                    const SkRect& circle,
                                                    float sigma);

std::unique_ptr<GrFragmentProcessor> MakeRectBlur(GrRecordingContext* context,
                                                  const GrShaderCaps& caps,
                                                  const SkRect& srcRect,
                                                  const SkMatrix& viewMatrix,
                                                  float transformedSigma);

std::unique_ptr<GrFragmentProcessor> MakeRRectBlur(GrRecordingContext* context,
                                                   float sigma,
                                                   float xformedSigma,
                                                   const SkRRect& srcRRect,
                                                   const SkRRect& devRRect);

}  // namespace GrBlurUtils

#endif
