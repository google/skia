
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
#define ATLAS_TEXTURE_HEIGHT 1024
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
bool GrAADistanceFieldPathRenderer::canDrawPath(const SkPath& path,
                                                const SkStrokeRec& stroke,
                                                const GrDrawTarget* target,
                                                bool antiAlias) const {
    
    // TODO: Support inverse fill
    // TODO: Support strokes
    if (!target->caps()->shaderDerivativeSupport() || !antiAlias || path.isInverseFillType()
        || path.isVolatile() || SkStrokeRec::kFill_Style != stroke.getStyle()) {
        return false;
    }

    // currently don't support perspective or scaling more than 3x
    const GrDrawState& drawState = target->getDrawState();
    const SkMatrix& vm = drawState.getViewMatrix();
    if (vm.hasPerspective() || vm.getMaxScale() > 3.0f) {
        return false;
    }
    
    // only support paths smaller than 64 x 64
    const SkRect& bounds = path.getBounds();
    return bounds.width() < 64.f && bounds.height() < 64.f;
}


GrPathRenderer::StencilSupport GrAADistanceFieldPathRenderer::onGetStencilSupport(
                                                                       const SkPath&,
                                                                       const SkStrokeRec&,
                                                                       const GrDrawTarget*) const {
    return GrPathRenderer::kNoSupport_StencilSupport;
}

////////////////////////////////////////////////////////////////////////////////

// position + texture coord
extern const GrVertexAttrib gSDFPathVertexAttribs[] = {
    { kVec2f_GrVertexAttribType, 0, kPosition_GrVertexAttribBinding },
    { kVec2f_GrVertexAttribType, sizeof(SkPoint), kGeometryProcessor_GrVertexAttribBinding }
};
static const size_t kSDFPathVASize = 2 * sizeof(SkPoint);

bool GrAADistanceFieldPathRenderer::onDrawPath(const SkPath& path,
                                               const SkStrokeRec& stroke,
                                               GrDrawTarget* target,
                                               bool antiAlias) {
    // we've already bailed on inverse filled paths, so this is safe
    if (path.isEmpty()) {
        return true;
    }

    SkASSERT(fContext);

    // check to see if path is cached
    // TODO: handle stroked vs. filled version of same path
    PathData* pathData = fPathCache.find(path.getGenerationID());
    if (NULL == pathData) {
        pathData = this->addPathToAtlas(path, stroke, antiAlias);
        if (NULL == pathData) {
            return false;
        }
    }

    // use signed distance field to render
    return this->internalDrawPath(path, pathData, target);
}

// factor used to scale the path prior to building distance field
const SkScalar kScaleFactor = 2.0f;
// padding around path bounds to allow for antialiased pixels
const SkScalar kAntiAliasPad = 1.0f;

GrAADistanceFieldPathRenderer::PathData* GrAADistanceFieldPathRenderer::addPathToAtlas(
                                                                        const SkPath& path,
                                                                        const SkStrokeRec& stroke,
                                                                        bool antiAlias) {

    // generate distance field and add to atlas
    if (NULL == fAtlas) {
        SkISize textureSize = SkISize::Make(ATLAS_TEXTURE_WIDTH, ATLAS_TEXTURE_HEIGHT);
        fAtlas = SkNEW_ARGS(GrAtlas, (fContext->getGpu(), kAlpha_8_GrPixelConfig,
                                      kNone_GrTextureFlags, textureSize,
                                      NUM_PLOTS_X, NUM_PLOTS_Y, false));
        if (NULL == fAtlas) {
            return NULL;
        }
    }
    
    const SkRect& bounds = path.getBounds();
    
    // generate bounding rect for bitmap draw
    SkRect scaledBounds = bounds;
    // scale up to improve maxification range
    scaledBounds.fLeft *= kScaleFactor;
    scaledBounds.fTop *= kScaleFactor;
    scaledBounds.fRight *= kScaleFactor;
    scaledBounds.fBottom *= kScaleFactor;
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
    drawMatrix.postScale(kScaleFactor, kScaleFactor);
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
    pathData->fGenID = path.getGenerationID();
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
            fPathCache.remove(pathData->fGenID);
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

bool GrAADistanceFieldPathRenderer::internalDrawPath(const SkPath& path,
                                                     const PathData* pathData,
                                                     GrDrawTarget* target) {
    GrTexture* texture = fAtlas->getTexture();
    GrDrawState* drawState = target->drawState();
    GrDrawState::AutoRestoreEffects are(drawState);
    
    SkASSERT(pathData->fPlot);
    GrDrawTarget::DrawToken drawToken = target->getCurrentDrawToken();
    pathData->fPlot->setDrawToken(drawToken);
    
    // make me some vertices
    drawState->setVertexAttribs<gSDFPathVertexAttribs>(SK_ARRAY_COUNT(gSDFPathVertexAttribs),
                                                       kSDFPathVASize);
    void* vertices = NULL;
    bool success = target->reserveVertexAndIndexSpace(4, 0, &vertices, NULL);
    GrAlwaysAssert(success);
    
    SkScalar dx = pathData->fBounds.fLeft;
    SkScalar dy = pathData->fBounds.fTop;
    SkScalar width = pathData->fBounds.width();
    SkScalar height = pathData->fBounds.height();
    
    SkScalar invScale = 1.0f/kScaleFactor;
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
    
    // set up any flags
    uint32_t flags = 0;
    const SkMatrix& vm = drawState->getViewMatrix();
    flags |= vm.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;
    
    GrTextureParams params(SkShader::kRepeat_TileMode, GrTextureParams::kBilerp_FilterMode);
    if (flags != fEffectFlags) {
        fCachedGeometryProcessor.reset(GrDistanceFieldNoGammaTextureEffect::Create(texture,
                                                                                   params,
                                                                                   flags));
        fEffectFlags = flags;
    }
    drawState->setGeometryProcessor(fCachedGeometryProcessor.get());

    vm.mapRect(&r);
    target->setIndexSourceToBuffer(fContext->getQuadIndexBuffer());
    target->drawIndexedInstances(kTriangles_GrPrimitiveType, 1, 4, 6, &r);
    target->resetVertexSource();
    
    return true;
}

