/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSharingProc.h"

#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkSerialProcs.h"

namespace  {

static sk_sp<SkData> serialize_image(SkImage* img, void* ctx) {
    SharingSerialContext* context = reinterpret_cast<SharingSerialContext*>(ctx);
    uint32_t id = img->uniqueID(); // get this process's id for the image. these are not hashes.
    // find out if we have already serialized this, and if so, what its in-file id is.
    auto iter = context->imageMap.find(id);
    if (iter == context->imageMap.end()) {
        // When not present, add its id to the map and return its usual serialized form.
        context->imageMap[id] = context->imageMap.size();
        return img->encodeToData();
    }
    uint32_t fid = context->imageMap[id];
    // if present, return only the in-file id we registered the first time we serialized it.
    return SkData::MakeWithCopy(&fid, sizeof(fid));
}

static sk_sp<SkImage> deserialize_image(const void* data, size_t length, void* ctx) {
    SharingDeserialContext* context = reinterpret_cast<SharingDeserialContext*>(ctx);
    uint32_t fid;
    // If the data is an image fid, look up an already deserialized image from our map
    if (length == sizeof(fid)) {
        memcpy(&fid, data, sizeof(fid));
        if (fid < 0 || fid >= context->images.size()) {
            SkDebugf("We do not have the data for image %d.\n", fid);
            return nullptr;
        }
        return context->images[fid];
    }
    // Otherwise, the data is an image, deserialise it, store it in our map at its fid.
    // TODO(nifong): make DeserialProcs accept sk_sp<SkData> so we don't have to copy this.
    sk_sp<SkData> dataView = SkData::MakeWithCopy(data, length);
    const sk_sp<SkImage> image = SkImage::MakeFromEncoded(std::move(dataView));
    context->images.push_back(image);
    return image;
}

} // namespace

SkSerialProcs SkSharingSerializationProcs(SkSharingSerialContext* ctx) {
  SkSerialProcs procs;
  procs.fImageProc = serialize_image;
  procs.fImageCtx = ctx;
  return procs;
}

SkDeserialProcs SkSharingDeserializationProcs(SkSharingDeserialContext* ctx) {
  SkDeserialProcs procs;
  procs.fImageProc = deserialize_image;
  procs.fImageCtx = ctx;
  return procs;
}

