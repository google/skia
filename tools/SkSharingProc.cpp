/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/SkSharingProc.h"

#include "include/codec/SkCodec.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkPicture.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include "include/private/base/SkLog.h"

static SkSerialReturnType collect_nontexture_images_proc(SkImage* img, void* ctx) {
    SkASSERT(img);
    SkSharingSerialContext* context = reinterpret_cast<SkSharingSerialContext*>(ctx);
    uint32_t originalId = img->uniqueID();
    sk_sp<SkImage>* imageInMap = context->fNonTexMap.find(originalId);
    if (!imageInMap) {
        context->fNonTexMap[originalId] = img->makeRasterImage(context->fDirectContext);
    }

    // This function implements a proc that is more generally for serialization, but
    // we really only want to build our map. The output of this function is ignored.
    return SkData::MakeEmpty();
}

void SkSharingSerialContext::collectNonTextureImagesFromPicture(
    const SkPicture* pic, SkSharingSerialContext* sharingCtx) {
    SkSerialProcs tempProc;
    tempProc.fImageCtx = sharingCtx;
    tempProc.fImageProc = collect_nontexture_images_proc;
    SkNullWStream ns;
    pic->serialize(&ns, &tempProc);
}

void SkSharingSerialContext::setDirectContext(GrDirectContext* ctx) {
    fDirectContext = ctx;
}

namespace SkSharingContext {

SkSerialReturnType serializeImage(SkImage* img, void* ctx) {
    SkSharingSerialContext* context = reinterpret_cast<SkSharingSerialContext*>(ctx);
    uint32_t id = img->uniqueID(); // get this process's id for the image. these are not hashes.
    // find out if we have already serialized this, and if so, what its in-file id is.
    int* fid = context->fImageMap.find(id);
    if (!fid) {
        // encode the image or it's non-texture replacement if one was collected
        sk_sp<SkImage>* replacementImage = context->fNonTexMap.find(id);
        if (replacementImage) {
            img = replacementImage->get();
        }
        auto data = SkPngEncoder::Encode(context->fDirectContext, img, {});
        if (!data) {
            // If encoding fails, we must return something. If we return null then SkWriteBuffer's
            // serialize_image which calls this proc will continue to try writing to the mskp file.
            SkBitmap bm;
            bm.allocPixels(SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
            SkCanvas canvas = SkCanvas(bm);
            canvas.clear(SK_ColorMAGENTA);
            data = SkPngEncoder::Encode(context->fDirectContext, bm.asImage().get(), {});
        }
        context->fImageMap[id] = context->fImageMap.count(); // Next in-file id
        return data;
    }
    // if present, return only the in-file id we registered the first time we serialized it.
    return SkData::MakeWithCopy(fid, sizeof(*fid));
}

sk_sp<SkImage> deserializeImage(sk_sp<SkData> data,
                                std::optional<SkAlphaType> alphaType,
                                void* ctx) {
    if (!data || data->empty() || !ctx) {
        SKIA_LOG_W("SkSharingContext::deserializeImage arguments invalid %p.\n", ctx);
        // Return something so the rest of the debugger can proceed.
        SkBitmap bm;
        bm.allocPixels(SkImageInfo::MakeN32Premul(1, 1));
        return bm.asImage();
    }
    SkSharingDeserialContext* context = reinterpret_cast<SkSharingDeserialContext*>(ctx);
    uint32_t fid = std::numeric_limits<uint32_t>::max();
    // If the data is an image fid, look up an already deserialized image from our map
    if (data->size() == sizeof(fid)) {
        data->copyRange(0, sizeof(fid), &fid);
        if (fid >= context->fImages.size()) {
            SKIA_LOG_E("Cannot deserialize using id, We do not have the data for image %u.\n", fid);
            return nullptr;
        }
        return context->fImages[fid];
    }
    // Otherwise, the data is an image, deserialise it, store it in our map at its fid.
    auto codec = SkPngDecoder::Decode(std::move(data), nullptr);
    if (!codec) {
        SKIA_LOG_E("Cannot deserialize image - might not be a PNG.\n");
        return nullptr;
    }
    auto image = SkCodecs::DeferredImage(std::move(codec), alphaType);
    if (!image) {
        SKIA_LOG_E("Cannot initialize image.\n");
        return nullptr;
    }
    image = image->makeRasterImage(nullptr);
    SkASSERT(image);
    context->fImages.push_back(image);
    return image;
}

}  // namespace SkSharingContext
