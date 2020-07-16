/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "experimental/svg/model/SkSVGImage.h"

#include "experimental/svg/model/SkSVGRenderContext.h"
#include "experimental/svg/model/SkSVGValue.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkTypes.h"

#include "include/utils/SkBase64.h"

sk_sp<SkImage> ImageFromBase64Encoded(const char* str) {
    static constexpr char kDataURIImagePrefix[] = "image/",
                          kDataURIEncodingStr[] = ";base64,";

    if (!strncmp(str, kDataURIImagePrefix, SK_ARRAY_COUNT(kDataURIImagePrefix) - 1)) {
        const char* encoding_start =
                strstr(str + SK_ARRAY_COUNT(kDataURIImagePrefix) - 1, kDataURIEncodingStr);
        if (encoding_start) {
            const char* data_start = encoding_start + SK_ARRAY_COUNT(kDataURIEncodingStr) - 1;

            SkBase64 b64;
            auto result = b64.decode(data_start, strlen(data_start));
            if (result == SkBase64::kNoError) {
                // TODO: the ergonomics of SkBase64 could use some work.
                auto data = SkData::MakeWithProc(
                        b64.getData(), b64.getDataSize(),
                        [](const void* ptr, void*) { delete[] static_cast<const char*>(ptr); },
                        /*ctx=*/nullptr);

                return SkImage::MakeFromEncoded(data);
            } else {
                SkDebugf("Failed to decode base64: error %d\n", result);
            }
        }
    }

    return nullptr;
}

SkSVGImage::SkSVGImage() : INHERITED(SkSVGTag::kImage) {}

void SkSVGImage::appendChild(sk_sp<SkSVGNode>) {
    SkDebugf("cannot append child nodes to this element.\n");
}

void SkSVGImage::setHref(const SkSVGStringType& href) {
    fImage = ImageFromBase64Encoded(href.c_str());

    if (!fImage) {
        SkDebugf("SkImage::MakeFromEncoded() failed\n");
    }
}

void SkSVGImage::setX(const SkSVGLength& x) { fX = x; }

void SkSVGImage::setY(const SkSVGLength& y) { fY = y; }

void SkSVGImage::setWidth(const SkSVGLength& width) { fWidth = width; }

void SkSVGImage::setHeight(const SkSVGLength& height) { fHeight = height; }

void SkSVGImage::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
        case SkSVGAttribute::kHref:
            if (const auto* href = v.as<SkSVGStringValue>()) {
                this->setHref(*href);
            }
            break;
        case SkSVGAttribute::kX:
            if (const auto* x = v.as<SkSVGLengthValue>()) {
                this->setX(*x);
            }
            break;
        case SkSVGAttribute::kY:
            if (const auto* y = v.as<SkSVGLengthValue>()) {
                this->setY(*y);
            }
            break;
        case SkSVGAttribute::kWidth:
            if (const auto* width = v.as<SkSVGLengthValue>()) {
                this->setWidth(*width);
            }
            break;
        case SkSVGAttribute::kHeight:
            if (const auto* height = v.as<SkSVGLengthValue>()) {
                this->setHeight(*height);
            }
            break;
        default:
            this->INHERITED::onSetAttribute(attr, v);
    }
}

SkRect SkSVGImage::resolve(const SkSVGRenderContext& ctx) const {
    return ctx.lengthContext().resolveRect(fX, fY, fWidth, fHeight);
}

void SkSVGImage::onRender(const SkSVGRenderContext& ctx) const {
    if (fImage) {
        // TODO(fuego): respect opacity and other presentation attributes.
        ctx.canvas()->drawImageRect(fImage, this->resolve(ctx), nullptr);
    }
}

SkPath SkSVGImage::onAsPath(const SkSVGRenderContext& ctx) const {
    SkPath path;
    path.addRect(this->resolve(ctx));

    this->mapToParent(&path);

    return path;
}
