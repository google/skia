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
class GrRecordingContext;
class GrYUVABackendTextureInfo;
class SkCanvas;
class SkImage;
class SkPromiseImageTexture;
class SkSurface;
struct SkYUVAIndex;

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

    sk_sp<SkDeferredDisplayList> detach();

    using PromiseImageTextureContext = void*;
    using PromiseImageTextureFulfillProc =
            sk_sp<SkPromiseImageTexture> (*)(PromiseImageTextureContext);
    using PromiseImageTextureReleaseProc = void (*)(PromiseImageTextureContext);

    /**
        Create a new SkImage that is very similar to an SkImage created by MakeFromTexture. The
        difference is that the caller need not have created the texture nor populated it with the
        image pixel data. Moreover, the SkImage may be created on a thread as the creation of the
        image does not require access to the backend API or GrContext. Instead of passing a
        GrBackendTexture the client supplies a description of the texture consisting of
        GrBackendFormat, width, height, and GrMipmapped state. The resulting SkImage can be drawn
        to a SkDeferredDisplayListRecorder or directly to a GPU-backed SkSurface.

        When the actual texture is required to perform a backend API draw, textureFulfillProc will
        be called to receive a GrBackendTexture. The properties of the GrBackendTexture must match
        those set during the SkImage creation, and it must refer to a valid existing texture in the
        backend API context/device, and be populated with the image pixel data. The texture cannot
        be deleted until textureReleaseProc is called.

        There is at most one call to each of textureFulfillProc and textureReleaseProc.
        textureReleaseProc is always called even if image creation fails or if the
        image is never fulfilled (e.g. it is never drawn or all draws are clipped out)

        This call is only valid if the SkDeferredDisplayListRecorder is backed by a GPU context.

        @param backendFormat       format of promised gpu texture
        @param width               width of promised gpu texture
        @param height              height of promised gpu texture
        @param mipMapped           mip mapped state of promised gpu texture
        @param colorSpace          range of colors; may be nullptr
        @param textureFulfillProc  function called to get actual gpu texture
        @param textureReleaseProc  function called when texture can be deleted
        @param textureContext      state passed to textureFulfillProc and textureReleaseProc
        @param version             controls when textureReleaseProc is called
        @return                    created SkImage, or nullptr
     */
    sk_sp<SkImage> makePromiseTexture(const GrBackendFormat& backendFormat,
                                      int width,
                                      int height,
                                      GrMipmapped mipMapped,
                                      GrSurfaceOrigin origin,
                                      SkColorType colorType,
                                      SkAlphaType alphaType,
                                      sk_sp<SkColorSpace> colorSpace,
                                      PromiseImageTextureFulfillProc textureFulfillProc,
                                      PromiseImageTextureReleaseProc textureReleaseProc,
                                      PromiseImageTextureContext textureContext);

    /**
        This entry point operates like 'makePromiseTexture' but it is used to construct a SkImage
        from YUV[A] data. The source data may be planar (i.e. spread across multiple textures). In
        the extreme Y, U, V, and A are all in different planes and thus the image is specified by
        four textures. 'yuvaBackendTextureInfo' describes the planar arrangement, texture formats,
        conversion to RGB, and origin of the textures. Separate 'textureFulfillProc' and
        'textureReleaseProc' calls are made for each texture. Each texture has its own
        PromiseImageTextureContext. If 'yuvaBackendTextureinfo' is not valid then no release proc
        calls are made. Otherwise, the calls will be made even on failure. 'textureContexts' has one
        entry for each of the up to four textures, as indicated by 'yuvaBackendTextureinfo'.

        Currently the mip mapped property of 'yuvaBackendTextureInfo' is ignored. However, in the
        near future it will be required that if it is kYes then textureFulfillProc must return
        a mip mapped texture for each plane in order to successfully draw the image.
     */
    sk_sp<SkImage> makeYUVAPromiseTexture(const GrYUVABackendTextureInfo& yuvaBackendTextureInfo,
                                          sk_sp<SkColorSpace> imageColorSpace,
                                          PromiseImageTextureFulfillProc textureFulfillProc,
                                          PromiseImageTextureReleaseProc textureReleaseProc,
                                          PromiseImageTextureContext textureContexts[]);

private:
    bool init();

    const SkSurfaceCharacterization             fCharacterization;

#if SK_SUPPORT_GPU
    sk_sp<GrRecordingContext>                   fContext;
    sk_sp<GrRenderTargetProxy>                  fTargetProxy;
    sk_sp<SkDeferredDisplayList::LazyProxyData> fLazyProxyData;
    sk_sp<SkSurface>                            fSurface;
#endif
};

#endif
