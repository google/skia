/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDirectContext_DEFINED
#define GrDirectContext_DEFINED

#include "include/private/GrContext.h"

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

    /**
     * The context normally assumes that no outsider is setting state
     * within the underlying 3D API's context/device/whatever. This call informs
     * the context that the state was modified and it should resend. Shouldn't
     * be called frequently for good performance.
     * The flag bits, state, is dependent on which backend is used by the
     * context, either GL or D3D (possible in future).
     */
    void resetContext(uint32_t state = kAll_GrBackendState);

    /**
     * If the backend is GrBackendApi::kOpenGL, then all texture unit/target combinations for which
     * the context has modified the bound texture will have texture id 0 bound. This does not
     * flush the context. Calling resetContext() does not change the set that will be bound
     * to texture id 0 on the next call to resetGLTextureBindings(). After this is called
     * all unit/target combinations are considered to have unmodified bindings until the context
     * subsequently modifies them (meaning if this is called twice in a row with no intervening
     * context usage then the second call is a no-op.)
     */
    void resetGLTextureBindings();

    /**
     * Abandons all GPU resources and assumes the underlying backend 3D API context is no longer
     * usable. Call this if you have lost the associated GPU context, and thus internal texture,
     * buffer, etc. references/IDs are now invalid. Calling this ensures that the destructors of the
     * GrContext and any of its created resource objects will not make backend 3D API calls. Content
     * rendered but not previously flushed may be lost. After this function is called all subsequent
     * calls on the GrContext will fail or be no-ops.
     *
     * The typical use case for this function is that the underlying 3D context was lost and further
     * API calls may crash.
     *
     * For Vulkan, even if the device becomes lost, the VkQueue, VkDevice, or VkInstance used to
     * create the context must be kept alive even after abandoning the context. Those objects must
     * live for the lifetime of the context object itself. The reason for this is so that
     * we can continue to delete any outstanding GrBackendTextures/RenderTargets which must be
     * cleaned up even in a device lost state.
     */
    void abandonContext() override;

    /**
     * Returns true if the context was abandoned or if the if the backend specific context has
     * gotten into an unrecoverarble, lost state (e.g. in Vulkan backend if we've gotten a
     * VK_ERROR_DEVICE_LOST). If the backend context is lost, this call will also abandon the
     * GrContext.
     */
    bool abandoned() override;

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

    using INHERITED = GrContext;
};


#endif
