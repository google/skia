/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkRasterHandleAllocator.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/core/SkVertices.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkSafe32.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "include/private/chromium/Slug.h"
#include "include/utils/SkNoDrawCanvas.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkMSAN.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkBlurMaskFilterImpl.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkDevice.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSurfacePriv.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkVerticesPriv.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"
#include "src/image/SkSurface_Base.h"
#include "src/text/GlyphRun.h"
#include "src/utils/SkPatchUtils.h"

#include <algorithm>
#include <memory>
#include <new>
#include <optional>
#include <tuple>
#include <utility>

#define RETURN_ON_NULL(ptr)     do { if (nullptr == (ptr)) return; } while (0)
#define RETURN_ON_FALSE(pred)   do { if (!(pred)) return; } while (0)

// This is a test: static_assert with no message is a c++17 feature,
// and std::max() is constexpr only since the c++14 stdlib.
static_assert(std::max(3,4) == 4);

using Slug = sktext::gpu::Slug;

///////////////////////////////////////////////////////////////////////////////////////////////////

SK_MAKE_BITMASK_OPS(SkCanvas::PredrawFlags)

/*
 *  Return true if the drawing this rect would hit every pixels in the canvas.
 *
 *  Returns false if
 *  - rect does not contain the canvas' bounds
 *  - paint is not fill
 *  - paint would blur or otherwise change the coverage of the rect
 */
bool SkCanvas::wouldOverwriteEntireSurface(const SkRect* rect, const SkPaint* paint,
                                           SkEnumBitMask<PredrawFlags> flags) const {
    // Convert flags to a ShaderOverrideOpacity enum
    auto overrideOpacity = (flags & PredrawFlags::kOpaqueShaderOverride) ?
                                    SkPaintPriv::kOpaque_ShaderOverrideOpacity :
                           (flags & PredrawFlags::kNonOpaqueShaderOverride) ?
                                    SkPaintPriv::kNotOpaque_ShaderOverrideOpacity :
                                    SkPaintPriv::kNone_ShaderOverrideOpacity;

    const SkISize size = this->getBaseLayerSize();
    const SkRect bounds = SkRect::MakeIWH(size.width(), size.height());

    // if we're clipped at all, we can't overwrite the entire surface
    {
        const SkDevice* root = this->rootDevice();
        const SkDevice* top = this->topDevice();
        if (root != top) {
            return false;   // we're in a saveLayer, so conservatively don't assume we'll overwrite
        }
        if (!root->isClipWideOpen()) {
            return false;
        }
    }

    if (rect) {
        if (!this->getTotalMatrix().isScaleTranslate()) {
            return false; // conservative
        }

        SkRect devRect;
        this->getTotalMatrix().mapRectScaleTranslate(&devRect, *rect);
        if (!devRect.contains(bounds)) {
            return false;
        }
    }

    if (paint) {
        SkPaint::Style paintStyle = paint->getStyle();
        if (!(paintStyle == SkPaint::kFill_Style ||
              paintStyle == SkPaint::kStrokeAndFill_Style)) {
            return false;
        }
        if (paint->getMaskFilter() || paint->getPathEffect() || paint->getImageFilter()) {
            return false; // conservative
        }
    }
    return SkPaintPriv::Overwrites(paint, overrideOpacity);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkCanvas::predrawNotify(bool willOverwritesEntireSurface) {
    if (fSurfaceBase) {
        if (!fSurfaceBase->aboutToDraw(willOverwritesEntireSurface
                                       ? SkSurface::kDiscard_ContentChangeMode
                                       : SkSurface::kRetain_ContentChangeMode)) {
            return false;
        }
    }
    return true;
}

bool SkCanvas::predrawNotify(const SkRect* rect, const SkPaint* paint,
                             SkEnumBitMask<PredrawFlags> flags) {
    if (fSurfaceBase) {
        SkSurface::ContentChangeMode mode = SkSurface::kRetain_ContentChangeMode;
        // Since willOverwriteAllPixels() may not be complete free to call, we only do so if
        // there is an outstanding snapshot, since w/o that, there will be no copy-on-write
        // and therefore we don't care which mode we're in.
        //
        if (fSurfaceBase->outstandingImageSnapshot()) {
            if (this->wouldOverwriteEntireSurface(rect, paint, flags)) {
                mode = SkSurface::kDiscard_ContentChangeMode;
            }
        }
        if (!fSurfaceBase->aboutToDraw(mode)) {
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

SkCanvas::Layer::Layer(sk_sp<SkDevice> device,
                       FilterSpan imageFilters,
                       const SkPaint& paint,
                       bool isCoverage,
                       bool includesPadding)
        : fDevice(std::move(device))
        , fImageFilters(imageFilters.data(), imageFilters.size())
        , fPaint(paint)
        , fIsCoverage(isCoverage)
        , fDiscard(false)
        , fIncludesPadding(includesPadding) {
    SkASSERT(fDevice);
    // Any image filter should have been pulled out and stored in 'imageFilter' so that 'paint'
    // can be used as-is to draw the result of the filter to the dst device.
    SkASSERT(!fPaint.getImageFilter());
}

SkCanvas::BackImage::BackImage(sk_sp<SkSpecialImage> img, SkIPoint loc)
                               :fImage(img), fLoc(loc) {}
SkCanvas::BackImage::BackImage(const BackImage&) = default;
SkCanvas::BackImage::BackImage(BackImage&&) = default;
SkCanvas::BackImage& SkCanvas::BackImage::operator=(const BackImage&) = default;
SkCanvas::BackImage::~BackImage() = default;

SkCanvas::MCRec::MCRec(SkDevice* device) : fDevice(device) {
    SkASSERT(fDevice);
}

SkCanvas::MCRec::MCRec(const MCRec* prev) : fDevice(prev->fDevice), fMatrix(prev->fMatrix) {
    SkASSERT(fDevice);
}

SkCanvas::MCRec::~MCRec() {}

void SkCanvas::MCRec::newLayer(sk_sp<SkDevice> layerDevice,
                               FilterSpan filters,
                               const SkPaint& restorePaint,
                               bool layerIsCoverage,
                               bool includesPadding) {
    SkASSERT(!fBackImage);
    fLayer = std::make_unique<Layer>(std::move(layerDevice),
                                     filters,
                                     restorePaint,
                                     layerIsCoverage,
                                     includesPadding);
    fDevice = fLayer->fDevice.get();
}

void SkCanvas::MCRec::reset(SkDevice* device) {
    SkASSERT(!fLayer);
    SkASSERT(device);
    SkASSERT(fDeferredSaveCount == 0);
    fDevice = device;
    fMatrix.setIdentity();
}

class SkCanvas::AutoUpdateQRBounds {
public:
    explicit AutoUpdateQRBounds(SkCanvas* canvas) : fCanvas(canvas) {
        // pre-condition, fQuickRejectBounds and other state should be valid before anything
        // modifies the device's clip.
        fCanvas->validateClip();
    }
    ~AutoUpdateQRBounds() {
        fCanvas->fQuickRejectBounds = fCanvas->computeDeviceClipBounds();
        // post-condition, we should remain valid after re-computing the bounds
        fCanvas->validateClip();
    }

private:
    SkCanvas* fCanvas;

    AutoUpdateQRBounds(AutoUpdateQRBounds&&) = delete;
    AutoUpdateQRBounds(const AutoUpdateQRBounds&) = delete;
    AutoUpdateQRBounds& operator=(AutoUpdateQRBounds&&) = delete;
    AutoUpdateQRBounds& operator=(const AutoUpdateQRBounds&) = delete;
};

/////////////////////////////////////////////////////////////////////////////

std::optional<AutoLayerForImageFilter> SkCanvas::aboutToDraw(
        const SkPaint& paint,
        const SkRect* rawBounds,
        SkEnumBitMask<PredrawFlags> flags) {
    if (flags & PredrawFlags::kCheckForOverwrite) {
        if (!this->predrawNotify(rawBounds, &paint, flags)) {
            return std::nullopt;
        }
    } else {
        if (!this->predrawNotify()) {
            return std::nullopt;
        }
    }

    // TODO: Eventually all devices will use this code path and this will just test 'flags'.
    const bool skipMaskFilterLayer = (flags & PredrawFlags::kSkipMaskFilterAutoLayer) ||
                                     !this->topDevice()->useDrawCoverageMaskForMaskFilters();
    return std::optional<AutoLayerForImageFilter>(
            std::in_place, this, paint, rawBounds, skipMaskFilterLayer);
}

std::optional<AutoLayerForImageFilter> SkCanvas::aboutToDraw(
        const SkPaint& paint,
        const SkRect* rawBounds) {
    return this->aboutToDraw(paint, rawBounds, PredrawFlags::kNone);
}

////////////////////////////////////////////////////////////////////////////

void SkCanvas::resetForNextPicture(const SkIRect& bounds) {
    this->restoreToCount(1);

    // We're peering through a lot of structs here.  Only at this scope do we know that the device
    // is a SkNoPixelsDevice.
    SkASSERT(fRootDevice->isNoPixelsDevice());
    SkNoPixelsDevice* asNoPixelsDevice = static_cast<SkNoPixelsDevice*>(fRootDevice.get());
    if (!asNoPixelsDevice->resetForNextPicture(bounds)) {
        fRootDevice = sk_make_sp<SkNoPixelsDevice>(bounds,
                                                   fRootDevice->surfaceProps(),
                                                   fRootDevice->imageInfo().refColorSpace());
    }

    fMCRec->reset(fRootDevice.get());
    fQuickRejectBounds = this->computeDeviceClipBounds();
}

void SkCanvas::init(sk_sp<SkDevice> device) {
    if (!device) {
        device = sk_make_sp<SkNoPixelsDevice>(SkIRect::MakeEmpty(), fProps);
    }

    // From this point on, SkCanvas will always have a device
    SkASSERT(device);

    fSaveCount = 1;
    fMCRec = new (fMCStack.push_back()) MCRec(device.get());

    // The root device and the canvas should always have the same pixel geometry
    SkASSERT(fProps.pixelGeometry() == device->surfaceProps().pixelGeometry());

    fSurfaceBase = nullptr;
    fRootDevice = std::move(device);
    fScratchGlyphRunBuilder = std::make_unique<sktext::GlyphRunBuilder>();
    fQuickRejectBounds = this->computeDeviceClipBounds();
}

SkCanvas::SkCanvas() : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage)) {
    this->init(nullptr);
}

SkCanvas::SkCanvas(int width, int height, const SkSurfaceProps* props)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
        , fProps(SkSurfacePropsCopyOrDefault(props)) {
    this->init(sk_make_sp<SkNoPixelsDevice>(
            SkIRect::MakeWH(std::max(width, 0), std::max(height, 0)), fProps));
}

SkCanvas::SkCanvas(const SkIRect& bounds)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage)) {
    SkIRect r = bounds.isEmpty() ? SkIRect::MakeEmpty() : bounds;
    this->init(sk_make_sp<SkNoPixelsDevice>(r, fProps));
}

SkCanvas::SkCanvas(sk_sp<SkDevice> device)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
        , fProps(device->surfaceProps()) {
    this->init(std::move(device));
}

SkCanvas::~SkCanvas() {
    // Mark all pending layers to be discarded during restore (rather than drawn)
    SkDeque::Iter iter(fMCStack, SkDeque::Iter::kFront_IterStart);
    for (;;) {
        MCRec* rec = (MCRec*)iter.next();
        if (!rec) {
            break;
        }
        if (rec->fLayer) {
            rec->fLayer->fDiscard = true;
        }
    }

    // free up the contents of our deque
    this->restoreToCount(1);    // restore everything but the last
    this->internalRestore();    // restore the last, since we're going away
}

SkSurface* SkCanvas::getSurface() const {
    return fSurfaceBase;
}

SkISize SkCanvas::getBaseLayerSize() const {
    return this->rootDevice()->imageInfo().dimensions();
}

SkDevice* SkCanvas::topDevice() const {
    SkASSERT(fMCRec->fDevice);
    return fMCRec->fDevice;
}

bool SkCanvas::readPixels(const SkPixmap& pm, int x, int y) {
    return pm.addr() && this->rootDevice()->readPixels(pm, x, y);
}

bool SkCanvas::readPixels(const SkImageInfo& dstInfo, void* dstP, size_t rowBytes, int x, int y) {
    return this->readPixels({ dstInfo, dstP, rowBytes}, x, y);
}

bool SkCanvas::readPixels(const SkBitmap& bm, int x, int y) {
    SkPixmap pm;
    return bm.peekPixels(&pm) && this->readPixels(pm, x, y);
}

bool SkCanvas::writePixels(const SkBitmap& bitmap, int x, int y) {
    SkPixmap pm;
    if (bitmap.peekPixels(&pm)) {
        return this->writePixels(pm.info(), pm.addr(), pm.rowBytes(), x, y);
    }
    return false;
}

bool SkCanvas::writePixels(const SkImageInfo& srcInfo, const void* pixels, size_t rowBytes,
                           int x, int y) {
    SkDevice* device = this->rootDevice();

    // This check gives us an early out and prevents generation ID churn on the surface.
    // This is purely optional: it is a subset of the checks performed by SkWritePixelsRec.
    SkIRect srcRect = SkIRect::MakeXYWH(x, y, srcInfo.width(), srcInfo.height());
    if (!srcRect.intersect({0, 0, device->width(), device->height()})) {
        return false;
    }

    // Tell our owning surface to bump its generation ID.
    const bool completeOverwrite = srcRect.size() == device->imageInfo().dimensions();
    if (!this->predrawNotify(completeOverwrite)) {
        return false;
    }

    // This can still fail, most notably in the case of a invalid color type or alpha type
    // conversion.  We could pull those checks into this function and avoid the unnecessary
    // generation ID bump.  But then we would be performing those checks twice, since they
    // are also necessary at the bitmap/pixmap entry points.
    return device->writePixels({srcInfo, pixels, rowBytes}, x, y);
}

//////////////////////////////////////////////////////////////////////////////

void SkCanvas::checkForDeferredSave() {
    if (fMCRec->fDeferredSaveCount > 0) {
        this->doSave();
    }
}

int SkCanvas::getSaveCount() const {
#ifdef SK_DEBUG
    int count = 0;
    SkDeque::Iter iter(fMCStack, SkDeque::Iter::kFront_IterStart);
    for (;;) {
        const MCRec* rec = (const MCRec*)iter.next();
        if (!rec) {
            break;
        }
        count += 1 + rec->fDeferredSaveCount;
    }
    SkASSERT(count == fSaveCount);
#endif
    return fSaveCount;
}

int SkCanvas::save() {
    fSaveCount += 1;
    fMCRec->fDeferredSaveCount += 1;
    return this->getSaveCount() - 1;  // return our prev value
}

void SkCanvas::doSave() {
    this->willSave();

    SkASSERT(fMCRec->fDeferredSaveCount > 0);
    fMCRec->fDeferredSaveCount -= 1;
    this->internalSave();
}

void SkCanvas::restore() {
    if (fMCRec->fDeferredSaveCount > 0) {
        SkASSERT(fSaveCount > 1);
        fSaveCount -= 1;
        fMCRec->fDeferredSaveCount -= 1;
    } else {
        // check for underflow
        if (fMCStack.count() > 1) {
            this->willRestore();
            SkASSERT(fSaveCount > 1);
            fSaveCount -= 1;
            this->internalRestore();
            this->didRestore();
        }
    }
}

void SkCanvas::restoreToCount(int count) {
    // safety check
    if (count < 1) {
        count = 1;
    }

    int n = this->getSaveCount() - count;
    for (int i = 0; i < n; ++i) {
        this->restore();
    }
}

void SkCanvas::internalSave() {
    fMCRec = new (fMCStack.push_back()) MCRec(fMCRec);

    this->topDevice()->pushClipStack();
}

int SkCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint) {
    return this->saveLayer(SaveLayerRec(bounds, paint, 0));
}

int SkCanvas::saveLayer(const SaveLayerRec& rec) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (rec.fPaint && rec.fPaint->nothingToDraw()) {
        // no need for the layer (or any of the draws until the matching restore()
        this->save();
        this->clipRect({0,0,0,0});
    } else {
        SaveLayerStrategy strategy = this->getSaveLayerStrategy(rec);
        fSaveCount += 1;
        this->internalSaveLayer(rec, strategy);
    }
    return this->getSaveCount() - 1;
}

int SkCanvas::only_axis_aligned_saveBehind(const SkRect* bounds) {
    if (bounds && !this->getLocalClipBounds().intersects(*bounds)) {
        // Assuming clips never expand, if the request bounds is outside of the current clip
        // there is no need to copy/restore the area, so just devolve back to a regular save.
        this->save();
    } else {
        bool doTheWork = this->onDoSaveBehind(bounds);
        fSaveCount += 1;
        this->internalSave();
        if (doTheWork) {
            this->internalSaveBehind(bounds);
        }
    }
    return this->getSaveCount() - 1;
}

// Helper function to compute the center reference point used for scale decomposition under
// non-linear transformations.
static skif::ParameterSpace<SkPoint> compute_decomposition_center(
        const SkM44& dstToLocal,
        std::optional<skif::ParameterSpace<SkRect>> contentBounds,
        const skif::DeviceSpace<SkIRect>& targetOutput) {
    // Will use the inverse and center of the device bounds if the content bounds aren't provided.
    SkRect rect = contentBounds ? SkRect(*contentBounds) : SkRect::Make(SkIRect(targetOutput));
    SkPoint center = {rect.centerX(), rect.centerY()};
    if (!contentBounds) {
        // Theoretically, the inverse transform could put center's homogeneous coord behind W = 0,
        // but that case is handled automatically in Mapping::decomposeCTM later.
        SkV4 mappedCenter = dstToLocal.map(center.fX, center.fY, 0.f, 1.f);
        center = {sk_ieee_float_divide(mappedCenter.x, mappedCenter.w),
                  sk_ieee_float_divide(mappedCenter.y, mappedCenter.w)};
    }

    return skif::ParameterSpace<SkPoint>(center);
}

// Helper when we need to upgrade a single filter to a FilterSpan
struct FilterToSpan {
    FilterToSpan(const SkImageFilter* filter) : fFilter(sk_ref_sp(filter)) {}

    operator SkCanvas::FilterSpan() {
        return fFilter ? SkCanvas::FilterSpan{&fFilter, 1} : SkCanvas::FilterSpan{};
    }

    sk_sp<SkImageFilter> fFilter;
};

// Compute suitable transformations and layer bounds for a new layer that will be used as the source
// input into 'filter' before being drawn into 'dst' via the returned skif::Mapping.
// Null filters are permitted and act as the identity. The returned mapping will be compatible with
// the image filter.
//
// An empty optional is returned if the layer mapping and bounds couldn't be determined, in which
// case the layer should be skipped. An instantiated optional can have an empty layer bounds rect
// if the image filter doesn't require an input image to produce a valid output.
static std::optional<std::pair<skif::Mapping, skif::LayerSpace<SkIRect>>>
get_layer_mapping_and_bounds(
        SkCanvas::FilterSpan filters,
        const SkM44& localToDst,
        const skif::DeviceSpace<SkIRect>& targetOutput,
        std::optional<skif::ParameterSpace<SkRect>> contentBounds = {},
        SkScalar scaleFactor = 1.0f) {
    SkM44 dstToLocal;
    if (!localToDst.isFinite() ||
        !localToDst.invert(&dstToLocal)) {
        return {};
    }

    skif::ParameterSpace<SkPoint> center =
            compute_decomposition_center(dstToLocal, contentBounds, targetOutput);

    // Determine initial mapping and a reasonable maximum dimension to prevent layer-to-device
    // transforms with perspective and skew from triggering excessive buffer allocations.
    skif::Mapping mapping;
    skif::MatrixCapability capability = skif::MatrixCapability::kComplex;
    for (const sk_sp<SkImageFilter>& filter : filters) {
        if (filter) {
            capability = std::min(capability, as_IFB(filter)->getCTMCapability());
        }
    }
    if (!mapping.decomposeCTM(localToDst, capability, center)) {
        return {};
    }
    // Push scale factor into layer matrix and device matrix (net no change, but the layer will have
    // its resolution adjusted in comparison to the final device).
    if (scaleFactor != 1.0f &&
        !mapping.adjustLayerSpace(SkM44::Scale(scaleFactor, scaleFactor))) {
        return {};
    }

    // Perspective and skew could exceed this since mapping.deviceToLayer(targetOutput) is
    // theoretically unbounded under those conditions. Under a 45 degree rotation, a layer needs to
    // be 2X larger per side of the prior device in order to fully cover it. We use the max of that
    // and 2048 for a reasonable upper limit (this allows small layers under extreme transforms to
    // use more relative resolution than a larger layer).
    static const int kMinDimThreshold = 2048;
    int maxLayerDim = std::max(Sk64_pin_to_s32(2 * std::max(SkIRect(targetOutput).width64(),
                                                            SkIRect(targetOutput).height64())),
                               kMinDimThreshold);

    auto baseLayerBounds = mapping.deviceToLayer(targetOutput);
    if (contentBounds) {
        // For better or for worse, user bounds currently act as a hard clip on the layer's
        // extent (i.e., they implement the CSS filter-effects 'filter region' feature).
        skif::LayerSpace<SkIRect> knownBounds = mapping.paramToLayer(*contentBounds).roundOut();
        if (!baseLayerBounds.intersect(knownBounds)) {
            baseLayerBounds = skif::LayerSpace<SkIRect>::Empty();
        }
    }

    skif::LayerSpace<SkIRect> layerBounds;
    if (!filters.empty()) {
        layerBounds = skif::LayerSpace<SkIRect>::Union(filters.size(), [&](int i) {
            return filters[i] ? as_IFB(filters[i])
                                        ->getInputBounds(mapping, targetOutput, contentBounds)
                              : baseLayerBounds;
        });
        // When a filter is involved, the layer size may be larger than the default maxLayerDim due
        // to required inputs for filters (e.g. a displacement map with a large radius).
        if (layerBounds.width() > maxLayerDim || layerBounds.height() > maxLayerDim) {
            skif::Mapping idealMapping{mapping.layerMatrix()};
            for (const sk_sp<SkImageFilter>& filter : filters) {
                if (filter) {
                    auto idealLayerBounds = as_IFB(filter)->getInputBounds(
                            idealMapping, targetOutput, contentBounds);
                    maxLayerDim = std::max(std::max(idealLayerBounds.width(),
                                                    idealLayerBounds.height()),
                                                    maxLayerDim);
                }
            }
        }
    } else {
        if (baseLayerBounds.isEmpty()) {
            return {};
        }
        layerBounds = baseLayerBounds;
    }

    if (layerBounds.width() > maxLayerDim || layerBounds.height() > maxLayerDim) {
        skif::LayerSpace<SkIRect> newLayerBounds(
                SkIRect::MakeWH(std::min(layerBounds.width(), maxLayerDim),
                                std::min(layerBounds.height(), maxLayerDim)));
        SkM44 adjust = SkM44::RectToRect(SkRect::Make(SkIRect(layerBounds)),
                                         SkRect::Make(SkIRect(newLayerBounds)));
        if (!mapping.adjustLayerSpace(adjust)) {
            return {};
        } else {
            layerBounds = newLayerBounds;
        }
    }

    return std::make_pair(mapping, layerBounds);
}

// Ideally image filters operate in the dst color type, but if there is insufficient alpha bits
// we move some bits from color channels into the alpha channel since that can greatly improve
// the quality of blurs and other filters.
static SkColorType image_filter_color_type(const SkColorInfo& dstInfo) {
    if (dstInfo.bytesPerPixel() <= 4 &&
        dstInfo.colorType() != kRGBA_8888_SkColorType &&
        dstInfo.colorType() != kBGRA_8888_SkColorType) {
        // "Upgrade" A8, G8, 565, 4444, 1010102, 101010x, and 888x to 8888
        return kN32_SkColorType;
    } else {
        return dstInfo.colorType();
    }
}

static skif::FilterResult apply_alpha_and_colorfilter(const skif::Context& ctx,
                                                      const skif::FilterResult& image,
                                                      const SkPaint& paint) {
    // The only effects that apply to layers (other than the SkImageFilter that made this image in
    // the first place) are transparency and color filters.
    skif::FilterResult result = image;
    if (paint.getAlphaf() < 1.f) {
        result = result.applyColorFilter(ctx, SkColorFilters::Blend(paint.getColor4f(),
                                                                    /*colorSpace=*/nullptr,
                                                                    SkBlendMode::kDstIn));
    }
    if (paint.getColorFilter()) {
        result = result.applyColorFilter(ctx, paint.refColorFilter());
    }
    return result;
}

void SkCanvas::internalDrawDeviceWithFilter(SkDevice* src,
                                            SkDevice* dst,
                                            FilterSpan filters,
                                            const SkPaint& paint,
                                            DeviceCompatibleWithFilter compat,
                                            const SkColorInfo& filterColorInfo,
                                            SkScalar scaleFactor,
                                            SkTileMode srcTileMode,
                                            bool srcIsCoverageLayer) {
    // The dst is always required, the src can be null if 'filter' is non-null and does not require
    // a source image. For regular filters, 'src' is the layer and 'dst' is the parent device. For
    // backdrop filters, 'src' is the parent device and 'dst' is the layer.
    SkASSERT(dst);

    sk_sp<SkColorSpace> filterColorSpace = filterColorInfo.refColorSpace();

    const SkColorType filterColorType =
            srcIsCoverageLayer ? kAlpha_8_SkColorType : image_filter_color_type(filterColorInfo);

    // 'filter' sees the src device's buffer as the implicit input image, and processes the image
    // in this device space (referred to as the "layer" space). However, the filter
    // parameters need to respect the current matrix, which is not necessarily the local matrix that
    // was set on 'src' (e.g. because we've popped src off the stack already).
    SkM44 localToSrc = src ? (src->globalToDevice() * fMCRec->fMatrix) : SkM44();
    SkISize srcDims = src ? src->imageInfo().dimensions() : SkISize::Make(0, 0);

    // Whether or not we need to make a transformed tmp image from 'src', and what that transform is
    skif::LayerSpace<SkMatrix> srcToLayer;

    skif::Mapping mapping;
    skif::LayerSpace<SkIRect> requiredInput;
    skif::DeviceSpace<SkIRect> outputBounds{dst->devClipBounds()};
    if (compat != DeviceCompatibleWithFilter::kUnknown) {
        // Just use the relative transform from src to dst and the src's whole image, since
        // internalSaveLayer should have already determined what was necessary. We explicitly
        // construct the inverse (dst->src) to avoid the case where src's and dst's coord transforms
        // were individually invertible by SkM44::invert() but their product is considered not
        // invertible by SkMatrix::invert(). When this happens the matrices are already poorly
        // conditioned so getRelativeTransform() gives us something reasonable.
        SkASSERT(src);
        SkASSERT(scaleFactor == 1.0f);
        SkASSERT(!srcDims.isEmpty());

        mapping = skif::Mapping(src->getRelativeTransform(*dst),
                                dst->getRelativeTransform(*src),
                                localToSrc);
        requiredInput = skif::LayerSpace<SkIRect>(SkIRect::MakeSize(srcDims));
        srcToLayer = skif::LayerSpace<SkMatrix>(SkMatrix::I());
    } else {
        // Compute the image filter mapping by decomposing the local->device matrix of dst and
        // re-determining the required input.
        auto mappingAndBounds = get_layer_mapping_and_bounds(
                filters, dst->localToDevice44(), outputBounds, {}, SkTPin(scaleFactor, 0.f, 1.f));
        if (!mappingAndBounds) {
            return;
        }

        std::tie(mapping, requiredInput) = *mappingAndBounds;
        if (src) {
            if (!requiredInput.isEmpty()) {
                // The above mapping transforms from local to dst's device space, where the layer
                // space represents the intermediate buffer. Now we need to determine the transform
                // from src to intermediate to prepare the input to the filter.
                SkM44 srcToLocal;
                if (!localToSrc.invert(&srcToLocal)) {
                    return;
                }
                srcToLayer = skif::LayerSpace<SkMatrix>((mapping.layerMatrix()*srcToLocal).asM33());
            } // Else no input is needed which can happen if a backdrop filter that doesn't use src
        } else {
            // Trust the caller that no input was required, but keep the calculated mapping
            requiredInput = skif::LayerSpace<SkIRect>::Empty();
        }
    }

    // Start out with an empty source image, to be replaced with the snapped 'src' device.
    auto backend = dst->createImageFilteringBackend(src ? src->surfaceProps() : dst->surfaceProps(),
                                                    filterColorType);
    skif::Stats stats;
    skif::Context ctx{std::move(backend),
                      mapping,
                      requiredInput,
                      skif::FilterResult{},
                      filterColorSpace.get(),
                      &stats};

    skif::FilterResult source;
    if (src && !requiredInput.isEmpty()) {
        skif::LayerSpace<SkIRect> srcSubset;
        if (!srcToLayer.inverseMapRect(requiredInput, &srcSubset)) {
            return;
        }

        // Include the layer in the offscreen count
        ctx.markNewSurface();

        auto availSrc = skif::LayerSpace<SkIRect>(src->size()).relevantSubset(
                srcSubset, srcTileMode);

        if (SkMatrix(srcToLayer).isScaleTranslate()) {
            // Apply the srcToLayer transformation directly while snapping an image from the src
            // device. Calculate the subset of requiredInput that corresponds to srcSubset that was
            // restricted to the actual src dimensions.
            auto requiredSubset = srcToLayer.mapRect(availSrc);
            if (requiredSubset.width() == availSrc.width() &&
                requiredSubset.height() == availSrc.height()) {
                // Unlike snapSpecialScaled(), snapSpecial() can avoid a copy when the underlying
                // representation permits it.
                source = {src->snapSpecial(SkIRect(availSrc)), requiredSubset.topLeft()};
            } else {
                SkASSERT(compat == DeviceCompatibleWithFilter::kUnknown);
                source = {src->snapSpecialScaled(SkIRect(availSrc),
                                                 SkISize(requiredSubset.size())),
                          requiredSubset.topLeft()};
                ctx.markNewSurface();
            }
        }

        if (compat == DeviceCompatibleWithFilter::kYesWithPadding) {
            // Padding was added to the source image when the 'src' SkDevice was created, so inset
            // to allow bounds tracking to skip shader-based tiling when possible.
            SkASSERT(!filters.empty());
            source = source.insetForSaveLayer();
        } else if (compat == DeviceCompatibleWithFilter::kYes) {
            // Do nothing, leave `source` as-is; FilterResult will automatically augment the image
            // sampling as needed to be visually equivalent to the more optimal kYesWithPadding case
        } else if (source) {
            // A backdrop filter that succeeded in snapSpecial() or snapSpecialScaled(), but since
            // the 'src' device wasn't prepared with 'requiredInput' in mind, add clamping.
            source = source.applyCrop(ctx, source.layerBounds(), srcTileMode);
        } else if (!requiredInput.isEmpty()) {
            // Otherwise snapSpecialScaled() failed or the transform was complex, so snap the source
            // image at its original resolution and then apply srcToLayer to map to the effective
            // layer coordinate space.
            source = {src->snapSpecial(SkIRect(availSrc)), availSrc.topLeft()};
            // We adjust the desired output of the applyCrop() because ctx was original set to
            // fulfill 'requiredInput', which is valid *after* we apply srcToLayer. Use the original
            // 'srcSubset' for the desired output so that the tilemode applied to the available
            // subset is not discarded as a no-op.
            source = source.applyCrop(ctx.withNewDesiredOutput(srcSubset),
                                      source.layerBounds(),
                                      srcTileMode)
                           .applyTransform(ctx, srcToLayer, SkFilterMode::kLinear);
        }
    } // else leave 'source' as the empty image

    // Evaluate the image filter, with a context pointing to the source snapped from 'src' and
    // possibly transformed into the intermediate layer coordinate space.
    ctx = ctx.withNewDesiredOutput(mapping.deviceToLayer(outputBounds))
             .withNewSource(source);

    // Here, we allow a single-element FilterSpan with a null entry, to simplify the loop:
    sk_sp<SkImageFilter> nullFilter;
    FilterSpan filtersOrNull = filters.empty() ? FilterSpan{&nullFilter, 1} : filters;

    for (const sk_sp<SkImageFilter>& filter : filtersOrNull) {
        auto result = filter ? as_IFB(filter)->filterImage(ctx) : source;

        if (srcIsCoverageLayer) {
            SkASSERT(dst->useDrawCoverageMaskForMaskFilters());
            // TODO: Can FilterResult optimize this in any meaningful way if it still has to go
            // through drawCoverageMask that requires an image (vs a coverage shader)?
            auto [coverageMask, origin] = result.imageAndOffset(ctx);
            if (coverageMask) {
                SkM44 deviceMatrixWithOffset = mapping.layerToDevice();
                deviceMatrixWithOffset.preTranslate(origin.x(), origin.y());
                dst->drawCoverageMask(
                        coverageMask.get(), deviceMatrixWithOffset.asM33(),
                        result.sampling(), paint);
            }
        } else {
            result = apply_alpha_and_colorfilter(ctx, result, paint);
            result.draw(ctx, dst, paint.getBlender());
        }
    }

    stats.reportStats();
}

void SkCanvas::internalSaveLayer(const SaveLayerRec& rec,
                                 SaveLayerStrategy strategy,
                                 bool coverageOnly) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    // Do this before we create the layer. We don't call the public save() since that would invoke a
    // possibly overridden virtual.
    this->internalSave();

    if (this->isClipEmpty()) {
        // Early out if the layer wouldn't draw anything
        return;
    }

    // Build up the paint for restoring the layer, taking only the pieces of rec.fPaint that are
    // relevant. Filtering is automatically chosen in internalDrawDeviceWithFilter based on the
    // device's coordinate space.
    SkPaint restorePaint(rec.fPaint ? *rec.fPaint : SkPaint());
    restorePaint.setStyle(SkPaint::kFill_Style); // a layer is filled out "infinitely"
    restorePaint.setPathEffect(nullptr);         // path effects are ignored for saved layers
    restorePaint.setMaskFilter(nullptr);         // mask filters are ignored for saved layers
    restorePaint.setImageFilter(nullptr);        // the image filter is held separately
    // Smooth non-axis-aligned layer edges; this automatically downgrades to non-AA for aligned
    // layer restores. This is done to match legacy behavior where the post-applied MatrixTransform
    // bilerp also smoothed cropped edges. See skbug.com/40042614
    restorePaint.setAntiAlias(true);

    sk_sp<SkImageFilter> paintFilter = rec.fPaint ? rec.fPaint->refImageFilter() : nullptr;
    FilterSpan filters = paintFilter ? FilterSpan{&paintFilter, 1} : rec.fFilters;
    if (filters.size() > kMaxFiltersPerLayer) {
        filters = filters.first(kMaxFiltersPerLayer);
    }
    const SkColorFilter* cf = restorePaint.getColorFilter();
    const SkBlender* blender = restorePaint.getBlender();

    // When this is false, restoring the layer filled with unmodified prior contents should be
    // identical to the prior contents, so we can restrict the layer even more than just the
    // clip bounds.
    bool filtersPriorDevice = rec.fBackdrop;
#if !defined(SK_LEGACY_INITWITHPREV_LAYER_SIZING)
    // A regular filter applied to a layer initialized with prior contents is somewhat
    // analogous to a backdrop filter so they are treated the same.
    // TODO(b/314968012): Chrome needs to be updated to clip saveAlphaLayer bounds explicitly when
    // it uses kInitWithPrevious and LCD text.
    filtersPriorDevice |= ((rec.fSaveLayerFlags & kInitWithPrevious_SaveLayerFlag) &&
             (!filters.empty() || cf || blender || restorePaint.getAlphaf() < 1.f));
#endif
    // If the restorePaint has a transparency-affecting colorfilter or blender, the output is
    // unbounded during restore(). `internalDrawDeviceWithFilter` automatically applies these
    // effects. When there's no image filter, SkDevice::drawDevice is used, which does
    // not apply effects beyond the layer's image so we mark `trivialRestore` as false too.
    // TODO: drawDevice() could be updated to apply transparency-affecting effects to a content-
    // clipped image, but this is the simplest solution when considering document-based SkDevices.
    const bool drawDeviceMustFillClip = filters.empty() &&
            ((cf && as_CFB(cf)->affectsTransparentBlack()) ||
                (blender && as_BB(blender)->affectsTransparentBlack()));
    const bool trivialRestore = !filtersPriorDevice && !drawDeviceMustFillClip;

    // Size the new layer relative to the prior device, which may already be aligned for filters.
    SkDevice* priorDevice = this->topDevice();
    skif::Mapping newLayerMapping;
    skif::LayerSpace<SkIRect> layerBounds;
    skif::DeviceSpace<SkIRect> outputBounds{priorDevice->devClipBounds()};

    std::optional<skif::ParameterSpace<SkRect>> contentBounds;
    // Set the bounds hint if provided and there's no further effects on prior device content
    if (rec.fBounds && trivialRestore) {
        contentBounds = skif::ParameterSpace<SkRect>(*rec.fBounds);
    }

    auto mappingAndBounds = get_layer_mapping_and_bounds(
            filters, priorDevice->localToDevice44(), outputBounds, contentBounds);

    auto abortLayer = [this]() {
        // The filtered content would not draw anything, or the new device space has an invalid
        // coordinate system, in which case we mark the current top device as empty so that nothing
        // draws until the canvas is restored past this saveLayer.
        AutoUpdateQRBounds aqr(this);
        this->topDevice()->clipRect(SkRect::MakeEmpty(), SkClipOp::kIntersect, /* aa */ false);
    };

    if (!mappingAndBounds) {
        abortLayer();
        return;
    }

    std::tie(newLayerMapping, layerBounds) = *mappingAndBounds;

    bool paddedLayer = false;
    if (layerBounds.isEmpty()) {
        // The image filter graph does not require any input, so we don't need to actually render
        // a new layer for the source image. This could be because the image filter itself will not
        // produce output, or that the filter DAG has no references to the dynamic source image.
        // In this case it still has an output that we need to render, but do so now since there is
        // no new layer pushed on the stack and the paired restore() will be a no-op.
        if (!filters.empty() && !priorDevice->isNoPixelsDevice()) {
            SkColorInfo filterColorInfo = priorDevice->imageInfo().colorInfo();
            if (rec.fColorSpace) {
                filterColorInfo = filterColorInfo.makeColorSpace(sk_ref_sp(rec.fColorSpace));
            }
            this->internalDrawDeviceWithFilter(/*src=*/nullptr, priorDevice, filters, restorePaint,
                                               DeviceCompatibleWithFilter::kUnknown,
                                               filterColorInfo);
        }

        // Regardless of if we drew the "restored" image filter or not, mark the layer as empty
        // until the restore() since we don't care about any of its content.
        abortLayer();
        return;
    } else {
        // TODO(b/329700315): Once dithers can be anchored more flexibly, we can return to
        // universally adding padding even for layers w/o filters. This change would simplify layer
        // prep and restore logic and allow us to flexibly switch the sampling to linear if NN has
        // issues on certain hardware.
        if (!filters.empty()) {
            // Add a buffer of padding so that image filtering can avoid accessing unitialized data
            // and switch from shader-decal'ing to clamping.
            auto paddedLayerBounds = layerBounds;
            paddedLayerBounds.outset(skif::LayerSpace<SkISize>({1, 1}));
            if (paddedLayerBounds.left() < layerBounds.left() &&
                paddedLayerBounds.top() < layerBounds.top() &&
                paddedLayerBounds.right() > layerBounds.right() &&
                paddedLayerBounds.bottom() > layerBounds.bottom()) {
                // The outset was not saturated to INT_MAX, so the transparent pixels can be
                // preserved.
                layerBounds = paddedLayerBounds;
                paddedLayer = true;
            }
        }
    }

    sk_sp<SkDevice> newDevice;
    if (strategy == kFullLayer_SaveLayerStrategy) {
        SkASSERT(!layerBounds.isEmpty());

        SkColorType layerColorType;
        if (coverageOnly) {
            layerColorType = kAlpha_8_SkColorType;
        } else {
            layerColorType = SkToBool(rec.fSaveLayerFlags & kF16ColorType)
                                    ? kRGBA_F16_SkColorType
                                    : image_filter_color_type(priorDevice->imageInfo().colorInfo());
        }
        SkImageInfo info =
                SkImageInfo::Make(layerBounds.width(),
                                  layerBounds.height(),
                                  layerColorType,
                                  kPremul_SkAlphaType,
                                  rec.fColorSpace ? sk_ref_sp(rec.fColorSpace)
                                                  : priorDevice->imageInfo().refColorSpace());

        SkPixelGeometry geo = rec.fSaveLayerFlags & kPreserveLCDText_SaveLayerFlag
                                      ? fProps.pixelGeometry()
                                      : kUnknown_SkPixelGeometry;
        const auto createInfo = SkDevice::CreateInfo(info, geo, fAllocator.get());
        // Use the original paint as a hint so that it includes the image filter
        newDevice = priorDevice->createDevice(createInfo, rec.fPaint);
    }

    bool initBackdrop = (rec.fSaveLayerFlags & kInitWithPrevious_SaveLayerFlag) || rec.fBackdrop;
    if (!newDevice) {
        // Either we weren't meant to allocate a full layer, or the full layer creation failed.
        // Using an explicit NoPixelsDevice lets us reflect what the layer state would have been
        // on success (or kFull_LayerStrategy) while squashing draw calls that target something that
        // doesn't exist.
        newDevice = sk_make_sp<SkNoPixelsDevice>(SkIRect::MakeWH(layerBounds.width(),
                                                                 layerBounds.height()),
                                                 fProps, this->imageInfo().refColorSpace());
        initBackdrop = false;
    }

    // Clip while the device coordinate space is the identity so it's easy to define the rect that
    // excludes the added padding pixels. This ensures they remain cleared to transparent black.
    if (paddedLayer) {
        newDevice->clipRect(SkRect::Make(newDevice->devClipBounds().makeInset(1, 1)),
                            SkClipOp::kIntersect, /*aa=*/false);
    }

    // Configure device to match determined mapping for any image filters.
    // The setDeviceCoordinateSystem applies the prior device's global transform since
    // 'newLayerMapping' only defines the transforms between the two devices and it must be updated
    // to the global coordinate system.
    newDevice->setDeviceCoordinateSystem(
            priorDevice->deviceToGlobal() * newLayerMapping.layerToDevice(),
            newLayerMapping.deviceToLayer() * priorDevice->globalToDevice(),
            newLayerMapping.layerMatrix(),
            layerBounds.left(),
            layerBounds.top());

    if (initBackdrop) {
        SkASSERT(!coverageOnly);
        SkPaint backdropPaint;
        FilterToSpan backdropAsSpan(rec.fBackdrop);
        // The new device was constructed to be compatible with 'filter', not necessarily
        // 'rec.fBackdrop', so allow DrawDeviceWithFilter to transform the prior device contents
        // if necessary to evaluate the backdrop filter. If no filters are involved, then the
        // devices differ by integer translations and are always compatible.
        bool scaleBackdrop = rec.fExperimentalBackdropScale != 1.0f;
        auto compat = (!filters.empty() || rec.fBackdrop || scaleBackdrop)
                ? DeviceCompatibleWithFilter::kUnknown : DeviceCompatibleWithFilter::kYes;
        // Using the color info of 'newDevice' is equivalent to using 'rec.fColorSpace'.
        this->internalDrawDeviceWithFilter(priorDevice,     // src
                                           newDevice.get(), // dst
                                           backdropAsSpan,
                                           backdropPaint,
                                           compat,
                                           newDevice->imageInfo().colorInfo(),
                                           rec.fExperimentalBackdropScale,
                                           rec.fBackdropTileMode);
    }

    fMCRec->newLayer(std::move(newDevice), filters, restorePaint, coverageOnly, paddedLayer);
    fQuickRejectBounds = this->computeDeviceClipBounds();
}

int SkCanvas::saveLayerAlphaf(const SkRect* bounds, float alpha) {
    if (alpha >= 1.0f) {
        return this->saveLayer(bounds, nullptr);
    } else {
        SkPaint tmpPaint;
        tmpPaint.setAlphaf(alpha);
        return this->saveLayer(bounds, &tmpPaint);
    }
}

void SkCanvas::internalSaveBehind(const SkRect* localBounds) {
    SkDevice* device = this->topDevice();

    // Map the local bounds into the top device's coordinate space (this is not
    // necessarily the full global CTM transform).
    SkIRect devBounds;
    if (localBounds) {
        SkRect tmp;
        device->localToDevice().mapRect(&tmp, *localBounds);
        if (!devBounds.intersect(tmp.round(), device->devClipBounds())) {
            devBounds.setEmpty();
        }
    } else {
        devBounds = device->devClipBounds();
    }
    if (devBounds.isEmpty()) {
        return;
    }

    // This is getting the special image from the current device, which is then drawn into (both by
    // a client, and the drawClippedToSaveBehind below). Since this is not saving a layer, with its
    // own device, we need to explicitly copy the back image contents so that its original content
    // is available when we splat it back later during restore.
    auto backImage = device->snapSpecial(devBounds, /* forceCopy= */ true);
    if (!backImage) {
        return;
    }

    // we really need the save, so we can wack the fMCRec
    this->checkForDeferredSave();

    fMCRec->fBackImage =
            std::make_unique<BackImage>(BackImage{std::move(backImage), devBounds.topLeft()});

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kClear);
    this->drawClippedToSaveBehind(paint);
}

void SkCanvas::internalRestore() {
    SkASSERT(!fMCStack.empty());

    // now detach these from fMCRec so we can pop(). Gets freed after its drawn
    std::unique_ptr<Layer> layer = std::move(fMCRec->fLayer);
    std::unique_ptr<BackImage> backImage = std::move(fMCRec->fBackImage);

    // now do the normal restore()
    fMCRec->~MCRec();       // balanced in save()
    fMCStack.pop_back();
    fMCRec = (MCRec*) fMCStack.back();

    if (!fMCRec) {
        // This was the last record, restored during the destruction of the SkCanvas
        return;
    }

    this->topDevice()->popClipStack();
    this->topDevice()->setGlobalCTM(fMCRec->fMatrix);

    if (backImage) {
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kDstOver);
        this->topDevice()->drawSpecial(backImage->fImage.get(),
                                       SkMatrix::Translate(backImage->fLoc),
                                       SkSamplingOptions(),
                                       paint);
    }

    // Draw the layer's device contents into the now-current older device. We can't call public
    // draw functions since we don't want to record them.
    if (layer && !layer->fDevice->isNoPixelsDevice() && !layer->fDiscard) {
        layer->fDevice->setImmutable();

        // Don't go through AutoLayerForImageFilter since device draws are so closely tied to
        // internalSaveLayer and internalRestore.
        if (this->predrawNotify()) {
            SkDevice* dstDev = this->topDevice();
            if (!layer->fImageFilters.empty()) {
                auto compat = layer->fIncludesPadding ? DeviceCompatibleWithFilter::kYesWithPadding
                                                      : DeviceCompatibleWithFilter::kYes;
                this->internalDrawDeviceWithFilter(layer->fDevice.get(), // src
                                                   dstDev,               // dst
                                                   layer->fImageFilters,
                                                   layer->fPaint,
                                                   compat,
                                                   layer->fDevice->imageInfo().colorInfo(),
                                                   /*scaleFactor=*/1.0f,
                                                   /*srcTileMode=*/SkTileMode::kDecal,
                                                   layer->fIsCoverage);
            } else {
                // NOTE: We don't just call internalDrawDeviceWithFilter with a null filter
                // because we want to take advantage of overridden drawDevice functions for
                // document-based devices.
                SkASSERT(!layer->fIsCoverage && !layer->fIncludesPadding);
                SkSamplingOptions sampling;
                dstDev->drawDevice(layer->fDevice.get(), sampling, layer->fPaint);
            }
        }
    }

    // Reset the clip restriction if the restore went past the save point that had added it.
    if (this->getSaveCount() < fClipRestrictionSaveCount) {
        fClipRestrictionRect.setEmpty();
        fClipRestrictionSaveCount = -1;
    }
    // Update the quick-reject bounds in case the restore changed the top device or the
    // removed save record had included modifications to the clip stack.
    fQuickRejectBounds = this->computeDeviceClipBounds();
    this->validateClip();
}

sk_sp<SkSurface> SkCanvas::makeSurface(const SkImageInfo& info, const SkSurfaceProps* props) {
    if (nullptr == props) {
        props = &fProps;
    }
    return this->onNewSurface(info, *props);
}

sk_sp<SkSurface> SkCanvas::onNewSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    return this->rootDevice()->makeSurface(info, props);
}

SkImageInfo SkCanvas::imageInfo() const {
    return this->onImageInfo();
}

SkImageInfo SkCanvas::onImageInfo() const {
    return this->rootDevice()->imageInfo();
}

bool SkCanvas::getProps(SkSurfaceProps* props) const {
    return this->onGetProps(props, /*top=*/false);
}

SkSurfaceProps SkCanvas::getBaseProps() const {
    SkSurfaceProps props;
    this->onGetProps(&props, /*top=*/false);
    return props;
}

SkSurfaceProps SkCanvas::getTopProps() const {
    SkSurfaceProps props;
    this->onGetProps(&props, /*top=*/true);
    return props;
}

bool SkCanvas::onGetProps(SkSurfaceProps* props, bool top) const {
    if (props) {
        *props = top ? topDevice()->surfaceProps() : fProps;
    }
    return true;
}

bool SkCanvas::peekPixels(SkPixmap* pmap) {
    return this->onPeekPixels(pmap);
}

bool SkCanvas::onPeekPixels(SkPixmap* pmap) {
    return this->rootDevice()->peekPixels(pmap);
}

void* SkCanvas::accessTopLayerPixels(SkImageInfo* info, size_t* rowBytes, SkIPoint* origin) {
    SkPixmap pmap;
    if (!this->onAccessTopLayerPixels(&pmap)) {
        return nullptr;
    }
    if (info) {
        *info = pmap.info();
    }
    if (rowBytes) {
        *rowBytes = pmap.rowBytes();
    }
    if (origin) {
        // If the caller requested the origin, they presumably are expecting the returned pixels to
        // be axis-aligned with the root canvas. If the top level device isn't axis aligned, that's
        // not the case. Until we update accessTopLayerPixels() to accept a coord space matrix
        // instead of an origin, just don't expose the pixels in that case. Note that this means
        // that layers with complex coordinate spaces can still report their pixels if the caller
        // does not ask for the origin (e.g. just to dump its output to a file, etc).
        if (this->topDevice()->isPixelAlignedToGlobal()) {
            *origin = this->topDevice()->getOrigin();
        } else {
            return nullptr;
        }
    }
    return pmap.writable_addr();
}

bool SkCanvas::onAccessTopLayerPixels(SkPixmap* pmap) {
    return this->topDevice()->accessPixels(pmap);
}

/////////////////////////////////////////////////////////////////////////////

void SkCanvas::translate(SkScalar dx, SkScalar dy) {
    if (dx || dy) {
        this->checkForDeferredSave();
        fMCRec->fMatrix.preTranslate(dx, dy);

        this->topDevice()->setGlobalCTM(fMCRec->fMatrix);

        this->didTranslate(dx,dy);
    }
}

void SkCanvas::scale(SkScalar sx, SkScalar sy) {
    if (sx != 1 || sy != 1) {
        this->checkForDeferredSave();
        fMCRec->fMatrix.preScale(sx, sy);

        this->topDevice()->setGlobalCTM(fMCRec->fMatrix);

        this->didScale(sx, sy);
    }
}

void SkCanvas::rotate(SkScalar degrees) {
    SkMatrix m;
    m.setRotate(degrees);
    this->concat(m);
}

void SkCanvas::rotate(SkScalar degrees, SkScalar px, SkScalar py) {
    SkMatrix m;
    m.setRotate(degrees, px, py);
    this->concat(m);
}

void SkCanvas::skew(SkScalar sx, SkScalar sy) {
    SkMatrix m;
    m.setSkew(sx, sy);
    this->concat(m);
}

void SkCanvas::concat(const SkMatrix& matrix) {
    if (matrix.isIdentity()) {
        return;
    }
    this->concat(SkM44(matrix));
}

void SkCanvas::internalConcat44(const SkM44& m) {
    this->checkForDeferredSave();

    fMCRec->fMatrix.preConcat(m);

    this->topDevice()->setGlobalCTM(fMCRec->fMatrix);
}

void SkCanvas::concat(const SkM44& m) {
    this->internalConcat44(m);
    // notify subclasses
    this->didConcat44(m);
}

void SkCanvas::internalSetMatrix(const SkM44& m) {
    fMCRec->fMatrix = m;

    this->topDevice()->setGlobalCTM(fMCRec->fMatrix);
}

void SkCanvas::setMatrix(const SkMatrix& matrix) {
    this->setMatrix(SkM44(matrix));
}

void SkCanvas::setMatrix(const SkM44& m) {
    this->checkForDeferredSave();
    this->internalSetMatrix(m);
    this->didSetM44(m);
}

void SkCanvas::resetMatrix() {
    this->setMatrix(SkM44());
}

//////////////////////////////////////////////////////////////////////////////

void SkCanvas::clipRect(const SkRect& rect, SkClipOp op, bool doAA) {
    if (!rect.isFinite()) {
        return;
    }
    this->checkForDeferredSave();
    ClipEdgeStyle edgeStyle = doAA ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;
    this->onClipRect(rect.makeSorted(), op, edgeStyle);
}

void SkCanvas::onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    SkASSERT(rect.isSorted());
    const bool isAA = kSoft_ClipEdgeStyle == edgeStyle;

    AutoUpdateQRBounds aqr(this);
    this->topDevice()->clipRect(rect, op, isAA);
}

void SkCanvas::androidFramework_setDeviceClipRestriction(const SkIRect& rect) {
    // The device clip restriction is a surface-space rectangular intersection that cannot be
    // drawn outside of. The rectangle is remembered so that subsequent resetClip calls still
    // respect the restriction. Other than clip resetting, all clip operations restrict the set
    // of renderable pixels, so once set, the restriction will be respected until the canvas
    // save stack is restored past the point this function was invoked. Unfortunately, the current
    // implementation relies on the clip stack of the underyling SkDevices, which leads to some
    // awkward behavioral interactions (see skbug.com/40043342).
    //
    // Namely, a canvas restore() could undo the clip restriction's rect, and if
    // setDeviceClipRestriction were called at a nested save level, there's no way to undo just the
    // prior restriction and re-apply the new one. It also only makes sense to apply to the base
    // device; any other device for a saved layer will be clipped back to the base device during its
    // matched restore. As such, we:
    // - Remember the save count that added the clip restriction and reset the rect to empty when
    //   we've restored past that point to keep our state in sync with the device's clip stack.
    // - We assert that we're on the base device when this is invoked.
    // - We assert that setDeviceClipRestriction() is only called when there was no prior
    //   restriction (cannot re-restrict, and prior state must have been reset by restoring the
    //   canvas state).
    // - Historically, the empty rect would reset the clip restriction but it only could do so
    //   partially since the device's clips wasn't adjusted. Resetting is now handled
    //   automatically via SkCanvas::restore(), so empty input rects are skipped.
    SkASSERT(this->topDevice() == this->rootDevice()); // shouldn't be in a nested layer
    // and shouldn't already have a restriction
    SkASSERT(fClipRestrictionSaveCount < 0 && fClipRestrictionRect.isEmpty());

    if (fClipRestrictionSaveCount < 0 && !rect.isEmpty()) {
        fClipRestrictionRect = rect;
        fClipRestrictionSaveCount = this->getSaveCount();

        // A non-empty clip restriction immediately applies an intersection op (ignoring the ctm).
        // so we have to resolve the save.
        this->checkForDeferredSave();
        AutoUpdateQRBounds aqr(this);
        // Use clipRegion() since that operates in canvas-space, whereas clipRect() would apply the
        // device's current transform first.
        this->topDevice()->clipRegion(SkRegion(rect), SkClipOp::kIntersect);
    }
}

void SkCanvas::internal_private_resetClip() {
    this->checkForDeferredSave();
    this->onResetClip();
}

void SkCanvas::onResetClip() {
    SkIRect deviceRestriction = this->topDevice()->imageInfo().bounds();
    if (fClipRestrictionSaveCount >= 0 && this->topDevice() == this->rootDevice()) {
        // Respect the device clip restriction when resetting the clip if we're on the base device.
        // If we're not on the base device, then the "reset" applies to the top device's clip stack,
        // and the clip restriction will be respected automatically during a restore of the layer.
        if (!deviceRestriction.intersect(fClipRestrictionRect)) {
            deviceRestriction = SkIRect::MakeEmpty();
        }
    }

    AutoUpdateQRBounds aqr(this);
    this->topDevice()->replaceClip(deviceRestriction);
}

void SkCanvas::clipRRect(const SkRRect& rrect, SkClipOp op, bool doAA) {
    this->checkForDeferredSave();
    ClipEdgeStyle edgeStyle = doAA ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;
    if (rrect.isRect()) {
        this->onClipRect(rrect.getBounds(), op, edgeStyle);
    } else {
        this->onClipRRect(rrect, op, edgeStyle);
    }
}

void SkCanvas::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    bool isAA = kSoft_ClipEdgeStyle == edgeStyle;

    AutoUpdateQRBounds aqr(this);
    this->topDevice()->clipRRect(rrect, op, isAA);
}

void SkCanvas::clipPath(const SkPath& path, SkClipOp op, bool doAA) {
    this->checkForDeferredSave();
    ClipEdgeStyle edgeStyle = doAA ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;

    if (!path.isInverseFillType() && fMCRec->fMatrix.asM33().rectStaysRect()) {
        SkRect r;
        if (path.isRect(&r)) {
            this->onClipRect(r, op, edgeStyle);
            return;
        }
        SkRRect rrect;
        if (path.isOval(&r)) {
            rrect.setOval(r);
            this->onClipRRect(rrect, op, edgeStyle);
            return;
        }
        if (path.isRRect(&rrect)) {
            this->onClipRRect(rrect, op, edgeStyle);
            return;
        }
    }

    this->onClipPath(path, op, edgeStyle);
}

void SkCanvas::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) {
    bool isAA = kSoft_ClipEdgeStyle == edgeStyle;

    AutoUpdateQRBounds aqr(this);
    this->topDevice()->clipPath(path, op, isAA);
}

void SkCanvas::clipShader(sk_sp<SkShader> sh, SkClipOp op) {
    if (sh) {
        if (sh->isOpaque()) {
            if (op == SkClipOp::kIntersect) {
                // we don't occlude anything, so skip this call
            } else {
                SkASSERT(op == SkClipOp::kDifference);
                // we occlude everything, so set the clip to empty
                this->clipRect({0,0,0,0});
            }
        } else {
            this->checkForDeferredSave();
            this->onClipShader(std::move(sh), op);
        }
    }
}

void SkCanvas::onClipShader(sk_sp<SkShader> sh, SkClipOp op) {
    AutoUpdateQRBounds aqr(this);
    this->topDevice()->clipShader(sh, op);
}

void SkCanvas::clipRegion(const SkRegion& rgn, SkClipOp op) {
    this->checkForDeferredSave();
    this->onClipRegion(rgn, op);
}

void SkCanvas::onClipRegion(const SkRegion& rgn, SkClipOp op) {
    AutoUpdateQRBounds aqr(this);
    this->topDevice()->clipRegion(rgn, op);
}

void SkCanvas::validateClip() const {
#ifdef SK_DEBUG
    SkRect tmp = this->computeDeviceClipBounds();
    if (this->isClipEmpty()) {
        SkASSERT(fQuickRejectBounds.isEmpty());
    } else {
        SkASSERT(tmp == fQuickRejectBounds);
    }
#endif
}

bool SkCanvas::androidFramework_isClipAA() const {
    return this->topDevice()->isClipAntiAliased();
}

void SkCanvas::temporary_internal_getRgnClip(SkRegion* rgn) {
    rgn->setEmpty();
    SkDevice* device = this->topDevice();
    if (device && device->isPixelAlignedToGlobal()) {
        device->android_utils_clipAsRgn(rgn);
        SkIPoint origin = device->getOrigin();
        if (origin.x() | origin.y()) {
            rgn->translate(origin.x(), origin.y());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool SkCanvas::isClipEmpty() const {
    return this->topDevice()->isClipEmpty();
}

bool SkCanvas::isClipRect() const {
    return this->topDevice()->isClipRect();
}

bool SkCanvas::quickReject(const SkRect& src) const {
#ifdef SK_DEBUG
    // Verify that fQuickRejectBounds are set properly.
    this->validateClip();
#endif

    SkRect devRect = SkMatrixPriv::MapRect(fMCRec->fMatrix, src);
    return !devRect.isFinite() || !devRect.intersects(fQuickRejectBounds);
}

bool SkCanvas::quickReject(const SkPath& path) const {
    return path.isEmpty() || this->quickReject(path.getBounds());
}

bool SkCanvas::internalQuickReject(const SkRect& bounds, const SkPaint& paint,
                                   const SkMatrix* matrix) {
    if (!bounds.isFinite() || paint.nothingToDraw()) {
        return true;
    }

    if (paint.canComputeFastBounds()) {
        SkRect tmp = matrix ? matrix->mapRect(bounds) : bounds;
        return this->quickReject(paint.computeFastBounds(tmp, &tmp));
    }

    return false;
}


SkRect SkCanvas::getLocalClipBounds() const {
    SkIRect ibounds = this->getDeviceClipBounds();
    if (ibounds.isEmpty()) {
        return SkRect::MakeEmpty();
    }

    auto inverse = fMCRec->fMatrix.asM33().invert();
    // if we can't invert the CTM, we can't return local clip bounds
    if (!inverse) {
        return SkRect::MakeEmpty();
    }

    // adjust it outwards in case we are antialiasing
    const int margin = 1;

    return inverse->mapRect(SkRect::Make(ibounds.makeOutset(margin, margin)));
}

SkIRect SkCanvas::getDeviceClipBounds() const {
    return this->computeDeviceClipBounds(/*outsetForAA=*/false).roundOut();
}

SkRect SkCanvas::computeDeviceClipBounds(bool outsetForAA) const {
    const SkDevice* dev = this->topDevice();
    if (dev->isClipEmpty()) {
        return SkRect::MakeEmpty();
    } else {
        SkRect devClipBounds =
                SkMatrixPriv::MapRect(dev->deviceToGlobal(), SkRect::Make(dev->devClipBounds()));
        if (outsetForAA) {
            // Expand bounds out by 1 in case we are anti-aliasing.  We store the
            // bounds as floats to enable a faster quick reject implementation.
            devClipBounds.outset(1.f, 1.f);
        }
        return devClipBounds;
    }
}

///////////////////////////////////////////////////////////////////////

SkMatrix SkCanvas::getTotalMatrix() const {
    return fMCRec->fMatrix.asM33();
}

SkM44 SkCanvas::getLocalToDevice() const {
    return fMCRec->fMatrix;
}

GrRecordingContext* SkCanvas::recordingContext() const {
    return this->topDevice()->recordingContext();
}

skgpu::graphite::Recorder* SkCanvas::recorder() const {
    return this->topDevice()->recorder();
}

SkRecorder* SkCanvas::baseRecorder() const {
    return this->topDevice()->baseRecorder();
}

void SkCanvas::drawDRRect(const SkRRect& outer, const SkRRect& inner,
                          const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (outer.isEmpty()) {
        return;
    }
    if (inner.isEmpty()) {
        this->drawRRect(outer, paint);
        return;
    }

    // We don't have this method (yet), but technically this is what we should
    // be able to return ...
    // if (!outer.contains(inner))) {
    //
    // For now at least check for containment of bounds
    if (!outer.getBounds().contains(inner.getBounds())) {
        return;
    }

    this->onDrawDRRect(outer, inner, paint);
}

void SkCanvas::drawPaint(const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    this->onDrawPaint(paint);
}

void SkCanvas::drawRect(const SkRect& r, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    // To avoid redundant logic in our culling code and various backends, we always sort rects
    // before passing them along.
    this->onDrawRect(r.makeSorted(), paint);
}

void SkCanvas::drawClippedToSaveBehind(const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    this->onDrawBehind(paint);
}

void SkCanvas::drawRegion(const SkRegion& region, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (region.isEmpty()) {
        return;
    }

    if (region.isRect()) {
        return this->drawIRect(region.getBounds(), paint);
    }

    this->onDrawRegion(region, paint);
}

void SkCanvas::drawOval(const SkRect& r, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    // To avoid redundant logic in our culling code and various backends, we always sort rects
    // before passing them along.
    this->onDrawOval(r.makeSorted(), paint);
}

void SkCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    this->onDrawRRect(rrect, paint);
}

void SkCanvas::drawPoints(PointMode mode, SkSpan<const SkPoint> pts, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (!pts.empty()) {
        this->onDrawPoints(mode, pts.size(), pts.data(), paint);
    }
}

void SkCanvas::drawVertices(const sk_sp<SkVertices>& vertices, SkBlendMode mode,
                            const SkPaint& paint) {
    this->drawVertices(vertices.get(), mode, paint);
}

void SkCanvas::drawVertices(const SkVertices* vertices, SkBlendMode mode, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    RETURN_ON_NULL(vertices);

    // We expect fans to be converted to triangles when building or deserializing SkVertices.
    SkASSERT(vertices->priv().mode() != SkVertices::kTriangleFan_VertexMode);

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    // Preserve legacy behavior for Android: ignore the SkShader if there are no texCoords present
    if (paint.getShader() && !vertices->priv().hasTexCoords()) {
        SkPaint noShaderPaint(paint);
        noShaderPaint.setShader(nullptr);
        this->onDrawVerticesObject(vertices, mode, noShaderPaint);
        return;
    }
#endif
    this->onDrawVerticesObject(vertices, mode, paint);
}

void SkCanvas::drawMesh(const SkMesh& mesh, sk_sp<SkBlender> blender, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (!blender) {
        blender = SkBlender::Mode(SkBlendMode::kModulate);
    }
    this->onDrawMesh(mesh, std::move(blender), paint);
}

void SkCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    this->onDrawPath(path, paint);
}

// Returns true if the rect can be "filled" : non-empty and finite
static bool fillable(const SkRect& r) {
    SkScalar w = r.width();
    SkScalar h = r.height();
    return SkIsFinite(w, h) && w > 0 && h > 0;
}

static SkPaint clean_paint_for_lattice(const SkPaint* paint) {
    SkPaint cleaned;
    if (paint) {
        cleaned = *paint;
        cleaned.setMaskFilter(nullptr);
        cleaned.setAntiAlias(false);
    }
    return cleaned;
}

void SkCanvas::drawImageNine(const SkImage* image, const SkIRect& center, const SkRect& dst,
                             SkFilterMode filter, const SkPaint* paint) {
    RETURN_ON_NULL(image);

    const int xdivs[] = {center.fLeft, center.fRight};
    const int ydivs[] = {center.fTop, center.fBottom};

    Lattice lat;
    lat.fXDivs = xdivs;
    lat.fYDivs = ydivs;
    lat.fRectTypes = nullptr;
    lat.fXCount = lat.fYCount = 2;
    lat.fBounds = nullptr;
    lat.fColors = nullptr;
    this->drawImageLattice(image, lat, dst, filter, paint);
}

void SkCanvas::drawImageLattice(const SkImage* image, const Lattice& lattice, const SkRect& dst,
                                SkFilterMode filter, const SkPaint* paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    RETURN_ON_NULL(image);
    if (dst.isEmpty()) {
        return;
    }

    SkIRect bounds;
    Lattice latticePlusBounds = lattice;
    if (!latticePlusBounds.fBounds) {
        bounds = SkIRect::MakeWH(image->width(), image->height());
        latticePlusBounds.fBounds = &bounds;
    }

    SkPaint latticePaint = clean_paint_for_lattice(paint);
    if (SkLatticeIter::Valid(image->width(), image->height(), latticePlusBounds)) {
        this->onDrawImageLattice2(image, latticePlusBounds, dst, filter, &latticePaint);
    } else {
        this->drawImageRect(image, SkRect::MakeIWH(image->width(), image->height()), dst,
                            SkSamplingOptions(filter), &latticePaint, kStrict_SrcRectConstraint);
    }
}

void SkCanvas::drawAtlas(const SkImage* atlas, SkSpan<const SkRSXform> xform,
                         SkSpan<const SkRect> tex, SkSpan<const SkColor> colors, SkBlendMode mode,
                         const SkSamplingOptions& sampling, const SkRect* cull,
                         const SkPaint* paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    RETURN_ON_NULL(atlas);
    size_t count = std::min(xform.size(), tex.size());
    if (!colors.empty()) {
        count = std::min(count, colors.size());
    }
    if (count == 0) {
        return;
    }
    this->onDrawAtlas2(atlas, xform.data(), tex.data(), colors.data(), count, mode, sampling,
                       cull, paint);
}

void SkCanvas::drawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (key) {
        this->onDrawAnnotation(rect, key, value);
    }
}

void SkCanvas::private_draw_shadow_rec(const SkPath& path, const SkDrawShadowRec& rec) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    this->onDrawShadowRec(path, rec);
}

void SkCanvas::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    // We don't test quickReject because the shadow outsets the path's bounds.
    // TODO(michaelludwig): Is it worth calling SkDrawShadowMetrics::GetLocalBounds here?
    if (!this->predrawNotify()) {
        return;
    }
    this->topDevice()->drawShadow(this, path, rec);
}

void SkCanvas::experimental_DrawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4],
                                           QuadAAFlags aaFlags, const SkColor4f& color,
                                           SkBlendMode mode) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    // Make sure the rect is sorted before passing it along
    this->onDrawEdgeAAQuad(rect.makeSorted(), clip, aaFlags, color, mode);
}

void SkCanvas::experimental_DrawEdgeAAImageSet(const ImageSetEntry imageSet[], int cnt,
                                               const SkPoint dstClips[],
                                               const SkMatrix preViewMatrices[],
                                               const SkSamplingOptions& sampling,
                                               const SkPaint* paint,
                                               SrcRectConstraint constraint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    // Route single, rectangular quads to drawImageRect() to take advantage of image filter
    // optimizations that avoid a layer.
    if (paint && (paint->getImageFilter() || paint->getMaskFilter()) && cnt == 1) {
        const auto& entry = imageSet[0];
        // If the preViewMatrix is skipped or a positive-scale + translate matrix, we can apply it
        // to the entry's dstRect w/o changing output behavior.
        const bool canMapDstRect = entry.fMatrixIndex < 0 ||
            (preViewMatrices[entry.fMatrixIndex].isScaleTranslate() &&
             preViewMatrices[entry.fMatrixIndex].getScaleX() > 0.f &&
             preViewMatrices[entry.fMatrixIndex].getScaleY() > 0.f);
        if (!entry.fHasClip && canMapDstRect) {
            SkRect dst = entry.fDstRect;
            if (entry.fMatrixIndex >= 0) {
                preViewMatrices[entry.fMatrixIndex].mapRect(&dst);
            }
            this->drawImageRect(entry.fImage.get(), entry.fSrcRect, dst,
                                sampling, paint, constraint);
            return;
        } // Else the entry is doing more than can be represented by drawImageRect
    } // Else no filter, or many entries that should be filtered together
    this->onDrawEdgeAAImageSet2(imageSet, cnt, dstClips, preViewMatrices, sampling, paint,
                                constraint);
}

//////////////////////////////////////////////////////////////////////////////
//  These are the virtual drawing methods
//////////////////////////////////////////////////////////////////////////////

void SkCanvas::onDiscard() {
    if (fSurfaceBase) {
        sk_ignore_unused_variable(fSurfaceBase->aboutToDraw(SkSurface::kDiscard_ContentChangeMode));
    }
}

void SkCanvas::onDrawPaint(const SkPaint& paint) {
    this->internalDrawPaint(paint);
}

void SkCanvas::internalDrawPaint(const SkPaint& paint) {
    // drawPaint does not call internalQuickReject() because computing its geometry is not free
    // (see getLocalClipBounds(), and the two conditions below are sufficient.
    if (paint.nothingToDraw() || this->isClipEmpty()) {
        return;
    }

    auto layer = this->aboutToDraw(paint, nullptr, PredrawFlags::kCheckForOverwrite);
    if (layer) {
        this->topDevice()->drawPaint(layer->paint());
    }
}

void SkCanvas::onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                            const SkPaint& paint) {
    if ((long)count <= 0 || paint.nothingToDraw()) {
        return;
    }
    SkASSERT(pts != nullptr);

    // Enforce paint style matches implicit behavior of drawPoints
    SkPaint strokePaint = paint;
    strokePaint.setStyle(SkPaint::kStroke_Style);

    SkRect boundsStorage;
    const SkRect* boundsPtr = nullptr;

    /*
     *  Computing the bounds can actually slow us down (since we check inside).
     *  But if there is a filter, then it is useful to limit the size of
     *  its offscreen, hence we only compute it in those cases.
     *
     *  Note: it would be "correct" to never compute this, it is just considered
     *        an optimization opportunity.
     */
    if (paint.getImageFilter() || paint.getMaskFilter()) {
        auto bounds = SkRect::Bounds({pts, count});
        if (!bounds) {
            return;
        }
        if (this->internalQuickReject(bounds.value(), strokePaint)) {
            return;
        }
        boundsStorage = bounds.value();
        boundsPtr = &boundsStorage;
    }

    auto layer = this->aboutToDraw(strokePaint, boundsPtr);
    if (layer) {
        this->topDevice()->drawPoints(mode, {pts, count}, layer->paint());
    }
}

const SkBlurMaskFilterImpl* SkCanvas::canAttemptBlurredRRectDraw(const SkPaint& paint) const {
    if (!this->topDevice()->useDrawCoverageMaskForMaskFilters()) {
        // Perform a regular draw in the legacy mask filter case.
        return nullptr;
    }

    if (paint.getPathEffect()) {
        return nullptr;
    }

    // TODO: Once stroke-and-fill goes away, we can check the paint's style directly.
    if (SkStrokeRec(paint).getStyle() != SkStrokeRec::kFill_Style) {
        return nullptr;
    }

    const SkMaskFilterBase* maskFilter = as_MFB(paint.getMaskFilter());
    if (!maskFilter || maskFilter->type() != SkMaskFilterBase::Type::kBlur) {
        return nullptr;
    }

    const SkBlurMaskFilterImpl* blurMaskFilter =
            static_cast<const SkBlurMaskFilterImpl*>(maskFilter);
    if (blurMaskFilter->blurStyle() != kNormal_SkBlurStyle) {
        return nullptr;
    }

    if (!this->getTotalMatrix().isSimilarity()) {
        // TODO: If the CTM does more than just translation, rotation, and uniform scale, then the
        // results of analytic blurring will be different than mask filter blurring. Skip the
        // specialized path in this case.
        return nullptr;
    }

    return blurMaskFilter;
}

std::optional<AutoLayerForImageFilter> SkCanvas::attemptBlurredRRectDraw(
        const SkRRect& rrect,
        const SkBlurMaskFilterImpl* blurMaskFilter,
        const SkPaint& paint,
        SkEnumBitMask<PredrawFlags> flags) {
    SkASSERT(blurMaskFilter && blurMaskFilter == this->canAttemptBlurredRRectDraw(paint) &&
             !(flags & PredrawFlags::kSkipMaskFilterAutoLayer));
    const SkRect& bounds = rrect.getBounds();

    auto layer = this->aboutToDraw(paint, &bounds, flags | PredrawFlags::kSkipMaskFilterAutoLayer);
    if (!layer) {
        // predrawNotify failed.
        return std::nullopt;
    }

    const float deviceSigma = blurMaskFilter->computeXformedSigma(this->getTotalMatrix());
    if (this->topDevice()->drawBlurredRRect(rrect, layer->paint(), deviceSigma)) {
        // Analytic draw was successful.
        return std::nullopt;
    }

    // Fall back on a regular draw, adding any mask filter layer we skipped earlier. We know the
    // paint has a mask filter here, otherwise we would have failed the can_attempt check above.
    layer->addMaskFilterLayer(&bounds);
    return layer;
}

void SkCanvas::onDrawRect(const SkRect& r, const SkPaint& paint) {
    SkASSERT(r.isSorted());
    if (this->internalQuickReject(r, paint)) {
        return;
    }

    std::optional<AutoLayerForImageFilter> layer;
    constexpr PredrawFlags kPredrawFlags = PredrawFlags::kCheckForOverwrite;

    if (const SkBlurMaskFilterImpl* blurMaskFilter = this->canAttemptBlurredRRectDraw(paint)) {
        // Returns a layer if a blurred draw was unsuccessful.
        layer = this->attemptBlurredRRectDraw(
                SkRRect::MakeRect(r), blurMaskFilter, paint, kPredrawFlags);
    } else {
        layer = this->aboutToDraw(paint, &r, kPredrawFlags);
    }

    if (layer) {
        this->topDevice()->drawRect(r, layer->paint());
    }
}

void SkCanvas::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    const SkRect bounds = SkRect::Make(region.getBounds());
    if (this->internalQuickReject(bounds, paint)) {
        return;
    }

    auto layer = this->aboutToDraw(paint, &bounds);
    if (layer) {
        this->topDevice()->drawRegion(region, layer->paint());
    }
}

void SkCanvas::onDrawBehind(const SkPaint& paint) {
    SkDevice* dev = this->topDevice();
    if (!dev) {
        return;
    }

    SkIRect bounds;
    SkDeque::Iter iter(fMCStack, SkDeque::Iter::kBack_IterStart);
    for (;;) {
        const MCRec* rec = (const MCRec*)iter.prev();
        if (!rec) {
            return; // no backimages, so nothing to draw
        }
        if (rec->fBackImage) {
            // drawBehind should only have been called when the saveBehind record is active;
            // if this fails, it means a real saveLayer was made w/o being restored first.
            SkASSERT(dev == rec->fDevice);
            bounds = SkIRect::MakeXYWH(rec->fBackImage->fLoc.fX, rec->fBackImage->fLoc.fY,
                                       rec->fBackImage->fImage->width(),
                                       rec->fBackImage->fImage->height());
            break;
        }
    }

    // The backimage location (and thus bounds) were defined in the device's space, so mark it
    // as a clip. We use a clip instead of just drawing a rect in case the paint has an image
    // filter on it (which is applied before any auto-layer so the filter is clipped).
    dev->pushClipStack();
    {
        // We also have to temporarily whack the device matrix since clipRegion is affected by the
        // global-to-device matrix and clipRect is affected by the local-to-device.
        SkAutoDeviceTransformRestore adtr(dev, SkM44());
        dev->clipRect(SkRect::Make(bounds), SkClipOp::kIntersect, /* aa */ false);
        // ~adtr will reset the local-to-device matrix so that drawPaint() shades correctly.
    }

    auto layer = this->aboutToDraw(paint);
    if (layer) {
        this->topDevice()->drawPaint(layer->paint());
    }

    dev->popClipStack();
}

void SkCanvas::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    SkASSERT(oval.isSorted());
    if (this->internalQuickReject(oval, paint)) {
        return;
    }

    std::optional<AutoLayerForImageFilter> layer;

    if (const SkBlurMaskFilterImpl* blurMaskFilter = this->canAttemptBlurredRRectDraw(paint)) {
        // Returns a layer if a blurred draw was unsuccessful.
        layer = this->attemptBlurredRRectDraw(
                SkRRect::MakeOval(oval), blurMaskFilter, paint, PredrawFlags::kNone);
    } else {
        layer = this->aboutToDraw(paint, &oval);
    }

    if (layer) {
        this->topDevice()->drawOval(oval, layer->paint());
    }
}

void SkCanvas::onDrawArc(const SkRect& oval, SkScalar startAngle,
                         SkScalar sweepAngle, bool useCenter,
                         const SkPaint& paint) {
    SkASSERT(oval.isSorted());
    if (this->internalQuickReject(oval, paint)) {
        return;
    }

    std::optional<AutoLayerForImageFilter> layer;

    // Arcs with sweeps >= 360° are ovals. In this case, attempt a specialized blurred draw.
    if (const SkBlurMaskFilterImpl* blurMaskFilter = this->canAttemptBlurredRRectDraw(paint);
        blurMaskFilter && SkScalarAbs(sweepAngle) >= 360.f) {
        // Returns a layer if a blurred draw was unsuccessful.
        layer = this->attemptBlurredRRectDraw(
                SkRRect::MakeOval(oval), blurMaskFilter, paint, PredrawFlags::kNone);
    } else {
        layer = this->aboutToDraw(paint, &oval);
    }

    if (layer) {
        this->topDevice()->drawArc(SkArc::Make(oval, startAngle, sweepAngle, useCenter),
                                   layer->paint());
    }
}

void SkCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    const SkRect& bounds = rrect.getBounds();

    // Delegating to simpler draw operations
    if (rrect.isRect()) {
        // call the non-virtual version
        this->SkCanvas::drawRect(bounds, paint);
        return;
    } else if (rrect.isOval()) {
        // call the non-virtual version
        this->SkCanvas::drawOval(bounds, paint);
        return;
    }

    if (this->internalQuickReject(bounds, paint)) {
        return;
    }

    std::optional<AutoLayerForImageFilter> layer;

    if (const SkBlurMaskFilterImpl* blurMaskFilter = this->canAttemptBlurredRRectDraw(paint)) {
        // Returns a layer if a blurred draw was unsuccessful.
        layer = this->attemptBlurredRRectDraw(rrect, blurMaskFilter, paint, PredrawFlags::kNone);
    } else {
        layer = this->aboutToDraw(paint, &bounds);
    }

    if (layer) {
        this->topDevice()->drawRRect(rrect, layer->paint());
    }
}

void SkCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    const SkRect& bounds = outer.getBounds();
    if (this->internalQuickReject(bounds, paint)) {
        return;
    }

    auto layer = this->aboutToDraw(paint, &bounds);
    if (layer) {
        this->topDevice()->drawDRRect(outer, inner, layer->paint());
    }
}

void SkCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    if (!path.isFinite()) {
        return;
    }

    const SkRect& pathBounds = path.getBounds();
    if (!path.isInverseFillType() && this->internalQuickReject(pathBounds, paint)) {
        return;
    }
    if (path.isInverseFillType() && pathBounds.width() <= 0 && pathBounds.height() <= 0) {
        this->internalDrawPaint(paint);
        return;
    }

    auto layer = this->aboutToDraw(paint, path.isInverseFillType() ? nullptr : &pathBounds);
    if (layer) {
        this->topDevice()->drawPath(path, layer->paint(), false);
    }
}

// Clean-up the paint to match the drawing semantics for drawImage et al. (skbug.com/40039059).
static SkPaint clean_paint_for_drawImage(const SkPaint* paint) {
    SkPaint cleaned;
    if (paint) {
        cleaned = *paint;
        cleaned.setStyle(SkPaint::kFill_Style);
        cleaned.setPathEffect(nullptr);
    }
    return cleaned;
}

// drawVertices fills triangles and ignores mask filter and path effect,
// so canonicalize the paint before checking quick reject.
static SkPaint clean_paint_for_drawVertices(SkPaint paint) {
    paint.setStyle(SkPaint::kFill_Style);
    paint.setMaskFilter(nullptr);
    paint.setPathEffect(nullptr);
    return paint;
}

// TODO: Delete this since it is no longer used
void SkCanvas::onDrawImage2(const SkImage* image, SkScalar x, SkScalar y,
                            const SkSamplingOptions& sampling, const SkPaint* paint) {
    SkUNREACHABLE;
}

static SkSamplingOptions clean_sampling_for_constraint(
        const SkSamplingOptions& sampling,
        SkCanvas::SrcRectConstraint constraint) {
    if (constraint == SkCanvas::kStrict_SrcRectConstraint) {
        if (sampling.mipmap != SkMipmapMode::kNone) {
            return SkSamplingOptions(sampling.filter);
        }
        if (sampling.isAniso()) {
            return SkSamplingOptions(SkFilterMode::kLinear);
        }
    }
    return sampling;
}

void SkCanvas::onDrawImageRect2(const SkImage* image, const SkRect& src, const SkRect& dst,
                                const SkSamplingOptions& sampling, const SkPaint* paint,
                                SrcRectConstraint constraint) {
    SkPaint realPaint = clean_paint_for_drawImage(paint);
    SkSamplingOptions realSampling = clean_sampling_for_constraint(sampling, constraint);

    if (this->internalQuickReject(dst, realPaint)) {
        return;
    }

    if (this->topDevice()->shouldDrawAsTiledImageRect()) {
        if (this->topDevice()->drawAsTiledImageRect(
                    this, image, &src, dst, realSampling, realPaint, constraint)) {
            return;
        }
    }

    // drawImageRect()'s behavior is modified by the presence of an image filter, a mask filter, a
    // color filter, the paint's alpha, the paint's blender, and--when it's an alpha-only image--
    // the paint's color or shader. When there's an image filter, the paint's blender is applied to
    // the result of the image filter function, but every other aspect would influence the source
    // image that's then rendered with src-over blending into a transparent temporary layer.
    //
    // However, skif::FilterResult can apply the paint alpha and any color filter often without
    // requiring a layer, and src-over blending onto a transparent dst is a no-op, so we can use the
    // input image directly as the source for filtering. When the image is alpha-only and must be
    // colorized, or when a mask filter would change the coverage we skip this optimization for
    // simplicity since *somehow* embedding colorization or mask blurring into the filter graph
    // would likely be equivalent to using the existing AutoLayerForImageFilter functionality.
    if (realPaint.getImageFilter() && !image->isAlphaOnly() && !realPaint.getMaskFilter()) {
        SkDevice* device = this->topDevice();

        skif::ParameterSpace<SkRect> imageBounds{dst};
        skif::DeviceSpace<SkIRect> outputBounds{device->devClipBounds()};
        FilterToSpan filterAsSpan(realPaint.getImageFilter());
        auto mappingAndBounds = get_layer_mapping_and_bounds(filterAsSpan,
                                                             device->localToDevice44(),
                                                             outputBounds,
                                                             imageBounds);
        if (!mappingAndBounds) {
            return;
        }
        if (!this->predrawNotify()) {
            return;
        }

        // Start out with an empty source image, to be replaced with the converted 'image', and a
        // desired output equal to the calculated initial source layer bounds, which accounts for
        // how the image filters will access 'image' (possibly different than just 'outputBounds').
        auto backend = device->createImageFilteringBackend(
                device->surfaceProps(),
                image_filter_color_type(device->imageInfo().colorInfo()));
        auto [mapping, srcBounds] = *mappingAndBounds;
        skif::Stats stats;
        skif::Context ctx{std::move(backend),
                          mapping,
                          srcBounds,
                          skif::FilterResult{},
                          device->imageInfo().colorSpace(),
                          &stats};

        auto source = skif::FilterResult::MakeFromImage(
                ctx, sk_ref_sp(image), src, imageBounds, sampling);
        // Apply effects that are normally processed on the draw *before* any layer/image filter.
        source = apply_alpha_and_colorfilter(ctx, source, realPaint);

        // Evaluate the image filter, with a context pointing to the source created directly from
        // 'image' (which will not require intermediate renderpasses when 'src' is integer aligned).
        // and a desired output matching the device clip bounds.
        ctx = ctx.withNewDesiredOutput(mapping.deviceToLayer(outputBounds))
                 .withNewSource(source);
        auto result = as_IFB(realPaint.getImageFilter())->filterImage(ctx);
        result.draw(ctx, device, realPaint.getBlender());
        stats.reportStats();
        return;
    }

    if (realPaint.getMaskFilter() && this->topDevice()->useDrawCoverageMaskForMaskFilters()) {
        // Route mask-filtered drawImages to drawRect() to use the auto-layer for mask filters,
        // which require all shading to be encoded in the paint.
        SkRect drawDst = SkModifyPaintAndDstForDrawImageRect(
                image, sampling, src, dst, constraint == kStrict_SrcRectConstraint, &realPaint);
        if (drawDst.isEmpty()) {
            return;
        } else {
            this->drawRect(drawDst, realPaint);
            return;
        }
    }

    auto layer = this->aboutToDraw(realPaint, &dst,
                                   PredrawFlags::kCheckForOverwrite |
                                   (image->isOpaque() ? PredrawFlags::kOpaqueShaderOverride
                                                      : PredrawFlags::kNonOpaqueShaderOverride));
    if (layer) {
        this->topDevice()->drawImageRect(image, &src, dst, realSampling, layer->paint(),
                                         constraint);
    }
}

void SkCanvas::onDrawImageLattice2(const SkImage* image, const Lattice& lattice, const SkRect& dst,
                                   SkFilterMode filter, const SkPaint* paint) {
    SkPaint realPaint = clean_paint_for_drawImage(paint);

    if (this->internalQuickReject(dst, realPaint)) {
        return;
    }

    auto layer = this->aboutToDraw(realPaint, &dst);
    if (layer) {
        this->topDevice()->drawImageLattice(image, lattice, dst, filter, layer->paint());
    }
}

void SkCanvas::drawImage(const SkImage* image, SkScalar x, SkScalar y,
                         const SkSamplingOptions& sampling, const SkPaint* paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    RETURN_ON_NULL(image);

    this->drawImageRect(image,
                        /*src=*/SkRect::MakeWH(image->width(), image->height()),
                        /*dst=*/SkRect::MakeXYWH(x, y, image->width(), image->height()),
                        sampling,
                        paint,
                        kFast_SrcRectConstraint);
}

void SkCanvas::drawImageRect(const SkImage* image, const SkRect& src, const SkRect& dst,
                             const SkSamplingOptions& sampling, const SkPaint* paint,
                             SrcRectConstraint constraint) {
    RETURN_ON_NULL(image);
    if (!fillable(dst) || !fillable(src)) {
        return;
    }
    this->onDrawImageRect2(image, src, dst, sampling, paint, constraint);
}

void SkCanvas::drawImageRect(const SkImage* image, const SkRect& dst,
                             const SkSamplingOptions& sampling, const SkPaint* paint) {
    RETURN_ON_NULL(image);
    this->drawImageRect(image, SkRect::MakeIWH(image->width(), image->height()), dst, sampling,
                        paint, kFast_SrcRectConstraint);
}

void SkCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                              const SkPaint& paint) {
    auto glyphRunList = fScratchGlyphRunBuilder->blobToGlyphRunList(*blob, {x, y});
    this->onDrawGlyphRunList(glyphRunList, paint);
}

void SkCanvas::onDrawGlyphRunList(const sktext::GlyphRunList& glyphRunList, const SkPaint& paint) {
    SkRect bounds = glyphRunList.sourceBoundsWithOrigin();
    if (this->internalQuickReject(bounds, paint)) {
        return;
    }

    // Text attempts to apply any SkMaskFilter internally and save the blurred masks in the
    // strike cache; if a glyph must be drawn as a path or drawable, SkDevice routes back to
    // this SkCanvas to retry, which will go through a function that does *not* skip the mask
    // filter layer.
    auto layer = this->aboutToDraw(paint, &bounds, PredrawFlags::kSkipMaskFilterAutoLayer);
    if (layer) {
        this->topDevice()->drawGlyphRunList(this, glyphRunList, layer->paint());
    }
}

sk_sp<Slug> SkCanvas::convertBlobToSlug(
        const SkTextBlob& blob, SkPoint origin, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    auto glyphRunList = fScratchGlyphRunBuilder->blobToGlyphRunList(blob, origin);
    return this->onConvertGlyphRunListToSlug(glyphRunList, paint);
}

sk_sp<Slug> SkCanvas::onConvertGlyphRunListToSlug(const sktext::GlyphRunList& glyphRunList,
                                                  const SkPaint& paint) {
    SkRect bounds = glyphRunList.sourceBoundsWithOrigin();
    if (bounds.isEmpty() || !bounds.isFinite() || paint.nothingToDraw()) {
        return nullptr;
    }
    // See comment in onDrawGlyphRunList()
    auto layer = this->aboutToDraw(paint, &bounds, PredrawFlags::kSkipMaskFilterAutoLayer);
    if (layer) {
        return this->topDevice()->convertGlyphRunListToSlug(glyphRunList, layer->paint());
    }
    return nullptr;
}

void SkCanvas::drawSlug(const Slug* slug, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (slug) {
        this->onDrawSlug(slug, paint);
    }
}

void SkCanvas::onDrawSlug(const Slug* slug, const SkPaint& paint) {
    SkRect bounds = slug->sourceBoundsWithOrigin();
    if (this->internalQuickReject(bounds, paint)) {
        return;
    }
    // See comment in onDrawGlyphRunList()
    auto layer = this->aboutToDraw(paint, &bounds, PredrawFlags::kSkipMaskFilterAutoLayer);
    if (layer) {
        this->topDevice()->drawSlug(this, slug, layer->paint());
    }
}

// These call the (virtual) onDraw... method
void SkCanvas::drawSimpleText(const void* text, size_t byteLength, SkTextEncoding encoding,
                              SkScalar x, SkScalar y, const SkFont& font, const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (byteLength) {
        sk_msan_assert_initialized(text, SkTAddOffset<const void>(text, byteLength));
        const sktext::GlyphRunList& glyphRunList =
            fScratchGlyphRunBuilder->textToGlyphRunList(
                    font, paint, text, byteLength, {x, y}, encoding);
        if (!glyphRunList.empty()) {
            this->onDrawGlyphRunList(glyphRunList, paint);
        }
    }
}

void SkCanvas::drawGlyphs(SkSpan<const SkGlyphID> glyphs, SkSpan<const SkPoint> positions,
                          SkSpan<const uint32_t> clusters, SkSpan<const char> utf8text,
                          SkPoint origin, const SkFont& font, const SkPaint& paint) {
    if (glyphs.empty()) { return; }

    sktext::GlyphRun glyphRun {
            font,
            positions,
            glyphs,
            utf8text,
            clusters,
            SkSpan<SkVector>()
    };

    sktext::GlyphRunList glyphRunList = fScratchGlyphRunBuilder->makeGlyphRunList(
            glyphRun, paint, origin);
    this->onDrawGlyphRunList(glyphRunList, paint);
}

void SkCanvas::drawGlyphs(SkSpan<const SkGlyphID> glyphs, SkSpan<const SkPoint> positions,
                          SkPoint origin, const SkFont& font, const SkPaint& paint) {
    if (glyphs.empty()) { return; }

    sktext::GlyphRun glyphRun {
        font,
        positions,
        glyphs,
        SkSpan<const char>(),
        SkSpan<const uint32_t>(),
        SkSpan<SkVector>()
    };

    sktext::GlyphRunList glyphRunList = fScratchGlyphRunBuilder->makeGlyphRunList(
            glyphRun, paint, origin);
    this->onDrawGlyphRunList(glyphRunList, paint);
}

void SkCanvas::drawGlyphsRSXform(SkSpan<const SkGlyphID> glyphs, SkSpan<const SkRSXform> xforms,
                                 SkPoint origin, const SkFont& font, const SkPaint& paint) {
    if (glyphs.empty()) { return; }

    auto [positions, rotateScales] =
            fScratchGlyphRunBuilder->convertRSXForm(xforms);

    sktext::GlyphRun glyphRun {
            font,
            positions,
            glyphs,
            SkSpan<const char>(),
            SkSpan<const uint32_t>(),
            rotateScales
    };
    sktext::GlyphRunList glyphRunList = fScratchGlyphRunBuilder->makeGlyphRunList(
            glyphRun, paint, origin);
    this->onDrawGlyphRunList(glyphRunList, paint);
}

void SkCanvas::drawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                            const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    RETURN_ON_NULL(blob);
    RETURN_ON_FALSE(blob->bounds().makeOffset(x, y).isFinite());

    // Overflow if more than 2^21 glyphs stopping a buffer overflow latter in the stack.
    // See chromium:1080481
    // TODO: can consider unrolling a few at a time if this limit becomes a problem.
    int totalGlyphCount = 0;
    constexpr int kMaxGlyphCount = 1 << 21;
    SkTextBlob::Iter i(*blob);
    SkTextBlob::Iter::Run r;
    while (i.next(&r)) {
        int glyphsLeft = kMaxGlyphCount - totalGlyphCount;
        RETURN_ON_FALSE(r.fGlyphCount <= glyphsLeft);
        totalGlyphCount += r.fGlyphCount;
    }

    this->onDrawTextBlob(blob, x, y, paint);
}

void SkCanvas::onDrawVerticesObject(const SkVertices* vertices, SkBlendMode bmode,
                                    const SkPaint& paint) {
    SkPaint simplePaint = clean_paint_for_drawVertices(paint);

    const SkRect& bounds = vertices->bounds();
    if (this->internalQuickReject(bounds, simplePaint)) {
        return;
    }

    auto layer = this->aboutToDraw(simplePaint, &bounds);
    if (layer) {
        this->topDevice()->drawVertices(vertices, SkBlender::Mode(bmode), layer->paint());
    }
}

void SkCanvas::onDrawMesh(const SkMesh& mesh, sk_sp<SkBlender> blender, const SkPaint& paint) {
    SkPaint simplePaint = clean_paint_for_drawVertices(paint);
    auto layer = this->aboutToDraw(simplePaint, nullptr);
    if (layer) {
        this->topDevice()->drawMesh(mesh, std::move(blender), paint);
    }
}

void SkCanvas::drawPatch(const SkPoint cubics[12], const SkColor colors[4],
                         const SkPoint texCoords[4], SkBlendMode bmode,
                         const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (nullptr == cubics) {
        return;
    }

    this->onDrawPatch(cubics, colors, texCoords, bmode, paint);
}

void SkCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                           const SkPoint texCoords[4], SkBlendMode bmode,
                           const SkPaint& paint) {
    auto bounds = SkRect::Bounds({cubics, (size_t)SkPatchUtils::kNumCtrlPts});
    if (!bounds) {
        return; // we don't draw if the bounds are not finite
    }

    // drawPatch has the same behavior restrictions as drawVertices
    SkPaint simplePaint = clean_paint_for_drawVertices(paint);

    // Since a patch is always within the convex hull of the control points, we discard it when its
    // bounding rectangle is completely outside the current clip.
    if (this->internalQuickReject(bounds.value(), simplePaint)) {
        return;
    }

    auto r = bounds.value();
    auto layer = this->aboutToDraw(simplePaint, &r);
    if (layer) {
        this->topDevice()->drawPatch(cubics, colors, texCoords, SkBlender::Mode(bmode),
                                     layer->paint());
    }
}

void SkCanvas::drawDrawable(SkDrawable* dr, SkScalar x, SkScalar y) {
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
    TRACE_EVENT0("skia", TRACE_FUNC);
#endif
    RETURN_ON_NULL(dr);
    if (x || y) {
        SkMatrix matrix = SkMatrix::Translate(x, y);
        this->onDrawDrawable(dr, &matrix);
    } else {
        this->onDrawDrawable(dr, nullptr);
    }
}

void SkCanvas::drawDrawable(SkDrawable* dr, const SkMatrix* matrix) {
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
    TRACE_EVENT0("skia", TRACE_FUNC);
#endif
    RETURN_ON_NULL(dr);
    if (matrix && matrix->isIdentity()) {
        matrix = nullptr;
    }
    this->onDrawDrawable(dr, matrix);
}

void SkCanvas::onDrawDrawable(SkDrawable* dr, const SkMatrix* matrix) {
    // drawable bounds are no longer reliable (e.g. android displaylist)
    // so don't use them for quick-reject
    if (this->predrawNotify()) {
        this->topDevice()->drawDrawable(this, dr, matrix);
    }
}

void SkCanvas::onDrawAtlas2(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                            const SkColor colors[], int count, SkBlendMode bmode,
                            const SkSamplingOptions& sampling, const SkRect* cull,
                            const SkPaint* paint) {
    // drawAtlas is a combination of drawVertices and drawImage...
    SkPaint realPaint = clean_paint_for_drawVertices(clean_paint_for_drawImage(paint));
    realPaint.setShader(atlas->makeShader(sampling));

    if (cull && this->internalQuickReject(*cull, realPaint)) {
        return;
    }

    // drawAtlas should not have mask filters on its paint, so we don't need to worry about
    // converting its "drawImage" behavior into the paint to work with the auto-mask-filter system.
    SkASSERT(!realPaint.getMaskFilter());
    auto layer = this->aboutToDraw(realPaint);
    if (layer) {
        this->topDevice()->drawAtlas({xform, count}, {tex, count}, {colors, colors ? count : 0},
                                     SkBlender::Mode(bmode), layer->paint());
    }
}

void SkCanvas::onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    SkASSERT(key);

    if (this->predrawNotify()) {
        this->topDevice()->drawAnnotation(rect, key, value);
    }
}

void SkCanvas::onDrawEdgeAAQuad(const SkRect& r, const SkPoint clip[4], QuadAAFlags edgeAA,
                                const SkColor4f& color, SkBlendMode mode) {
    SkASSERT(r.isSorted());

    SkPaint paint{color};
    paint.setBlendMode(mode);
    if (this->internalQuickReject(r, paint)) {
        return;
    }

    if (this->predrawNotify()) {
        this->topDevice()->drawEdgeAAQuad(r, clip, edgeAA, color, mode);
    }
}

void SkCanvas::onDrawEdgeAAImageSet2(const ImageSetEntry imageSet[], int count,
                                     const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                     const SkSamplingOptions& sampling, const SkPaint* paint,
                                     SrcRectConstraint constraint) {
    if (count <= 0) {
        // Nothing to draw
        return;
    }

    SkPaint realPaint = clean_paint_for_drawImage(paint);
    SkSamplingOptions realSampling = clean_sampling_for_constraint(sampling, constraint);

    // We could calculate the set's dstRect union to always check quickReject(), but we can't reject
    // individual entries and Chromium's occlusion culling already makes it likely that at least one
    // entry will be visible. So, we only calculate the draw bounds when it's trivial (count == 1),
    // or we need it for the autolooper (since it greatly improves image filter perf).
    bool needsAutoLayer = SkToBool(realPaint.getImageFilter() || realPaint.getMaskFilter());
    bool setBoundsValid = count == 1 || needsAutoLayer;
    SkRect setBounds = imageSet[0].fDstRect;
    if (imageSet[0].fMatrixIndex >= 0) {
        // Account for the per-entry transform that is applied prior to the CTM when drawing
        preViewMatrices[imageSet[0].fMatrixIndex].mapRect(&setBounds);
    }
    if (needsAutoLayer) {
        for (int i = 1; i < count; ++i) {
            SkRect entryBounds = imageSet[i].fDstRect;
            if (imageSet[i].fMatrixIndex >= 0) {
                preViewMatrices[imageSet[i].fMatrixIndex].mapRect(&entryBounds);
            }
            setBounds.joinPossiblyEmptyRect(entryBounds);
        }
    }

    // If we happen to have the draw bounds, though, might as well check quickReject().
    if (setBoundsValid && this->internalQuickReject(setBounds, realPaint)) {
        return;
    }

    if (realPaint.getMaskFilter() && this->topDevice()->useDrawCoverageMaskForMaskFilters()) {
        // Route mask-filtered drawEdgeAAImageSets to drawEdgeAAQuad() or drawImageRect()
        // to use the auto-layer for mask filters, which require all shading to be encoded in
        // the paint.
        int dstClipIndex = 0;
        for (int i = 0; i < count; ++i) {
            SkPaint imagePaint = realPaint;
            SkRect drawDst = SkModifyPaintAndDstForDrawImageRect(
                                imageSet[i].fImage.get(), sampling,
                                imageSet[i].fSrcRect, imageSet[i].fDstRect,
                                constraint == kStrict_SrcRectConstraint, &imagePaint);
            if (drawDst.isEmpty()) {
                return;
            }

            auto layer = this->aboutToDraw(imagePaint, &drawDst);
            if (layer) {
                // Since we can't call mapRect to apply any preview matrix and drawEdgeAAQuad
                // doesn't take an optional matrix, we can modify the local-to-device matrix
                // of the layers top device.
                if (imageSet[i].fMatrixIndex >= 0) {
                    this->topDevice()->setLocalToDevice(
                        this->topDevice()->localToDevice44() *
                        SkM44(preViewMatrices[imageSet[i].fMatrixIndex]));
                }

                // Call drawEdgeAAImageSet on each image one at a time, to correctly
                // paint the image.
                this->topDevice()->drawEdgeAAQuad(drawDst,
                                                  imageSet[i].fHasClip ? dstClips + dstClipIndex
                                                                        : nullptr,
                                                  (QuadAAFlags)imageSet[i].fAAFlags,
                                                  layer->paint().getColor4f(),
                                                  SkBlendMode::kSrcOver);
            }
            dstClipIndex += 4 * imageSet[i].fHasClip;
        }
        return;
    }

    auto layer = this->aboutToDraw(realPaint, setBoundsValid ? &setBounds : nullptr);
    if (layer) {
        this->topDevice()->drawEdgeAAImageSet(imageSet, count, dstClips, preViewMatrices,
                                              realSampling, layer->paint(), constraint);
    }
}

//////////////////////////////////////////////////////////////////////////////
// These methods are NOT virtual, and therefore must call back into virtual
// methods, rather than actually drawing themselves.
//////////////////////////////////////////////////////////////////////////////

void SkCanvas::drawColor(const SkColor4f& c, SkBlendMode mode) {
    SkPaint paint;
    paint.setColor(c);
    paint.setBlendMode(mode);
    this->drawPaint(paint);
}

void SkCanvas::drawPoint(SkScalar x, SkScalar y, const SkPaint& paint) {
    const SkPoint pt[1] = {{ x, y }};
    this->drawPoints(kPoints_PointMode, pt, paint);
}

void SkCanvas::drawLine(SkScalar x0, SkScalar y0, SkScalar x1, SkScalar y1, const SkPaint& paint) {
    SkPoint pts[2];
    pts[0].set(x0, y0);
    pts[1].set(x1, y1);
    this->drawPoints(kLines_PointMode, pts, paint);
}

void SkCanvas::drawCircle(SkScalar cx, SkScalar cy, SkScalar radius, const SkPaint& paint) {
    if (radius < 0) {
        radius = 0;
    }

    SkRect  r;
    r.setLTRB(cx - radius, cy - radius, cx + radius, cy + radius);
    this->drawOval(r, paint);
}

void SkCanvas::drawRoundRect(const SkRect& r, SkScalar rx, SkScalar ry,
                             const SkPaint& paint) {
    if (rx > 0 && ry > 0) {
        SkRRect rrect;
        rrect.setRectXY(r, rx, ry);
        this->drawRRect(rrect, paint);
    } else {
        this->drawRect(r, paint);
    }
}

void SkCanvas::drawArc(const SkRect& oval, SkScalar startAngle,
                       SkScalar sweepAngle, bool useCenter,
                       const SkPaint& paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (oval.isEmpty() || !sweepAngle) {
        return;
    }
    this->onDrawArc(oval, startAngle, sweepAngle, useCenter, paint);
}

///////////////////////////////////////////////////////////////////////////////
#ifdef SK_DISABLE_SKPICTURE
void SkCanvas::drawPicture(const SkPicture* picture, const SkMatrix* matrix, const SkPaint* paint) {}


void SkCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                             const SkPaint* paint) {}
#else

void SkCanvas::drawPicture(const SkPicture* picture, const SkMatrix* matrix, const SkPaint* paint) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    RETURN_ON_NULL(picture);

    if (matrix && matrix->isIdentity()) {
        matrix = nullptr;
    }
    if (picture->approximateOpCount() <= kMaxPictureOpsToUnrollInsteadOfRef) {
        SkAutoCanvasMatrixPaint acmp(this, matrix, paint, picture->cullRect());
        picture->playback(this);
    } else {
        this->onDrawPicture(picture, matrix, paint);
    }
}

void SkCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                             const SkPaint* paint) {
    if (this->internalQuickReject(picture->cullRect(), paint ? *paint : SkPaint{}, matrix)) {
        return;
    }

    SkAutoCanvasMatrixPaint acmp(this, matrix, paint, picture->cullRect());
    picture->playback(this);
}
#endif

///////////////////////////////////////////////////////////////////////////////

SkCanvas::ImageSetEntry::ImageSetEntry() = default;
SkCanvas::ImageSetEntry::~ImageSetEntry() = default;
SkCanvas::ImageSetEntry::ImageSetEntry(const ImageSetEntry&) = default;
SkCanvas::ImageSetEntry& SkCanvas::ImageSetEntry::operator=(const ImageSetEntry&) = default;

SkCanvas::ImageSetEntry::ImageSetEntry(sk_sp<const SkImage> image, const SkRect& srcRect,
                                       const SkRect& dstRect, int matrixIndex, float alpha,
                                       unsigned aaFlags, bool hasClip)
                : fImage(std::move(image))
                , fSrcRect(srcRect)
                , fDstRect(dstRect)
                , fMatrixIndex(matrixIndex)
                , fAlpha(alpha)
                , fAAFlags(aaFlags)
                , fHasClip(hasClip) {}

SkCanvas::ImageSetEntry::ImageSetEntry(sk_sp<const SkImage> image, const SkRect& srcRect,
                                       const SkRect& dstRect, float alpha, unsigned aaFlags)
                : fImage(std::move(image))
                , fSrcRect(srcRect)
                , fDstRect(dstRect)
                , fAlpha(alpha)
                , fAAFlags(aaFlags) {}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkCanvas> SkCanvas::MakeRasterDirect(const SkImageInfo& info, void* pixels,
                                                     size_t rowBytes, const SkSurfaceProps* props) {
    if (!SkSurfaceValidateRasterInfo(info, rowBytes)) {
        return nullptr;
    }

    SkBitmap bitmap;
    if (!bitmap.installPixels(info, pixels, rowBytes)) {
        return nullptr;
    }

    return props ?
        std::make_unique<SkCanvas>(bitmap, *props) :
        std::make_unique<SkCanvas>(bitmap);
}

///////////////////////////////////////////////////////////////////////////////

SkNoDrawCanvas::SkNoDrawCanvas(int width, int height)
    : INHERITED(SkIRect::MakeWH(width, height)) {}

SkNoDrawCanvas::SkNoDrawCanvas(const SkIRect& bounds)
    : INHERITED(bounds) {}

SkCanvas::SaveLayerStrategy SkNoDrawCanvas::getSaveLayerStrategy(const SaveLayerRec& rec) {
    (void)this->INHERITED::getSaveLayerStrategy(rec);
    return kNoLayer_SaveLayerStrategy;
}

bool SkNoDrawCanvas::onDoSaveBehind(const SkRect*) {
    return false;
}

///////////////////////////////////////////////////////////////////////////////

static_assert((int)SkRegion::kDifference_Op == (int)SkClipOp::kDifference, "");
static_assert((int)SkRegion::kIntersect_Op  == (int)SkClipOp::kIntersect, "");

///////////////////////////////////////////////////////////////////////////////////////////////////

SkRasterHandleAllocator::Handle SkCanvas::accessTopRasterHandle() const {
    const SkDevice* dev = this->topDevice();
    if (fAllocator) {
        SkRasterHandleAllocator::Handle handle = dev->getRasterHandle();
        SkIRect clip = dev->devClipBounds();
        if (!clip.intersect({0, 0, dev->width(), dev->height()})) {
            clip.setEmpty();
        }

        fAllocator->updateHandle(handle, dev->localToDevice(), clip);
        return handle;
    }
    return nullptr;
}

static bool install(SkBitmap* bm, const SkImageInfo& info,
                    const SkRasterHandleAllocator::Rec& rec) {
    return bm->installPixels(info, rec.fPixels, rec.fRowBytes, rec.fReleaseProc, rec.fReleaseCtx);
}

SkRasterHandleAllocator::Handle SkRasterHandleAllocator::allocBitmap(const SkImageInfo& info,
                                                                     SkBitmap* bm) {
    SkRasterHandleAllocator::Rec rec;
    if (!this->allocHandle(info, &rec) || !install(bm, info, rec)) {
        return nullptr;
    }
    return rec.fHandle;
}

std::unique_ptr<SkCanvas>
SkRasterHandleAllocator::MakeCanvas(std::unique_ptr<SkRasterHandleAllocator> alloc,
                                    const SkImageInfo& info, const Rec* rec,
                                    const SkSurfaceProps* props) {
    if (!alloc || !SkSurfaceValidateRasterInfo(info, rec ? rec->fRowBytes : kIgnoreRowBytesValue)) {
        return nullptr;
    }

    SkBitmap bm;
    Handle hndl;

    if (rec) {
        hndl = install(&bm, info, *rec) ? rec->fHandle : nullptr;
    } else {
        hndl = alloc->allocBitmap(info, &bm);
    }
    return hndl ? std::unique_ptr<SkCanvas>(new SkCanvas(bm, std::move(alloc), hndl, props))
                : nullptr;
}
