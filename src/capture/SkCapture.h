/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCapture_DEFINED
#define SkCapture_DEFINED

#include "include/core/SkAlphaType.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"

#include <optional>

class SkData;
class SkImage;
class SkPicture;

class SkCanvas;
class SkCaptureCanvas;

// TODO(b/412351769): Make this class public.
class SkCapture : public SkRefCnt {
public:
    struct Metadata {
        uint32_t version;
        uint32_t numPictures;
    };

    static sk_sp<SkCapture> MakeFromData(sk_sp<SkData>);
    // TODO: instead of a make from pictures factory, the CaptureManager might just need hooks into
    // the to build it over time. Move the SkPictures (fPictures) here and just maintain that in one
    // place.
    static sk_sp<SkCapture> MakeFromPictures(skia_private::TArray<sk_sp<SkPicture>>);
    sk_sp<SkData> serializeCapture();

    // TODO: Pictures being grabbed by index is not intuitive and leave the capture disorganized.
    // This should be deleted once SkPictures are organized by Surface and grouped by Recording.
    sk_sp<SkPicture> getPicture(int i) const;
    Metadata getMetadata() const;

private:
    // TODO: add more awareness of the image meta data to a SkCaptureContext object
    static sk_sp<SkData> serializeImageProc(SkImage* img, void* ctx);
    static sk_sp<SkImage> deserializeImageProc(sk_sp<SkData>,
                                               std::optional<SkAlphaType>, void* ctx);

    Metadata fMetadata;
    //TODO(b/412351769): Replace pictures with SkCapturePicture structs that also include
    // picture metadata
    skia_private::TArray<sk_sp<SkPicture>> fPictures;

    static const uint32_t kVersion = 0; // Until this version is 1 or greater, active development
                                        // will make this unstable.
};

#endif //SkCapture_DEFINED
