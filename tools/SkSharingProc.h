/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSharingProc_DEFINED
#define SkSharingProc_DEFINED

#include <unordered_map>
#include <vector>

#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkSerialProcs.h"
#include "src/core/SkTHash.h"

/**
 * This serial proc serializes each image it encounters only once, using their uniqueId as the
 * property for sameness.
 *
 * It's most basic usage involves setting your imageProc to SkSharingSerialContext::serializeImage
 * and creating an SkSharingSerialContext in an appropriate scope to outlive all the images that
 * will be encountered before serialization.
 *
 * Optionally, collectNonTextureImagesFromPicture can be called with an SkSharingContext and an
 * SkPicture that may reference not-yet-released texture backed images. It will make non-texture
 * copies if necessary and store them in the context. If present, they will be used in the
 * final serialization.
 *
 * This is intended to be used on Android with MultiPictureDocument's onEndPage parameter, in a
 * lambda that captures the context, because MPD cannot make assumptions about the type of proc it
 * receives and clients (Chrome) build MPD without this source file.
 */

struct SkSharingSerialContext {
    // --- Data and and function for optional texture collection pass --- //

    // A map from uniqueID of images referenced by commands to non-texture images
    // collected at the end of each frame.
    skia_private::THashMap<uint32_t, sk_sp<SkImage>> fNonTexMap;

    // Collects any non-texture images referenced by the picture and stores non-texture copies
    // in the fNonTexMap of the provided SkSharingContext
    static void collectNonTextureImagesFromPicture(
        const SkPicture* pic, SkSharingSerialContext* sharingCtx);


    // --- Data and serialization function for regular use --- //

    // A map from the ids from SkImage::uniqueID() to ids used within the file
    // The keys are ids of original images, not of non-texture copies
    skia_private::THashMap<uint32_t, int> fImageMap;

    // A serial proc that shares images between subpictures
    // To use this, create an instance of SkSerialProcs and populate it this way.
    // The client must retain ownership of the context.
    // auto ctx = std::make_unique<SkSharingSerialContext>()
    // SkSerialProcs procs;
    // procs.fImageProc = SkSharingSerialContext::serializeImage;
    // procs.fImageCtx = ctx.get();
    static sk_sp<SkData> serializeImage(SkImage* img, void* ctx);
};

struct SkSharingDeserialContext {
    // a list of unique images in the order they were encountered in the file
    // Subsequent occurrences of an image refer to it by it's index in this list.
    std::vector<sk_sp<SkImage>> fImages;

    // A deserial proc that can interpret id's in place of images as references to previous images.
    // Can also deserialize a SKP where all images are inlined (it's backwards compatible)
    static sk_sp<SkImage> deserializeImage(const void* data, size_t length, void* ctx);
};

#endif
