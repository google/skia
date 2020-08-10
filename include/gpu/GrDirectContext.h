/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDirectContext_DEFINED
#define GrDirectContext_DEFINED

#include "include/gpu/GrContext.h"

class GrAtlasManager;
class GrSmallPathAtlasMgr;

class SK_API GrDirectContext : public GrContext {
public:
#ifdef SK_GL
    /**
     * Creates a GrDirectContext for a backend context. If no GrGLInterface is provided then the
     * result of GrGLMakeNativeInterface() is used if it succeeds.
     */
    static sk_sp<GrDirectContext> MakeGL(sk_sp<const GrGLInterface>, const GrContextOptions&);
    static sk_sp<GrDirectContext> MakeGL(sk_sp<const GrGLInterface>);
    static sk_sp<GrDirectContext> MakeGL(const GrContextOptions&);
    static sk_sp<GrDirectContext> MakeGL();
#endif

#ifdef SK_VULKAN
    /**
     * The Vulkan context (VkQueue, VkDevice, VkInstance) must be kept alive until the returned
     * GrDirectContext is destroyed. This also means that any objects created with this
     * GrDirectContext (e.g. SkSurfaces, SkImages, etc.) must also be released as they may hold
     * refs on the GrDirectContext. Once all these objects and the GrDirectContext are released,
     * then it is safe to delete the vulkan objects.
     */
    static sk_sp<GrDirectContext> MakeVulkan(const GrVkBackendContext&, const GrContextOptions&);
    static sk_sp<GrDirectContext> MakeVulkan(const GrVkBackendContext&);
#endif

#ifdef SK_METAL
    /**
     * Makes a GrDirectContext which uses Metal as the backend. The device parameter is an
     * MTLDevice and queue is an MTLCommandQueue which should be used by the backend. These objects
     * must have a ref on them which can be transferred to Ganesh which will release the ref
     * when the GrDirectContext is destroyed.
     */
    static sk_sp<GrDirectContext> MakeMetal(void* device, void* queue, const GrContextOptions&);
    static sk_sp<GrDirectContext> MakeMetal(void* device, void* queue);
#endif

#ifdef SK_DIRECT3D
    /**
     * Makes a GrDirectContext which uses Direct3D as the backend. The Direct3D context
     * must be kept alive until the returned GrDirectContext is first destroyed or abandoned.
     */
    static sk_sp<GrDirectContext> MakeDirect3D(const GrD3DBackendContext&, const GrContextOptions&);
    static sk_sp<GrDirectContext> MakeDirect3D(const GrD3DBackendContext&);
#endif

#ifdef SK_DAWN
    static sk_sp<GrDirectContext> MakeDawn(const wgpu::Device&,
                                           const GrContextOptions&);
    static sk_sp<GrDirectContext> MakeDawn(const wgpu::Device&);
#endif

    static sk_sp<GrDirectContext> MakeMock(const GrMockOptions*, const GrContextOptions&);
    static sk_sp<GrDirectContext> MakeMock(const GrMockOptions*);

    ~GrDirectContext() override;

    void abandonContext() override;

    void releaseResourcesAndAbandonContext() override;

    void freeGpuResources() override;

protected:
    GrDirectContext(GrBackendApi backend, const GrContextOptions& options);

    bool init() override;

    GrAtlasManager* onGetAtlasManager() override { return fAtlasManager.get(); }
    GrSmallPathAtlasMgr* onGetSmallPathAtlasMgr() override;

    GrDirectContext* asDirectContext() override { return this; }

private:
    std::unique_ptr<GrAtlasManager> fAtlasManager;

    std::unique_ptr<GrSmallPathAtlasMgr> fSmallPathAtlasMgr;

    typedef GrContext INHERITED;
};


#endif
