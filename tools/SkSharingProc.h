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

#include "include/core/SkImage.h"
#include "include/core/SkSerialProcs.h"

struct SkSharingSerialContext {
    // A map from the ids from SkImage::uniqueID() to ids used within the file
    std::unordered_map<uint32_t, uint32_t> imageMap;
};
struct SkSharingDeserialContext {
    // a list of unique images in the order they were encountered in the file
    // Subsequent occurrences of an image refer to it by it's index in this list.
    std::vector<sk_sp<SkImage>> images;
};
// A serial proc that shares images between subpictures
SkSerialProcs SkSharingSerializationProcs(SkSharingSerialContext* ctx);
// A deserial proc that can interpret id's in place of images as references to previous images.
// Can also deserialize a SKP where all images are inlined (it's backwards compatible)
SkDeserialProcs SkSharingDeserializationProcs(SkSharingDeserialContext* ctx);

#endif
