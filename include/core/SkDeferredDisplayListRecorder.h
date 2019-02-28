/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeferredDisplayListRecorder_DEFINED
#define SkDeferredDisplayListRecorder_DEFINED

#include "../private/SkDeferredDisplayList.h"
#include "SkImageInfo.h"
#include "SkRefCnt.h"
#include "SkSurfaceCharacterization.h"
#include "SkTypes.h"

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

    // Deprecated alias. To be removed.
    using TextureContext = PromiseImageTextureContext;

    // Legacy enum. To be removed.
    enum class DelayReleaseCallback { kYes = true };

    /**
        Create a new SkImage that is very similar to an SkImage created by MakeFromTexture. The main
        difference is that the client doesn't have the backend texture on the gpu yet but they know
        all the properties of the texture. So instead of passing in a GrBackendTexture the client
        supplies a GrBackendFormat, width, height, and GrMipMapped state.

        When we actually send the draw calls to the GPU, we will call the textureFulfillProc and
        the client will return a GrBackendTexture to us. The properties of the GrBackendTexture must
        match those set during the SkImage creation, and it must have a valid backend gpu texture.
        The gpu texture supplied by the client must stay valid until we call the textureReleaseProc.

        When all the following are true:
            * the promise image is deleted,
            * any SkDeferredDisplayLists that recorded draws referencing the image are deleted,
            * and the texture is safe to delete in the underlying API with respect to drawn
              SkDeferredDisplayLists that reference the image
        the textureReleaseProc and then textureDoneProc are called. The texture can be deleted
        by the client as soon as textureReleaseProc is called. There is at most one call to each of
        textureFulfillProc, textureReleaseProc, and textureDoneProc. textureDoneProc is always
        called even if image creation fails or if the image is never fulfilled (e.g. it is never
        drawn). If textureFulfillProc is called then textureReleaseProc will always be called even
        if textureFulfillProc fails.


        This call is only valid if the SkDeferredDisplayListRecorder is backed by a gpu context.

        @param backendFormat       format of promised gpu texture
        @param width               width of promised gpu texture
        @param height              height of promised gpu texture
        @param mipMapped           mip mapped state of promised gpu texture
        @param origin              one of: kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin
        @param colorType           one of: kUnknown_SkColorType, kAlpha_8_SkColorType,
                                   kRGB_565_SkColorType, kARGB_4444_SkColorType,
                                   kRGBA_8888_SkColorType, kBGRA_8888_SkColorType,
                                   kGray_8_SkColorType, kRGBA_F16_SkColorType
        @param alphaType           one of: kUnknown_SkAlphaType, kOpaque_SkAlphaType,
                                   kPremul_SkAlphaType, kUnpremul_SkAlphaType
        @param colorSpace          range of colors; may be nullptr
        @param textureFulfillProc  function called to get actual gpu texture
        @param textureReleaseProc  function called when texture can be released
        @param promiseDoneProc     function called when we will no longer call textureFulfillProc
        @param textureContext      state passed to textureFulfillProc and textureReleaseProc
        @return                    created SkImage, or nullptr
     */
    sk_sp<SkImage> makePromiseTexture(const GrBackendFormat& backendFormat,
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
                                      PromiseImageTextureContext textureContext);
    // Legacy API bridge version. To be removed.
    sk_sp<SkImage> makePromiseTexture(const GrBackendFormat& backendFormat,
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
                                      DelayReleaseCallback) {
        return this->makePromiseTexture(backendFormat,
                                        width,
                                        height,
                                        mipMapped,
                                        origin,
                                        colorType,
                                        alphaType,
                                        colorSpace,
                                        textureFulfillProc,
                                        textureReleaseProc,
                                        textureDoneProc,
                                        textureContext);
    }

    /**
        This entry point operates the same as 'makePromiseTexture' except that its
        textureFulfillProc can be called up to four times to fetch the required YUVA
        planes (passing a different textureContext to each call). So, if the 'yuvaIndices'
        indicate that only the first two backend textures are used, 'textureFulfillProc' will
        be called with the first two 'textureContexts'.
     */
    sk_sp<SkImage> makeYUVAPromiseTexture(SkYUVColorSpace yuvColorSpace,
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
                                          PromiseImageTextureContext textureContexts[]);
    // Legacy API bridge version. To be removed.
    sk_sp<SkImage> makeYUVAPromiseTexture(SkYUVColorSpace yuvColorSpace,
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
                                          DelayReleaseCallback) {
        return this->makeYUVAPromiseTexture(yuvColorSpace,
                                            yuvaFormats,
                                            yuvaSizes,
                                            yuvaIndices,
                                            imageWidth,
                                            imageHeight,
                                            imageOrigin,
                                            imageColorSpace,
                                            textureFulfillProc,
                                            textureReleaseProc,
                                            textureDoneProc,
                                            textureContexts);
    }

private:
    bool init();

    const SkSurfaceCharacterization             fCharacterization;

#if SK_SUPPORT_GPU
    sk_sp<GrContext>                            fContext;
    sk_sp<SkDeferredDisplayList::LazyProxyData> fLazyProxyData;
    sk_sp<SkSurface>                            fSurface;
#endif
};

#endif
