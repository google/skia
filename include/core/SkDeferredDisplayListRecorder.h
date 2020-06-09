/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeferredDisplayListRecorder_DEFINED
#define SkDeferredDisplayListRecorder_DEFINED

#include "include/core/SkDeferredDisplayList.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "include/core/SkTypes.h"

class GrBackendFormat;
class GrBackendTexture;
class GrContext;
class SkCanvas;
class SkImage;
class SkPromiseImageTexture;
class SkSurface;
struct SkYUVAIndex;
struct SkYUVASizeInfo;

/*
 * This class is intended to be used as:
 *   Get an SkSurfaceCharacterization representing the intended gpu-backed destination SkSurface
 *   Create one of these (an SkDDLMaker) on the stack
 *   Get the canvas and render into it
 *   Snap off and hold on to an SkDeferredDisplayList
 *   Once your app actually needs the pixels, call SkSurface::draw(SkDeferredDisplayList*)
 *
 * This class never accesses the GPU but performs all the cpu work it can. It
 * is thread-safe (i.e., one can break a scene into tiles and perform their cpu-side
 * work in parallel ahead of time).
 */
class SK_API SkDeferredDisplayListRecorder {
public:
    SkDeferredDisplayListRecorder(const SkSurfaceCharacterization&);
    ~SkDeferredDisplayListRecorder();

    const SkSurfaceCharacterization& characterization() const {
        return fCharacterization;
    }

    // The backing canvas will become invalid (and this entry point will return
    // null) once 'detach' is called.
    // Note: ownership of the SkCanvas is not transferred via this call.
    SkCanvas* getCanvas();

    std::unique_ptr<SkDeferredDisplayList> detach();

    using PromiseImageTextureContext = void*;
    using PromiseImageTextureFulfillProc =
            sk_sp<SkPromiseImageTexture> (*)(PromiseImageTextureContext);
    using PromiseImageTextureReleaseProc = void (*)(PromiseImageTextureContext);
    using PromiseImageTextureDoneProc = void (*)(PromiseImageTextureContext);

    enum class PromiseImageApiVersion { kLegacy, kNew };

    /**
        Create a new SkImage that is very similar to an SkImage created by MakeFromTexture. The
        difference is that the caller need not have created the texture nor populated it with the
        image pixel data. Moreover, the SkImage may be created on a thread as the creation of the
        image does not require access to the backend API or GrContext. Instead of passing a
        GrBackendTexture the client supplies a description of the texture consisting of
        GrBackendFormat, width, height, and GrMipMapped state. The resulting SkImage can be drawn
        to a SkDeferredDisplayListRecorder or directly to a GPU-backed SkSurface.

        When the actual texture is required to perform a backend API draw, textureFulfillProc will
        be called to receive a GrBackendTexture. The properties of the GrBackendTexture must match
        those set during the SkImage creation, and it must refer to a valid existing texture in the
        backend API context/device, and be populated with the image pixel data. The texture contents
        cannot be modified until textureReleaseProc is called. The texture cannot be deleted until
        textureDoneProc is called.

        When all the following are true:
            * the promise SkImage is deleted,
            * any SkDeferredDisplayLists that recorded draws referencing the image are deleted,
            * and all draws referencing the texture have been flushed (via GrContext::flush or
              SkSurface::flush)
        the textureReleaseProc is called. When the following additional constraint is met
           * the texture is safe to delete in the underlying API
        the textureDoneProc is called. For some APIs (e.g. GL) the two states are equivalent.
        However, for others (e.g. Vulkan) they are not as it is not legal to delete a texture until
        the GPU work referencing it has completed.

        There is at most one call to each of textureFulfillProc, textureReleaseProc, and
        textureDoneProc. textureDoneProc is always called even if image creation fails or if the
        image is never fulfilled (e.g. it is never drawn or all draws are clipped out). If
        textureFulfillProc is called then textureReleaseProc will always be called even if
        textureFulfillProc failed.

        If 'version' is set to kLegacy then the textureReleaseProc call is delayed until the
        conditions for textureDoneProc are met and then they are both called.

        This call is only valid if the SkDeferredDisplayListRecorder is backed by a GPU context.

        @param backendFormat       format of promised gpu texture
        @param width               width of promised gpu texture
        @param height              height of promised gpu texture
        @param mipMapped           mip mapped state of promised gpu texture
        @param colorSpace          range of colors; may be nullptr
        @param textureFulfillProc  function called to get actual gpu texture
        @param textureReleaseProc  function called when texture can be released
        @param textureDoneProc     function called when we will no longer call textureFulfillProc
        @param textureContext      state passed to textureFulfillProc and textureReleaseProc
        @param version             controls when textureReleaseProc is called
        @return                    created SkImage, or nullptr
     */
    sk_sp<SkImage> makePromiseTexture(
            const GrBackendFormat& backendFormat,
            int width,
            int height,
            GrMipMapped mipMapped,
            GrSurfaceOrigin origin,
            SkColorType colorType,
            SkAlphaType alphaType,
            sk_sp<SkColorSpace> colorSpace,
            PromiseImageTextureFulfillProc textureFulfillProc,
            PromiseImageTextureReleaseProc textureReleaseProc,
            PromiseImageTextureDoneProc textureDoneProc,
            PromiseImageTextureContext textureContext,
            PromiseImageApiVersion version = PromiseImageApiVersion::kLegacy);

    /**
        This entry point operates like 'makePromiseTexture' but it is used to construct a SkImage
        from YUV[A] data. The source data may be planar (i.e. spread across multiple textures). In
        the extreme Y, U, V, and A are all in different planes and thus the image is specified by
        four textures. 'yuvaIndices' specifies the mapping from texture color channels to Y, U, V,
        and possibly A components. It therefore indicates how many unique textures compose the full
        image. Separate textureFulfillProc, textureReleaseProc, and textureDoneProc calls are made
        for each texture and each texture has its own PromiseImageTextureContext. 'yuvFormats',
        'yuvaSizes', and 'textureContexts' have one entry for each of the up to four textures, as
        indicated by 'yuvaIndices'.
     */
    sk_sp<SkImage> makeYUVAPromiseTexture(
            SkYUVColorSpace yuvColorSpace,
            const GrBackendFormat yuvaFormats[],
            const SkISize yuvaSizes[],
            const SkYUVAIndex yuvaIndices[4],
            int imageWidth,
            int imageHeight,
            GrSurfaceOrigin imageOrigin,
            sk_sp<SkColorSpace> imageColorSpace,
            PromiseImageTextureFulfillProc textureFulfillProc,
            PromiseImageTextureReleaseProc textureReleaseProc,
            PromiseImageTextureDoneProc textureDoneProc,
            PromiseImageTextureContext textureContexts[],
            PromiseImageApiVersion version = PromiseImageApiVersion::kLegacy);

private:
    bool init();

    const SkSurfaceCharacterization             fCharacterization;

#if SK_SUPPORT_GPU
    sk_sp<GrContext>                            fContext;
    sk_sp<GrRenderTargetProxy>                  fTargetProxy;
    sk_sp<SkDeferredDisplayList::LazyProxyData> fLazyProxyData;
    sk_sp<SkSurface>                            fSurface;
#endif
};

#endif
