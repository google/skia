/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/SKPSlide.h"

#include "include/codec/SkCodec.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPicture.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTo.h"

#if defined(SK_CODEC_DECODES_PNG_WITH_RUST)
#include "include/codec/SkPngRustDecoder.h"
#else
#include "include/codec/SkPngDecoder.h"
#endif

#include <memory>
#include <utility>

SKPSlide::SKPSlide(const SkString& name, const SkString& path)
        : SKPSlide(name, SkStream::MakeFromFile(path.c_str())) {
}

SKPSlide::SKPSlide(const SkString& name, std::unique_ptr<SkStream> stream)
        : fStream(std::move(stream)) {
    fName = name;
}

SKPSlide::~SKPSlide() {}

void SKPSlide::draw(SkCanvas* canvas) {
    if (fPic) {
        bool isOffset = SkToBool(fCullRect.left() | fCullRect.top());
        if (isOffset) {
            canvas->save();
            canvas->translate(SkIntToScalar(-fCullRect.left()), SkIntToScalar(-fCullRect.top()));
        }

        canvas->drawPicture(fPic.get());

        if (isOffset) {
            canvas->restore();
        }
    }
}

void SKPSlide::load(SkScalar, SkScalar) {
    if (!fStream) {
        SkDebugf("No skp stream for slide %s.\n", fName.c_str());
        return;
    }
    fStream->rewind();

    SkDeserialProcs procs;
    procs.fImageDataProc =
            [](sk_sp<SkData> data, std::optional<SkAlphaType>, void*) -> sk_sp<SkImage> {
#if defined(SK_CODEC_DECODES_PNG_WITH_RUST)
        std::unique_ptr<SkStream> stream = SkMemoryStream::Make(data);
        auto codec = SkPngRustDecoder::Decode(std::move(stream), nullptr, nullptr);
#else
        auto codec = SkPngDecoder::Decode(data, nullptr, nullptr);
#endif
        if (!codec) {
            SkDebugf("Invalid png data detected\n");
            return nullptr;
        }
        return std::get<0>(codec->getImage());
    };
    fPic = SkPicture::MakeFromStream(fStream.get(), &procs);
    if (!fPic) {
        SkDebugf("Could not parse SkPicture from skp stream for slide %s.\n", fName.c_str());
        return;
    }
    fCullRect = fPic->cullRect().roundOut();
}

void SKPSlide::unload() {
    fPic.reset(nullptr);
}
