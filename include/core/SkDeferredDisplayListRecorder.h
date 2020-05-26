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
 *   Create one of these (an SkDeferredDisplayListRecorder) on the stack
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

    // TODO: Remove this API after migrating users over to the GrContext version. Currently these
    // types are defined here in addition to GrContext.h to support building this old API when
    // SK_SUPPORT_GPU=0.
    using PromiseImageTextureContext = void*;
    using PromiseImageTextureFulfillProc =
            sk_sp<SkPromiseImageTexture> (*)(PromiseImageTextureContext);
    using PromiseImageTextureReleaseProc = void (*)(PromiseImageTextureContext);
    using PromiseImageTextureDoneProc = void (*)(PromiseImageTextureContext);

    enum class PromiseImageApiVersion { kLegacy, kNew };

    /** Old name for `GrContext::makePromiseTexture` */
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

    /** Old name for `GrContext::makeYUVAPromiseTexture` */
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
    sk_sp<SkDeferredDisplayList::LazyProxyData> fLazyProxyData;
    sk_sp<SkSurface>                            fSurface;
#endif
};

#endif
