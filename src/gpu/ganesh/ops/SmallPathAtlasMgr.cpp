/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/ops/SmallPathAtlasMgr.h"

#include "include/core/SkSize.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/ops/SmallPathShapeData.h"

#include <cstddef>

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

using MaskFormat = skgpu::MaskFormat;

#ifdef DF_PATH_TRACKING
static int g_NumCachedShapes = 0;
static int g_NumFreedShapes = 0;
#endif

namespace skgpu::ganesh {

SmallPathAtlasMgr::SmallPathAtlasMgr() {}

SmallPathAtlasMgr::~SmallPathAtlasMgr() {
    this->reset();
}

void SmallPathAtlasMgr::reset() {
    ShapeDataList::Iter iter;
    iter.init(fShapeList, ShapeDataList::Iter::kHead_IterStart);
    SmallPathShapeData* shapeData;
    while ((shapeData = iter.get())) {
        iter.next();
        delete shapeData;
    }

    fShapeList.reset();
    fShapeCache.reset();

#ifdef DF_PATH_TRACKING
    SkDebugf("Cached shapes: %d, freed shapes: %d\n", g_NumCachedShapes, g_NumFreedShapes);
#endif

    fAtlas = nullptr;
}

bool SmallPathAtlasMgr::initAtlas(GrProxyProvider* proxyProvider, const GrCaps* caps) {
    if (fAtlas) {
        return true;
    }

    static constexpr size_t kMaxAtlasTextureBytes = 2048 * 2048;
    static constexpr size_t kPlotWidth = 512;
    static constexpr size_t kPlotHeight = 256;

    GrColorType atlasColorType = GrColorType::kAlpha_8;
    const GrBackendFormat format = caps->getDefaultBackendFormat(atlasColorType,
                                                                 GrRenderable::kNo);

    GrDrawOpAtlasConfig atlasConfig(caps->maxTextureSize(), kMaxAtlasTextureBytes);
    SkISize size = atlasConfig.atlasDimensions(MaskFormat::kA8);
    fAtlas = GrDrawOpAtlas::Make(proxyProvider, format,
                                 GrColorTypeToSkColorType(atlasColorType),
                                 GrColorTypeBytesPerPixel(atlasColorType),
                                 size.width(), size.height(),
                                 kPlotWidth, kPlotHeight, this,
                                 GrDrawOpAtlas::AllowMultitexturing::kYes,
                                 this,
                                 /*label=*/"SmallPathAtlas");

    return SkToBool(fAtlas);
}

void SmallPathAtlasMgr::deleteCacheEntry(SmallPathShapeData* shapeData) {
    fShapeCache.remove(shapeData->fKey);
    fShapeList.remove(shapeData);
    delete shapeData;
}

SmallPathShapeData* SmallPathAtlasMgr::findOrCreate(const SmallPathShapeDataKey& key) {
    auto shapeData = fShapeCache.find(key);
    if (!shapeData) {
        // TODO: move the key into the ctor
        shapeData = new SmallPathShapeData(key);
        fShapeCache.add(shapeData);
        fShapeList.addToTail(shapeData);
#ifdef DF_PATH_TRACKING
        ++g_NumCachedShapes;
#endif
    } else if (!fAtlas->hasID(shapeData->fAtlasLocator.plotLocator())) {
        shapeData->fAtlasLocator.invalidatePlotLocator();
    }

    return shapeData;
}

SmallPathShapeData* SmallPathAtlasMgr::findOrCreate(const GrStyledShape& shape,
                                                    int desiredDimension) {
    SmallPathShapeDataKey key(shape, desiredDimension);

    // TODO: move the key into 'findOrCreate'
    return this->findOrCreate(key);
}

SmallPathShapeData* SmallPathAtlasMgr::findOrCreate(const GrStyledShape& shape,
                                                    const SkMatrix& ctm) {
    SmallPathShapeDataKey key(shape, ctm);

    // TODO: move the key into 'findOrCreate'
    return this->findOrCreate(key);
}

GrDrawOpAtlas::ErrorCode SmallPathAtlasMgr::addToAtlas(GrResourceProvider* resourceProvider,
                                                       GrDeferredUploadTarget* target,
                                                       int width, int height, const void* image,
                                                       skgpu::AtlasLocator* locator) {
    return fAtlas->addToAtlas(resourceProvider, target, width, height, image, locator);
}

void SmallPathAtlasMgr::setUseToken(SmallPathShapeData* shapeData,
                                    skgpu::AtlasToken token) {
    fAtlas->setLastUseToken(shapeData->fAtlasLocator, token);
}

// Callback to clear out internal path cache when eviction occurs
void SmallPathAtlasMgr::evict(skgpu::PlotLocator plotLocator) {
    // remove any paths that use this plot
    ShapeDataList::Iter iter;
    iter.init(fShapeList, ShapeDataList::Iter::kHead_IterStart);
    SmallPathShapeData* shapeData;
    while ((shapeData = iter.get())) {
        iter.next();
        if (plotLocator == shapeData->fAtlasLocator.plotLocator()) {
            fShapeCache.remove(shapeData->fKey);
            fShapeList.remove(shapeData);
            delete shapeData;
#ifdef DF_PATH_TRACKING
            ++g_NumFreedShapes;
#endif
        }
    }
}

}  // namespace skgpu::ganesh

#endif // SK_ENABLE_OPTIMIZE_SIZE
