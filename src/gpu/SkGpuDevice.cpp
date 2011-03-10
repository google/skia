/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrContext.h"
#include "GrTextContext.h"

#include "SkGpuDevice.h"
#include "SkGpuDeviceFactory.h"
#include "SkGrTexturePixelRef.h"

#include "SkDrawProcs.h"
#include "SkGlyphCache.h"

#define CACHE_LAYER_TEXTURES 1

#if 0
    extern bool (*gShouldDrawProc)();
    #define CHECK_SHOULD_DRAW(draw)                             \
        do {                                                    \
            if (gShouldDrawProc && !gShouldDrawProc()) return;  \
            this->prepareRenderTarget(draw);                    \
        } while (0)
#else
    #define CHECK_SHOULD_DRAW(draw) this->prepareRenderTarget(draw)
#endif

class SkAutoExtMatrix {
public:
    SkAutoExtMatrix(const SkMatrix* extMatrix) {
        if (extMatrix) {
            SkGr::SkMatrix2GrMatrix(*extMatrix, &fMatrix);
            fExtMatrix = &fMatrix;
        } else {
            fExtMatrix = NULL;
        }
    }
    const GrMatrix* extMatrix() const { return fExtMatrix; }

private:
    GrMatrix    fMatrix;
    GrMatrix*   fExtMatrix; // NULL or &fMatrix
};

///////////////////////////////////////////////////////////////////////////////

SkGpuDevice::SkAutoCachedTexture::
             SkAutoCachedTexture(SkGpuDevice* device,
                                 const SkBitmap& bitmap,
                                 const GrSamplerState& sampler,
                                 GrTexture** texture) {
    GrAssert(texture);
    fTex = NULL;
    *texture = this->set(device, bitmap, sampler);
}

SkGpuDevice::SkAutoCachedTexture::SkAutoCachedTexture() {
    fTex = NULL;
}

GrTexture* SkGpuDevice::SkAutoCachedTexture::set(SkGpuDevice* device,
                                                 const SkBitmap& bitmap,
                                                 const GrSamplerState& sampler) {
    if (fTex) {
        fDevice->unlockCachedTexture(fTex);
    }
    fDevice = device;
    GrTexture* texture = (GrTexture*)bitmap.getTexture();
    if (texture) {
        // return the native texture
        fTex = NULL;
    } else {
        // look it up in our cache
        fTex = device->lockCachedTexture(bitmap, sampler, &texture, false);
    }
    return texture;
}

SkGpuDevice::SkAutoCachedTexture::~SkAutoCachedTexture() {
    if (fTex) {
        fDevice->unlockCachedTexture(fTex);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool gDoTraceDraw;

struct GrSkDrawProcs : public SkDrawProcs {
public:
    GrContext* fContext;
    GrTextContext* fTextContext;
    GrFontScaler* fFontScaler;  // cached in the skia glyphcache
};

///////////////////////////////////////////////////////////////////////////////

GrRenderTarget* SkGpuDevice::Current3DApiRenderTarget() {
    return (GrRenderTarget*) -1;
}

SkGpuDevice::SkGpuDevice(GrContext* context,
                         const SkBitmap& bitmap,
                         GrRenderTarget* renderTargetOrNull)
        : SkDevice(NULL, bitmap, (NULL == renderTargetOrNull)) {

    fNeedPrepareRenderTarget = false;
    fDrawProcs = NULL;

    fContext = context;
    fContext->ref();

    fCache = NULL;
    fTexture = NULL;
    fRenderTarget = NULL;
    fNeedClear = false;

    if (NULL == renderTargetOrNull) {
        SkBitmap::Config c = bitmap.config();
        if (c != SkBitmap::kRGB_565_Config) {
            c = SkBitmap::kARGB_8888_Config;
        }
        SkBitmap bm;
        bm.setConfig(c, this->width(), this->height());

#if CACHE_LAYER_TEXTURES

        fCache = this->lockCachedTexture(bm, GrSamplerState::ClampNoFilter(),
                       &fTexture, true);
        if (fCache) {
            SkASSERT(NULL != fTexture);
            SkASSERT(NULL != fTexture->asRenderTarget());
        }
#else
        const GrGpu::TextureDesc desc = {
            GrGpu::kRenderTarget_TextureFlag,
            GrGpu::kNone_AALevel,
            this->width(),
            this->height(),
            SkGr::Bitmap2PixelConfig(bm)
        };

        fTexture = fContext->createUncachedTexture(desc, NULL, 0);
#endif
        if (NULL != fTexture) {
            fRenderTarget = fTexture->asRenderTarget();

            GrAssert(NULL != fRenderTarget);

            // we defer the actual clear until our gainFocus()
            fNeedClear = true;

            // wrap the bitmap with a pixelref to expose our texture
            SkGrTexturePixelRef* pr = new SkGrTexturePixelRef(fTexture);
            this->setPixelRef(pr, 0)->unref();
        } else {
            GrPrintf("--- failed to create gpu-offscreen [%d %d]\n",
                     this->width(), this->height());
            GrAssert(false);
        }
    } else if (Current3DApiRenderTarget() == renderTargetOrNull) {
        fRenderTarget = fContext->createRenderTargetFrom3DApiState();
    } else {
        fRenderTarget = renderTargetOrNull;
        fRenderTarget->ref();
    }
}

SkGpuDevice::~SkGpuDevice() {
    if (fDrawProcs) {
        delete fDrawProcs;
    }

    if (fCache) {
        GrAssert(NULL != fTexture);
        GrAssert(fRenderTarget == fTexture->asRenderTarget());
        // IMPORTANT: reattach the rendertarget/tex back to the cache.
        fContext->reattachAndUnlockCachedTexture((GrTextureEntry*)fCache);
    } else if (NULL != fTexture) {
        GrAssert(!CACHE_LAYER_TEXTURES);
        GrAssert(fRenderTarget == fTexture->asRenderTarget());
        fTexture->unref();
    } else if (NULL != fRenderTarget) {
        fRenderTarget->unref();
    }
    fContext->unref();
}

intptr_t SkGpuDevice::getLayerTextureHandle() const {
    if (fTexture) {
        return fTexture->getTextureHandle();
    } else {
        return 0;
    }
}
///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::makeRenderTargetCurrent() {
    fContext->setRenderTarget(fRenderTarget);
    fContext->flush(true);
    fNeedPrepareRenderTarget = true;
}

///////////////////////////////////////////////////////////////////////////////

bool SkGpuDevice::readPixels(const SkIRect& srcRect, SkBitmap* bitmap) {
    SkIRect bounds;
    bounds.set(0, 0, this->width(), this->height());
    if (!bounds.intersect(srcRect)) {
        return false;
    }

    const int w = bounds.width();
    const int h = bounds.height();
    SkBitmap tmp;
    // note we explicitly specify our rowBytes to be snug (no gap between rows)
    tmp.setConfig(SkBitmap::kARGB_8888_Config, w, h, w * 4);
    if (!tmp.allocPixels()) {
        return false;
    }

    SkAutoLockPixels alp(tmp);
    fContext->setRenderTarget(fRenderTarget);
    // we aren't setting the clip or matrix, so mark as dirty
    // we don't need to set them for this call and don't have them anyway
    fNeedPrepareRenderTarget = true;

    if (!fContext->readPixels(bounds.fLeft, bounds.fTop,
                              bounds.width(), bounds.height(),
                              GrTexture::kRGBA_8888_PixelConfig,
                              tmp.getPixels())) {
        return false;
    }

    tmp.swap(*bitmap);
    return true;
}

void SkGpuDevice::writePixels(const SkBitmap& bitmap, int x, int y) {
    SkAutoLockPixels alp(bitmap);
    if (!bitmap.readyToDraw()) {
        return;
    }
    GrTexture::PixelConfig config = SkGr::BitmapConfig2PixelConfig(bitmap.config(),
                                                                   bitmap.isOpaque());
    fContext->setRenderTarget(fRenderTarget);
    // we aren't setting the clip or matrix, so mark as dirty
    // we don't need to set them for this call and don't have them anyway
    fNeedPrepareRenderTarget = true;

    fContext->writePixels(x, y, bitmap.width(), bitmap.height(),
                          config, bitmap.getPixels(), bitmap.rowBytes());
}

///////////////////////////////////////////////////////////////////////////////

static void convert_matrixclip(GrContext* context, const SkMatrix& matrix,
                               const SkClipStack& clipStack,
                               const SkRegion& clipRegion,
                               const SkIPoint& origin) {
    GrMatrix grmat;
    SkGr::SkMatrix2GrMatrix(matrix, &grmat);
    context->setMatrix(grmat);

    SkGrClipIterator iter;
    iter.reset(clipStack);
    const SkIRect& skBounds = clipRegion.getBounds();
    GrRect bounds;
    bounds.setLTRB(GrIntToScalar(skBounds.fLeft),
                   GrIntToScalar(skBounds.fTop),
                   GrIntToScalar(skBounds.fRight),
                   GrIntToScalar(skBounds.fBottom));
    GrClip grc(&iter, GrIntToScalar(-origin.x()), GrIntToScalar(-origin.y()),
               &bounds);
    context->setClip(grc);
}

// call this ever each draw call, to ensure that the context reflects our state,
// and not the state from some other canvas/device
void SkGpuDevice::prepareRenderTarget(const SkDraw& draw) {
    if (fNeedPrepareRenderTarget ||
        fContext->getRenderTarget() != fRenderTarget) {

        fContext->setRenderTarget(fRenderTarget);
        SkASSERT(draw.fClipStack);
        convert_matrixclip(fContext, *draw.fMatrix,
                           *draw.fClipStack, *draw.fClip, this->getOrigin());
        fNeedPrepareRenderTarget = false;
    }
}

void SkGpuDevice::setMatrixClip(const SkMatrix& matrix, const SkRegion& clip,
                                const SkClipStack& clipStack) {
    this->INHERITED::setMatrixClip(matrix, clip, clipStack);

    convert_matrixclip(fContext, matrix, clipStack, clip, this->getOrigin());
}

void SkGpuDevice::gainFocus(SkCanvas* canvas, const SkMatrix& matrix,
                            const SkRegion& clip, const SkClipStack& clipStack) {

    fContext->setRenderTarget(fRenderTarget);

    this->INHERITED::gainFocus(canvas, matrix, clip, clipStack);

    convert_matrixclip(fContext, matrix, clipStack, clip, this->getOrigin());

    if (fNeedClear) {
        fContext->eraseColor(0x0);
        fNeedClear = false;
    }
}

bool SkGpuDevice::bindDeviceAsTexture(GrPaint* paint) {
    if (NULL != fTexture) {
        paint->setTexture(fTexture);
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

SK_COMPILE_ASSERT(SkShader::kNone_BitmapType == 0, shader_type_mismatch);
SK_COMPILE_ASSERT(SkShader::kDefault_BitmapType == 1, shader_type_mismatch);
SK_COMPILE_ASSERT(SkShader::kRadial_BitmapType == 2, shader_type_mismatch);
SK_COMPILE_ASSERT(SkShader::kSweep_BitmapType == 3, shader_type_mismatch);
SK_COMPILE_ASSERT(SkShader::kTwoPointRadial_BitmapType == 4,
                  shader_type_mismatch);
SK_COMPILE_ASSERT(SkShader::kLast_BitmapType == 4, shader_type_mismatch);

static const GrSamplerState::SampleMode sk_bmp_type_to_sample_mode[] = {
    (GrSamplerState::SampleMode) -1,                    // kNone_BitmapType
    GrSamplerState::kNormal_SampleMode,                 // kDefault_BitmapType
    GrSamplerState::kRadial_SampleMode,                 // kRadial_BitmapType
    GrSamplerState::kSweep_SampleMode,                  // kSweep_BitmapType
    GrSamplerState::kRadial2_SampleMode,                // kTwoPointRadial_BitmapType
};

bool SkGpuDevice::skPaint2GrPaintNoShader(const SkPaint& skPaint,
                                          bool justAlpha,
                                          GrPaint* grPaint) {

    grPaint->fDither    = skPaint.isDither();
    grPaint->fAntiAlias = skPaint.isAntiAlias();

    SkXfermode::Coeff sm = SkXfermode::kOne_Coeff;
    SkXfermode::Coeff dm = SkXfermode::kISA_Coeff;

    SkXfermode* mode = skPaint.getXfermode();
    if (mode) {
        if (!mode->asCoeff(&sm, &dm)) {
            SkDEBUGCODE(SkDebugf("Unsupported xfer mode.\n");)
#if 0
            return false;
#endif
        }
    }
    grPaint->fSrcBlendCoeff = sk_blend_to_grblend(sm);
    grPaint->fDstBlendCoeff = sk_blend_to_grblend(dm);

    if (justAlpha) {
        uint8_t alpha = skPaint.getAlpha();
        grPaint->fColor = GrColorPackRGBA(alpha, alpha, alpha, alpha);
    } else {
        grPaint->fColor = SkGr::SkColor2GrColor(skPaint.getColor());
        grPaint->setTexture(NULL);
    }
    return true;
}

bool SkGpuDevice::skPaint2GrPaintShader(const SkPaint& skPaint,
                                        SkAutoCachedTexture* act,
                                        const SkMatrix& ctm,
                                        GrPaint* grPaint) {

    SkASSERT(NULL != act);

    SkShader* shader = skPaint.getShader();
    if (NULL == shader) {
        return this->skPaint2GrPaintNoShader(skPaint, false, grPaint);
        grPaint->setTexture(NULL);
        return true;
    } else if (!this->skPaint2GrPaintNoShader(skPaint, true, grPaint)) {
        return false;
    }

    SkPaint noAlphaPaint(skPaint);
    noAlphaPaint.setAlpha(255);
    shader->setContext(this->accessBitmap(false), noAlphaPaint, ctm);

    SkBitmap bitmap;
    SkMatrix matrix;
    SkShader::TileMode tileModes[2];
    SkScalar twoPointParams[3];
    SkShader::BitmapType bmptype = shader->asABitmap(&bitmap, &matrix,
                                                     tileModes, twoPointParams);

    GrSamplerState::SampleMode sampleMode = sk_bmp_type_to_sample_mode[bmptype];
    if (-1 == sampleMode) {
        SkDebugf("shader->asABitmap() == kNone_BitmapType\n");
        return false;
    }
    grPaint->fSampler.setSampleMode(sampleMode);
    grPaint->fSampler.setFilter(skPaint.isFilterBitmap());
    grPaint->fSampler.setWrapX(sk_tile_mode_to_grwrap(tileModes[0]));
    grPaint->fSampler.setWrapY(sk_tile_mode_to_grwrap(tileModes[1]));
    if (GrSamplerState::kRadial2_SampleMode == sampleMode) {
        grPaint->fSampler.setRadial2Params(twoPointParams[0],
                                           twoPointParams[1],
                                           twoPointParams[2] < 0);
    }

    GrTexture* texture = act->set(this, bitmap, grPaint->fSampler);
    if (NULL == texture) {
        SkDebugf("Couldn't convert bitmap to texture.\n");
        return false;
    }
    grPaint->setTexture(texture);

    // since our texture coords will be in local space, we wack the texture
    // matrix to map them back into 0...1 before we load it
    SkMatrix localM;
    if (shader->getLocalMatrix(&localM)) {
        SkMatrix inverse;
        if (localM.invert(&inverse)) {
            matrix.preConcat(inverse);
        }
    }
    if (SkShader::kDefault_BitmapType == bmptype) {
        GrScalar sx = GrFixedToScalar(GR_Fixed1 / bitmap.width());
        GrScalar sy = GrFixedToScalar(GR_Fixed1 / bitmap.height());
        matrix.postScale(sx, sy);
    } else if (SkShader::kRadial_BitmapType == bmptype) {
        GrScalar s = GrFixedToScalar(GR_Fixed1 / bitmap.width());
        matrix.postScale(s, s);
    }
    GrMatrix grMat;
    SkGr::SkMatrix2GrMatrix(matrix, &grMat);
    grPaint->fSampler.setMatrix(grMat);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

class SkPositionSource {
public:
    SkPositionSource(const SkPoint* points, int count)
        : fPoints(points), fCount(count) {}

    int count() const { return fCount; }

    void writeValue(int i, GrPoint* dstPosition) const {
        SkASSERT(i < fCount);
        dstPosition->fX = SkScalarToGrScalar(fPoints[i].fX);
        dstPosition->fY = SkScalarToGrScalar(fPoints[i].fY);
    }
private:
    const SkPoint*  fPoints;
    int             fCount;
};

class SkTexCoordSource {
public:
    SkTexCoordSource(const SkPoint* coords)
        : fCoords(coords) {}

    void writeValue(int i, GrPoint* dstCoord) const {
        dstCoord->fX = SkScalarToGrScalar(fCoords[i].fX);
        dstCoord->fY = SkScalarToGrScalar(fCoords[i].fY);
    }
private:
    const SkPoint*  fCoords;
};

class SkColorSource {
public:
    SkColorSource(const SkColor* colors) : fColors(colors) {}

    void writeValue(int i, GrColor* dstColor) const {
        *dstColor = SkGr::SkColor2GrColor(fColors[i]);
    }
private:
    const SkColor* fColors;
};

class SkIndexSource {
public:
    SkIndexSource(const uint16_t* indices, int count)
        : fIndices(indices), fCount(count) {
    }

    int count() const { return fCount; }

    void writeValue(int i, uint16_t* dstIndex) const {
        *dstIndex = fIndices[i];
    }

private:
    const uint16_t* fIndices;
    int             fCount;
};

///////////////////////////////////////////////////////////////////////////////

#if 0 // not currently being used so don't compile,

// can be used for positions or texture coordinates

class SkRectFanSource {
public:
    SkRectFanSource(const SkRect& rect) : fRect(rect) {}

    int count() const { return 4; }

    void writeValue(int i, GrPoint* dstPoint) const {
        SkASSERT(i < 4);
        dstPoint->fX = SkScalarToGrScalar((i % 3) ? fRect.fRight :
                                                    fRect.fLeft);
        dstPoint->fY = SkScalarToGrScalar((i < 2) ? fRect.fTop  :
                                                    fRect.fBottom);
    }
private:
    const SkRect&   fRect;
};

class SkIRectFanSource {
public:
    SkIRectFanSource(const SkIRect& rect) : fRect(rect) {}

    int count() const { return 4; }

    void writeValue(int i, GrPoint* dstPoint) const {
        SkASSERT(i < 4);
        dstPoint->fX = (i % 3) ? GrIntToScalar(fRect.fRight) :
                                 GrIntToScalar(fRect.fLeft);
        dstPoint->fY = (i < 2) ? GrIntToScalar(fRect.fTop)  :
                                 GrIntToScalar(fRect.fBottom);
    }
private:
    const SkIRect&   fRect;
};

class SkMatRectFanSource {
public:
    SkMatRectFanSource(const SkRect& rect, const SkMatrix& matrix)
        : fRect(rect), fMatrix(matrix) {}

    int count() const { return 4; }

    void writeValue(int i, GrPoint* dstPoint) const {
        SkASSERT(i < 4);

#if SK_SCALAR_IS_GR_SCALAR
        fMatrix.mapXY((i % 3) ? fRect.fRight : fRect.fLeft,
                      (i < 2) ? fRect.fTop   : fRect.fBottom,
                      (SkPoint*)dstPoint);
#else
        SkPoint dst;
        fMatrix.mapXY((i % 3) ? fRect.fRight : fRect.fLeft,
                      (i < 2) ? fRect.fTop   : fRect.fBottom,
                      &dst);
        dstPoint->fX = SkScalarToGrScalar(dst.fX);
        dstPoint->fY = SkScalarToGrScalar(dst.fY);
#endif
    }
private:
    const SkRect&   fRect;
    const SkMatrix& fMatrix;
};

#endif

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawPaint(const SkDraw& draw, const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);

    GrPaint grPaint;
    SkAutoCachedTexture act;
    if (!this->skPaint2GrPaintShader(paint, &act, *draw.fMatrix, &grPaint)) {
        return;
    }

    fContext->drawPaint(grPaint);
}

// must be in SkCanvas::PointMode order
static const GrPrimitiveType gPointMode2PrimtiveType[] = {
    kPoints_PrimitiveType,
    kLines_PrimitiveType,
    kLineStrip_PrimitiveType
};

void SkGpuDevice::drawPoints(const SkDraw& draw, SkCanvas::PointMode mode,
                             size_t count, const SkPoint pts[], const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);

    SkScalar width = paint.getStrokeWidth();
    if (width < 0) {
        return;
    }

    // we only handle hairlines here, else we let the SkDraw call our drawPath()
    if (width > 0) {
        draw.drawPoints(mode, count, pts, paint, true);
        return;
    }

    GrPaint grPaint;
    SkAutoCachedTexture act;
    if (!this->skPaint2GrPaintShader(paint, &act, *draw.fMatrix, &grPaint)) {
        return;
    }

#if SK_SCALAR_IS_GR_SCALAR
    fContext->drawVertices(grPaint,
                           gPointMode2PrimtiveType[mode],
                           count,
                           (GrPoint*)pts,
                           NULL,
                           NULL,
                           NULL,
                           0);
#else
    fContext->drawCustomVertices(grPaint,
                                 gPointMode2PrimtiveType[mode],
                                 SkPositionSource(pts, count));
#endif
}

void SkGpuDevice::drawRect(const SkDraw& draw, const SkRect& rect,
                          const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);

    bool doStroke = paint.getStyle() == SkPaint::kStroke_Style;
    SkScalar width = paint.getStrokeWidth();

    /*
        We have special code for hairline strokes, miter-strokes, and fills.
        Anything else we just call our path code.
     */
    bool usePath = doStroke && width > 0 &&
                    paint.getStrokeJoin() != SkPaint::kMiter_Join;
    // another reason we might need to call drawPath...
    if (paint.getMaskFilter()) {
        usePath = true;
    }

    if (usePath) {
        SkPath path;
        path.addRect(rect);
        this->drawPath(draw, path, paint, NULL, true);
        return;
    }

    GrPaint grPaint;
    SkAutoCachedTexture act;
    if (!this->skPaint2GrPaintShader(paint, &act, *draw.fMatrix,  &grPaint)) {
        return;
    }
    fContext->drawRect(grPaint, Sk2Gr(rect), doStroke ? width : -1);
}

#include "SkMaskFilter.h"
#include "SkBounder.h"

static bool drawWithMaskFilter(GrContext* context, const SkPath& path,
                               SkMaskFilter* filter, const SkMatrix& matrix,
                               const SkRegion& clip, SkBounder* bounder,
                               GrPaint* grp) {
    SkMask  srcM, dstM;

    if (!SkDraw::DrawToMask(path, &clip.getBounds(), filter, &matrix, &srcM,
                            SkMask::kComputeBoundsAndRenderImage_CreateMode)) {
        return false;
    }

    SkAutoMaskImage autoSrc(&srcM, false);

    if (!filter->filterMask(&dstM, srcM, matrix, NULL)) {
        return false;
    }
    // this will free-up dstM when we're done (allocated in filterMask())
    SkAutoMaskImage autoDst(&dstM, false);

    if (clip.quickReject(dstM.fBounds)) {
        return false;
    }
    if (bounder && !bounder->doIRect(dstM.fBounds)) {
        return false;
    }

    // we now have a device-aligned 8bit mask in dstM, ready to be drawn using
    // the current clip (and identity matrix) and grpaint settings

    GrAutoMatrix avm(context, GrMatrix::I());

    const GrGpu::TextureDesc desc = {
        0,
        GrGpu::kNone_AALevel,
        dstM.fBounds.width(),
        dstM.fBounds.height(),
        GrTexture::kAlpha_8_PixelConfig
    };

    GrTexture* texture = context->createUncachedTexture(desc, dstM.fImage,
                                                        dstM.fRowBytes);
    if (NULL == texture) {
        return false;
    }

    grp->setTexture(texture);
    texture->unref();
    grp->fSampler.setClampNoFilter();

    GrRect d;
    d.setLTRB(GrIntToScalar(dstM.fBounds.fLeft),
              GrIntToScalar(dstM.fBounds.fTop),
              GrIntToScalar(dstM.fBounds.fRight),
              GrIntToScalar(dstM.fBounds.fBottom));
    GrRect s;
    s.setLTRB(0, 0, GR_Scalar1, GR_Scalar1);
    context->drawRectToRect(*grp, d, s);
    return true;
}

void SkGpuDevice::drawPath(const SkDraw& draw, const SkPath& origSrcPath,
                           const SkPaint& paint, const SkMatrix* prePathMatrix,
                           bool pathIsMutable) {
    CHECK_SHOULD_DRAW(draw);

    GrPaint grPaint;
    SkAutoCachedTexture act;
    if (!this->skPaint2GrPaintShader(paint, &act, *draw.fMatrix, &grPaint)) {
        return;
    }

    // BEGIN lift from SkDraw::drawPath()

    SkPath*         pathPtr = const_cast<SkPath*>(&origSrcPath);
    bool            doFill = true;
    SkPath          tmpPath;

    if (prePathMatrix) {
        SkPath* result = pathPtr;

        if (!pathIsMutable) {
            result = &tmpPath;
            pathIsMutable = true;
        }
        // should I push prePathMatrix on our MV stack temporarily, instead
        // of applying it here? See SkDraw.cpp
        pathPtr->transform(*prePathMatrix, result);
        pathPtr = result;
    }
    // at this point we're done with prePathMatrix
    SkDEBUGCODE(prePathMatrix = (const SkMatrix*)0x50FF8001;)

    if (paint.getPathEffect() || paint.getStyle() != SkPaint::kFill_Style) {
        doFill = paint.getFillPath(*pathPtr, &tmpPath);
        pathPtr = &tmpPath;
    }

    // END lift from SkDraw::drawPath()

    if (paint.getMaskFilter()) {
        // avoid possibly allocating a new path in transform if we can
        SkPath* devPathPtr = pathIsMutable ? pathPtr : &tmpPath;

        // transform the path into device space
        pathPtr->transform(*draw.fMatrix, devPathPtr);

        drawWithMaskFilter(fContext, *devPathPtr, paint.getMaskFilter(),
                           *draw.fMatrix, *draw.fClip, draw.fBounder, &grPaint);
        return;
    }

    GrPathFill fill = kHairLine_PathFill;

    if (doFill) {
        switch (pathPtr->getFillType()) {
            case SkPath::kWinding_FillType:
                fill = kWinding_PathFill;
                break;
            case SkPath::kEvenOdd_FillType:
                fill = kEvenOdd_PathFill;
                break;
            case SkPath::kInverseWinding_FillType:
                fill = kInverseWinding_PathFill;
                break;
            case SkPath::kInverseEvenOdd_FillType:
                fill = kInverseEvenOdd_PathFill;
                break;
            default:
                SkDebugf("Unsupported path fill type\n");
                return;
        }
    }

    SkGrPathIter iter(*pathPtr);
    fContext->drawPath(grPaint, &iter, fill);
}

void SkGpuDevice::drawBitmap(const SkDraw& draw,
                             const SkBitmap& bitmap,
                             const SkIRect* srcRectPtr,
                             const SkMatrix& m,
                             const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);

    SkIRect srcRect;
    if (NULL == srcRectPtr) {
        srcRect.set(0, 0, bitmap.width(), bitmap.height());
    } else {
        srcRect = *srcRectPtr;
    }

    GrPaint grPaint;
    if (!this->skPaint2GrPaintNoShader(paint, true, &grPaint)) {
        return;
    }
    grPaint.fSampler.setFilter(paint.isFilterBitmap());

    const int maxTextureDim = fContext->getMaxTextureDimension();
    if (bitmap.getTexture() || (bitmap.width() <= maxTextureDim &&
                                bitmap.height() <= maxTextureDim)) {
        // take the fast case
        this->internalDrawBitmap(draw, bitmap, srcRect, m, &grPaint);
        return;
    }

    // undo the translate done by SkCanvas
    int DX = SkMax32(0, srcRect.fLeft);
    int DY = SkMax32(0, srcRect.fTop);
    // compute clip bounds in local coordinates
    SkIRect clipRect;
    {
        SkRect r;
        r.set(draw.fClip->getBounds());
        SkMatrix matrix, inverse;
        matrix.setConcat(*draw.fMatrix, m);
        if (!matrix.invert(&inverse)) {
            return;
        }
        inverse.mapRect(&r);
        r.roundOut(&clipRect);
        // apply the canvas' translate to our local clip
        clipRect.offset(DX, DY);
    }

    int nx = bitmap.width() / maxTextureDim;
    int ny = bitmap.height() / maxTextureDim;
    for (int x = 0; x <= nx; x++) {
        for (int y = 0; y <= ny; y++) {
            SkIRect tileR;
            tileR.set(x * maxTextureDim, y * maxTextureDim,
                      (x + 1) * maxTextureDim, (y + 1) * maxTextureDim);
            if (!SkIRect::Intersects(tileR, clipRect)) {
                continue;
            }

            SkIRect srcR = tileR;
            if (!srcR.intersect(srcRect)) {
                continue;
            }

            SkBitmap tmpB;
            if (bitmap.extractSubset(&tmpB, tileR)) {
                // now offset it to make it "local" to our tmp bitmap
                srcR.offset(-tileR.fLeft, -tileR.fTop);

                SkMatrix tmpM(m);
                {
                    int dx = tileR.fLeft - DX + SkMax32(0, srcR.fLeft);
                    int dy = tileR.fTop -  DY + SkMax32(0, srcR.fTop);
                    tmpM.preTranslate(SkIntToScalar(dx), SkIntToScalar(dy));
                }
                this->internalDrawBitmap(draw, tmpB, srcR, tmpM, &grPaint);
            }
        }
    }
}

/*
 *  This is called by drawBitmap(), which has to handle images that may be too
 *  large to be represented by a single texture.
 *
 *  internalDrawBitmap assumes that the specified bitmap will fit in a texture
 *  and that non-texture portion of the GrPaint has already been setup.
 */
void SkGpuDevice::internalDrawBitmap(const SkDraw& draw,
                                     const SkBitmap& bitmap,
                                     const SkIRect& srcRect,
                                     const SkMatrix& m,
                                     GrPaint* grPaint) {
    SkASSERT(bitmap.width() <= fContext->getMaxTextureDimension() &&
             bitmap.height() <= fContext->getMaxTextureDimension());

    SkAutoLockPixels alp(bitmap);
    if (!bitmap.getTexture() && !bitmap.readyToDraw()) {
        return;
    }

    grPaint->fSampler.setWrapX(GrSamplerState::kClamp_WrapMode);
    grPaint->fSampler.setWrapY(GrSamplerState::kClamp_WrapMode);
    grPaint->fSampler.setSampleMode(GrSamplerState::kNormal_SampleMode);
    grPaint->fSampler.setMatrix(GrMatrix::I());

    GrTexture* texture;
    SkAutoCachedTexture act(this, bitmap, grPaint->fSampler, &texture);
    if (NULL == texture) {
        return;
    }

    grPaint->setTexture(texture);

    GrRect dstRect(0, 0, GrIntToScalar(srcRect.width()), GrIntToScalar(srcRect.height()));
    GrRect paintRect;
    paintRect.setLTRB(GrFixedToScalar((srcRect.fLeft << 16)   / bitmap.width()),
                      GrFixedToScalar((srcRect.fTop << 16)    / bitmap.height()),
                      GrFixedToScalar((srcRect.fRight << 16)  / bitmap.width()),
                      GrFixedToScalar((srcRect.fBottom << 16) / bitmap.height()));

    GrMatrix grMat;
    SkGr::SkMatrix2GrMatrix(m, &grMat);

    fContext->drawRectToRect(*grPaint, dstRect, paintRect, &grMat);
}

void SkGpuDevice::drawSprite(const SkDraw& draw, const SkBitmap& bitmap,
                            int left, int top, const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);

    SkAutoLockPixels alp(bitmap);
    if (!bitmap.getTexture() && !bitmap.readyToDraw()) {
        return;
    }

    GrPaint grPaint;
    if(!this->skPaint2GrPaintNoShader(paint, true, &grPaint)) {
        return;
    }

    GrAutoMatrix avm(fContext, GrMatrix::I());

    GrTexture* texture;
    grPaint.fSampler.setClampNoFilter();
    SkAutoCachedTexture act(this, bitmap, grPaint.fSampler, &texture);

    grPaint.setTexture(texture);

    fContext->drawRectToRect(grPaint,
                             GrRect(GrIntToScalar(left), GrIntToScalar(top),
                                    GrIntToScalar(left + bitmap.width()),
                                    GrIntToScalar(top + bitmap.height())),
                             GrRect(0, 0, GR_Scalar1, GR_Scalar1));
}

void SkGpuDevice::drawDevice(const SkDraw& draw, SkDevice* dev,
                            int x, int y, const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);

    GrPaint grPaint;
    if (!((SkGpuDevice*)dev)->bindDeviceAsTexture(&grPaint) ||
        !this->skPaint2GrPaintNoShader(paint, true, &grPaint)) {
        return;
    }

    SkASSERT(NULL != grPaint.getTexture());

    const SkBitmap& bm = dev->accessBitmap(false);
    int w = bm.width();
    int h = bm.height();

    GrAutoMatrix avm(fContext, GrMatrix::I());

    grPaint.fSampler.setClampNoFilter();

    fContext->drawRectToRect(grPaint,
                             GrRect(GrIntToScalar(x),
                                    GrIntToScalar(y),
                                    GrIntToScalar(x + w),
                                    GrIntToScalar(y + h)),
                             GrRect(0, 0, GR_Scalar1, GR_Scalar1));
}

///////////////////////////////////////////////////////////////////////////////

// must be in SkCanvas::VertexMode order
static const GrPrimitiveType gVertexMode2PrimitiveType[] = {
    kTriangles_PrimitiveType,
    kTriangleStrip_PrimitiveType,
    kTriangleFan_PrimitiveType,
};

void SkGpuDevice::drawVertices(const SkDraw& draw, SkCanvas::VertexMode vmode,
                              int vertexCount, const SkPoint vertices[],
                              const SkPoint texs[], const SkColor colors[],
                              SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);

    GrPaint grPaint;
    SkAutoCachedTexture act;
    // we ignore the shader if texs is null.
    if (NULL == texs) {
        if (!this->skPaint2GrPaintNoShader(paint, false, &grPaint)) {
            return;
        }
    } else {
        if (!this->skPaint2GrPaintShader(paint, &act,
                                         *draw.fMatrix,
                                         &grPaint)) {
            return;
        }
    }

    if (NULL != xmode && NULL != texs && NULL != colors) {
        SkXfermode::Mode mode;
        if (!SkXfermode::IsMode(xmode, &mode) ||
            SkXfermode::kMultiply_Mode != mode) {
            SkDebugf("Unsupported vertex-color/texture xfer mode.\n");
#if 0
            return
#endif
        }
    }

#if SK_SCALAR_IS_GR_SCALAR
    // even if GrColor and SkColor byte offsets match we need
    // to perform pre-multiply.
    if (NULL == colors) {
        fContext->drawVertices(grPaint,
                               gVertexMode2PrimitiveType[vmode],
                               vertexCount,
                               (GrPoint*) vertices,
                               (GrPoint*) texs,
                               NULL,
                               indices,
                               indexCount);
    } else
#endif
    {
        SkTexCoordSource texSrc(texs);
        SkColorSource colSrc(colors);
        SkIndexSource idxSrc(indices, indexCount);

        fContext->drawCustomVertices(grPaint,
                                     gVertexMode2PrimitiveType[vmode],
                                     SkPositionSource(vertices, vertexCount),
                                     (NULL == texs) ? NULL : &texSrc,
                                     (NULL == colors) ? NULL : &colSrc,
                                     (NULL == indices) ? NULL : &idxSrc);
    }
}

///////////////////////////////////////////////////////////////////////////////

static void GlyphCacheAuxProc(void* data) {
    delete (GrFontScaler*)data;
}

static GrFontScaler* get_gr_font_scaler(SkGlyphCache* cache) {
    void* auxData;
    GrFontScaler* scaler = NULL;
    if (cache->getAuxProcData(GlyphCacheAuxProc, &auxData)) {
        scaler = (GrFontScaler*)auxData;
    }
    if (NULL == scaler) {
        scaler = new SkGrFontScaler(cache);
        cache->setAuxProc(GlyphCacheAuxProc, scaler);
    }
    return scaler;
}

static void SkGPU_Draw1Glyph(const SkDraw1Glyph& state,
                             SkFixed fx, SkFixed fy,
                             const SkGlyph& glyph) {
    SkASSERT(glyph.fWidth > 0 && glyph.fHeight > 0);

    GrSkDrawProcs* procs = (GrSkDrawProcs*)state.fDraw->fProcs;

    if (NULL == procs->fFontScaler) {
        procs->fFontScaler = get_gr_font_scaler(state.fCache);
    }
    procs->fTextContext->drawPackedGlyph(GrGlyph::Pack(glyph.getGlyphID(), fx, 0),
                                         SkIntToFixed(SkFixedFloor(fx)), fy,
                                         procs->fFontScaler);
}

SkDrawProcs* SkGpuDevice::initDrawForText(GrTextContext* context) {

    // deferred allocation
    if (NULL == fDrawProcs) {
        fDrawProcs = new GrSkDrawProcs;
        fDrawProcs->fD1GProc = SkGPU_Draw1Glyph;
        fDrawProcs->fContext = fContext;
    }

    // init our (and GL's) state
    fDrawProcs->fTextContext = context;
    fDrawProcs->fFontScaler = NULL;
    return fDrawProcs;
}

void SkGpuDevice::drawText(const SkDraw& draw, const void* text,
                          size_t byteLength, SkScalar x, SkScalar y,
                          const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);

    if (draw.fMatrix->getType() & SkMatrix::kPerspective_Mask) {
        // this guy will just call our drawPath()
        draw.drawText((const char*)text, byteLength, x, y, paint);
    } else {
        SkAutoExtMatrix aem(draw.fExtMatrix);
        SkDraw myDraw(draw);

        GrPaint grPaint;
        SkAutoCachedTexture act;

        if (!this->skPaint2GrPaintShader(paint, &act, *draw.fMatrix, &grPaint)) {
            return;
        }
        GrTextContext context(fContext, grPaint, aem.extMatrix());
        myDraw.fProcs = this->initDrawForText(&context);
        this->INHERITED::drawText(myDraw, text, byteLength, x, y, paint);
    }
}

void SkGpuDevice::drawPosText(const SkDraw& draw, const void* text,
                             size_t byteLength, const SkScalar pos[],
                             SkScalar constY, int scalarsPerPos,
                             const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);

    if (draw.fMatrix->getType() & SkMatrix::kPerspective_Mask) {
        // this guy will just call our drawPath()
        draw.drawPosText((const char*)text, byteLength, pos, constY,
                         scalarsPerPos, paint);
    } else {
        SkAutoExtMatrix aem(draw.fExtMatrix);
        SkDraw myDraw(draw);

        GrPaint grPaint;
        SkAutoCachedTexture act;
        if (!this->skPaint2GrPaintShader(paint, &act, *draw.fMatrix, &grPaint)) {
            return;
        }

        GrTextContext context(fContext, grPaint, aem.extMatrix());
        myDraw.fProcs = this->initDrawForText(&context);
        this->INHERITED::drawPosText(myDraw, text, byteLength, pos, constY,
                                     scalarsPerPos, paint);
    }
}

void SkGpuDevice::drawTextOnPath(const SkDraw& draw, const void* text,
                                size_t len, const SkPath& path,
                                const SkMatrix* m, const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);

    SkASSERT(draw.fDevice == this);
    draw.drawTextOnPath((const char*)text, len, path, m, paint);
}

///////////////////////////////////////////////////////////////////////////////

SkGpuDevice::TexCache* SkGpuDevice::lockCachedTexture(const SkBitmap& bitmap,
                                                  const GrSamplerState& sampler,
                                                  GrTexture** texture,
                                                  bool forDeviceRenderTarget) {
    GrContext* ctx = this->context();
    uint32_t p0, p1;
    if (forDeviceRenderTarget) {
        p0 = p1 = -1;
    } else {
        p0 = bitmap.getGenerationID();
        p1 = bitmap.pixelRefOffset();
    }

    GrTexture* newTexture = NULL;
    GrTextureKey key(p0, p1, bitmap.width(), bitmap.height());
    GrTextureEntry* entry = ctx->findAndLockTexture(&key, sampler);

    if (NULL == entry) {

        if (forDeviceRenderTarget) {
            const GrGpu::TextureDesc desc = {
                GrGpu::kRenderTarget_TextureFlag,
                GrGpu::kNone_AALevel,
                bitmap.width(),
                bitmap.height(),
                SkGr::Bitmap2PixelConfig(bitmap)
            };
            entry = ctx->createAndLockTexture(&key, sampler, desc, NULL, 0);

        } else {
            entry = sk_gr_create_bitmap_texture(ctx, &key, sampler, bitmap);
        }
        if (NULL == entry) {
            GrPrintf("---- failed to create texture for cache [%d %d]\n",
                     bitmap.width(), bitmap.height());
        }
    }

    if (NULL != entry) {
        newTexture = entry->texture();
        if (texture) {
            *texture = newTexture;
        }
        // IMPORTANT: We can't allow another SkGpuDevice to get this
        // cache entry until this one is destroyed!
        if (forDeviceRenderTarget) {
            ctx->detachCachedTexture(entry);
        }
    }
    return (TexCache*)entry;
}

void SkGpuDevice::unlockCachedTexture(TexCache* cache) {
    this->context()->unlockTexture((GrTextureEntry*)cache);
}

///////////////////////////////////////////////////////////////////////////////

SkGpuDeviceFactory::SkGpuDeviceFactory(GrContext* context,
                                       GrRenderTarget* rootRenderTarget)
        : fContext(context) {

    GrAssert(NULL != context);
    GrAssert(NULL != rootRenderTarget);

    // check this now rather than passing this value to SkGpuDevice cons.
    // we want the rt that is bound *now* in the 3D API, not the one
    // at the time of newDevice.
    if (SkGpuDevice::Current3DApiRenderTarget() == rootRenderTarget) {
        fRootRenderTarget = context->createRenderTargetFrom3DApiState();
    } else {
        fRootRenderTarget = rootRenderTarget;
        rootRenderTarget->ref();
    }
    context->ref();

}

SkGpuDeviceFactory::~SkGpuDeviceFactory() {
    fContext->unref();
    fRootRenderTarget->unref();
}

SkDevice* SkGpuDeviceFactory::newDevice(SkCanvas*, SkBitmap::Config config,
                                        int width, int height,
                                        bool isOpaque, bool isLayer) {
    SkBitmap bm;
    bm.setConfig(config, width, height);
    bm.setIsOpaque(isOpaque);
    return new SkGpuDevice(fContext, bm, isLayer ?  NULL : fRootRenderTarget);
}

