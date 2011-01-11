/*
    Copyright 2010 Google Inc.

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
        device->context()->setTexture(texture);
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

SkGpuDevice::SkGpuDevice(GrContext* context, const SkBitmap& bitmap, bool isLayer)
        : SkDevice(NULL, bitmap, false) {

    fNeedPrepareRenderTarget = false;
    fDrawProcs = NULL;

    // should I ref() this, and then unref in destructor? <mrr>
    fContext = context;

    fCache = NULL;
    fTexture = NULL;
    fRenderTarget = NULL;
    fNeedClear = false;

    if (isLayer) {
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
            SkASSERT(fTexture->isRenderTarget());
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
        }
    }

    if (NULL == fRenderTarget) {
        GrAssert(NULL == fCache);
        GrAssert(NULL == fTexture);

        fRenderTarget = fContext->currentRenderTarget();
        fRenderTarget->ref();
        fContext->setDefaultRenderTargetSize(this->width(), this->height());
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
}

void SkGpuDevice::bindDeviceToTargetHandle(intptr_t handle) {
    if (fCache) {
        GrAssert(NULL != fTexture);
        GrAssert(fRenderTarget == fTexture->asRenderTarget());
        // IMPORTANT: reattach the rendertarget/tex back to the cache.
        fContext->reattachAndUnlockCachedTexture((GrTextureEntry*)fCache);
    } else if (NULL != fTexture) {
        GrAssert(!CACHE_LAYER_TEXTURES);
        fTexture->unref();
    } else if (NULL != fRenderTarget) {
        fRenderTarget->unref();
    }

    fCache  = NULL;
    fTexture = NULL;
    fRenderTarget = fContext->createPlatformRenderTarget(handle,
                                                         this->width(),
                                                         this->height());
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
                               const SkRegion& clip) {
    GrMatrix grmat;
    SkGr::SkMatrix2GrMatrix(matrix, &grmat);
    context->setViewMatrix(grmat);

    SkGrClipIterator iter;
    iter.reset(clip);
    GrClip grc(&iter);
    if (context->getClip() == grc) {
    } else {
        context->setClip(grc);
    }
}

// call this ever each draw call, to ensure that the context reflects our state,
// and not the state from some other canvas/device
void SkGpuDevice::prepareRenderTarget(const SkDraw& draw) {
    if (fNeedPrepareRenderTarget ||
        fContext->currentRenderTarget() != fRenderTarget) {

        fContext->setRenderTarget(fRenderTarget);
        convert_matrixclip(fContext, *draw.fMatrix, *draw.fClip);
        fNeedPrepareRenderTarget = false;
    }
}

void SkGpuDevice::setMatrixClip(const SkMatrix& matrix, const SkRegion& clip) {
    this->INHERITED::setMatrixClip(matrix, clip);

    convert_matrixclip(fContext, matrix, clip);
}

void SkGpuDevice::gainFocus(SkCanvas* canvas, const SkMatrix& matrix,
                            const SkRegion& clip) {
    fContext->setRenderTarget(fRenderTarget);

    this->INHERITED::gainFocus(canvas, matrix, clip);

    convert_matrixclip(fContext, matrix, clip);

    if (fNeedClear) {
        fContext->eraseColor(0x0);
        fNeedClear = false;
    }
}

bool SkGpuDevice::bindDeviceAsTexture(SkPoint* max) {
    if (NULL != fTexture) {
        fContext->setTexture(fTexture);
        if (NULL != max) {
            max->set(SkFixedToScalar((width() << 16) /
                                     fTexture->allocWidth()),
                     SkFixedToScalar((height() << 16) /
                                     fTexture->allocHeight()));
        }
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

// must be in the same order as SkXfermode::Coeff in SkXfermode.h

SkGpuDevice::AutoPaintShader::AutoPaintShader() {
    fSuccess = false;
    fTexture = NULL;
}

SkGpuDevice::AutoPaintShader::AutoPaintShader(SkGpuDevice* device,
                                              const SkPaint& paint,
                                              const SkMatrix& matrix) {
    fSuccess = false;
    fTexture = NULL;
    this->init(device, paint, matrix);
}

void SkGpuDevice::AutoPaintShader::init(SkGpuDevice* device,
                                        const SkPaint& paint,
                                        const SkMatrix& ctm) {
    fSuccess = true;
    GrContext* ctx = device->context();
    sk_gr_set_paint(ctx, paint); // should we pass true for justAlpha if we have a shader/texture?

    SkShader* shader = paint.getShader();
    if (NULL == shader) {
        return;
    }

    if (!shader->setContext(device->accessBitmap(false), paint, ctm)) {
        fSuccess = false;
        return;
    }

    GrSamplerState::SampleMode sampleMode;
    SkBitmap bitmap;
    SkMatrix matrix;
    SkShader::TileMode tileModes[2];
    SkScalar twoPointParams[3];
    SkShader::BitmapType bmptype = shader->asABitmap(&bitmap, &matrix,
                                                     tileModes, twoPointParams);

    switch (bmptype) {
    case SkShader::kNone_BitmapType:
        SkDebugf("shader->asABitmap() == kNone_BitmapType");
        return;
    case SkShader::kDefault_BitmapType:
        sampleMode = GrSamplerState::kNormal_SampleMode;
        break;
    case SkShader::kRadial_BitmapType:
        sampleMode = GrSamplerState::kRadial_SampleMode;
        break;
    case SkShader::kSweep_BitmapType:
        sampleMode = GrSamplerState::kSweep_SampleMode;
        break;
    case SkShader::kTwoPointRadial_BitmapType:
        sampleMode = GrSamplerState::kRadial2_SampleMode;
        break;
    default:
        SkASSERT("Unexpected return from asABitmap");
        return;
    }

    bitmap.lockPixels();
    if (!bitmap.getTexture() && !bitmap.readyToDraw()) {
        return;
    }

    // see if we've already cached the bitmap from the shader
    GrSamplerState samplerState(sk_tile_mode_to_grwrap(tileModes[0]),
                                sk_tile_mode_to_grwrap(tileModes[1]),
                                sampleMode,
                                paint.isFilterBitmap());

    if (GrSamplerState::kRadial2_SampleMode == sampleMode) {
        samplerState.setRadial2Params(twoPointParams[0],
                                      twoPointParams[1],
                                      twoPointParams[2] < 0);
    }

    GrTexture* texture = fCachedTexture.set(device, bitmap, samplerState);
    if (NULL == texture) {
        return;
    }

    // the lock has already called setTexture for us
    ctx->setSamplerState(samplerState);

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
        GrScalar sx = (GR_Scalar1 * texture->contentWidth()) /
                      (bitmap.width() * texture->allocWidth());
        GrScalar sy = (GR_Scalar1 * texture->contentHeight()) /
                      (bitmap.height() * texture->allocHeight());
        matrix.postScale(sx, sy);

    } else if (SkShader::kRadial_BitmapType == bmptype) {
        GrScalar s = (GR_Scalar1 * texture->contentWidth()) /
                     (bitmap.width() * texture->allocWidth());
        matrix.postScale(s, s);
    }
    GrMatrix grmat;
    SkGr::SkMatrix2GrMatrix(matrix, &grmat);
    ctx->setTextureMatrix(grmat);

    // since we're going to use a shader/texture, we don't want the color,
    // just its alpha
    ctx->setAlpha(paint.getAlpha());
    // report that we have setup the texture
    fSuccess = true;
    fTexture = texture;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawPaint(const SkDraw& draw, const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);

    AutoPaintShader   shader(this, paint, *draw.fMatrix);
    if (shader.failed()) {
        return;
    }
    fContext->drawFull(shader.useTex());
}

// must be in SkCanvas::PointMode order
static const GrGpu::PrimitiveType gPointMode2PrimtiveType[] = {
    GrGpu::kPoints_PrimitiveType,
    GrGpu::kLines_PrimitiveType,
    GrGpu::kLineStrip_PrimitiveType
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

    AutoPaintShader shader(this, paint, *draw.fMatrix);
    if (shader.failed()) {
        return;
    }

    GrVertexLayout layout = shader.useTex() ?
                            GrDrawTarget::kPositionAsTexCoord_VertexLayoutBit :
                            0;
#if SK_SCALAR_IS_GR_SCALAR
    fContext->setVertexSourceToArray(pts, layout);
    fContext->drawNonIndexed(gPointMode2PrimtiveType[mode], 0, count);
#else
    GrPoint* v;
    fContext->reserveAndLockGeometry(layout, count, 0, (void**)&v, NULL);
    for (size_t i = 0; i < count; ++i) {
        v[i].set(SkScalarToGrScalar(pts[i].fX), SkScalarToGrScalar(pts[i].fY));
    }
    fContext->drawNonIndexed(gPointMode2PrimtiveType[mode], 0, count);
    fContext->releaseReservedGeometry();
#endif

}

void SkGpuDevice::drawRect(const SkDraw& draw, const SkRect& rect,
                          const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);

    bool doStroke = paint.getStyle() == SkPaint::kStroke_Style;
    SkScalar width = paint.getStrokeWidth();

    /*
        We have special code for hairline strokes, miter-strokes, and fills.
        Anything else we just call our path code. (i.e. non-miter thick stroke)
     */
    if (doStroke && width > 0 && paint.getStrokeJoin() != SkPaint::kMiter_Join) {
        SkPath path;
        path.addRect(rect);
        this->drawPath(draw, path, paint, NULL, true);
        return;
    }

    AutoPaintShader shader(this, paint, *draw.fMatrix);
    if (shader.failed()) {
        return;
    }

    fContext->drawRect(Sk2Gr(rect), shader.useTex(), doStroke ? width : -1);
}

void SkGpuDevice::drawPath(const SkDraw& draw, const SkPath& path,
                           const SkPaint& paint, const SkMatrix* prePathMatrix,
                           bool pathIsMutable) {
    CHECK_SHOULD_DRAW(draw);

    AutoPaintShader shader(this, paint, *draw.fMatrix);
    if (shader.failed()) {
        return;
    }

    const SkPath* pathPtr = &path;
    SkPath  tmpPath;

    if (prePathMatrix) {
        if (pathIsMutable) {
            const_cast<SkPath*>(pathPtr)->transform(*prePathMatrix);
        } else {
            path.transform(*prePathMatrix, &tmpPath);
            pathPtr = &tmpPath;
        }
    }

    SkPath               fillPath;
    GrContext::PathFills fill = GrContext::kHairLine_PathFill;

    if (paint.getFillPath(*pathPtr, &fillPath)) {
        switch (fillPath.getFillType()) {
            case SkPath::kWinding_FillType:
                fill = GrContext::kWinding_PathFill;
                break;
            case SkPath::kEvenOdd_FillType:
                fill = GrContext::kEvenOdd_PathFill;
                break;
            case SkPath::kInverseWinding_FillType:
                fill = GrContext::kInverseWinding_PathFill;
                break;
            case SkPath::kInverseEvenOdd_FillType:
                fill = GrContext::kInverseEvenOdd_PathFill;
                break;
            default:
                SkDebugf("Unsupported path fill type");
                return;
        }
    }

    SkGrPathIter iter(fillPath);
    fContext->drawPath(&iter, fill, shader.useTex());
}

/*
 *  This value must not exceed the GPU's texture dimension limit, but it can
 *  be smaller, if that helps avoid very large single textures hurting the
 *  cache.
 */
#define MAX_TEXTURE_DIM     512

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

    if (bitmap.getTexture() || (bitmap.width() <= MAX_TEXTURE_DIM &&
                                bitmap.height() <= MAX_TEXTURE_DIM)) {
        // take the fast case
        this->internalDrawBitmap(draw, bitmap, srcRect, m, paint);
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

    int nx = bitmap.width() / MAX_TEXTURE_DIM;
    int ny = bitmap.height() / MAX_TEXTURE_DIM;
    for (int x = 0; x <= nx; x++) {
        for (int y = 0; y <= ny; y++) {
            SkIRect tileR;
            tileR.set(x * MAX_TEXTURE_DIM, y * MAX_TEXTURE_DIM,
                      (x + 1) * MAX_TEXTURE_DIM, (y + 1) * MAX_TEXTURE_DIM);
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
                this->internalDrawBitmap(draw, tmpB, srcR, tmpM, paint);
            }
        }
    }
}

/*
 *  This is called by drawBitmap(), which has to handle images that may be too
 *  large to be represented by a single texture.
 *
 *  internalDrawBitmap assumes that the specified bitmap will fit in a texture.
 */
void SkGpuDevice::internalDrawBitmap(const SkDraw& draw,
                                     const SkBitmap& bitmap,
                                     const SkIRect& srcRect,
                                     const SkMatrix& m,
                                     const SkPaint& paint) {
    SkASSERT(bitmap.width() <= MAX_TEXTURE_DIM &&
             bitmap.height() <= MAX_TEXTURE_DIM);

    SkAutoLockPixels alp(bitmap);
    if (!bitmap.getTexture() && !bitmap.readyToDraw()) {
        return;
    }

    GrSamplerState sampler(paint.isFilterBitmap()); // defaults to clamp
    // the lock has already called setTexture for us
    fContext->setSamplerState(sampler);

    GrTexture* texture;
    SkAutoCachedTexture act(this, bitmap, sampler, &texture);
    if (NULL == texture) {
        return;
    }

    GrVertexLayout layout = GrDrawTarget::kSeparateTexCoord_VertexLayoutBit;

    GrPoint* vertex;
    if (!fContext->reserveAndLockGeometry(layout, 4,
                                          0, GrTCast<void**>(&vertex), NULL)) {
        return;
    }

    {
        GrMatrix grmat;
        SkGr::SkMatrix2GrMatrix(m, &grmat);
        vertex[0].setIRectFan(0, 0, srcRect.width(), srcRect.height(),
                              2*sizeof(GrPoint));
        grmat.mapPointsWithStride(vertex, 2*sizeof(GrPoint), 4);
    }

    SkScalar left   = SkFixedToScalar((srcRect.fLeft << 16) /
                                      texture->allocWidth());
    SkScalar right  = SkFixedToScalar((srcRect.fRight << 16) /
                                      texture->allocWidth());
    SkScalar top    = SkFixedToScalar((srcRect.fTop << 16) /
                                      texture->allocHeight());
    SkScalar bottom = SkFixedToScalar((srcRect.fBottom << 16) /
                                      texture->allocHeight());
    vertex[1].setRectFan(left, top, right, bottom, 2*sizeof(GrPoint));

    fContext->setTextureMatrix(GrMatrix::I());
    // now draw the mesh
    sk_gr_set_paint(fContext, paint, true);
    fContext->drawNonIndexed(GrGpu::kTriangleFan_PrimitiveType, 0, 4);
    fContext->releaseReservedGeometry();
}

static void gl_drawSprite(GrContext* ctx,
                          int x, int y, int w, int h, const SkPoint& max,
                          const SkPaint& paint) {
    GrAutoViewMatrix avm(ctx, GrMatrix::I());

    ctx->setSamplerState(GrSamplerState::ClampNoFilter());
    ctx->setTextureMatrix(GrMatrix::I());

    GrPoint* vertex;
    GrVertexLayout layout = GrGpu::kSeparateTexCoord_VertexLayoutBit;
    if (!ctx->reserveAndLockGeometry(layout, 4, 0,
                                     GrTCast<void**>(&vertex), NULL)) {
        return;
    }

    vertex[1].setRectFan(0, 0, max.fX, max.fY, 2*sizeof(GrPoint));

    vertex[0].setIRectFan(x, y, x + w, y + h, 2*sizeof(GrPoint));

    sk_gr_set_paint(ctx, paint, true);
    // should look to use glDrawTexi() has we do for text...
    ctx->drawNonIndexed(GrGpu::kTriangleFan_PrimitiveType, 0, 4);
    ctx->releaseReservedGeometry();
}

void SkGpuDevice::drawSprite(const SkDraw& draw, const SkBitmap& bitmap,
                            int left, int top, const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);

    SkAutoLockPixels alp(bitmap);
    if (!bitmap.getTexture() && !bitmap.readyToDraw()) {
        return;
    }

    SkPoint max;
    GrTexture* texture;
    SkAutoCachedTexture act(this, bitmap, GrSamplerState::ClampNoFilter(),
                            &texture);

    max.set(SkFixedToScalar((texture->contentWidth() << 16) /
                             texture->allocWidth()),
            SkFixedToScalar((texture->contentHeight() << 16) /
                            texture->allocHeight()));
    gl_drawSprite(fContext, left, top, bitmap.width(), bitmap.height(), max, paint);
}

void SkGpuDevice::drawDevice(const SkDraw& draw, SkDevice* dev,
                            int x, int y, const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);

    SkPoint max;
    if (((SkGpuDevice*)dev)->bindDeviceAsTexture(&max)) {
        const SkBitmap& bm = dev->accessBitmap(false);
        int w = bm.width();
        int h = bm.height();
        gl_drawSprite(fContext, x, y, w, h, max, paint);
    }
}

///////////////////////////////////////////////////////////////////////////////

// must be in SkCanvas::VertexMode order
static const GrGpu::PrimitiveType gVertexMode2PrimitiveType[] = {
    GrGpu::kTriangles_PrimitiveType,
    GrGpu::kTriangleStrip_PrimitiveType,
    GrGpu::kTriangleFan_PrimitiveType,
};

void SkGpuDevice::drawVertices(const SkDraw& draw, SkCanvas::VertexMode vmode,
                              int vertexCount, const SkPoint vertices[],
                              const SkPoint texs[], const SkColor colors[],
                              SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);

    sk_gr_set_paint(fContext, paint);

    TexCache* cache = NULL;

    bool useTexture = false;

    AutoPaintShader autoShader;

    if (texs) {
        autoShader.init(this, paint, *draw.fMatrix);

        if (autoShader.failed()) {
            return;
        }
        useTexture = autoShader.useTex();
    }

    bool releaseVerts = false;
    GrVertexLayout layout = 0;
    if (useTexture) {
        layout |= GrDrawTarget::kSeparateTexCoord_VertexLayoutBit;
    }
    if (NULL != colors) {
        layout |= GrDrawTarget::kColor_VertexLayoutBit;
    }

    #if SK_SCALAR_IS_GR_SCALAR
    if (!layout) {
        fContext->setVertexSourceToArray(vertices, layout);
    } else
    #endif
    {
        void* verts;
        releaseVerts = true;
        if (!fContext->reserveAndLockGeometry(layout, vertexCount, 0,
                                              &verts, NULL)) {
            return;
        }
        int texOffset, colorOffset;
        uint32_t stride = GrDrawTarget::VertexSizeAndOffsets(layout,
                                                             &texOffset,
                                                             &colorOffset);
        for (int i = 0; i < vertexCount; ++i) {
            GrPoint* p = (GrPoint*)((intptr_t)verts + i * stride);
            p->set(SkScalarToGrScalar(vertices[i].fX),
                   SkScalarToGrScalar(vertices[i].fY));
            if (texOffset > 0) {
                GrPoint* t = (GrPoint*)((intptr_t)p + texOffset);
                t->set(SkScalarToGrScalar(texs[i].fX),
                       SkScalarToGrScalar(texs[i].fY));
            }
            if (colorOffset > 0) {
                uint32_t* color = (uint32_t*) ((intptr_t)p + colorOffset);
                *color = SkGr::SkColor2GrColor(colors[i]);
            }
        }
    }
    if (indices) {
        fContext->setIndexSourceToArray(indices);
        fContext->drawIndexed(gVertexMode2PrimitiveType[vmode], 0, 0,
                                 vertexCount, indexCount);
    } else {
        fContext->drawNonIndexed(gVertexMode2PrimitiveType[vmode],
                                 0, vertexCount);
    }
    if (cache) {
        this->unlockCachedTexture(cache);
    }
    if (releaseVerts) {
        fContext->releaseReservedGeometry();
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

SkDrawProcs* SkGpuDevice::initDrawForText(const SkPaint& paint,
                                          GrTextContext* context) {

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
        sk_gr_set_paint(fContext, paint);
        GrTextContext context(fContext, aem.extMatrix());
        myDraw.fProcs = this->initDrawForText(paint, &context);
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
        sk_gr_set_paint(fContext, paint);
        GrTextContext context(fContext, aem.extMatrix());
        myDraw.fProcs = this->initDrawForText(paint, &context);
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
        ctx->setTexture(newTexture);
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

SkGpuDeviceFactory::SkGpuDeviceFactory(GrContext* context) : fContext(context) {
    context->ref();
}

SkGpuDeviceFactory::~SkGpuDeviceFactory() {
    fContext->unref();
}

SkDevice* SkGpuDeviceFactory::newDevice(SkCanvas*, SkBitmap::Config config,
                                        int width, int height,
                                        bool isOpaque, bool isLayer) {
    SkBitmap bm;
    bm.setConfig(config, width, height);
    bm.setIsOpaque(isOpaque);
    return new SkGpuDevice(fContext, bm, isLayer);
}

