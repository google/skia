/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/SkSharingProc.h"

#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkSerialProcs.h"

sk_sp<SkData> SkSharingSerialContext::serializeImage(SkImage* img, void* ctx) {
    SkSharingSerialContext* context = reinterpret_cast<SkSharingSerialContext*>(ctx);
    uint32_t id = img->uniqueID(); // get this process's id for the image. these are not hashes.
    // find out if we have already serialized this, and if so, what its in-file id is.
    auto iter = context->fImageMap.find(id);
    if (iter == context->fImageMap.end()) {
        // When not present, add its id to the map and return its usual serialized form.
        context->fImageMap[id] = context->fImageMap.size();
        return img->encodeToData();
    }
    uint32_t fid = context->fImageMap[id];
    // if present, return only the in-file id we registered the first time we serialized it.
    return SkData::MakeWithCopy(&fid, sizeof(fid));
}

sk_sp<SkImage> SkSharingDeserialContext::deserializeImage(
  const void* data, size_t length, void* ctx) {
    SkSharingDeserialContext* context = reinterpret_cast<SkSharingDeserialContext*>(ctx);
    uint32_t fid;
    // If the data is an image fid, look up an already deserialized image from our map
    if (length == sizeof(fid)) {
        memcpy(&fid, data, sizeof(fid));
        if (fid >= context->fImages.size()) {
            SkDebugf("We do not have the data for image %d.\n", fid);
            return nullptr;
        }
        return context->fImages[fid];
    }
    // Otherwise, the data is an image, deserialise it, store it in our map at its fid.
    // TODO(nifong): make DeserialProcs accept sk_sp<SkData> so we don't have to copy this.
    sk_sp<SkData> dataView = SkData::MakeWithCopy(data, length);
    const sk_sp<SkImage> image = SkImage::MakeFromEncoded(std::move(dataView));
    context->fImages.push_back(image);
    return image;
}
