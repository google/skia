/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Image_DEFINED
#define skgpu_graphite_Image_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"

struct SkIRect;

namespace skgpu::graphite {
    class Recorder;
}

namespace SkImages {
/** Returns subset of this image as a texture-backed image.

    Returns nullptr if any of the following are true:
      - Subset is empty
      - Subset is not contained inside the image's bounds
      - Pixels in the source image could not be read or copied
      - The source image is texture-backed and context does not match the source image's context.

    @param recorder the non-null recorder in which to create the new image.
    @param img     Source image
    @param subset  bounds of returned SkImage
    @param props   properties the returned SkImage must possess (e.g. mipmaps)
    @return        the subsetted image, uploaded as a texture, or nullptr
*/
SK_API sk_sp<SkImage> SubsetTextureFrom(skgpu::graphite::Recorder* recorder,
                                        const SkImage* img,
                                        const SkIRect& subset,
                                        SkImage::RequiredImageProperties props = {});
} // namespace SkImages


#endif // skgpu_graphite_Image_DEFINED
