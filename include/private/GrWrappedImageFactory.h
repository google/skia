/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrWrappedImageFactory_DEFINED
#define GrWrappedImageFactory_DEFINED

#include <memory>
#include "include/core/SkImageInfo.h"
#include "include/gpu/GrTypes.h"

class GrBackendTexture;
class GrContext;
class SkImage;
class SkPixmap;
class SkSurface;

// This enum completely locks down the number of planes, the bit-depth of each
// plane and their relative sizes for each supported format.
enum class YUVAPlanarType {
    kInvalid = 0,
    // NV12 consists of two textures:
    //    0: a single channel 8-bit Y plane whose size determines that of the final image
    //    1: a 2-channel UV plane which is 2x2 down-sampled relative to the Y-plane
    kNV12,
};

/*
 * A factory object for creating SkImages that wrap a particular backend texture.
 *
 * Example use case:
 *    create a wrappedImageFactory (WIF) and hold on to it
 *    call writePixels & snap a new image (SkImage1)
 *    call drawImage and drop the client ref on SkImage1
 *    check that canWritePixels returns true
 *    call writePixels again & snap another image (SkImage2)
 *    call drawImage and drop the client ref on SkImage2
 *    flush
 *    delete WIF
 *    after receiving the textureReleaseProc, return the backendTexture to the external pool
 */
class GrWrappedImageFactory {
public:
    ~GrWrappedImageFactory();

    typedef void* ReleaseContext;
    typedef void (*TextureReleaseProc)(ReleaseContext);

    // Create a wrapped image factory that will produce images that wrap the provided
    // backend texture. Call writePixels to update the content of the backend texture.
    // The textureReleaseProc will be called when all references (including the returned
    // GrWrappedImageFactory’s reference) to the backend texture are dropped.
    static std::unique_ptr<GrWrappedImageFactory> Make(GrContext*,
                                                       const GrBackendTexture&,
                                                       GrSurfaceOrigin origin,
                                                       SkColorType colorType,
                                                       SkAlphaType alphaType,
                                                       sk_sp<SkColorSpace> colorSpace,
                                                       TextureReleaseProc,
                                                       ReleaseContext);

    // Same but for creating YUVA planar images.
    static std::unique_ptr<GrWrappedImageFactory> MakeYUVA(GrContext*,
                                                           YUVAPlanarType,
                                                           SkYUVColorSpace,
                                                           const GrBackendTexture[],
                                                           GrSurfaceOrigin origin,
                                                           sk_sp<SkColorSpace> colorSpace,
                                                           TextureReleaseProc,
                                                           ReleaseContext);

    // Can a new cycle of writePixels/makeImageSnapshot begin? This will return true if there
    // are no outstanding SkImages accessing the backend texture. Clients should usually
    // be aware of the status of their images but in corner cases like DDLs and picture
    // recording canvases the created SkImages could be long lived.
    bool canWritePixels() const;

    // This call will queue a task to perform the writePixels. Skia will hold a ref on ‘pixelData’
    // as long as it is needed. ‘pixelData’ must contain the raw pixel data which matches
    // the implicit image info of the backend format(s).
    // This call will fail if there are any outstanding SkImages accessing the backend textures
    // (i.e., canWritePixels would’ve returned false).
    //     @param plane        0 for non-YUV images, the plane to modify for YUV images
    //     @param pixelData  the pixel data to write.
    //                                     Note: no conversion is performed on ‘pixelData’. It is
    //                                     uploaded as is to the backend texture.
    //     @return                   true if successful; false otherwise
    // Note: in the future we can add support for uploading miplevels.
    bool writePixels(int plane, sk_sp<SkData> pixelData);

    // Create an SkImage wrapping the backend texture which, when drawn, will contain the data
    // from the last call to writePixels. Returns the same image on successive calls if no
    // successful writePixels has occurred.
    sk_sp<SkImage> makeImageSnapshot();

private:
    GrWrappedImageFactory(GrContext*,
                          const GrBackendTexture&,
                          GrSurfaceOrigin,
                          const SkImageInfo& ii,
                          TextureReleaseProc,
                          ReleaseContext);

    GrWrappedImageFactory(GrContext* context,
                          YUVAPlanarType planarType,
                          SkYUVColorSpace yuvColorSpace,
                          const GrBackendTexture beTextures,
                          GrSurfaceOrigin origin,
                          const SkImageInfo& ii,
                          TextureReleaseProc releaseProc,
                          ReleaseContext releaseContext);

    SkImageInfo                fII;            // the image info of the resulting image(s)

    YUVAPlanarType             fPlanarType;
    SkYUVColorSpace            fYUVColorSpace;

    // For now, we cheat and create surfaces while we're working on the implementation
    SkTArray<sk_sp<SkSurface>> fSurfaces;
};

#endif
