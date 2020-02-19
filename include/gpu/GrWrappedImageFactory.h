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

class GrBackendTexture;
class GrContext;
class SkImage;
class SkPixmap;

enum class YUVAPlanarType {
    kNV12, // 8-bit Y plane + 2x2 down sampled interleaved U/V planes (2 textures)
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
                                                       SkColorType colorType,
                                                       SkAlphaType alphaType,
                                                       sk_sp<SkColorSpace> colorSpace,
                                                       TextureReleaseProc,
                                                       ReleaseContext);

    // Same but for creating YUVA planar images.
    static std::unique_ptr<GrWrappedImageFactory> MakeYUVA(GrContext*,
                                                           YUVAPlanarType
                                                           GrBackendTexture[],
                                                           TextureReleaseProc,
                                                           ReleaseContext);

    // Can a new cycle of writePixels/makeImageSnapshot begin? This will return true if there
    // are no outstanding SkImages accessing the backend texture. Clients should usually
    // be aware of the status of their images but in corner cases like DDLs and picture
    // recording canvases the created SkImages could be long lived.
    bool canWritePixels() const;

    // This call will fail if there are any outstanding SkImages accessing the backend textures
    // (i.e., canWritePixels would’ve returned false).
    bool writePixels(int plane, SkColorType, sk_sp<SkData> levelData, int numLevels);

    // Like above but updates a subrect of the base level
    bool writePixels(int plane, SkIPoint point, const SkPixmap& data);

    // Create an SkImage wrapping the backend texture which, when drawn, will contain the data
    // from the last call to writePixels. Returns the same image on successive calls if no successful
    // writePixels has occurred.
    sk_sp<SkImage> makeImageSnapshot();

private:
    GrWrappedImageFactory(GrContext* context,
                          const GrBackendTexture& beTex,
                          SkColorType colorType,
                          SkAlphaType alphaType,
                          sk_sp<SkColorSpace> colorSpace,
                          TextureReleaseProc releaseProc,
                          ReleaseContext releaseContext);

    // For now, we cheat and create a surface while we're working on the implementation
    sk_sp<SkSurface> fSurface;
};

#endif
