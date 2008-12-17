#include "SkGLCanvas.h"
#include "SkGLDevice.h"
#include "SkBlitter.h"
#include "SkDraw.h"
#include "SkDrawProcs.h"
#include "SkGL.h"
#include "SkTemplates.h"
#include "SkUtils.h"
#include "SkXfermode.h"

#ifdef SK_GL_DEVICE_FBO
    #define USE_FBO_DEVICE
    #include "SkGLDevice_FBO.h"
#else
    #define USE_SWLAYER_DEVICE
    #include "SkGLDevice_SWLayer.h"
#endif

// maximum number of entries in our texture cache (before purging)
#define kTexCountMax_Default    256
// maximum number of bytes used (by gl) for the texture cache (before purging)
#define kTexSizeMax_Default     (4 * 1024 * 1024)

///////////////////////////////////////////////////////////////////////////////

SkGLCanvas::SkGLCanvas() {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_SCISSOR_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    fViewportSize.set(0, 0);
}

SkGLCanvas::~SkGLCanvas() {
    // call this now, while our override of restore() is in effect
    this->restoreToCount(1);
}

///////////////////////////////////////////////////////////////////////////////

bool SkGLCanvas::getViewport(SkIPoint* size) const {
    if (size) {
        *size = fViewportSize;
    }
    return true;
}

bool SkGLCanvas::setViewport(int width, int height) {
    fViewportSize.set(width, height);

    const bool isOpaque = false; // should this be a parameter to setViewport?
    const bool isForLayer = false;   // viewport is the base layer
    SkDevice* device = this->createDevice(SkBitmap::kARGB_8888_Config, width,
                                          height, isOpaque, isForLayer);
    this->setDevice(device)->unref();

    return true;
}

SkDevice* SkGLCanvas::createDevice(SkBitmap::Config, int width, int height,
                                   bool isOpaque, bool isForLayer) {
    SkBitmap bitmap;
    
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
    bitmap.setIsOpaque(isOpaque);

#ifdef USE_FBO_DEVICE
    return SkNEW_ARGS(SkGLDevice_FBO, (bitmap, isForLayer));
#elif defined(USE_SWLAYER_DEVICE)
    if (isForLayer) {
        bitmap.allocPixels();
        if (!bitmap.isOpaque()) {
            bitmap.eraseColor(0);
        }
        return SkNEW_ARGS(SkGLDevice_SWLayer, (bitmap));
    } else {
        return SkNEW_ARGS(SkGLDevice, (bitmap, isForLayer));
    }
#else
    return SkNEW_ARGS(SkGLDevice, (bitmap, isForLayer));
#endif
}

///////////////////////////////////////////////////////////////////////////////

#include "SkTextureCache.h"
#include "SkThread.h"

static SkMutex gTextureCacheMutex;
static SkTextureCache gTextureCache(kTexCountMax_Default, kTexSizeMax_Default);

SkGLDevice::TexCache* SkGLDevice::LockTexCache(const SkBitmap& bitmap,
                                                 GLuint* name, SkPoint* size) {
    SkAutoMutexAcquire amc(gTextureCacheMutex);
    
    SkTextureCache::Entry* entry = gTextureCache.lock(bitmap);
    if (NULL != entry) {
        if (name) {
            *name = entry->name();
        }
        if (size) {
            *size = entry->texSize();
        }
    }
    return (TexCache*)entry;
}

void SkGLDevice::UnlockTexCache(TexCache* cache) {
    SkAutoMutexAcquire amc(gTextureCacheMutex);
    gTextureCache.unlock((SkTextureCache::Entry*)cache);
}

// public exposure of texture cache settings

size_t SkGLCanvas::GetTextureCacheMaxCount() {
    SkAutoMutexAcquire amc(gTextureCacheMutex);
    return gTextureCache.getMaxCount();
}

size_t SkGLCanvas::GetTextureCacheMaxSize() {
    SkAutoMutexAcquire amc(gTextureCacheMutex);
    return gTextureCache.getMaxSize();
}

void SkGLCanvas::SetTextureCacheMaxCount(size_t count) {
    SkAutoMutexAcquire amc(gTextureCacheMutex);
    gTextureCache.setMaxCount(count);
}

void SkGLCanvas::SetTextureCacheMaxSize(size_t size) {
    SkAutoMutexAcquire amc(gTextureCacheMutex);
    gTextureCache.setMaxSize(size);
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGLTextCache.h"

static bool deleteCachesProc(SkGlyphCache* cache, void* texturesAreValid) {
    void* auxData;
    if (cache->getAuxProcData(SkGLDevice::GlyphCacheAuxProc, &auxData)) {
        bool valid = texturesAreValid != NULL;
        SkGLTextCache* textCache = static_cast<SkGLTextCache*>(auxData);
        // call this before delete, in case valid is false
        textCache->deleteAllStrikes(valid);
        // now free the memory for the cache itself
        SkDELETE(textCache);
        // now remove the entry in the glyphcache (does not call the proc)
        cache->removeAuxProc(SkGLDevice::GlyphCacheAuxProc);
    }
    return false;   // keep going
}

void SkGLCanvas::DeleteAllTextures() {
    // free the textures in our cache

    gTextureCacheMutex.acquire();
    gTextureCache.deleteAllCaches(true);
    gTextureCacheMutex.release();
    
    // now free the textures in the font cache
    
    SkGlyphCache::VisitAllCaches(deleteCachesProc, reinterpret_cast<void*>(true));
}

void SkGLCanvas::AbandonAllTextures() {
    // abandon the textures in our cache

    gTextureCacheMutex.acquire();
    gTextureCache.deleteAllCaches(false);
    gTextureCacheMutex.release();

    // abandon the textures in the font cache
    
    SkGlyphCache::VisitAllCaches(deleteCachesProc, reinterpret_cast<void*>(false));
}

