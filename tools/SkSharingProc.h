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

struct SkSharingSerialContext {
    // A map from the ids from SkImage::uniqueID() to ids used within the file
    std::unordered_map<uint32_t, uint32_t> fImageMap;

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
