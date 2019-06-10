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

#include "SkImage.h"
#include "SkSerialProcs.h"

namespace sharingproc {

struct SharingSerialContext {
    // A map from the ids from SkImage::uniqueID() to ids used within the file
    std::unordered_map<uint32_t, uint32_t> imageMap;
    std::unordered_map<uint32_t, uint32_t> typefaceMap;
};
struct SharingDeserialContext {
    // a list of unique images in the order they were encountered in the file
    // Subsequent occurences of an image refer to it by it's index in this list.
    std::vector<sk_sp<SkImage>> images;
    std::vector<sk_sp<SkTypeface>> typefaces;
};
// A serial proc that shares images and typefaces between subpictures
SkSerialProcs SharingSerializationProcs(SharingSerialContext* ctx);
SkDeserialProcs SharingDeserializationProcs(SharingDeserialContext* ctx);

} // namespace sharingproc
#endif
