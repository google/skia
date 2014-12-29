
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAADistanceFieldPathRenderer.h"

#include "GrAtlas.h"
#include "GrContext.h"
#include "GrDrawState.h"
#include "GrSurfacePriv.h"
#include "GrSWMaskHelper.h"
#include "GrTexturePriv.h"
#include "effects/GrDistanceFieldTextureEffect.h"

#include "SkDistanceFieldGen.h"
#include "SkRTConf.h"

#define ATLAS_TEXTURE_WIDTH 1024
#define ATLAS_TEXTURE_HEIGHT 2048
#define PLOT_WIDTH  256
#define PLOT_HEIGHT 256

#define NUM_PLOTS_X   (ATLAS_TEXTURE_WIDTH / PLOT_WIDTH)
#define NUM_PLOTS_Y   (ATLAS_TEXTURE_HEIGHT / PLOT_HEIGHT)

SK_CONF_DECLARE(bool, c_DumpPathCache, "gpu.dumpPathCache", false,
                "Dump the contents of the path cache before every purge.");

#ifdef DF_PATH_TRACKING
static int g_NumCachedPaths = 0;
static int g_NumFreedPaths = 0;
#endif

// mip levels
static const int kSmallMIP = 32;
static const int kMediumMIP = 78;
static const int kLargeMIP = 192;

////////////////////////////////////////////////////////////////////////////////
GrAADistanceFieldPathRenderer::GrAADistanceFieldPathRenderer(GrContext* context)
    : fContext(context)
    , fAtlas(NULL)
    , fEffectFlags(kInvalid_DistanceFieldEffectFlag) {
}

GrAADistanceFieldPathRenderer::~GrAADistanceFieldPathRenderer() {
    PathDataList::Iter iter;
    iter.init(fPathList, PathDataList::Iter::kHead_IterStart);
    PathData* pathData;
    while ((pathData = iter.get())) {
        iter.next();
        fPathList.remove(pathData);
        SkDELETE(pathData);
    }
    
    SkDELETE(fAtlas);

#ifdef DF_PATH_TRACKING
    SkDebugf("Cached paths: %d, freed paths: %d\n", g_NumCachedPaths, g_NumFreedPaths);
#endif
}

////////////////////////////////////////////////////////////////////////////////
bool GrAADistanceFieldPathRenderer::canDrawPath(const GrDrawTarget* target,
                                                const GrDrawState* drawState,
                                                const SkMatrix& viewMatrix,
                                                const SkPath& path,
                                                const SkStrokeRec& stroke,
                                                bool antiAlias) const {
    
    // TODO: Support inverse fill
    // TODO: Support strokes
    if (!target->caps()->shaderDerivativeSupport() || !antiAlias || path.isInverseFillType()
        || path.isVolatile() || SkStrokeRec::kFill_Style != stroke.getStyle()) {
        return false;
    }

    // currently don't support perspective
    if (viewMatrix.hasPerspective()) {
        return false;
    }
    
    // only support paths smaller than 64x64, scaled to less than 256x256
    // the goal is to accelerate rendering of lots of small paths that may be scaling
    SkScalar maxScale = viewMatrix.getMaxScale();
    const SkRect& bounds = path.getBounds();
    SkScalar maxDim = SkMaxScalar(bounds.width(), bounds.height());
    return maxDim < 64.f && maxDim*maxScale < 256.f;
}


GrPathRenderer::StencilSupport
GrAADistanceFieldPathRenderer::onGetStencilSupport(const GrDrawTarget*,
                                                   const GrDrawState*,
                                                   const SkPath&,
                                                   const SkStrokeRec&) const {
    return GrPathRenderer::kNoSupport_StencilSupport;
}

////////////////////////////////////////////////////////////////////////////////

bool GrAADistanceFieldPathRenderer::onDrawPath(GrDrawTarget* target,
                                               GrDrawState* drawState,
                                               GrColor color,
                                               const SkMatrix& viewMatrix,
                                               const SkPath& path,
                                               const SkStrokeRec& stroke,
                                               bool antiAlias) {
    // we've already bailed on inverse filled paths, so this is safe
    if (path.isEmpty()) {
        return true;
    }

    SkASSERT(fContext);

    // get mip level
    SkScalar maxScale = viewMatrix.getMaxScale();
    const SkRect& bounds = path.getBounds();
    SkScalar maxDim = SkMaxScalar(bounds.width(), bounds.height());
    SkScalar size = maxScale*maxDim;
    uint32_t desiredDimension;
    if (size <= kSmallMIP) {
        desiredDimension = kSmallMIP;
    } else if (size <= kMediumMIP) {
        desiredDimension = kMediumMIP;
    } else {
        desiredDimension = kLargeMIP;
    }

    // check to see if path is cached
    // TODO: handle stroked vs. filled version of same path
    PathData::Key key = { path.getGenerationID(), desiredDimension };
    PathData* pathData = fPathCache.find(key);
    if (NULL == pathData) {
        SkScalar scale = desiredDimension/maxDim;
        pathData = this->addPathToAtlas(path, stroke, antiAlias, desiredDimension, scale);
        if (NULL == pathData) {
            return false;
        }
    }

    // use signed distance field to render
    return this->internalDrawPath(target, drawState, color, viewMatrix, path, pathData);
}

// padding around path bounds to allow for antialiased pixels
const SkScalar kAntiAliasPad = 1.0f;

GrAADistanceFieldPathRenderer::PathData* GrAADistanceFieldPathRenderer::addPathToAtlas(
                                                                        const SkPath& path,
                                                                        const SkStrokeRec& stroke,
                                                                        bool antiAlias,
                                                                        uint32_t dimension,
                                                                        SkScalar scale) {

    // generate distance field and add to atlas
    if (NULL == fAtlas) {
        SkISize textureSize = SkISize::Make(ATLAS_TEXTURE_WIDTH, ATLAS_TEXTURE_HEIGHT);
        fAtlas = SkNEW_ARGS(GrAtlas, (fContext->getGpu(), kAlpha_8_GrPixelConfig,
                                      kNone_GrSurfaceFlags, textureSize,
                                      NUM_PLOTS_X, NUM_PLOTS_Y, false));
        if (NULL == fAtlas) {
            return NULL;
        }
    }
    
    const SkRect& bounds = path.getBounds();
    
    // generate bounding rect for bitmap draw
    SkRect scaledBounds = bounds;
    // scale to mip level size
    scaledBounds.fLeft *= scale;
    scaledBounds.fTop *= scale;
    scaledBounds.fRight *= scale;
    scaledBounds.fBottom *= scale;
    // move the origin to an integer boundary (gives better results)
    SkScalar dx = SkScalarFraction(scaledBounds.fLeft);
    SkScalar dy = SkScalarFraction(scaledBounds.fTop);
    scaledBounds.offset(-dx, -dy);
    // get integer boundary
    SkIRect devPathBounds;
    scaledBounds.roundOut(&devPathBounds);
    // pad to allow room for antialiasing
    devPathBounds.outset(SkScalarCeilToInt(kAntiAliasPad), SkScalarCeilToInt(kAntiAliasPad));
    // move origin to upper left corner
    devPathBounds.offsetTo(0,0);
    
    // draw path to bitmap
    SkMatrix drawMatrix;
    drawMatrix.setTranslate(-bounds.left(), -bounds.top());
    drawMatrix.postScale(scale, scale);
    drawMatrix.postTranslate(kAntiAliasPad, kAntiAliasPad);
    GrSWMaskHelper helper(fContext);
    
    if (!helper.init(devPathBounds, &drawMatrix)) {
        return NULL;
    }
    helper.draw(path, stroke, SkRegion::kReplace_Op, antiAlias, 0xFF);
    
    // generate signed distance field
    devPathBounds.outset(SK_DistanceFieldPad, SK_DistanceFieldPad);
    int width = devPathBounds.width();
    int height = devPathBounds.height();
    SkAutoSMalloc<1024> dfStorage(width*height*sizeof(unsigned char));
    helper.toSDF((unsigned char*) dfStorage.get());
    
    // add to atlas
    SkIPoint16  atlasLocation;
    GrPlot* plot = fAtlas->addToAtlas(&fPlotUsage, width, height, dfStorage.get(),
                                      &atlasLocation);
    
    // if atlas full
    if (NULL == plot) {
        if (this->freeUnusedPlot()) {
            plot = fAtlas->addToAtlas(&fPlotUsage, width, height, dfStorage.get(),
                                      &atlasLocation);
            if (plot) {
                goto HAS_ATLAS;
            }
        }
    
        if (c_DumpPathCache) {
#ifdef SK_DEVELOPER
            GrTexture* texture = fAtlas->getTexture();
            texture->surfacePriv().savePixels("pathcache.png");
#endif
        }
        
        // before we purge the cache, we must flush any accumulated draws
        fContext->flush();
        
        if (this->freeUnusedPlot()) {
            plot = fAtlas->addToAtlas(&fPlotUsage, width, height, dfStorage.get(),
                                      &atlasLocation);
            if (plot) {
                goto HAS_ATLAS;
            }
        }
            
        return NULL;
    }
    
HAS_ATLAS:
    // add to cache
    PathData* pathData = SkNEW(PathData);
    pathData->fKey.fGenID = path.getGenerationID();
    pathData->fKey.fDimension = dimension;
    pathData->fScale = scale;
    pathData->fPlot = plot;
    // change the scaled rect to match the size of the inset distance field
    scaledBounds.fRight = scaledBounds.fLeft +
        SkIntToScalar(devPathBounds.width() - 2*SK_DistanceFieldInset);
    scaledBounds.fBottom = scaledBounds.fTop +
        SkIntToScalar(devPathBounds.height() - 2*SK_DistanceFieldInset);
    // shift the origin to the correct place relative to the distance field
    // need to also restore the fractional translation
    scaledBounds.offset(-SkIntToScalar(SK_DistanceFieldInset) - kAntiAliasPad + dx,
                        -SkIntToScalar(SK_DistanceFieldInset) - kAntiAliasPad + dy);
    pathData->fBounds = scaledBounds;
    // origin we render from is inset from distance field edge
    atlasLocation.fX += SK_DistanceFieldInset;
    atlasLocation.fY += SK_DistanceFieldInset;
    pathData->fAtlasLocation = atlasLocation;
    
    fPathCache.add(pathData);
    fPathList.addToTail(pathData);
#ifdef DF_PATH_TRACKING
    ++g_NumCachedPaths;
#endif

    return pathData;
}

bool GrAADistanceFieldPathRenderer::freeUnusedPlot() {
    // find an unused plot
    GrPlot* plot = fAtlas->getUnusedPlot();
    if (NULL == plot) {
        return false;
    }
    plot->resetRects();

    // remove any paths that use this plot
    PathDataList::Iter iter;
    iter.init(fPathList, PathDataList::Iter::kHead_IterStart);
    PathData* pathData;
    while ((pathData = iter.get())) {
        iter.next();
        if (plot == pathData->fPlot) {
            fPathCache.remove(pathData->fKey);
            fPathList.remove(pathData);
            SkDELETE(pathData);
#ifdef DF_PATH_TRACKING
            ++g_NumFreedPaths;
#endif
        }
    }
        
    // tell the atlas to free the plot
    GrAtlas::RemovePlot(&fPlotUsage, plot);
    
    return true;
}

bool GrAADistanceFieldPathRenderer::internalDrawPath(GrDrawTarget* target,
                                                     GrDrawState* drawState,
                                                     GrColor color,
                                                     const SkMatrix& viewMatrix,
                                                     const SkPath& path,
                                                     const PathData* pathData) {
    GrTexture* texture = fAtlas->getTexture();
    GrDrawState::AutoRestoreEffects are(drawState);
    
    SkASSERT(pathData->fPlot);
    GrDrawTarget::DrawToken drawToken = target->getCurrentDrawToken();
    pathData->fPlot->setDrawToken(drawToken);
    
    // set up any flags
    uint32_t flags = 0;
    flags |= viewMatrix.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;

    GrTextureParams params(SkShader::kRepeat_TileMode, GrTextureParams::kBilerp_FilterMode);
    if (flags != fEffectFlags || fCachedGeometryProcessor->color() != color ||
        !fCachedGeometryProcessor->viewMatrix().cheapEqualTo(viewMatrix)) {
        fCachedGeometryProcessor.reset(GrDistanceFieldNoGammaTextureEffect::Create(color,
                                                                                   viewMatrix,
                                                                                   texture,
                                                                                   params,
                                                                                   flags,
                                                                                   false));
        fEffectFlags = flags;
    }

    void* vertices = NULL;
    bool success = target->reserveVertexAndIndexSpace(4,
                                                      fCachedGeometryProcessor->getVertexStride(),
                                                      0, &vertices, NULL);
    SkASSERT(fCachedGeometryProcessor->getVertexStride() == 2 * sizeof(SkPoint));
    GrAlwaysAssert(success);
    
    SkScalar dx = pathData->fBounds.fLeft;
    SkScalar dy = pathData->fBounds.fTop;
    SkScalar width = pathData->fBounds.width();
    SkScalar height = pathData->fBounds.height();
    
    SkScalar invScale = 1.0f/pathData->fScale;
    dx *= invScale;
    dy *= invScale;
    width *= invScale;
    height *= invScale;
    
    SkFixed tx = SkIntToFixed(pathData->fAtlasLocation.fX);
    SkFixed ty = SkIntToFixed(pathData->fAtlasLocation.fY);
    SkFixed tw = SkScalarToFixed(pathData->fBounds.width());
    SkFixed th = SkScalarToFixed(pathData->fBounds.height());
    
    // vertex positions
    SkRect r = SkRect::MakeXYWH(dx, dy, width, height);    
    size_t vertSize = 2 * sizeof(SkPoint);
    SkPoint* positions = reinterpret_cast<SkPoint*>(vertices);
    positions->setRectFan(r.left(), r.top(), r.right(), r.bottom(), vertSize);
    
    // vertex texture coords
    intptr_t intPtr = reinterpret_cast<intptr_t>(positions);
    SkPoint* textureCoords = reinterpret_cast<SkPoint*>(intPtr + vertSize - sizeof(SkPoint));
    textureCoords->setRectFan(SkFixedToFloat(texture->texturePriv().normalizeFixedX(tx)),
                              SkFixedToFloat(texture->texturePriv().normalizeFixedY(ty)),
                              SkFixedToFloat(texture->texturePriv().normalizeFixedX(tx + tw)),
                              SkFixedToFloat(texture->texturePriv().normalizeFixedY(ty + th)),
                              vertSize);
    
    viewMatrix.mapRect(&r);
    target->setIndexSourceToBuffer(fContext->getQuadIndexBuffer());
    target->drawIndexedInstances(drawState, fCachedGeometryProcessor.get(),
                                 kTriangles_GrPrimitiveType, 1, 4, 6, &r);
    target->resetVertexSource();
    
    return true;
}

