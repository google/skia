/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureProvider_DEFINED
#define GrTextureProvider_DEFINED

#include "GrTexture.h"

class SK_API GrTextureProvider {
public:
    ///////////////////////////////////////////////////////////////////////////
    // Textures

    /**
     * Creates a new texture in the resource cache and returns it. The caller owns a
     * ref on the returned texture which must be balanced by a call to unref.
     *
     * @param desc      Description of the texture properties.
     * @param budgeted  Does the texture count against the resource cache budget?
     * @param srcData   Pointer to the pixel values (optional).
     * @param rowBytes  The number of bytes between rows of the texture. Zero
     *                  implies tightly packed rows. For compressed pixel configs, this
     *                  field is ignored.
     */
    GrTexture* createTexture(const GrSurfaceDesc& desc, bool budgeted, const void* srcData,
                             size_t rowBytes);

    /** Shortcut for creating a texture with no initial data to upload. */
    GrTexture* createTexture(const GrSurfaceDesc& desc, bool budgeted) {
        return this->createTexture(desc, budgeted, NULL, 0);
    }

    /** Assigns a unique key to the texture. The texture will be findable via this key using
        findTextureByUniqueKey(). If an existing texture has this key, it's key will be removed. */
    void assignUniqueKeyToTexture(const GrUniqueKey& key, GrTexture* texture) {
        this->assignUniqueKeyToResource(key, texture);
    }

    /** Finds a texture by unique key. If the texture is found it is ref'ed and returned. */
    GrTexture* findAndRefTextureByUniqueKey(const GrUniqueKey& key) {
        GrGpuResource* resource = this->findAndRefResourceByUniqueKey(key);
        if (resource) {
            GrTexture* texture = static_cast<GrSurface*>(resource)->asTexture();
            SkASSERT(texture);
            return texture;
        }
        return NULL;
    }

    /**
     * Determines whether a texture is associated with the unique key. If the texture is found it
     * will not be locked or returned. This call does not affect the priority of the resource for
     * deletion.
     */
    bool existsTextureWithUniqueKey(const GrUniqueKey& key) const {
        return this->existsResourceWithUniqueKey(key);
    }

    /**
     * Enum that determines how closely a returned scratch texture must match
     * a provided GrSurfaceDesc. TODO: Remove this. createTexture() should be used
     * for exact match and refScratchTexture() should be replaced with createApproxTexture().
     */
    enum ScratchTexMatch {
        /**
         * Finds a texture that exactly matches the descriptor.
         */
        kExact_ScratchTexMatch,
        /**
         * Finds a texture that approximately matches the descriptor. Will be
         * at least as large in width and height as desc specifies. If desc
         * specifies that texture is a render target then result will be a
         * render target. If desc specifies a render target and doesn't set the
         * no stencil flag then result will have a stencil. Format and aa level
         * will always match.
         */
        kApprox_ScratchTexMatch
    };

    /**
     * Returns a texture matching the desc. It's contents are unknown. The caller
     * owns a ref on the returned texture and must balance with a call to unref.
     * It is guaranteed that the same texture will not be returned in subsequent
     * calls until all refs to the texture are dropped.
     *
     * internalFlag is a temporary workaround until changes in the internal
     * architecture are complete. Use the default value.
     *
     * TODO: Once internal flag can be removed, this should be replaced with
     * createApproxTexture() and exact textures should be created with
     * createTexture().
     */
    GrTexture* refScratchTexture(const GrSurfaceDesc&, ScratchTexMatch match,
                                 bool internalFlag = false);

    ///////////////////////////////////////////////////////////////////////////
    // Wrapped Backend Surfaces

    /**
     * Wraps an existing texture with a GrTexture object.
     *
     * OpenGL: if the object is a texture Gr may change its GL texture params
     *         when it is drawn.
     *
     * @param  desc     description of the object to create.
     *
     * @return GrTexture object or NULL on failure.
     */
    GrTexture* wrapBackendTexture(const GrBackendTextureDesc& desc);

    /**
     * Wraps an existing render target with a GrRenderTarget object. It is
     * similar to wrapBackendTexture but can be used to draw into surfaces
     * that are not also textures (e.g. FBO 0 in OpenGL, or an MSAA buffer that
     * the client will resolve to a texture).
     *
     * @param  desc     description of the object to create.
     *
     * @return GrTexture object or NULL on failure.
     */
     GrRenderTarget* wrapBackendRenderTarget(const GrBackendRenderTargetDesc& desc);

protected:
    GrTextureProvider(GrGpu* gpu, GrResourceCache* cache) : fCache(cache), fGpu(gpu) {}

    /**
     * Assigns a unique key to a resource. If the key is associated with another resource that
     * association is removed and replaced by this resource.
     */
    void assignUniqueKeyToResource(const GrUniqueKey&, GrGpuResource*);

    /**
     * Finds a resource in the cache, based on the specified key. This is intended for use in
     * conjunction with addResourceToCache(). The return value will be NULL if not found. The
     * caller must balance with a call to unref().
     */
    GrGpuResource* findAndRefResourceByUniqueKey(const GrUniqueKey&);

    /**
     * Determines whether a resource is in the cache. If the resource is found it
     * will not be locked or returned. This call does not affect the priority of
     * the resource for deletion.
     */
    bool existsResourceWithUniqueKey(const GrUniqueKey& key) const;

    GrTexture* internalRefScratchTexture(const GrSurfaceDesc&, uint32_t flags);

    void abandon() {
        fCache = NULL;
        fGpu = NULL;
    }

    GrResourceCache* cache() { return fCache; }
    const GrResourceCache* cache() const { return fCache; }

    GrGpu* gpu() { return fGpu; }
    const GrGpu* gpu() const { return fGpu; }

private:
    bool isAbandoned() const {
        SkASSERT(SkToBool(fGpu) == SkToBool(fCache));
        return !SkToBool(fCache);
    }

    GrResourceCache* fCache;
    GrGpu* fGpu;
};

#endif
