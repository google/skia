/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSharingProc.h"

#include "SkData.h"
#include "SkImage.h"
#include "SkSerialProcs.h"

namespace sharingproc {
namespace  {

sk_sp<SkData> SerializeImage(SkImage* img, void* ctx) {
    SharingSerialContext* context = reinterpret_cast<SharingSerialContext*>(ctx);
    uint32_t id = img->uniqueID(); // get an id for the image
    // find it in the set of already serialized images
    auto iter = context->imageMap.find(id);
    if (iter == context->imageMap.end()) {
        // When not present, add it's id to the map and return it's usual serialized form.
        context->imageMap[id] = true;
        return nullptr; // nullptr cases skia's default serialization behavior to be used.
    }
    // if present, return only it's id.
    return SkData::MakeWithCopy(&id, sizeof(id));
}

sk_sp<SkImage> DeserializeImage(const void* data, size_t length, void* ctx) {
    SharingDeserialContext* context = reinterpret_cast<SharingDeserialContext*>(ctx);
    uint32_t id;
    // If the data is an image id, look up an already deserialized image from our map
    if (length == sizeof(id)) {
        memcpy(&id, data, sizeof(id));
        auto iter = context->imageMap.find(id);
        if (iter == context->imageMap.end() || !iter->second) {
            // We do not have the data from a previous subpicture. this is a corrupted file
            return nullptr;
        }
        return iter->second;
    }
    // otherwise, the data is an image, deserialise it, store it in our map at it's id.
    // TODO(nifong): make DeserialProcs accept sk_sp<SkData> so we don't have to copy this.
    sk_sp<SkData> dataView = SkData::MakeWithCopy(data, length);
    sk_sp<SkImage> image = SkImage::MakeFromEncoded(std::move(dataView));
    context->imageMap[id] = image;
    return image;
}

} // namespace

SkSerialProcs SharingSerializationProcs(SharingSerialContext* ctx) {
  SkSerialProcs procs;
  procs.fImageProc = SerializeImage;
  procs.fImageCtx = ctx;
  return procs;
}

SkDeserialProcs SharingDeserializationProcs(SharingDeserialContext* ctx) {
  SkDeserialProcs procs;
  procs.fImageProc = DeserializeImage;
  procs.fImageCtx = ctx;
  return procs;
}

} // namespace sharingproc
