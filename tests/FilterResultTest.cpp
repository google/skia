/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkDevice.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/EncodeUtils.h"
#include "tools/gpu/ContextType.h"

#include <cmath>
#include <initializer_list>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>


#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/SpecialImage_Graphite.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/TextureUtils.h"
#endif


#if defined(SK_GANESH)
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
struct GrContextOptions;
#endif

class SkShader;

using namespace skia_private;
using namespace skif;

// NOTE: Not in anonymous so that FilterResult can friend it
class FilterResultTestAccess {
public:
    static void Draw(const skif::Context& ctx,
                     SkDevice* device,
                     const skif::FilterResult& image,
                     bool preserveDeviceState) {
        image.draw(ctx, device, preserveDeviceState, /*blender=*/nullptr);
    }

    static sk_sp<SkShader> AsShader(const skif::Context& ctx,
                                    const skif::FilterResult& image,
                                    const skif::LayerSpace<SkIRect>& sampleBounds) {
        return image.asShader(ctx, FilterResult::kDefaultSampling,
                              FilterResult::ShaderFlags::kNone, sampleBounds);
    }

    static skif::FilterResult Rescale(const skif::Context& ctx,
                                      const skif::FilterResult& image,
                                      const skif::LayerSpace<SkSize> scale) {
        return image.rescale(ctx, scale, /*enforceDecal=*/false);
    }

    static void TrackStats(skif::Context* ctx, skif::Stats* stats) {
        ctx->fStats = stats;
    }
};

namespace {

// Parameters controlling the fuzziness matching of expected and actual images.
// NOTE: When image fuzzy diffing fails it will print the expected image, the actual image, and
// an "error" image where all bad pixels have been set to red. You can select all three base64
// encoded PNGs, copy them, and run the following command to view in detail:
//   xsel -o | viewer --file stdin

static constexpr float kRGBTolerance = 8.f / 255.f;
static constexpr float kAATolerance  = 2.f / 255.f;
static constexpr float kDefaultMaxAllowedPercentImageDiff = 1.f;
static const float kFuzzyKernel[3][3] = {{0.9f, 0.9f, 0.9f},
                                         {0.9f, 1.0f, 0.9f},
                                         {0.9f, 0.9f, 0.9f}};
static_assert(std::size(kFuzzyKernel) == std::size(kFuzzyKernel[0]), "Kernel must be square");
static constexpr int kKernelSize = std::size(kFuzzyKernel);

bool colorfilter_equals(const SkColorFilter* actual, const SkColorFilter* expected) {
    if (!actual || !expected) {
        return !actual && !expected; // both null
    }
    // The two filter objects are equal if they serialize to the same structure
    sk_sp<SkData> actualData = actual->serialize();
    sk_sp<SkData> expectedData = expected->serialize();
    return actualData && actualData->equals(expectedData.get());
}

void clear_device(SkDevice* device) {
    SkPaint p;
    p.setColor4f(SkColors::kTransparent, /*colorSpace=*/nullptr);
    p.setBlendMode(SkBlendMode::kSrc);
    device->drawPaint(p);
}

static constexpr SkTileMode kTileModes[4] = {SkTileMode::kClamp,
                                             SkTileMode::kRepeat,
                                             SkTileMode::kMirror,
                                             SkTileMode::kDecal};

enum class Expect {
    kDeferredImage, // i.e. modified properties of FilterResult instead of rendering
    kNewImage,      // i.e. rendered a new image before modifying other properties
    kEmptyImage,    // i.e. everything is transparent black
};

class ApplyAction {
    struct TransformParams {
        LayerSpace<SkMatrix> fMatrix;
        SkSamplingOptions fSampling;
    };
    struct CropParams {
        LayerSpace<SkIRect> fRect;
        SkTileMode fTileMode;
        // Sometimes the expected bounds due to cropping and tiling are too hard to automate with
        // simple test code.
        std::optional<LayerSpace<SkIRect>> fExpectedBounds;
    };
    struct RescaleParams {
        LayerSpace<SkSize> fScale;
    };

public:
    ApplyAction(const SkMatrix& transform,
                const SkSamplingOptions& sampling,
                Expect expectation,
                const SkSamplingOptions& expectedSampling,
                SkTileMode expectedTileMode,
                sk_sp<SkColorFilter> expectedColorFilter)
            : fAction{TransformParams{LayerSpace<SkMatrix>(transform), sampling}}
            , fExpectation(expectation)
            , fExpectedSampling(expectedSampling)
            , fExpectedTileMode(expectedTileMode)
            , fExpectedColorFilter(std::move(expectedColorFilter)) {}

    ApplyAction(const SkIRect& cropRect,
                SkTileMode tileMode,
                std::optional<LayerSpace<SkIRect>> expectedBounds,
                Expect expectation,
                const SkSamplingOptions& expectedSampling,
                SkTileMode expectedTileMode,
                sk_sp<SkColorFilter> expectedColorFilter)
            : fAction{CropParams{LayerSpace<SkIRect>(cropRect), tileMode, expectedBounds}}
            , fExpectation(expectation)
            , fExpectedSampling(expectedSampling)
            , fExpectedTileMode(expectedTileMode)
            , fExpectedColorFilter(std::move(expectedColorFilter)) {}

    ApplyAction(sk_sp<SkColorFilter> colorFilter,
                Expect expectation,
                const SkSamplingOptions& expectedSampling,
                SkTileMode expectedTileMode,
                sk_sp<SkColorFilter> expectedColorFilter)
            : fAction(std::move(colorFilter))
            , fExpectation(expectation)
            , fExpectedSampling(expectedSampling)
            , fExpectedTileMode(expectedTileMode)
            , fExpectedColorFilter(std::move(expectedColorFilter)) {}

    ApplyAction(LayerSpace<SkSize> scale,
                Expect expectation,
                const SkSamplingOptions& expectedSampling,
                SkTileMode expectedTileMode,
                sk_sp<SkColorFilter> expectedColorFilter)
            : fAction(RescaleParams{scale})
            , fExpectation(expectation)
            , fExpectedSampling(expectedSampling)
            , fExpectedTileMode(expectedTileMode)
            , fExpectedColorFilter(std::move(expectedColorFilter)) {}

    // Test-simplified logic for bounds propagation similar to how image filters calculate bounds
    // while evaluating a filter DAG, which is outside of skif::FilterResult's responsibilities.
    LayerSpace<SkIRect> requiredInput(const LayerSpace<SkIRect>& desiredOutput) const {
        if (auto* t = std::get_if<TransformParams>(&fAction)) {
            LayerSpace<SkIRect> out;
            return t->fMatrix.inverseMapRect(desiredOutput, &out)
                    ? out : LayerSpace<SkIRect>::Empty();
        } else if (auto* c = std::get_if<CropParams>(&fAction)) {
            LayerSpace<SkIRect> intersection = c->fRect;
            if (c->fTileMode == SkTileMode::kDecal && !intersection.intersect(desiredOutput)) {
                intersection = LayerSpace<SkIRect>::Empty();
            }
            return intersection;
        } else if (std::holds_alternative<sk_sp<SkColorFilter>>(fAction) ||
                   std::holds_alternative<RescaleParams>(fAction)) {
            return desiredOutput;
        }
        SkUNREACHABLE;
    }

    // Performs the action to be tested
    FilterResult apply(const Context& ctx, const FilterResult& in) const {
        if (auto* t = std::get_if<TransformParams>(&fAction)) {
            return in.applyTransform(ctx, t->fMatrix, t->fSampling);
        } else if (auto* c = std::get_if<CropParams>(&fAction)) {
            return in.applyCrop(ctx, c->fRect, c->fTileMode);
        } else if (auto* cf = std::get_if<sk_sp<SkColorFilter>>(&fAction)) {
            return in.applyColorFilter(ctx, *cf);
        } else if (auto* s = std::get_if<RescaleParams>(&fAction)) {
            return FilterResultTestAccess::Rescale(ctx, in, s->fScale);
        }
        SkUNREACHABLE;
    }

    Expect expectation() const { return fExpectation; }
    const SkSamplingOptions& expectedSampling() const { return fExpectedSampling; }
    SkTileMode expectedTileMode() const { return fExpectedTileMode; }
    const SkColorFilter* expectedColorFilter() const { return fExpectedColorFilter.get(); }

    int expectedOffscreenSurfaces() const {
        if (fExpectation != Expect::kNewImage) {
            return 0;
        }
        if (auto* s = std::get_if<RescaleParams>(&fAction)) {
            float minScale = std::min(s->fScale.width(), s->fScale.height());
            if (minScale >= 1.f - 0.001f) {
                return 1;
            } else {
                int steps = 0;
                do {
                    steps++;
                    minScale *= 2.f;
                } while(minScale < 0.8f);
                return steps;
            }
        } else {
            return 1;
        }
    }

    LayerSpace<SkIRect> expectedBounds(const LayerSpace<SkIRect>& inputBounds) const {
        // This assumes anything outside 'inputBounds' is transparent black.
        if (auto* t = std::get_if<TransformParams>(&fAction)) {
            if (inputBounds.isEmpty()) {
                return LayerSpace<SkIRect>::Empty();
            }
            return t->fMatrix.mapRect(inputBounds);
        } else if (auto* c = std::get_if<CropParams>(&fAction)) {
            if (c->fExpectedBounds) {
                return *c->fExpectedBounds;
            }

            LayerSpace<SkIRect> intersection = c->fRect;
            if (!intersection.intersect(inputBounds)) {
                return LayerSpace<SkIRect>::Empty();
            }
            return c->fTileMode == SkTileMode::kDecal
                    ? intersection : LayerSpace<SkIRect>(SkRectPriv::MakeILarge());
        } else if (auto* cf = std::get_if<sk_sp<SkColorFilter>>(&fAction)) {
            if (as_CFB(*cf)->affectsTransparentBlack()) {
                // Fills out infinitely
                return LayerSpace<SkIRect>(SkRectPriv::MakeILarge());
            } else {
                return inputBounds;
            }
        } else if (std::holds_alternative<RescaleParams>(fAction)) {
            return inputBounds;
        }
        SkUNREACHABLE;
    }

    sk_sp<SkSpecialImage> renderExpectedImage(const Context& ctx,
                                              sk_sp<SkSpecialImage> source,
                                              LayerSpace<SkIPoint> origin,
                                              const LayerSpace<SkIRect>& desiredOutput) const {
        SkASSERT(source);

        Expect effectiveExpectation = fExpectation;
        SkISize size(desiredOutput.size());
        if (desiredOutput.isEmpty()) {
            size = {1, 1};
            effectiveExpectation = Expect::kEmptyImage;
        }

        auto device = ctx.backend()->makeDevice(size, ctx.refColorSpace());
        SkCanvas canvas{device};
        canvas.clear(SK_ColorTRANSPARENT);
        canvas.translate(-desiredOutput.left(), -desiredOutput.top());

        LayerSpace<SkIRect> sourceBounds{
                SkIRect::MakeXYWH(origin.x(), origin.y(), source->width(), source->height())};
        LayerSpace<SkIRect> expectedBounds = this->expectedBounds(sourceBounds);

        canvas.clipIRect(SkIRect(expectedBounds), SkClipOp::kIntersect);

        if (effectiveExpectation != Expect::kEmptyImage) {
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setBlendMode(SkBlendMode::kSrc);
            // Start with NN to match exact subsetting FilterResult does for deferred images
            SkSamplingOptions sampling = {};
            SkTileMode tileMode = SkTileMode::kDecal;
            if (auto* t = std::get_if<TransformParams>(&fAction)) {
                SkMatrix m{t->fMatrix};
                // FilterResult treats default/bilerp filtering as NN when it has an integer
                // translation, so only change 'sampling' when that is not the case.
                if (!m.isTranslate() ||
                    !SkScalarIsInt(m.getTranslateX()) ||
                    !SkScalarIsInt(m.getTranslateY())) {
                    sampling = t->fSampling;
                }
                canvas.concat(m);
            } else if (auto* c = std::get_if<CropParams>(&fAction)) {
                LayerSpace<SkIRect> imageBounds(
                        SkIRect::MakeXYWH(origin.x(), origin.y(),
                                          source->width(), source->height()));
                if (c->fTileMode == SkTileMode::kDecal || imageBounds.contains(c->fRect)) {
                    // Extract a subset of the image
                    SkAssertResult(imageBounds.intersect(c->fRect));
                    source = source->makeSubset({imageBounds.left() - origin.x(),
                                                 imageBounds.top() - origin.y(),
                                                 imageBounds.right() - origin.x(),
                                                 imageBounds.bottom() - origin.y()});
                    origin = imageBounds.topLeft();
                } else {
                    // A non-decal tile mode where the image doesn't cover the crop requires the
                    // image to be padded out with transparency so the tiling matches 'fRect'.
                    SkISize paddedSize = SkISize(c->fRect.size());
                    auto paddedDevice = ctx.backend()->makeDevice(paddedSize, ctx.refColorSpace());
                    clear_device(paddedDevice.get());
                    paddedDevice->drawSpecial(source.get(),
                                              SkMatrix::Translate(origin.x() - c->fRect.left(),
                                                                  origin.y() - c->fRect.top()),
                                              /*sampling=*/{},
                                              /*paint=*/{});
                    source = paddedDevice->snapSpecial(SkIRect::MakeSize(paddedSize));
                    origin = c->fRect.topLeft();
                }
                tileMode = c->fTileMode;
            } else if (auto* cf = std::get_if<sk_sp<SkColorFilter>>(&fAction)) {
                paint.setColorFilter(*cf);
            } else if (auto* s = std::get_if<RescaleParams>(&fAction)) {
                // Don't redraw with an identity scale since sampling errors creep in on some GPUs
                if (s->fScale.width() != 1.f || s->fScale.height() != 1.f) {
                    SkISize lowResSize = {sk_float_ceil2int(source->width() * s->fScale.width()),
                                        sk_float_ceil2int(source->height() * s->fScale.height())};
                    while (source->width() != lowResSize.width() ||
                        source->height() != lowResSize.height()) {
                        float sx = std::max(0.5f, lowResSize.width() / (float) source->width());
                        float sy = std::max(0.5f, lowResSize.height() / (float) source->height());
                        SkISize stepSize = {sk_float_ceil2int(source->width() * sx),
                                            sk_float_ceil2int(source->height() * sy)};
                        auto stepDevice = ctx.backend()->makeDevice(stepSize, ctx.refColorSpace());
                        clear_device(stepDevice.get());
                        stepDevice->drawSpecial(source.get(),
                                                SkMatrix::Scale(sx, sy),
                                                SkFilterMode::kLinear,
                                                /*paint=*/{});
                        source = stepDevice->snapSpecial(SkIRect::MakeSize(stepSize));
                    }

                    // Adjust to draw the low-res image upscaled to fill the original image bounds
                    sampling = SkFilterMode::kLinear;
                    tileMode = SkTileMode::kClamp;
                    canvas.translate(origin.x(), origin.y());
                    canvas.scale(1.f / s->fScale.width(), 1.f / s->fScale.height());
                    origin = LayerSpace<SkIPoint>({0, 0});
                }
            }
            // else it's a rescale action, but for the expected image leave it unmodified.
            paint.setShader(source->asShader(tileMode,
                                             sampling,
                                             SkMatrix::Translate(origin.x(), origin.y())));
            canvas.drawPaint(paint);
        }
        return device->snapSpecial(SkIRect::MakeSize(size));
    }

private:
    // Action
    std::variant<TransformParams,     // for applyTransform()
                 CropParams,          // for applyCrop()
                 sk_sp<SkColorFilter>,// for applyColorFilter()
                 RescaleParams        // for rescale()
                > fAction;

    // Expectation
    Expect fExpectation;
    SkSamplingOptions fExpectedSampling;
    SkTileMode fExpectedTileMode;
    sk_sp<SkColorFilter> fExpectedColorFilter;
    // The expected desired outputs and layer bounds are calculated automatically based on the
    // action type and parameters to simplify test case specification.
};


class FilterResultImageResolver {
public:
    enum class Method {
        kImageAndOffset,
        kShader,
        kClippedShader,
        kDrawToCanvas
    };

    FilterResultImageResolver(Method method) : fMethod(method) {}

    const char* methodName() const {
        switch (fMethod) {
            case Method::kImageAndOffset: return "imageAndOffset";
            case Method::kShader:         return "asShader";
            case Method::kClippedShader:  return "asShaderClipped";
            case Method::kDrawToCanvas:   return "drawToCanvas";
        }
        SkUNREACHABLE;
    }

    std::pair<sk_sp<SkSpecialImage>, SkIPoint> resolve(const Context& ctx,
                                                       const FilterResult& image) const {
        if (fMethod == Method::kImageAndOffset) {
            SkIPoint origin;
            sk_sp<SkSpecialImage> resolved = image.imageAndOffset(ctx, &origin);
            return {resolved, origin};
        } else {
            if (ctx.desiredOutput().isEmpty()) {
                return {nullptr, {}};
            }

            auto device = ctx.backend()->makeDevice(SkISize(ctx.desiredOutput().size()),
                                                    ctx.refColorSpace());
            SkASSERT(device);

            SkCanvas canvas{device};
            canvas.clear(SK_ColorTRANSPARENT);
            canvas.translate(-ctx.desiredOutput().left(), -ctx.desiredOutput().top());

            if (fMethod == Method::kShader || fMethod == Method::kClippedShader) {
                skif::LayerSpace<SkIRect> sampleBounds;
                if (fMethod == Method::kShader) {
                    // asShader() applies layer bounds by resolving automatically
                    // (e.g. kDrawToCanvas), if sampleBounds is larger than the layer bounds. Since
                    // we want to test the unclipped shader version, pass in layerBounds() for
                    // sampleBounds and add a clip to the canvas instead.
                    canvas.clipIRect(SkIRect(image.layerBounds()));
                    sampleBounds = image.layerBounds();
                } else {
                    sampleBounds = ctx.desiredOutput();
                }

                SkPaint paint;
                paint.setShader(FilterResultTestAccess::AsShader(ctx, image, sampleBounds));
                canvas.drawPaint(paint);
            } else {
                SkASSERT(fMethod == Method::kDrawToCanvas);
                FilterResultTestAccess::Draw(ctx, device.get(), image,
                                             /*preserveDeviceState=*/false);
            }

            return {device->snapSpecial(SkIRect::MakeWH(ctx.desiredOutput().width(),
                                                        ctx.desiredOutput().height())),
                    SkIPoint(ctx.desiredOutput().topLeft())};
        }
    }

private:
    Method fMethod;
};

class TestRunner {
    static constexpr SkColorType kColorType = kRGBA_8888_SkColorType;
public:
    // Raster-backed TestRunner
    TestRunner(skiatest::Reporter* reporter)
            : fReporter(reporter)
            , fBackend(skif::MakeRasterBackend(/*surfaceProps=*/{}, kColorType)) {}

    // Ganesh-backed TestRunner
#if defined(SK_GANESH)
    TestRunner(skiatest::Reporter* reporter, GrDirectContext* context)
            : fReporter(reporter)
            , fDirectContext(context)
            , fBackend(skif::MakeGaneshBackend(sk_ref_sp(context),
                                               kTopLeft_GrSurfaceOrigin,
                                               /*surfaceProps=*/{},
                                               kColorType)) {}
#endif

    // Graphite-backed TestRunner
#if defined(SK_GRAPHITE)
    TestRunner(skiatest::Reporter* reporter, skgpu::graphite::Recorder* recorder)
            : fReporter(reporter)
            , fRecorder(recorder)
            , fBackend(skif::MakeGraphiteBackend(recorder, /*surfaceProps=*/{}, kColorType)) {}
#endif

    // Let TestRunner be passed in to places that take a Reporter* or to REPORTER_ASSERT etc.
    operator skiatest::Reporter*() const { return fReporter; }
    skiatest::Reporter* operator->() const { return fReporter; }

    skif::Backend* backend() const { return fBackend.get(); }
    sk_sp<skif::Backend> refBackend() const { return fBackend; }

    bool compareImages(const skif::Context& ctx,
                       SkSpecialImage* expectedImage,
                       SkIPoint expectedOrigin,
                       const FilterResult& actual,
                       float allowedPercentImageDiff,
                       int transparentCheckBorderTolerance) {
        SkASSERT(expectedImage);

        SkBitmap expectedBM = this->readPixels(expectedImage);

        // Resolve actual using all 4 methods to ensure they are approximately equal to the expected
        // (which is used as a proxy for being approximately equal to each other).
        return this->compareImages(ctx, expectedBM, expectedOrigin, actual,
                                   FilterResultImageResolver::Method::kImageAndOffset,
                                   allowedPercentImageDiff, transparentCheckBorderTolerance) &&
               this->compareImages(ctx, expectedBM, expectedOrigin, actual,
                                   FilterResultImageResolver::Method::kShader,
                                   allowedPercentImageDiff, transparentCheckBorderTolerance) &&
               this->compareImages(ctx, expectedBM, expectedOrigin, actual,
                                   FilterResultImageResolver::Method::kClippedShader,
                                   allowedPercentImageDiff, transparentCheckBorderTolerance) &&
               this->compareImages(ctx, expectedBM, expectedOrigin, actual,
                                   FilterResultImageResolver::Method::kDrawToCanvas,
                                   allowedPercentImageDiff, transparentCheckBorderTolerance);
    }

private:

    bool compareImages(const skif::Context& ctx, const SkBitmap& expected, SkIPoint expectedOrigin,
                       const FilterResult& actual, FilterResultImageResolver::Method method,
                       float allowedPercentImageDiff, int transparentCheckBorderTolerance) {
        FilterResultImageResolver resolver{method};
        auto [actualImage, actualOrigin] = resolver.resolve(ctx, actual);

        SkBitmap actualBM = this->readPixels(actualImage.get()); // empty if actualImage is null
        TArray<SkIPoint> badPixels;
        if (!this->compareBitmaps(expected, expectedOrigin, actualBM, actualOrigin,
                                  allowedPercentImageDiff, transparentCheckBorderTolerance,
                                  &badPixels)) {
            SkDebugf("FilterResult comparison failed for method %s\n", resolver.methodName());
            this->logBitmaps(expected, actualBM, badPixels);
            return false;
        }
        return true;
    }


    bool compareBitmaps(const SkBitmap& expected,
                        SkIPoint expectedOrigin,
                        const SkBitmap& actual,
                        SkIPoint actualOrigin,
                        float allowedPercentImageDiff,
                        int transparentCheckBorderTolerance,
                        TArray<SkIPoint>* badPixels) {
        SkIRect excludeTransparentCheck; // region in expectedBM that can be non-transparent
        if (actual.empty()) {
            // A null image in a FilterResult is equivalent to transparent black, so we should
            // expect the contents of 'expectedImage' to be transparent black.
            excludeTransparentCheck = SkIRect::MakeEmpty();
        } else {
            // The actual image bounds should be contained in the expected image's bounds.
            SkIRect actualBounds = SkIRect::MakeXYWH(actualOrigin.x(), actualOrigin.y(),
                                                     actual.width(), actual.height());
            SkIRect expectedBounds = SkIRect::MakeXYWH(expectedOrigin.x(), expectedOrigin.y(),
                                                       expected.width(), expected.height());
            const bool contained = expectedBounds.contains(actualBounds);
            REPORTER_ASSERT(fReporter, contained,
                    "actual image [%d %d %d %d] not contained within expected [%d %d %d %d]",
                    actualBounds.fLeft, actualBounds.fTop,
                    actualBounds.fRight, actualBounds.fBottom,
                    expectedBounds.fLeft, expectedBounds.fTop,
                    expectedBounds.fRight, expectedBounds.fBottom);
            if (!contained) {
                return false;
            }

            // The actual pixels should match fairly closely with the expected, allowing for minor
            // differences from consolidating actions into a single render, etc.
            int errorCount = 0;
            SkIPoint offset = actualOrigin - expectedOrigin;
            for (int y = 0; y < actual.height(); ++y) {
                for (int x = 0; x < actual.width(); ++x) {
                    SkIPoint ep = {x + offset.x(), y + offset.y()};
                    SkColor4f expectedColor = expected.getColor4f(ep.fX, ep.fY);
                    SkColor4f actualColor = actual.getColor4f(x, y);
                    if (actualColor != expectedColor &&
                        !this->approxColor(this->boxFilter(actual, x, y),
                                           this->boxFilter(expected, ep.fX, ep.fY))) {
                        badPixels->push_back(ep);
                        errorCount++;
                    }
                }
            }

            const int totalCount = expected.width() * expected.height();
            const float percentError = 100.f * errorCount / (float) totalCount;
            const bool approxMatch = percentError <= allowedPercentImageDiff;
            REPORTER_ASSERT(fReporter, approxMatch,
                            "%d pixels were too different from %d total (%f %% vs. %f %%)",
                            errorCount, totalCount, percentError, allowedPercentImageDiff);
            if (!approxMatch) {
                return false;
            }

            // The expected pixels outside of the actual bounds should be transparent, otherwise
            // the actual image is not returning enough data.
            excludeTransparentCheck = actualBounds.makeOffset(-expectedOrigin);
            // Add per-test padding to the exclusion, which is used when there is upscaling in the
            // expected image that bleeds beyond the layer bounds, but is hard to enforce in the
            // simplified expectation rendering.
            excludeTransparentCheck.outset(transparentCheckBorderTolerance,
                                           transparentCheckBorderTolerance);
        }

        int badTransparencyCount = 0;
        for (int y = 0; y < expected.height(); ++y) {
            for (int x = 0; x < expected.width(); ++x) {
                if (!excludeTransparentCheck.isEmpty() && excludeTransparentCheck.contains(x, y)) {
                    continue;
                }

                // If we are on the edge of the transparency exclusion bounds, allow pixels to be
                // up to 2 off to account for sloppy GPU rendering (seen on some Android devices).
                // This is still visually "transparent" and definitely make sure that
                // off-transparency does not extend across the entire surface (tolerance = 0).
                const bool onEdge = !excludeTransparentCheck.isEmpty() &&
                                    excludeTransparentCheck.makeOutset(1, 1).contains(x, y);
                if (!this->approxColor(expected.getColor4f(x, y), SkColors::kTransparent,
                                       onEdge ? kAATolerance : 0.f)) {
                    badPixels->push_back({x, y});
                    badTransparencyCount++;
                }
            }
        }

        REPORTER_ASSERT(fReporter, badTransparencyCount == 0, "Unexpected non-transparent pixels");
        return badTransparencyCount == 0;
    }

    bool approxColor(const SkColor4f& a,
                     const SkColor4f& b,
                     float tolerance = kRGBTolerance) const {
        SkPMColor4f apm = a.premul();
        SkPMColor4f bpm = b.premul();
        // Calculate red-mean, a lowcost approximation of color difference that gives reasonable
        // results for the types of acceptable differences resulting from collapsing compatible
        // SkSamplingOptions or slightly different AA on shape boundaries.
        // See https://www.compuphase.com/cmetric.htm
        float r = (apm.fR + bpm.fR) / 2.f;
        float dr = (apm.fR - bpm.fR);
        float dg = (apm.fG - bpm.fG);
        float db = (apm.fB - bpm.fB);
        float delta = sqrt((2.f + r)*dr*dr + 4.f*dg*dg + (2.f + (1.f - r))*db*db);
        return delta <= tolerance;
    }

    SkColor4f boxFilter(const SkBitmap& bm, int x, int y) const {
        static constexpr int kKernelOffset = kKernelSize / 2;
        SkPMColor4f sum = {0.f, 0.f, 0.f, 0.f};
        float netWeight = 0.f;
        for (int sy = y - kKernelOffset; sy <= y + kKernelOffset; ++sy) {
            for (int sx = x - kKernelOffset; sx <= x + kKernelOffset; ++sx) {
                float weight = kFuzzyKernel[sy - y + kKernelOffset][sx - x + kKernelOffset];

                if (sx < 0 || sx >= bm.width() || sy < 0 || sy >= bm.height()) {
                    // Treat outside image as transparent black, this is necessary to get
                    // consistent comparisons between expected and actual images where the actual
                    // is cropped as tightly as possible.
                    netWeight += weight;
                    continue;
                }

                SkPMColor4f c = bm.getColor4f(sx, sy).premul() * weight;
                sum.fR += c.fR;
                sum.fG += c.fG;
                sum.fB += c.fB;
                sum.fA += c.fA;
                netWeight += weight;
            }
        }
        SkASSERT(netWeight > 0.f);
        return sum.unpremul() * (1.f / netWeight);
    }

    SkBitmap readPixels(const SkSpecialImage* specialImage) const {
        if (!specialImage) {
            return SkBitmap(); // an empty bitmap
        }

        [[maybe_unused]] int srcX = specialImage->subset().fLeft;
        [[maybe_unused]] int srcY = specialImage->subset().fTop;
        SkImageInfo ii = SkImageInfo::Make(specialImage->dimensions(),
                                           specialImage->colorInfo());
        SkBitmap bm;
        bm.allocPixels(ii);
#if defined(SK_GANESH)
        if (fDirectContext) {
            // Ganesh backed, just use the SkImage::readPixels API
            SkASSERT(specialImage->isGaneshBacked());
            sk_sp<SkImage> image = specialImage->asImage();
            SkAssertResult(image->readPixels(fDirectContext, bm.pixmap(), srcX, srcY));
        } else
#endif
#if defined(SK_GRAPHITE)
        if (fRecorder) {
            // Graphite backed, so use the private testing-only synchronous API
            SkASSERT(specialImage->isGraphiteBacked());
            auto view = SkSpecialImages::AsTextureProxyView(specialImage);
            auto proxyII = ii.makeWH(view.width(), view.height());
            SkAssertResult(fRecorder->priv().context()->priv().readPixels(
                    bm.pixmap(), view.proxy(), proxyII, srcX, srcY));
        } else
#endif
        {
            // Assume it's raster backed, so use AsBitmap directly
            SkAssertResult(SkSpecialImages::AsBitmap(specialImage, &bm));
        }

        return bm;
    }

    void logBitmaps(const SkBitmap& expected,
                    const SkBitmap& actual,
                    const TArray<SkIPoint>& badPixels) {
        if (fLoggedErrorImage) {
            return; // no more spam
        }

        SkString expectedURL;
        ToolUtils::BitmapToBase64DataURI(expected, &expectedURL);
        SkDebugf("Expected:\n%s\n\n", expectedURL.c_str());

        if (!actual.empty()) {
            SkString actualURL;
            ToolUtils::BitmapToBase64DataURI(actual, &actualURL);
            SkDebugf("Actual:\n%s\n\n", actualURL.c_str());
        } else {
            SkDebugf("Actual: null (fully transparent)\n\n");
        }

        if (!badPixels.empty()) {
            for (auto p : badPixels) {
                expected.erase(SkColors::kRed, SkIRect::MakeXYWH(p.fX, p.fY, 1, 1));
            }
            SkString markedURL;
            ToolUtils::BitmapToBase64DataURI(expected, &markedURL);
            SkDebugf("Errors:\n%s\n\n", markedURL.c_str());
        }

        fLoggedErrorImage = true;
    }

    skiatest::Reporter* fReporter;
#if defined(SK_GANESH)
    GrDirectContext* fDirectContext = nullptr;
#endif
#if defined(SK_GRAPHITE)
    skgpu::graphite::Recorder* fRecorder = nullptr;
#endif

    sk_sp<skif::Backend> fBackend;

    bool fLoggedErrorImage = false; // only do this once per test runner
};

class TestCase {
public:
    TestCase(TestRunner& runner,
             std::string name,
             float allowedPercentImageDiff=kDefaultMaxAllowedPercentImageDiff,
             int transparentCheckBorderTolerance=0)
            : fRunner(runner)
            , fName(name)
            , fAllowedPercentImageDiff(allowedPercentImageDiff)
            , fTransparentCheckBorderTolerance(transparentCheckBorderTolerance)
            , fSourceBounds(LayerSpace<SkIRect>::Empty())
            , fDesiredOutput(LayerSpace<SkIRect>::Empty()) {}

    TestCase& source(const SkIRect& bounds) {
        fSourceBounds = LayerSpace<SkIRect>(bounds);
        return *this;
    }


    TestCase& applyCrop(const SkIRect& crop, Expect expectation) {
        return this->applyCrop(crop, SkTileMode::kDecal, expectation);
    }

    TestCase& applyCrop(const SkIRect& crop,
                        SkTileMode tileMode,
                        Expect expectation,
                        std::optional<SkTileMode> expectedTileMode = {},
                        std::optional<SkIRect> expectedBounds = {}) {
        // Fill-in automated expectations, which is to equal 'tileMode' when not overridden.
        if (!expectedTileMode) {
            expectedTileMode = tileMode;
        }
        std::optional<LayerSpace<SkIRect>> expectedLayerBounds;
        if (expectedBounds) {
            expectedLayerBounds = LayerSpace<SkIRect>(*expectedBounds);
        }
        fActions.emplace_back(crop, tileMode, expectedLayerBounds, expectation,
                              this->getDefaultExpectedSampling(expectation),
                              *expectedTileMode,
                              this->getDefaultExpectedColorFilter(expectation));
        return *this;
    }

    TestCase& applyTransform(const SkMatrix& matrix, Expect expectation) {
        return this->applyTransform(matrix, FilterResult::kDefaultSampling, expectation);
    }

    TestCase& applyTransform(const SkMatrix& matrix,
                             const SkSamplingOptions& sampling,
                             Expect expectation,
                             std::optional<SkSamplingOptions> expectedSampling = {}) {
        // Fill-in automated expectations, which is simply that if it's not explicitly provided we
        // assume the result's sampling equals what was passed to applyTransform().
        if (!expectedSampling.has_value()) {
            expectedSampling = sampling;
        }
        fActions.emplace_back(matrix, sampling, expectation, *expectedSampling,
                              this->getDefaultExpectedTileMode(expectation,
                                                               /*cfAffectsTransparency=*/false),
                              this->getDefaultExpectedColorFilter(expectation));
        return *this;
    }

    TestCase& applyColorFilter(sk_sp<SkColorFilter> colorFilter,
                               Expect expectation,
                               std::optional<sk_sp<SkColorFilter>> expectedColorFilter = {}) {
        // The expected color filter is the composition of the default expectation (e.g. last
        // color filter or null for a new image) and the new 'colorFilter'. Compose() automatically
        // returns 'colorFilter' if the inner filter is null.
        if (!expectedColorFilter.has_value()) {
            expectedColorFilter = SkColorFilters::Compose(
                    colorFilter, this->getDefaultExpectedColorFilter(expectation));
        }
        const bool affectsTransparent = as_CFB(colorFilter)->affectsTransparentBlack();
        fActions.emplace_back(std::move(colorFilter), expectation,
                              this->getDefaultExpectedSampling(expectation),
                              this->getDefaultExpectedTileMode(expectation, affectsTransparent),
                              std::move(*expectedColorFilter));
        return *this;
    }

    TestCase& rescale(SkSize scale,
                      Expect expectation,
                      std::optional<SkTileMode> expectedTileMode = {}) {
        SkASSERT(!fActions.empty());
        if (!expectedTileMode) {
            expectedTileMode = this->getDefaultExpectedTileMode(expectation,
                                                                /*cfAffectsTransparency=*/false);
        }
        fActions.emplace_back(skif::LayerSpace<SkSize>(scale), expectation,
                              this->getDefaultExpectedSampling(expectation),
                              *expectedTileMode,
                              this->getDefaultExpectedColorFilter(expectation));
        return *this;
    }

    void run(const SkIRect& requestedOutput) const {
        skiatest::ReporterContext caseLabel(fRunner, fName);
        this->run(requestedOutput, /*backPropagateDesiredOutput=*/true);
        this->run(requestedOutput, /*backPropagateDesiredOutput=*/false);
    }

    void run(const SkIRect& requestedOutput, bool backPropagateDesiredOutput) const {
        SkASSERT(!fActions.empty()); // It's a bad test case if there aren't any actions

        skiatest::ReporterContext backPropagate(
                fRunner, SkStringPrintf("backpropagate output: %d", backPropagateDesiredOutput));

        auto desiredOutput = LayerSpace<SkIRect>(requestedOutput);
        std::vector<LayerSpace<SkIRect>> desiredOutputs;
        desiredOutputs.resize(fActions.size(), desiredOutput);
        if (!backPropagateDesiredOutput) {
            // Set the desired output to be equal to the expected output so that there is no
            // further restriction of what's computed for early actions to then be ruled out by
            // subsequent actions.
            auto inputBounds = fSourceBounds;
            for (int i = 0; i < (int) fActions.size() - 1; ++i) {
                desiredOutputs[i] = fActions[i].expectedBounds(inputBounds);
                // If the output for the ith action is infinite, leave it for now and expand the
                // input bounds for action i+1. The infinite bounds will be replaced by the
                // back-propagated desired output of the next action.
                if (SkIRect(desiredOutputs[i]) == SkRectPriv::MakeILarge()) {
                    inputBounds.outset(LayerSpace<SkISize>({25, 25}));
                } else {
                    inputBounds = desiredOutputs[i];
                }
            }
        }
        // Fill out regular back-propagated desired outputs and cleanup infinite outputs
        for (int i = (int) fActions.size() - 2; i >= 0; --i) {
            if (backPropagateDesiredOutput ||
                SkIRect(desiredOutputs[i]) == SkRectPriv::MakeILarge()) {
                desiredOutputs[i] = fActions[i+1].requiredInput(desiredOutputs[i+1]);
            }
        }

        // Create the source image
        sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGB();
        FilterResult source;
        if (!fSourceBounds.isEmpty()) {
            sk_sp<SkDevice> sourceSurface =
                    fRunner.backend()->makeDevice(SkISize(fSourceBounds.size()), colorSpace);

            const SkColor colors[] = { SK_ColorMAGENTA,
                                       SK_ColorRED,
                                       SK_ColorYELLOW,
                                       SK_ColorGREEN,
                                       SK_ColorCYAN,
                                       SK_ColorBLUE };
            SkMatrix rotation = SkMatrix::RotateDeg(15.f, {fSourceBounds.width() / 2.f,
                                                           fSourceBounds.height() / 2.f});

            SkCanvas canvas{sourceSurface};
            canvas.clear(SK_ColorBLACK);
            canvas.concat(rotation);

            int color = 0;
            SkRect coverBounds;
            SkRect dstBounds = SkRect::Make(canvas.imageInfo().bounds());
            SkAssertResult(SkMatrixPriv::InverseMapRect(rotation, &coverBounds, dstBounds));

            float sz = fSourceBounds.width() <= 16.f || fSourceBounds.height() <= 16.f ? 2.f : 8.f;
            for (float y = coverBounds.fTop; y < coverBounds.fBottom; y += sz) {
                for (float x = coverBounds.fLeft; x < coverBounds.fRight; x += sz) {
                    SkPaint p;
                    p.setColor(colors[(color++) % std::size(colors)]);
                    canvas.drawRect(SkRect::MakeXYWH(x, y, sz, sz), p);
                }
            }

            SkIRect subset = SkIRect::MakeWH(fSourceBounds.width(), fSourceBounds.height());
            source = FilterResult(sourceSurface->snapSpecial(subset), fSourceBounds.topLeft());
        }

        Stats stats;
        Context baseContext{fRunner.refBackend(),
                            skif::Mapping{SkMatrix::I()},
                            skif::LayerSpace<SkIRect>::Empty(),
                            source,
                            colorSpace.get(),
                            &stats};

        // Applying modifiers to FilterResult might produce a new image, but hopefully it's
        // able to merge properties and even re-order operations to minimize the number of offscreen
        // surfaces that it creates. To validate that this is producing an equivalent image, we
        // track what to expect by rendering each action every time without any optimization.
        sk_sp<SkSpecialImage> expectedImage = source.refImage();
        LayerSpace<SkIPoint> expectedOrigin = source.layerBounds().topLeft();
        // The expected image can't ever be null, so we produce a transparent black image instead.
        if (!expectedImage) {
            sk_sp<SkDevice> expectedSurface = fRunner.backend()->makeDevice({1, 1}, colorSpace);
            clear_device(expectedSurface.get());
            expectedImage = expectedSurface->snapSpecial(SkIRect::MakeWH(1, 1));
            expectedOrigin = LayerSpace<SkIPoint>({0, 0});
        }
        SkASSERT(expectedImage);

        int expectedNumOffscreenSurfaces = 0;
        int expectedShaderTiledDraws = 0; // includes shader-based clamping for simplicity

        // Apply each action and validate, from first to last action
        for (int i = 0; i < (int) fActions.size(); ++i) {
            skiatest::ReporterContext actionLabel(fRunner, SkStringPrintf("action %d", i));
            auto ctx = baseContext.withNewDesiredOutput(desiredOutputs[i]);
            FilterResultTestAccess::TrackStats(&ctx, &stats);

            FilterResult output = fActions[i].apply(ctx, source);
            // Validate consistency of the output
            REPORTER_ASSERT(fRunner, SkToBool(output.image()) == !output.layerBounds().isEmpty());

            LayerSpace<SkIRect> expectedBounds = fActions[i].expectedBounds(source.layerBounds());
            Expect correctedExpectation = fActions[i].expectation();
            if (SkIRect(expectedBounds) == SkRectPriv::MakeILarge()) {
                // An expected image filling out to infinity should have an actual image that
                // fills the desired output.
                expectedBounds = desiredOutputs[i];
                if (desiredOutputs[i].isEmpty()) {
                    correctedExpectation = Expect::kEmptyImage;
                }
            } else if (!expectedBounds.intersect(desiredOutputs[i])) {
                // Test cases should provide image expectations for the case where desired output
                // is not back-propagated. When desired output is back-propagated, it can lead to
                // earlier actions becoming empty actions.
                REPORTER_ASSERT(fRunner, fActions[i].expectation() == Expect::kEmptyImage ||
                                         backPropagateDesiredOutput);
                expectedBounds = LayerSpace<SkIRect>::Empty();
                correctedExpectation = Expect::kEmptyImage;
            }

            int numIntermediateSurfaces = fActions[i].expectedOffscreenSurfaces();
            expectedNumOffscreenSurfaces += numIntermediateSurfaces;

            bool actualNewImage = output.image() &&
                    (!source.image() || output.image()->uniqueID() != source.image()->uniqueID());
            switch(correctedExpectation) {
                case Expect::kNewImage:
                    REPORTER_ASSERT(fRunner, actualNewImage);
                    if (source && !source.image()->isExactFit()) {
                        expectedShaderTiledDraws += numIntermediateSurfaces;
                    }
                    break;
                case Expect::kDeferredImage:
                    REPORTER_ASSERT(fRunner, !actualNewImage && output.image());
                    break;
                case Expect::kEmptyImage:
                    REPORTER_ASSERT(fRunner, !actualNewImage && !output.image());
                    break;
            }

            // Validate layer bounds and sampling when we expect a new or deferred image
            if (output.image()) {
                REPORTER_ASSERT(fRunner, !expectedBounds.isEmpty());
                REPORTER_ASSERT(fRunner, SkIRect(output.layerBounds()) == SkIRect(expectedBounds));
                REPORTER_ASSERT(fRunner, output.sampling() == fActions[i].expectedSampling());
                REPORTER_ASSERT(fRunner, output.tileMode() == fActions[i].expectedTileMode());
                REPORTER_ASSERT(fRunner, colorfilter_equals(output.colorFilter(),
                                                            fActions[i].expectedColorFilter()));
            }

            FilterResultTestAccess::TrackStats(&ctx, nullptr);
            expectedImage = fActions[i].renderExpectedImage(ctx,
                                                            std::move(expectedImage),
                                                            expectedOrigin,
                                                            desiredOutputs[i]);
            expectedOrigin = desiredOutputs[i].topLeft();
            if (!fRunner.compareImages(ctx,
                                       expectedImage.get(),
                                       SkIPoint(expectedOrigin),
                                       output,
                                       fAllowedPercentImageDiff,
                                       fTransparentCheckBorderTolerance)) {
                // If one iteration is incorrect, its failures will likely cascade to further
                // actions so end now as the test has failed.
                break;
            }
            source = output;
        }

        // Verify overall stats behavior
        REPORTER_ASSERT(fRunner, expectedNumOffscreenSurfaces == stats.fNumOffscreenSurfaces,
                        "expected %d, got %d",
                        expectedNumOffscreenSurfaces, stats.fNumOffscreenSurfaces);

        REPORTER_ASSERT(fRunner,
                        expectedShaderTiledDraws ==
                                stats.fNumShaderBasedTilingDraws + stats.fNumShaderClampedDraws,
                        "expected %d, got %d + %d",
                        expectedShaderTiledDraws,
                        stats.fNumShaderBasedTilingDraws, stats.fNumShaderClampedDraws);
    }

private:
    // By default an action that doesn't define its own sampling options will not change sampling
    // unless it produces a new image. Otherwise it inherits the prior action's expectation.
    SkSamplingOptions getDefaultExpectedSampling(Expect expectation) const {
        if (expectation != Expect::kDeferredImage || fActions.empty()) {
            return FilterResult::kDefaultSampling;
        } else {
            return fActions[fActions.size() - 1].expectedSampling();
        }
    }
    // By default an action that doesn't define its own tiling will not change the tiling, unless it
    // produces a new image, at which point it becomes kDecal again.
    SkTileMode getDefaultExpectedTileMode(Expect expectation, bool cfAffectsTransparency) const {
        if (expectation == Expect::kNewImage && cfAffectsTransparency) {
            return SkTileMode::kClamp;
        } else if (expectation != Expect::kDeferredImage || fActions.empty()) {
            return SkTileMode::kDecal;
        } else {
            return fActions[fActions.size() - 1].expectedTileMode();
        }
    }
    // By default an action that doesn't define its own color filter will not change filtering,
    // unless it produces a new image. Otherwise it inherits the prior action's expectations.
    sk_sp<SkColorFilter> getDefaultExpectedColorFilter(Expect expectation) const {
        if (expectation != Expect::kDeferredImage || fActions.empty()) {
            return nullptr;
        } else {
            return sk_ref_sp(fActions[fActions.size() - 1].expectedColorFilter());
        }
    }

    TestRunner& fRunner;
    std::string fName;
    float fAllowedPercentImageDiff;
    int   fTransparentCheckBorderTolerance;

    // Used to construct an SkSpecialImage of the given size/location filled with the known pattern.
    LayerSpace<SkIRect> fSourceBounds;

    // The intended area to fill with the result, controlled by outside factors (e.g. clip bounds)
    LayerSpace<SkIRect> fDesiredOutput;

    std::vector<ApplyAction> fActions;
};

// ----------------------------------------------------------------------------
// Utilities to create color filters for the unit tests

sk_sp<SkColorFilter> alpha_modulate(float v) {
    // dst-in blending with src = (1,1,1,v) = dst * v
    auto cf = SkColorFilters::Blend({1.f,1.f,1.f,v}, /*colorSpace=*/nullptr, SkBlendMode::kDstIn);
    SkASSERT(cf && !as_CFB(cf)->affectsTransparentBlack());
    return cf;
}

sk_sp<SkColorFilter> affect_transparent(SkColor4f color) {
    auto cf = SkColorFilters::Blend(color, /*colorSpace=*/nullptr, SkBlendMode::kPlus);
    SkASSERT(cf && as_CFB(cf)->affectsTransparentBlack());
    return cf;
}

// ----------------------------------------------------------------------------

// TODO(skbug.com/14607) - Run FilterResultTests on Dawn and ANGLE backends, too

#if defined(SK_GANESH)
#define DEF_GANESH_TEST_SUITE(name, ctsEnforcement)          \
    DEF_GANESH_TEST_FOR_CONTEXTS(FilterResult_ganesh_##name, \
                                 skgpu::IsNativeBackend,     \
                                 r,                          \
                                 ctxInfo,                    \
                                 nullptr,                    \
                                 ctsEnforcement) {           \
        TestRunner runner(r, ctxInfo.directContext());       \
        test_suite_##name(runner);                           \
    }
#else
#define DEF_GANESH_TEST_SUITE(name) // do nothing
#endif

#if defined(SK_GRAPHITE)
#define DEF_GRAPHITE_TEST_SUITE(name, ctsEnforcement)                            \
    DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(FilterResult_graphite_##name, \
                                                   skgpu::IsNativeBackend,       \
                                                   r,                            \
                                                   context,                      \
                                                   testContext,                  \
                                                   true,                         \
                                                   ctsEnforcement) {             \
        using namespace skgpu::graphite;                                         \
        auto recorder = context->makeRecorder();                                 \
        TestRunner runner(r, recorder.get());                                    \
        test_suite_##name(runner);                                               \
        std::unique_ptr<Recording> recording = recorder->snap();                 \
        if (!recording) {                                                        \
            ERRORF(r, "Failed to make recording");                               \
            return;                                                              \
        }                                                                        \
        InsertRecordingInfo insertInfo;                                          \
        insertInfo.fRecording = recording.get();                                 \
        context->insertRecording(insertInfo);                                    \
        testContext->syncedSubmit(context);                                      \
    }
#else
#define DEF_GRAPHITE_TEST_SUITE(name) // do nothing
#endif

#define DEF_TEST_SUITE(name, runner, ganeshCtsEnforcement, graphiteCtsEnforcement) \
    static void test_suite_##name(TestRunner&); \
    /* TODO(b/274901800): Uncomment to enable Graphite test execution. */ \
    /* DEF_GRAPHITE_TEST_SUITE(name, graphiteCtsEnforcement) */ \
    DEF_GANESH_TEST_SUITE(name, ganeshCtsEnforcement) \
    DEF_TEST(FilterResult_raster_##name, reporter) { \
        TestRunner runner(reporter); \
        test_suite_##name(runner); \
    } \
    void test_suite_##name(TestRunner& runner)

// ----------------------------------------------------------------------------
// Empty input/output tests

DEF_TEST_SUITE(EmptySource, r, CtsEnforcement::kApiLevel_T, CtsEnforcement::kNextRelease) {
    // This is testing that an empty input image is handled by the applied actions without having
    // to generate new images, or that it can produce a new image from nothing when it affects
    // transparent black.
    for (SkTileMode tm : kTileModes) {
        TestCase(r, "applyCrop() to empty source")
                .source(SkIRect::MakeEmpty())
                .applyCrop({0, 0, 10, 10}, tm, Expect::kEmptyImage)
                .run(/*requestedOutput=*/{0, 0, 20, 20});
    }

    TestCase(r, "applyTransform() to empty source")
            .source(SkIRect::MakeEmpty())
            .applyTransform(SkMatrix::Translate(10.f, 10.f), Expect::kEmptyImage)
            .run(/*requestedOutput=*/{10, 10, 20, 20});

    TestCase(r, "applyColorFilter() to empty source")
            .source(SkIRect::MakeEmpty())
            .applyColorFilter(alpha_modulate(0.5f), Expect::kEmptyImage)
            .run(/*requestedOutput=*/{0, 0, 10, 10});

    TestCase(r, "Transparency-affecting color filter overrules empty source")
            .source(SkIRect::MakeEmpty())
            .applyColorFilter(affect_transparent(SkColors::kBlue), Expect::kNewImage,
                              /*expectedColorFilter=*/nullptr) // CF applied ASAP to make a new img
            .run(/*requestedOutput=*/{0, 0, 10, 10});
}

DEF_TEST_SUITE(EmptyDesiredOutput, r, CtsEnforcement::kApiLevel_T, CtsEnforcement::kNextRelease) {
    // This is testing that an empty requested output is propagated through the applied actions so
    // that no actual images are generated.
    for (SkTileMode tm : kTileModes) {
        TestCase(r, "applyCrop() + empty output becomes empty")
                .source({0, 0, 10, 10})
                .applyCrop({2, 2, 8, 8}, tm, Expect::kEmptyImage)
                .run(/*requestedOutput=*/SkIRect::MakeEmpty());
    }

    TestCase(r, "applyTransform() + empty output becomes empty")
            .source({0, 0, 10, 10})
            .applyTransform(SkMatrix::RotateDeg(10.f), Expect::kEmptyImage)
            .run(/*requestedOutput=*/SkIRect::MakeEmpty());

    TestCase(r, "applyColorFilter() + empty output becomes empty")
            .source({0, 0, 10, 10})
            .applyColorFilter(alpha_modulate(0.5f), Expect::kEmptyImage)
            .run(/*requestedOutput=*/SkIRect::MakeEmpty());

    TestCase(r, "Transpency-affecting color filter + empty output is empty")
            .source({0, 0, 10, 10})
            .applyColorFilter(affect_transparent(SkColors::kBlue), Expect::kEmptyImage)
            .run(/*requestedOutput=*/SkIRect::MakeEmpty());
}

// ----------------------------------------------------------------------------
// applyCrop() tests

DEF_TEST_SUITE(Crop, r, CtsEnforcement::kApiLevel_T, CtsEnforcement::kNextRelease) {
    // This is testing all the combinations of how the src, crop, and requested output rectangles
    // can interact while still resulting in a deferred image. The exception is non-decal tile
    // modes where the crop rect includes transparent pixels not filled by the source, which
    // requires a new image to ensure tiling matches the crop geometry.
    for (SkTileMode tm : kTileModes) {
        const Expect nonDecalExpectsNewImage = tm == SkTileMode::kDecal ? Expect::kDeferredImage
                                                                        : Expect::kNewImage;
        TestCase(r, "applyCrop() contained in source and output")
                .source({0, 0, 20, 20})
                .applyCrop({8, 8, 12, 12}, tm, Expect::kDeferredImage)
                .run(/*requestedOutput=*/{4, 4, 16, 16});

        TestCase(r, "applyCrop() contained in source, intersects output")
                .source({0, 0, 20, 20})
                .applyCrop({4, 4, 12, 12}, tm, Expect::kDeferredImage)
                .run(/*requestedOutput=*/{8, 8, 16, 16});

        TestCase(r, "applyCrop() intersects source, contained in output")
                .source({10, 10, 20, 20})
                .applyCrop({4, 4, 16, 16}, tm, nonDecalExpectsNewImage)
                .run(/*requestedOutput=*/{0, 0, 20, 20});

        TestCase(r, "applyCrop() intersects source and output")
                .source({0, 0, 10, 10})
                .applyCrop({5, -5, 15, 5}, tm, nonDecalExpectsNewImage)
                .run(/*requestedOutput=*/{7, -2, 12, 8});

        TestCase(r, "applyCrop() contains source, intersects output")
                .source({4, 4, 16, 16})
                .applyCrop({0, 0, 20, 20}, tm, nonDecalExpectsNewImage)
                .run(/*requestedOutput=*/{-5, -5, 18, 18});

        // In these cases, cropping with a non-decal tile mode can be discarded because the output
        // bounds are entirely within the crop so no tiled edges would be visible.
        TestCase(r, "applyCrop() intersects source, contains output")
                .source({0, 0, 20, 20})
                .applyCrop({-5, 5, 25, 15}, tm, Expect::kDeferredImage, SkTileMode::kDecal)
                .run(/*requestedOutput=*/{0, 5, 20, 15});

        TestCase(r, "applyCrop() contains source and output")
                .source({0, 0, 10, 10})
                .applyCrop({-5, -5, 15, 15}, tm, Expect::kDeferredImage, SkTileMode::kDecal)
                .run(/*requestedOutput=*/{1, 1, 9, 9});
    }
}

DEF_TEST_SUITE(CropDisjointFromSourceAndOutput, r, CtsEnforcement::kApiLevel_T,
               CtsEnforcement::kNextRelease) {
    // This tests all the combinations of src, crop, and requested output rectangles that result in
    // an empty image without any of the rectangles being empty themselves. The exception is for
    // non-decal tile modes when the source and crop still intersect. In that case the non-empty
    // content is tiled into the disjoint output rect, producing a non-empty image.
    for (SkTileMode tm : kTileModes) {
        TestCase(r, "applyCrop() disjoint from source, intersects output")
                .source({0, 0, 10, 10})
                .applyCrop({11, 11, 20, 20}, tm, Expect::kEmptyImage)
                .run(/*requestedOutput=*/{0, 0, 15, 15});

        TestCase(r, "applyCrop() disjoint from source, intersects output disjoint from source")
                .source({0, 0, 10, 10})
                .applyCrop({11, 11, 20, 20}, tm, Expect::kEmptyImage)
                .run(/*requestedOutput=*/{12, 12, 18, 18});

        TestCase(r, "applyCrop() disjoint from source and output")
                .source({0, 0, 10, 10})
                .applyCrop({12, 12, 18, 18}, tm, Expect::kEmptyImage)
                .run(/*requestedOutput=*/{-1, -1, 11, 11});

        TestCase(r, "applyCrop() disjoint from source and output disjoint from source")
                .source({0, 0, 10, 10})
                .applyCrop({-10, 10, -1, -1}, tm, Expect::kEmptyImage)
                .run(/*requestedOutput=*/{11, 11, 20, 20});

        // When the source and crop intersect but are disjoint from the output, the behavior depends
        // on the tile mode. For periodic tile modes, certain geometries can still be deferred by
        // conversion to a transform, but to keep expectations simple we pick bounds such that the
        // tiling can't be dropped. See PeriodicTileCrops for other scenarios.
        Expect nonDecalExpectsImage = tm == SkTileMode::kDecal ? Expect::kEmptyImage :
                                      tm == SkTileMode::kClamp ? Expect::kDeferredImage
                                                               : Expect::kNewImage;
        TestCase(r, "applyCrop() intersects source, disjoint from output disjoint from source")
                .source({0, 0, 10, 10})
                .applyCrop({-5, -5, 5, 5}, tm, nonDecalExpectsImage)
                .run(/*requestedOutput=*/{12, 12, 18, 18});

        TestCase(r, "applyCrop() intersects source, disjoint from output")
                    .source({0, 0, 10, 10})
                    .applyCrop({-5, -5, 5, 5}, tm, nonDecalExpectsImage)
                    .run(/*requestedOutput=*/{6, 6, 18, 18});
    }
}

DEF_TEST_SUITE(EmptyCrop, r, CtsEnforcement::kApiLevel_T, CtsEnforcement::kNextRelease) {
    for (SkTileMode tm : kTileModes) {
        TestCase(r, "applyCrop() is empty")
                .source({0, 0, 10, 10})
                .applyCrop(SkIRect::MakeEmpty(), tm, Expect::kEmptyImage)
                .run(/*requestedOutput=*/{0, 0, 10, 10});

        TestCase(r, "applyCrop() emptiness propagates")
                .source({0, 0, 10, 10})
                .applyCrop({1, 1, 9, 9}, tm, Expect::kDeferredImage)
                .applyCrop(SkIRect::MakeEmpty(), tm, Expect::kEmptyImage)
                .run(/*requestedOutput=*/{0, 0, 10, 10});
    }
}

DEF_TEST_SUITE(DisjointCrops, r, CtsEnforcement::kApiLevel_T, CtsEnforcement::kNextRelease) {
    for (SkTileMode tm : kTileModes) {
        TestCase(r, "Disjoint applyCrop() after kDecal become empty")
                .source({0, 0, 10, 10})
                .applyCrop({0, 0, 4, 4}, SkTileMode::kDecal, Expect::kDeferredImage)
                .applyCrop({6, 6, 10, 10}, tm, Expect::kEmptyImage)
                .run(/*requestedOutput=*/{0, 0, 10, 10});

        if (tm != SkTileMode::kDecal) {
            TestCase(r, "Disjoint tiling applyCrop() before kDecal is not empty and combines")
                    .source({0, 0, 10, 10})
                    .applyCrop({0, 0, 4, 4}, tm, Expect::kDeferredImage)
                    .applyCrop({6, 6, 10, 10}, SkTileMode::kDecal, Expect::kDeferredImage, tm)
                    .run(/*requestedOutput=*/{0, 0, 10, 10});

            TestCase(r, "Disjoint non-decal applyCrops() are not empty")
                .source({0, 0, 10, 10})
                .applyCrop({0, 0, 4, 4}, tm, Expect::kDeferredImage)
                .applyCrop({6, 6, 10, 10}, tm, tm == SkTileMode::kClamp ? Expect::kDeferredImage
                                                                        : Expect::kNewImage)
                .run(/*requestedOutput=*/{0, 0, 10, 10});
        }
    }
}

DEF_TEST_SUITE(IntersectingCrops, r, CtsEnforcement::kApiLevel_T, CtsEnforcement::kNextRelease) {
    for (SkTileMode tm : kTileModes) {
        TestCase(r, "Decal applyCrop() always combines with any other crop")
                .source({0, 0, 20, 20})
                .applyCrop({5, 5, 15, 15}, tm, Expect::kDeferredImage)
                .applyCrop({10, 10, 20, 20}, SkTileMode::kDecal, Expect::kDeferredImage, tm)
                .run(/*requestedOutput=*/{0, 0, 20, 20});

        if (tm != SkTileMode::kDecal) {
            TestCase(r, "Decal applyCrop() before non-decal crop requires new image")
                    .source({0, 0, 20, 20})
                    .applyCrop({5, 5, 15, 15}, SkTileMode::kDecal, Expect::kDeferredImage)
                    .applyCrop({10, 10, 20, 20}, tm, Expect::kNewImage)
                    .run(/*requestedOutput=*/{0, 0, 20, 20});

            TestCase(r, "Consecutive non-decal crops combine if both are clamp")
                    .source({0, 0, 20, 20})
                    .applyCrop({5, 5, 15, 15}, tm, Expect::kDeferredImage)
                    .applyCrop({10, 10, 20, 20}, tm,
                               tm == SkTileMode::kClamp ? Expect::kDeferredImage
                                                        : Expect::kNewImage)
                    .run(/*requestedOutput=*/{0, 0, 20, 20});
        }
    }
}

DEF_TEST_SUITE(PeriodicTileCrops, r, CtsEnforcement::kApiLevel_T, CtsEnforcement::kNextRelease) {
    for (SkTileMode tm : {SkTileMode::kRepeat, SkTileMode::kMirror}) {
        // In these tests, the crop periodically tiles such that it covers the desired output so
        // the prior image can be simply transformed.
        TestCase(r, "Periodic applyCrop() becomes a transform")
                .source({0, 0, 20, 20})
                .applyCrop({5, 5, 15, 15}, tm, Expect::kDeferredImage,
                           /*expectedTileMode=*/SkTileMode::kDecal)
                .run(/*requestedOutput=*/{25, 25, 35, 35});

        TestCase(r, "Periodic applyCrop() with partial transparency still becomes a transform")
                .source({0, 0, 20, 20})
                .applyCrop({-5, -5, 15, 15}, tm, Expect::kDeferredImage,
                           /*expectedTileMode=*/SkTileMode::kDecal,
                           /*expectedBounds=*/tm == SkTileMode::kRepeat ? SkIRect{20,20,35,35}
                                                                        : SkIRect{15,15,30,30})
                .run(/*requestedOutput*/{15, 15, 35, 35});

        TestCase(r, "Periodic applyCrop() after complex transform can still simplify")
                .source({0, 0, 20, 20})
                .applyTransform(SkMatrix::RotateDeg(15.f, {10.f, 10.f}), Expect::kDeferredImage)
                .applyCrop({-5, -5, 25, 25}, tm, Expect::kDeferredImage,
                           /*expectedTileMode=*/SkTileMode::kDecal,
                           /*expectedBounds*/SkIRect{57,57,83,83}) // source+15 degree rotation
                .run(/*requestedOutput=*/{55,55,85,85});

        // In these tests, the crop's periodic boundary intersects with the output so it should not
        // simplify to just a transform.
        TestCase(r, "Periodic applyCrop() with visible edge does not become a transform")
                .source({0, 0, 20, 20})
                .applyCrop({5, 5, 15, 15}, tm, Expect::kDeferredImage)
                .run(/*requestedOutput=*/{10, 10, 20, 20});

        TestCase(r, "Periodic applyCrop() with visible edge and transparency creates new image")
                .source({0, 0, 20, 20})
                .applyCrop({-5, -5, 15, 15}, tm, Expect::kNewImage)
                .run(/*requestedOutput=*/{10, 10, 20, 20});

        TestCase(r, "Periodic applyCropp() with visible edge and complex transform creates image")
                .source({0, 0, 20, 20})
                .applyTransform(SkMatrix::RotateDeg(15.f, {10.f, 10.f}), Expect::kDeferredImage)
                .applyCrop({-5, -5, 25, 25}, tm, Expect::kNewImage)
                .run(/*requestedOutput=*/{20, 20, 50, 50});
    }
}

DEF_TEST_SUITE(DecalThenClamp, r, CtsEnforcement::kApiLevel_T, CtsEnforcement::kNextRelease) {
    TestCase(r, "Decal then clamp crop uses 1px buffer around intersection")
            .source({0, 0, 20, 20})
            .applyCrop({3, 3, 17, 17}, SkTileMode::kDecal, Expect::kDeferredImage)
            .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
            .applyCrop({3, 3, 20, 20}, SkTileMode::kClamp, Expect::kNewImage, SkTileMode::kClamp)
            .run(/*requestedOutput=*/{0, 0, 20, 20});

    TestCase(r, "Decal then clamp crop uses 1px buffer around intersection, w/ alpha color filter")
            .source({0, 0, 20, 20})
            .applyCrop({3, 3, 17, 17}, SkTileMode::kDecal, Expect::kDeferredImage)
            .applyColorFilter(affect_transparent(SkColors::kCyan), Expect::kDeferredImage)
            .applyCrop({0, 0, 17, 17}, SkTileMode::kClamp, Expect::kNewImage, SkTileMode::kClamp)
            .run(/*requestedOutput=*/{0, 0, 20, 20});
}

// ----------------------------------------------------------------------------
// applyTransform() tests

DEF_TEST_SUITE(Transform, r, CtsEnforcement::kApiLevel_T, CtsEnforcement::kNextRelease) {
    TestCase(r, "applyTransform() integer translate")
            .source({0, 0, 10, 10})
            .applyTransform(SkMatrix::Translate(5, 5), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 10, 10});

    TestCase(r, "applyTransform() fractional translate")
            .source({0, 0, 10, 10})
            .applyTransform(SkMatrix::Translate(1.5f, 3.24f), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 10, 10});

    TestCase(r, "applyTransform() scale")
            .source({0, 0, 24, 24})
            .applyTransform(SkMatrix::Scale(2.2f, 3.1f), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{-16, -16, 96, 96});

    // NOTE: complex is anything beyond a scale+translate. See SkImageFilter_Base::MatrixCapability.
    TestCase(r, "applyTransform() with complex transform")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::RotateDeg(10.f, {4.f, 4.f}), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 16, 16});
}

DEF_TEST_SUITE(CompatibleSamplingConcatsTransforms, r, CtsEnforcement::kApiLevel_T,
               CtsEnforcement::kNextRelease) {
    TestCase(r, "linear + linear combine")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkFilterMode::kLinear, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkFilterMode::kLinear, Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    TestCase(r, "equiv. bicubics combine")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkCubicResampler::Mitchell(), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkCubicResampler::Mitchell(), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    TestCase(r, "linear + bicubic becomes bicubic")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkFilterMode::kLinear, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkCubicResampler::Mitchell(), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    TestCase(r, "bicubic + linear becomes bicubic")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkCubicResampler::Mitchell(), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkFilterMode::kLinear, Expect::kDeferredImage,
                            /*expectedSampling=*/SkCubicResampler::Mitchell())
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    TestCase(r, "aniso picks max level to combine")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions::Aniso(4.f), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions::Aniso(2.f), Expect::kDeferredImage,
                            /*expectedSampling=*/SkSamplingOptions::Aniso(4.f))
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    TestCase(r, "aniso picks max level to combine (other direction)")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions::Aniso(2.f), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions::Aniso(4.f), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    TestCase(r, "linear + aniso becomes aniso")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkFilterMode::kLinear, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions::Aniso(2.f), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    TestCase(r, "aniso + linear stays aniso")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions::Aniso(4.f), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkFilterMode::kLinear, Expect::kDeferredImage,
                            /*expectedSampling=*/SkSamplingOptions::Aniso(4.f))
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    // TODO: Add cases for mipmapping once that becomes relevant (SkSpecialImage does not have
    // mipmaps right now).
}

DEF_TEST_SUITE(IncompatibleSamplingResolvesImages, r, CtsEnforcement::kApiLevel_T,
               CtsEnforcement::kNextRelease) {
    TestCase(r, "different bicubics do not combine")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkCubicResampler::Mitchell(), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkCubicResampler::CatmullRom(), Expect::kNewImage)
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    TestCase(r, "nearest + linear do not combine")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkFilterMode::kNearest, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkFilterMode::kLinear, Expect::kNewImage)
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    TestCase(r, "linear + nearest do not combine")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkFilterMode::kLinear, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkFilterMode::kNearest, Expect::kNewImage)
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    TestCase(r, "bicubic + aniso do not combine")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkCubicResampler::Mitchell(), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions::Aniso(4.f), Expect::kNewImage)
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    TestCase(r, "aniso + bicubic do not combine")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkSamplingOptions::Aniso(4.f), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkCubicResampler::Mitchell(), Expect::kNewImage)
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    TestCase(r, "nearest + nearest do not combine")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkFilterMode::kNearest, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkFilterMode::kNearest, Expect::kNewImage)
            .run(/*requestedOutput=*/{0, 0, 16, 16});
}

DEF_TEST_SUITE(IntegerOffsetIgnoresNearestSampling, r, CtsEnforcement::kApiLevel_T,
               CtsEnforcement::kNextRelease) {
    // Bicubic is used here to reflect that it should use the non-NN sampling and just needs to be
    // something other than the default to detect that it got carried through.
    TestCase(r, "integer translate+NN then bicubic combines")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::Translate(2, 2),
                            SkFilterMode::kNearest, Expect::kDeferredImage,
                            FilterResult::kDefaultSampling)
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkCubicResampler::Mitchell(), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    TestCase(r, "bicubic then integer translate+NN combines")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::RotateDeg(2.f, {4.f, 4.f}),
                            SkCubicResampler::Mitchell(), Expect::kDeferredImage)
            .applyTransform(SkMatrix::Translate(2, 2),
                            SkFilterMode::kNearest, Expect::kDeferredImage,
                            /*expectedSampling=*/SkCubicResampler::Mitchell())
            .run(/*requestedOutput=*/{0, 0, 16, 16});
}

// ----------------------------------------------------------------------------
// applyTransform() interacting with applyCrop()

DEF_TEST_SUITE(TransformBecomesEmpty, r, CtsEnforcement::kApiLevel_T,
               CtsEnforcement::kNextRelease) {
    TestCase(r, "Transform moves src image outside of requested output")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::Translate(10.f, 10.f), Expect::kEmptyImage)
            .run(/*requestedOutput=*/{0, 0, 8, 8});

    TestCase(r, "Transform moves src image outside of crop")
            .source({0, 0, 8, 8})
            .applyTransform(SkMatrix::Translate(10.f, 10.f), Expect::kDeferredImage)
            .applyCrop({2, 2, 6, 6}, Expect::kEmptyImage)
            .run(/*requestedOutput=*/{0, 0, 20, 20});

    TestCase(r, "Transform moves cropped image outside of requested output")
            .source({0, 0, 8, 8})
            .applyCrop({1, 1, 4, 4}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::Translate(-5.f, -5.f), Expect::kEmptyImage)
            .run(/*requestedOutput=*/{0, 0, 8, 8});
}

DEF_TEST_SUITE(TransformAndCrop, r, CtsEnforcement::kApiLevel_T, CtsEnforcement::kNextRelease) {
    TestCase(r, "Crop after transform can always apply")
            .source({0, 0, 16, 16})
            .applyTransform(SkMatrix::RotateDeg(45.f, {3.f, 4.f}), Expect::kDeferredImage)
            .applyCrop({2, 2, 15, 15}, Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    // TODO: Expand this test case to be arbitrary float S+T transforms when FilterResult tracks
    // both a srcRect and dstRect.
    TestCase(r, "Crop after translate is lifted to image subset")
            .source({0, 0, 32, 32})
            .applyTransform(SkMatrix::Translate(12.f, 8.f), Expect::kDeferredImage)
            .applyCrop({16, 16, 24, 24}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(45.f, {16.f, 16.f}), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 32, 32});

    TestCase(r, "Transform after unlifted crop triggers new image")
            .source({0, 0, 16, 16})
            .applyTransform(SkMatrix::RotateDeg(45.f, {8.f, 8.f}), Expect::kDeferredImage)
            .applyCrop({1, 1, 15, 15}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(-10.f, {8.f, 4.f}), Expect::kNewImage)
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    TestCase(r, "Transform after unlifted crop with interior output does not trigger new image")
            .source({0, 0, 16, 16})
            .applyTransform(SkMatrix::RotateDeg(45.f, {8.f, 8.f}), Expect::kDeferredImage)
            .applyCrop({1, 1, 15, 15}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(-10.f, {8.f, 4.f}), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{4, 4, 12, 12});

    TestCase(r, "Translate after unlifted crop does not trigger new image")
            .source({0, 0, 16, 16})
            .applyTransform(SkMatrix::RotateDeg(5.f, {8.f, 8.f}), Expect::kDeferredImage)
            .applyCrop({2, 2, 14, 14}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::Translate(4.f, 6.f), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 16, 16});

    TestCase(r, "Transform after large no-op crop does not trigger new image")
            .source({0, 0, 64, 64})
            .applyTransform(SkMatrix::RotateDeg(45.f, {32.f, 32.f}), Expect::kDeferredImage)
            .applyCrop({-64, -64, 128, 128}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(-30.f, {32.f, 32.f}), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 64, 64});
}

DEF_TEST_SUITE(TransformAndTile, r, CtsEnforcement::kApiLevel_T, CtsEnforcement::kNextRelease) {
    // Test interactions of non-decal tile modes and transforms
    for (SkTileMode tm : kTileModes) {
        if (tm == SkTileMode::kDecal) {
            continue;
        }

        TestCase(r, "Transform after tile mode does not trigger new image")
                .source({0, 0, 64, 64})
                .applyCrop({2, 2, 32, 32}, tm, Expect::kDeferredImage)
                .applyTransform(SkMatrix::RotateDeg(20.f, {16.f, 8.f}), Expect::kDeferredImage)
                .run(/*requestedOutput=*/{0, 0, 64, 64});

        TestCase(r, "Integer transform before tile mode does not trigger new image")
                .source({0, 0, 32, 32})
                .applyTransform(SkMatrix::Translate(16.f, 16.f), Expect::kDeferredImage)
                .applyCrop({20, 20, 40, 40}, tm, Expect::kDeferredImage)
                .run(/*requestedOutput=*/{0, 0, 64, 64});

        TestCase(r, "Non-integer transform before tile mode triggers new image")
                .source({0, 0, 50, 40})
                .applyTransform(SkMatrix::RotateDeg(-30.f, {20.f, 10.f}), Expect::kDeferredImage)
                .applyCrop({10, 10, 30, 30}, tm, Expect::kNewImage)
                .run(/*requestedOutput=*/{0, 0, 50, 50});

        TestCase(r, "Non-integer transform before tiling defers image if edges are hidden")
                .source({0, 0, 64, 64})
                .applyTransform(SkMatrix::RotateDeg(45.f, {32.f, 32.f}), Expect::kDeferredImage)
                .applyCrop({10, 10, 50, 50}, tm, Expect::kDeferredImage,
                           /*expectedTileMode=*/SkTileMode::kDecal)
                .run(/*requestedOutput=*/{11, 11, 49, 49});
    }
}

// ----------------------------------------------------------------------------
// applyColorFilter() and interactions with transforms/crops

DEF_TEST_SUITE(ColorFilter, r, CtsEnforcement::kApiLevel_T, CtsEnforcement::kNextRelease) {
    TestCase(r, "applyColorFilter() defers image")
            .source({0, 0, 24, 24})
            .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 32, 32});

    TestCase(r, "applyColorFilter() composes with other color filters")
            .source({0, 0, 24, 24})
            .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
            .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 32, 32});

    TestCase(r, "Transparency-affecting color filter fills output")
            .source({0, 0, 24, 24})
            .applyColorFilter(affect_transparent(SkColors::kBlue), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{-8, -8, 32, 32});

    // Since there is no cropping between the composed color filters, transparency-affecting CFs
    // can still compose together.
    TestCase(r, "Transparency-affecting composition fills output (ATBx2)")
            .source({0, 0, 24, 24})
            .applyColorFilter(affect_transparent(SkColors::kBlue), Expect::kDeferredImage)
            .applyColorFilter(affect_transparent(SkColors::kRed), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{-8, -8, 32, 32});

    TestCase(r, "Transparency-affecting composition fills output (ATB,reg)")
            .source({0, 0, 24, 24})
            .applyColorFilter(affect_transparent(SkColors::kBlue), Expect::kDeferredImage)
            .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{-8, -8, 32, 32});

    TestCase(r, "Transparency-affecting composition fills output (reg,ATB)")
            .source({0, 0, 24, 24})
            .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
            .applyColorFilter(affect_transparent(SkColors::kBlue), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{-8, -8, 32, 32});
}

DEF_TEST_SUITE(TransformedColorFilter, r, CtsEnforcement::kApiLevel_T,
               CtsEnforcement::kNextRelease) {
    TestCase(r, "Transform composes with regular CF")
            .source({0, 0, 24, 24})
            .applyTransform(SkMatrix::RotateDeg(45.f, {12, 12}), Expect::kDeferredImage)
            .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 24, 24});

    TestCase(r, "Regular CF composes with transform")
            .source({0, 0, 24, 24})
            .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(45.f, {12, 12}), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 24, 24});

    TestCase(r, "Transform composes with transparency-affecting CF")
            .source({0, 0, 24, 24})
            .applyTransform(SkMatrix::RotateDeg(45.f, {12, 12}), Expect::kDeferredImage)
            .applyColorFilter(affect_transparent(SkColors::kBlue), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 24, 24});

    // NOTE: Because there is no explicit crop between the color filter and the transform,
    // output bounds propagation means the layer bounds of the applied color filter are never
    // visible post transform. This is detected and allows the transform to be composed without
    // producing an intermediate image. See later tests for when a crop prevents this optimization.
    TestCase(r, "Transparency-affecting CF composes with transform")
            .source({0, 0, 24, 24})
            .applyColorFilter(affect_transparent(SkColors::kBlue), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(45.f, {12, 12}), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{-50, -50, 50, 50});
}

DEF_TEST_SUITE(TransformBetweenColorFilters, r, CtsEnforcement::kApiLevel_T,
               CtsEnforcement::kNextRelease) {
    // NOTE: The lack of explicit crops allows all of these operations to be optimized as well.
    TestCase(r, "Transform between regular color filters")
            .source({0, 0, 24, 24})
            .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(45.f, {12, 12}), Expect::kDeferredImage)
            .applyColorFilter(alpha_modulate(0.75f), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 24, 24});

    TestCase(r, "Transform between transparency-affecting color filters")
            .source({0, 0, 24, 24})
            .applyColorFilter(affect_transparent(SkColors::kBlue), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(45.f, {12, 12}), Expect::kDeferredImage)
            .applyColorFilter(affect_transparent(SkColors::kGreen), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 24, 24});

    TestCase(r, "Transform between ATB and regular color filters")
            .source({0, 0, 24, 24})
            .applyColorFilter(affect_transparent(SkColors::kBlue), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(45.f, {12, 12}), Expect::kDeferredImage)
            .applyColorFilter(alpha_modulate(0.75f), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 24, 24});

    TestCase(r, "Transform between regular and ATB color filters")
            .source({0, 0, 24, 24})
            .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(45.f, {12, 12}), Expect::kDeferredImage)
            .applyColorFilter(affect_transparent(SkColors::kGreen), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 24, 24});
}

DEF_TEST_SUITE(ColorFilterBetweenTransforms, r, CtsEnforcement::kApiLevel_T,
               CtsEnforcement::kNextRelease) {
    TestCase(r, "Regular color filter between transforms")
            .source({0, 0, 24, 24})
            .applyTransform(SkMatrix::RotateDeg(20.f, {12, 12}), Expect::kDeferredImage)
            .applyColorFilter(alpha_modulate(0.8f), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(10.f, {5.f, 8.f}), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 24, 24});

    TestCase(r, "Transparency-affecting color filter between transforms")
            .source({0, 0, 24, 24})
            .applyTransform(SkMatrix::RotateDeg(20.f, {12, 12}), Expect::kDeferredImage)
            .applyColorFilter(affect_transparent(SkColors::kRed), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(10.f, {5.f, 8.f}), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 24, 24});
}

DEF_TEST_SUITE(CroppedColorFilter, r, CtsEnforcement::kApiLevel_T, CtsEnforcement::kNextRelease) {
    for (SkTileMode tm : kTileModes) {
        TestCase(r, "Regular color filter after empty crop stays empty")
                .source({0, 0, 16, 16})
                .applyCrop(SkIRect::MakeEmpty(), tm, Expect::kEmptyImage)
                .applyColorFilter(alpha_modulate(0.2f), Expect::kEmptyImage)
                .run(/*requestedOutput=*/{0, 0, 16, 16});

        TestCase(r, "Transparency-affecting color filter after empty crop creates new image")
                .source({0, 0, 16, 16})
                .applyCrop(SkIRect::MakeEmpty(), tm, Expect::kEmptyImage)
                .applyColorFilter(affect_transparent(SkColors::kRed), Expect::kNewImage,
                                  /*expectedColorFilter=*/nullptr) // CF applied ASAP to new img
                .run(/*requestedOutput=*/{0, 0, 16, 16});

        TestCase(r, "Regular color filter composes with crop")
                .source({0, 0, 32, 32})
                .applyColorFilter(alpha_modulate(0.7f), Expect::kDeferredImage)
                .applyCrop({8, 8, 24, 24}, tm, Expect::kDeferredImage)
                .run(/*requestedOutput=*/{0, 0, 32, 32});

        TestCase(r, "Crop composes with regular color filter")
                .source({0, 0, 32, 32})
                .applyCrop({8, 8, 24, 24}, tm, Expect::kDeferredImage)
                .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
                .run(/*requestedOutput=*/{0, 0, 32, 32});
            // FIXME need to disable the stats tracking for renderExpected() and compare()

        TestCase(r, "Transparency-affecting color filter restricted by crop")
                .source({0, 0, 32, 32})
                .applyColorFilter(affect_transparent(SkColors::kRed), Expect::kDeferredImage)
                .applyCrop({8, 8, 24, 24}, tm, Expect::kDeferredImage)
                .run(/*requestedOutput=*/{0, 0, 32, 32});

        TestCase(r, "Crop composes with transparency-affecting color filter")
                .source({0, 0, 32, 32})
                .applyCrop({8, 8, 24, 24}, tm, Expect::kDeferredImage)
                .applyColorFilter(affect_transparent(SkColors::kRed), Expect::kDeferredImage)
                .run(/*requestedOutput=*/{0, 0, 32, 32});
    }
}

DEF_TEST_SUITE(CropBetweenColorFilters, r, CtsEnforcement::kApiLevel_T,
               CtsEnforcement::kNextRelease) {
    for (SkTileMode tm : kTileModes) {
        TestCase(r, "Crop between regular color filters")
                .source({0, 0, 32, 32})
                .applyColorFilter(alpha_modulate(0.8f), Expect::kDeferredImage)
                .applyCrop({8, 8, 24, 24}, tm, Expect::kDeferredImage)
                .applyColorFilter(alpha_modulate(0.4f), Expect::kDeferredImage)
                .run(/*requestedOutput=*/{0, 0, 32, 32});

        if (tm == SkTileMode::kDecal) {
            TestCase(r, "Crop between transparency-affecting color filters requires new image")
                    .source({0, 0, 32, 32})
                    .applyColorFilter(affect_transparent(SkColors::kGreen), Expect::kDeferredImage)
                    .applyCrop({8, 8, 24, 24}, SkTileMode::kDecal, Expect::kDeferredImage)
                    .applyColorFilter(affect_transparent(SkColors::kRed), Expect::kNewImage)
                    .run(/*requestedOutput=*/{0, 0, 32, 32});

            TestCase(r, "Output-constrained crop between transparency-affecting filters does not")
                    .source({0, 0, 32, 32})
                    .applyColorFilter(affect_transparent(SkColors::kGreen), Expect::kDeferredImage)
                    .applyCrop({8, 8, 24, 24}, SkTileMode::kDecal, Expect::kDeferredImage)
                    .applyColorFilter(affect_transparent(SkColors::kRed), Expect::kDeferredImage)
                    .run(/*requestedOutput=*/{8, 8, 24, 24});
        } else {
            TestCase(r, "Tiling between transparency-affecting color filters defers image")
                    .source({0, 0, 32, 32})
                    .applyColorFilter(affect_transparent(SkColors::kGreen), Expect::kDeferredImage)
                    .applyCrop({8, 8, 24, 24}, tm, Expect::kDeferredImage)
                    .applyColorFilter(affect_transparent(SkColors::kRed), Expect::kDeferredImage)
                    .run(/*requestedOutput=*/{0, 0, 32, 32});
        }

        TestCase(r, "Crop between regular and ATB color filters")
                .source({0, 0, 32, 32})
                .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
                .applyCrop({8, 8, 24, 24}, tm, Expect::kDeferredImage)
                .applyColorFilter(affect_transparent(SkColors::kRed), Expect::kDeferredImage)
                .run(/*requestedOutput=*/{0, 0, 32, 32});

        TestCase(r, "Crop between ATB and regular color filters")
                .source({0, 0, 32, 32})
                .applyColorFilter(affect_transparent(SkColors::kRed), Expect::kDeferredImage)
                .applyCrop({8, 8, 24, 24}, tm, Expect::kDeferredImage)
                .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
                .run(/*requestedOutput=*/{0, 0, 32, 32});
    }
}

DEF_TEST_SUITE(ColorFilterBetweenCrops, r, CtsEnforcement::kApiLevel_T,
               CtsEnforcement::kNextRelease) {
    for (SkTileMode firstTM : kTileModes) {
        for (SkTileMode secondTM : kTileModes) {
            Expect newImageIfNotDecalOrDoubleClamp =
                    secondTM != SkTileMode::kDecal &&
                    !(secondTM == SkTileMode::kClamp && firstTM == SkTileMode::kClamp) ?
                            Expect::kNewImage : Expect::kDeferredImage;

            TestCase(r, "Regular color filter between crops")
                    .source({0, 0, 32, 32})
                    .applyCrop({4, 4, 24, 24}, firstTM, Expect::kDeferredImage)
                    .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
                    .applyCrop({15, 15, 32, 32}, secondTM, newImageIfNotDecalOrDoubleClamp,
                               secondTM == SkTileMode::kDecal ? firstTM : secondTM)
                    .run(/*requestedOutput=*/{0, 0, 32, 32});

            TestCase(r, "Transparency-affecting color filter between crops")
                    .source({0, 0, 32, 32})
                    .applyCrop({4, 4, 24, 24}, firstTM, Expect::kDeferredImage)
                    .applyColorFilter(affect_transparent(SkColors::kGreen), Expect::kDeferredImage)
                    .applyCrop({15, 15, 32, 32}, secondTM, newImageIfNotDecalOrDoubleClamp,
                               secondTM == SkTileMode::kDecal ? firstTM : secondTM)
                    .run(/*requestedOutput=*/{0, 0, 32, 32});
        }
    }
}

DEF_TEST_SUITE(CroppedTransformedColorFilter, r, CtsEnforcement::kApiLevel_T,
               CtsEnforcement::kNextRelease) {
    TestCase(r, "Transform -> crop -> regular color filter")
            .source({0, 0, 32, 32})
            .applyTransform(SkMatrix::RotateDeg(30.f, {16, 16}), Expect::kDeferredImage)
            .applyCrop({2, 2, 30, 30}, Expect::kDeferredImage)
            .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 32, 32});

    TestCase(r, "Transform -> regular color filter -> crop")
            .source({0, 0, 32, 32})
            .applyTransform(SkMatrix::RotateDeg(30.f, {16, 16}), Expect::kDeferredImage)
            .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
            .applyCrop({2, 2, 30, 30}, Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 32, 32});

    TestCase(r, "Crop -> transform -> regular color filter")
            .source({0, 0, 32, 32})
            .applyCrop({2, 2, 30, 30}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(30.f, {16, 16}), Expect::kDeferredImage)
            .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 32, 32});

    TestCase(r, "Crop -> regular color filter -> transform")
            .source({0, 0, 32, 32})
            .applyCrop({2, 2, 30, 30}, Expect::kDeferredImage)
            .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(30.f, {16, 16}), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 32, 32});

    TestCase(r, "Regular color filter -> transform -> crop")
            .source({0, 0, 32, 32})
            .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(30.f, {16, 16}), Expect::kDeferredImage)
            .applyCrop({2, 2, 30, 30}, Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 32, 32});

    TestCase(r, "Regular color filter -> crop -> transform")
            .source({0, 0, 32, 32})
            .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
            .applyCrop({2, 2, 30, 30}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(30.f, {16, 16}), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 32, 32});
}

DEF_TEST_SUITE(CroppedTransformedTransparencyAffectingColorFilter, r, CtsEnforcement::kApiLevel_T,
               CtsEnforcement::kNextRelease) {
    // When the crop is not between the transform and transparency-affecting color filter,
    // either the order of operations or the bounds propagation means that every action can be
    // deferred. Below, when the crop is between the two actions, new images are triggered.
    TestCase(r, "Transform -> transparency-affecting color filter -> crop")
            .source({0, 0, 32, 32})
            .applyTransform(SkMatrix::RotateDeg(30.f, {16, 16}), Expect::kDeferredImage)
            .applyColorFilter(affect_transparent(SkColors::kGreen), Expect::kDeferredImage)
            .applyCrop({2, 2, 30, 30}, Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 32, 32});

    TestCase(r, "Crop -> transform -> transparency-affecting color filter")
            .source({0, 0, 32, 32})
            .applyCrop({2, 2, 30, 30}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(30.f, {16, 16}), Expect::kDeferredImage)
            .applyColorFilter(affect_transparent(SkColors::kGreen), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 32, 32});

    TestCase(r, "Crop -> transparency-affecting color filter -> transform")
            .source({0, 0, 32, 32})
            .applyCrop({2, 2, 30, 30}, Expect::kDeferredImage)
            .applyColorFilter(affect_transparent(SkColors::kGreen), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(30.f, {16, 16}), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 32, 32});

    TestCase(r, "Transparency-affecting color filter -> transform -> crop")
            .source({0, 0, 32, 32})
            .applyColorFilter(affect_transparent(SkColors::kGreen), Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(30.f, {16, 16}), Expect::kDeferredImage)
            .applyCrop({2, 2, 30, 30}, Expect::kDeferredImage)
            .run(/*requestedOutput=*/{0, 0, 32, 32});

    // Since the crop is between the transform and color filter (or vice versa), transparency
    // outside the crop is introduced that should not be affected by the color filter were no
    // new image to be created.
    TestCase(r, "Transform -> crop -> transparency-affecting color filter")
            .source({0, 0, 32, 32})
            .applyTransform(SkMatrix::RotateDeg(30.f, {16, 16}), Expect::kDeferredImage)
            .applyCrop({2, 2, 30, 30}, Expect::kDeferredImage)
            .applyColorFilter(affect_transparent(SkColors::kGreen), Expect::kNewImage)
            .run(/*requestedOutput=*/{0, 0, 32, 32});

    TestCase(r, "Transparency-affecting color filter -> crop -> transform")
            .source({0, 0, 32, 32})
            .applyColorFilter(affect_transparent(SkColors::kGreen), Expect::kDeferredImage)
            .applyCrop({2, 2, 30, 30}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(30.f, {16, 16}), Expect::kNewImage)
            .run(/*requestedOutput=*/{0, 0, 32, 32});

    // However if the output is small enough to fit within the transformed interior, the
    // transparency is not visible.
    TestCase(r, "Transform -> crop -> transparency-affecting color filter")
            .source({0, 0, 32, 32})
            .applyTransform(SkMatrix::RotateDeg(30.f, {16, 16}), Expect::kDeferredImage)
            .applyCrop({2, 2, 30, 30}, Expect::kDeferredImage)
            .applyColorFilter(affect_transparent(SkColors::kGreen), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{15, 15, 21, 21});

    TestCase(r, "Transparency-affecting color filter -> crop -> transform")
            .source({0, 0, 32, 32})
            .applyColorFilter(affect_transparent(SkColors::kGreen), Expect::kDeferredImage)
            .applyCrop({2, 2, 30, 30}, Expect::kDeferredImage)
            .applyTransform(SkMatrix::RotateDeg(30.f, {16, 16}), Expect::kDeferredImage)
            .run(/*requestedOutput=*/{15, 15, 21, 21});
}

DEF_TEST_SUITE(BackdropFilterRotated, r,
               CtsEnforcement::kNextRelease, CtsEnforcement::kNextRelease) {
    // These values are extracted from a cc_unittest that had a 200x200 image, with a 10-degree
    // rotated 100x200 layer over the right half of the base image, with a backdrop blur. The
    // rotation forces SkCanvas to crop and transform the base device's content to be aligned with
    // the layer space of the blur. The rotation is such that the backdrop image must be clamped
    // (hence the first crop) and the clamp tiling remains visible in the layer image. However,
    // floating point precision in the layer bounds analysis was causing FilterResult to think that
    // the layer decal was also visible so the first crop would be resolved before the transform was
    // applied.
    //
    // While it's expected that the second clamping crop (part of the blur effect), forces the
    // transform and first clamp to be resolved, we were incorrectly producing two new images
    // instead of just one.
    TestCase(r, "Layer decal shouldn't be visible")
            .source({65, 0, 199, 200})
            .applyCrop({65, 0, 199, 200}, SkTileMode::kClamp, Expect::kDeferredImage)
            .applyTransform(SkMatrix::MakeAll( 0.984808f, 0.173648f, -98.4808f,
                                              -0.173648f, 0.984808f,  17.3648f,
                                               0.000000f, 0.000000f,   1.0000f),
                            Expect::kDeferredImage)
            .applyCrop({0, 0, 100, 200}, SkTileMode::kClamp, Expect::kNewImage)
            .run(/*requestedOutput=*/{-15, -15, 115, 215});
}

// Nearly identity rescales are treated as the identity
static constexpr SkSize kNearlyIdentity = {0.999f, 0.999f};

DEF_TEST_SUITE(RescaleWithTileMode, r,
               CtsEnforcement::kNextRelease, CtsEnforcement::kNextRelease) {
    for (SkTileMode tm : kTileModes) {
        TestCase(r, "Identity rescale is a no-op")
                .source({0, 0, 50, 50})
                .applyCrop({0, 0, 50, 50}, tm, Expect::kDeferredImage)
                .rescale({1.f, 1.f}, Expect::kDeferredImage)
                .run(/*requestedOutput=*/{-5, -5, 55, 55});

        TestCase(r, "Near identity rescale is a no-op",
                 kDefaultMaxAllowedPercentImageDiff,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 1 : 0)
                .source({0, 0, 50, 50})
                .applyCrop({0, 0, 50, 50}, tm, Expect::kDeferredImage)
                .rescale(kNearlyIdentity, Expect::kDeferredImage)
                .run(/*requestedOutput=*/{-5, -5, 55, 55});

        // NOTE: As the scale factor decreases and more decimation steps are required, the testing
        // allowed tolerances increase greatly. These were chosen as "acceptable" after reviewing
        // the expected vs. actual images. The results diverge due to differences in the simple
        // expected decimation and the actual rescale() implementation, as well as how small the
        // final images become.
        //
        // Similarly, the allowed transparent border tolerance must be increased for kDecal tests
        // because the expected image's content is expanded by a larger and larger factor during its
        // upscale.

        // kDecal tiling is applied during the downsample by writing a transparent buffer, so the
        // tile mode can simplify to kClamp (which is more efficient for shader-based tiling).
        const SkTileMode expectedTileMode = tm == SkTileMode::kDecal ? SkTileMode::kClamp : tm;
        TestCase(r, "1-step rescale preserves tile mode",
                 kDefaultMaxAllowedPercentImageDiff,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 1 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .rescale({0.5f, 0.5f}, Expect::kNewImage, expectedTileMode)
                .run(/*requestedOutput=*/{0, 0, 80, 80});

        const bool periodic = tm == SkTileMode::kRepeat || tm == SkTileMode::kMirror;
        TestCase(r, "2-step rescale preserves tile mode",
                 /*allowedPercentImageDiff=*/tm == SkTileMode::kDecal ? 5.9f
                                                                      : periodic ? 2.5f : 1.f,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 2 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .rescale({0.25f, 0.25f}, Expect::kNewImage, expectedTileMode)
                .run(/*requestedOutput=*/{0, 0, 80, 80});

        TestCase(r, "2-step rescale with near-identity elision",
                 /*allowedPercentImageDiff=*/tm == SkTileMode::kDecal ? 37.46f
                                                                      : periodic ? 73.85 : 52.2f,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 6 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .rescale({0.22f, 0.22f}, Expect::kNewImage, expectedTileMode)
                .run(/*requestedOutput=*/{0, 0, 80, 80});

        TestCase(r, "3-step rescale preserves tile mode",
                 /*allowedPercentImageDiff=*/tm == SkTileMode::kDecal ? 49.f
                                                                      : periodic ? 90.5f : 71.f,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 10 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .rescale({0.155f, 0.155f}, Expect::kNewImage, expectedTileMode)
                .run(/*requestedOutput=*/{0, 0, 80, 80});

        // Non-uniform scales
        TestCase(r, "Identity X axis, near-identity Y axis is a no-op",
                 kDefaultMaxAllowedPercentImageDiff,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 1 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .rescale({1.f, kNearlyIdentity.height()}, Expect::kDeferredImage)
                .run(/*requestedOutput=*/{0, 0, 80, 80});
        TestCase(r, "Near-identity X axis, identity Y axis is a no-op",
                 kDefaultMaxAllowedPercentImageDiff,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 1 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .rescale({kNearlyIdentity.width(), 1.f}, Expect::kDeferredImage)
                .run(/*requestedOutput=*/{0, 0, 80, 80});

        TestCase(r, "Identity X axis, 1-step Y axis preserves tile mode",
                 /*allowedPercentImageDiff=*/tm == SkTileMode::kMirror ? 1.2f : 1.f,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 1 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .rescale({1.f, 0.5f}, Expect::kNewImage, expectedTileMode)
                .run(/*requestedOutput=*/{0, 0, 80, 80});
        TestCase(r, "Near-identity X axis, 1-step Y axis preserves tile mode",
                 /*allowedPercentImageDiff=*/tm == SkTileMode::kMirror ? 1.7f : 1.f,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 1 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .rescale({kNearlyIdentity.width(), 0.5f}, Expect::kNewImage, expectedTileMode)
                .run(/*requestedOutput=*/{0, 0, 80, 80});
        TestCase(r, "Identity X axis, 2-step Y axis preserves tile mode",
                 /*allowedPercentImageDiff=*/3.1f,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 2 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .rescale({1.f, 0.25f}, Expect::kNewImage, expectedTileMode)
                .run(/*requestedOutput=*/{0, 0, 80, 80});
        TestCase(r, "1-step X axis, 2-step Y axis preserves tile mode",
                 /*allowedPercentImageDiff=*/tm == SkTileMode::kDecal ? 22.f
                                                                      : periodic ? 55.f : 29.5f,
                /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 5 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .rescale({.55f, 0.27f}, Expect::kNewImage, expectedTileMode)
                .run(/*requestedOutput=*/{0, 0, 80, 80});

        TestCase(r, "1-step X axis, identity Y axis preserves tile mode",
                 /*allowedPercentImageDiff=*/tm == SkTileMode::kMirror ? 1.2f : 1.f,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 1 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .rescale({0.5f, 1.f}, Expect::kNewImage, expectedTileMode)
                .run(/*requestedOutput=*/{0, 0, 80, 80});
        TestCase(r, "1-step X axis, near-identity Y axis preserves tile mode",
                 /*allowedPercentImageDiff=*/tm == SkTileMode::kMirror ? 1.7f : 1.f,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 1 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .rescale({0.5f, kNearlyIdentity.height()}, Expect::kNewImage, expectedTileMode)
                .run(/*requestedOutput=*/{0, 0, 80, 80});
        TestCase(r, "2-step X axis, identity Y axis preserves tile mode",
                 /*allowedPercentImageDiff=*/3.1f,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 2 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .rescale({0.25f, 1.f}, Expect::kNewImage, expectedTileMode)
                .run(/*requestedOutput=*/{0, 0, 80, 80});
        TestCase(r, "2-step X axis, 1-step Y axis preserves tile mode",
                 /*allowedPercentImageDiff=*/tm == SkTileMode::kDecal ? 22.f
                                                                      : periodic ? 55.f : 29.5f,
                /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 5 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .rescale({.27f, 0.55f}, Expect::kNewImage, expectedTileMode)
                .run(/*requestedOutput=*/{0, 0, 80, 80});

        // Chained decal tile modes don't create the circumstances of interest.
        if (tm == SkTileMode::kDecal) {
            continue;
        }
        TestCase(r, "Rescale applies layer bounds",
                 kDefaultMaxAllowedPercentImageDiff,
                 /*transparentCheckBorderTolerance=*/1)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .applyCrop({4, 4, 76, 76}, SkTileMode::kDecal, Expect::kDeferredImage,
                           /*expectedTileMode=*/tm)
                .rescale({0.5f, 0.5f}, Expect::kNewImage, /*expectedTileMode=*/SkTileMode::kClamp)
                .run(/*requestedOutput=*/{0, 0, 80, 80});
    }
}

DEF_TEST_SUITE(RescaleWithTransform, r,
               CtsEnforcement::kNextRelease, CtsEnforcement::kNextRelease) {
    for (SkTileMode tm : kTileModes) {
        TestCase(r, "Identity rescale defers integer translation")
                .source({0, 0, 50, 50})
                .applyCrop({0, 0, 50, 50}, SkTileMode::kMirror, Expect::kDeferredImage)
                .applyTransform(SkMatrix::Translate(-10.f, -10.f), Expect::kDeferredImage)
                .rescale({1.f, 1.f}, Expect::kDeferredImage)
                .run(/*requestedOutput=*/{-15, -15, 45, 45});

        TestCase(r, "Identity rescale applies complex transform")
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .applyTransform(SkMatrix::RotateDeg(45.f, {16.f, 16.f}), Expect::kDeferredImage)
                .rescale({1.f, 1.f}, Expect::kNewImage, SkTileMode::kClamp)
                .run(/*requestedOutput=*/{0, 0, 80, 80});

        TestCase(r, "Near-identity rescale defers integer translation")
                .source({0, 0, 50, 50})
                .applyCrop({0, 0, 50, 50}, SkTileMode::kMirror, Expect::kDeferredImage)
                .applyTransform(SkMatrix::Translate(-10.f, -10.f), Expect::kDeferredImage)
                .rescale(kNearlyIdentity, Expect::kDeferredImage)
                .run(/*requestedOutput=*/{-15, -15, 45, 45});

        TestCase(r, "Near-identity rescale applies complex transform")
                .source({0, 0, 50, 50})
                .applyCrop({0, 0, 50, 50}, SkTileMode::kClamp, Expect::kDeferredImage)
                .applyTransform(SkMatrix::RotateDeg(15.f, {25.f, 25.f}), Expect::kDeferredImage)
                .rescale(kNearlyIdentity, Expect::kNewImage, SkTileMode::kClamp)
                .run(/*requestedOutput=*/{-5, -5, 55, 55});

        TestCase(r, "1-step rescale applies complex transform in first step",
                 kDefaultMaxAllowedPercentImageDiff,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 1 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .applyTransform(SkMatrix::RotateDeg(45.f, {16.f, 16.f}), Expect::kDeferredImage)
                .rescale({0.5f, 0.5f}, Expect::kNewImage, SkTileMode::kClamp)
                .run(/*requestedOutput=*/{0, 0, 80, 80});

        const bool periodic = tm == SkTileMode::kRepeat || tm == SkTileMode::kMirror;
        TestCase(r, "2-step rescale applies complex transform",
                 /*allowedPercentImageDiff=*/periodic ? 6.72f : 1.61f,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 4 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .applyTransform(SkMatrix::RotateDeg(45.f, {16.f, 16.f}), Expect::kDeferredImage)
                .rescale({0.25f, 0.25f}, Expect::kNewImage, /*expectedTileMode=*/SkTileMode::kClamp)
                .run(/*requestedOutput=*/{0, 0, 80, 80});
    }
}

DEF_TEST_SUITE(RescaleWithColorFilter, r,
               CtsEnforcement::kNextRelease, CtsEnforcement::kNextRelease) {
    for (SkTileMode tm : kTileModes) {
        // The color filter (simple and transparency-affecting) should be applied with a 1px
        // boundary around the rest of the image being rescaled when decal-tiled, so its result is
        // clamped tiled instead (vs. having to prepare and scale a larger, flood-filled image).
        SkTileMode expectedTileMode = tm == SkTileMode::kDecal ? SkTileMode::kClamp : tm;

        TestCase(r, "Identity rescale applies color filter but defers tile mode")
                .source({0, 0, 50, 50})
                .applyCrop({0, 0, 50, 50}, tm, Expect::kDeferredImage)
                .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
                .rescale({1.f, 1.f}, Expect::kNewImage, expectedTileMode)
                .run(/*requestedOutput=*/{-5, -5, 55, 55});

        TestCase(r, "Near-identity rescale applies color filter but defers tile mode",
                 kDefaultMaxAllowedPercentImageDiff,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 1 : 0)
                .source({0, 0, 50, 50})
                .applyCrop({0, 0, 50, 50}, tm, Expect::kDeferredImage)
                .applyColorFilter(alpha_modulate(0.5f), Expect::kDeferredImage)
                .rescale(kNearlyIdentity, Expect::kNewImage, expectedTileMode)
                .run(/*requestedOutput=*/{-5, -5, 55, 55});

        TestCase(r, "Rescale applies color filter but defers tile mode",
                 kDefaultMaxAllowedPercentImageDiff,
                 /*transparentCheckBorderTolerance=*/tm == SkTileMode::kDecal ? 1 : 0)
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .applyColorFilter(alpha_modulate(0.75f), Expect::kDeferredImage)
                .rescale({0.5f, 0.5f}, Expect::kNewImage, expectedTileMode)
                .run(/*requestedOutput=*/{0, 0, 80, 80});

        TestCase(r, "Rescale applies transparency-affecting color filter but defers tile mode")
                .source({16, 16, 64, 64})
                .applyCrop({16, 16, 64, 64}, tm, Expect::kDeferredImage)
                .applyColorFilter(affect_transparent(SkColors::kGreen), Expect::kDeferredImage)
                .rescale({0.5f, 0.5f}, Expect::kNewImage, expectedTileMode)
                .run(/*requestedOutput=*/{0, 0, 80, 80});
    }
}

} // anonymous namespace
