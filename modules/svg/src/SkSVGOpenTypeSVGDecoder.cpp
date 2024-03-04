/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/codec/SkJpegDecoder.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkColor.h"
#include "include/core/SkOpenTypeSVGDecoder.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "modules/skresources/include/SkResources.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGNode.h"
#include "modules/svg/include/SkSVGOpenTypeSVGDecoder.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGSVG.h"
#include "modules/svg/include/SkSVGUse.h"
#include "src/base/SkBase64.h"
#include "src/core/SkEnumerate.h"

#include <memory>

using namespace skia_private;

namespace {
class DataResourceProvider final : public skresources::ResourceProvider {
public:
    static sk_sp<skresources::ResourceProvider> Make() {
        return sk_sp<skresources::ResourceProvider>(new DataResourceProvider());
    }

    sk_sp<skresources::ImageAsset> loadImageAsset(const char rpath[],
                                                  const char rname[],
                                                  const char rid[]) const override {
        if (auto data = decode_datauri("data:image/", rname)) {
            std::unique_ptr<SkCodec> codec = nullptr;
            if (SkPngDecoder::IsPng(data->bytes(), data->size())) {
                codec = SkPngDecoder::Decode(data, nullptr);
            } else if (SkJpegDecoder::IsJpeg(data->bytes(), data->size())) {
                codec = SkJpegDecoder::Decode(data, nullptr);
            } else {
                // The spec says only JPEG or PNG should be used to encode the embedded data.
                // https://learn.microsoft.com/en-us/typography/opentype/spec/svg#svg-capability-requirements-and-restrictions
                SkDEBUGFAIL("Unsupported codec");
                return nullptr;
            }
            if (!codec) {
                return nullptr;
            }
            return skresources::MultiFrameImageAsset::Make(std::move(codec));
        }
        return nullptr;
    }

private:
    DataResourceProvider() = default;

    static sk_sp<SkData> decode_datauri(const char prefix[], const char uri[]) {
        // We only handle B64 encoded image dataURIs: data:image/<type>;base64,<data>
        // (https://en.wikipedia.org/wiki/Data_URI_scheme)
        static constexpr char kDataURIEncodingStr[] = ";base64,";

        const size_t prefixLen = strlen(prefix);
        if (strncmp(uri, prefix, prefixLen) != 0) {
            return nullptr;
        }

        const char* encoding = strstr(uri + prefixLen, kDataURIEncodingStr);
        if (!encoding) {
            return nullptr;
        }

        const char* b64Data = encoding + std::size(kDataURIEncodingStr) - 1;
        size_t b64DataLen = strlen(b64Data);
        size_t dataLen;
        if (SkBase64::Decode(b64Data, b64DataLen, nullptr, &dataLen) != SkBase64::kNoError) {
            return nullptr;
        }

        sk_sp<SkData> data = SkData::MakeUninitialized(dataLen);
        void* rawData = data->writable_data();
        if (SkBase64::Decode(b64Data, b64DataLen, rawData, &dataLen) != SkBase64::kNoError) {
            return nullptr;
        }

        return data;
    }

    using INHERITED = ResourceProvider;
};
}  // namespace

SkSVGOpenTypeSVGDecoder::SkSVGOpenTypeSVGDecoder(sk_sp<SkSVGDOM> skSvg, size_t approximateSize)
    : fSkSvg(std::move(skSvg))
    , fApproximateSize(approximateSize)
{}

SkSVGOpenTypeSVGDecoder::~SkSVGOpenTypeSVGDecoder() = default;

std::unique_ptr<SkOpenTypeSVGDecoder> SkSVGOpenTypeSVGDecoder::Make(const uint8_t* svg,
                                                                    size_t svgLength) {
    std::unique_ptr<SkStreamAsset> stream = SkMemoryStream::MakeDirect(svg, svgLength);
    if (!stream) {
        return nullptr;
    }
    SkSVGDOM::Builder builder;
    builder.setResourceProvider(DataResourceProvider::Make());
    sk_sp<SkSVGDOM> skSvg = builder.make(*stream);
    if (!skSvg) {
        return nullptr;
    }
    return std::unique_ptr<SkOpenTypeSVGDecoder>(
        new SkSVGOpenTypeSVGDecoder(std::move(skSvg), svgLength));
}

size_t SkSVGOpenTypeSVGDecoder::approximateSize() {
    // TODO
    return fApproximateSize;
}

bool SkSVGOpenTypeSVGDecoder::render(SkCanvas& canvas, int upem, SkGlyphID glyphId,
                                     SkColor foregroundColor, SkSpan<SkColor> palette) {
    SkSize emSize = SkSize::Make(SkScalar(upem), SkScalar(upem));
    fSkSvg->setContainerSize(emSize);

    SkSVGPresentationContext pctx;
    pctx.fInherited.fColor.set(foregroundColor);

    THashMap<SkString, SkSVGColorType> namedColors;
    if (!palette.empty()) {
        for (auto&& [i, color] : SkMakeEnumerate(palette)) {
            constexpr const size_t colorStringLen = sizeof("color") - 1;
            char colorIdString[colorStringLen + kSkStrAppendU32_MaxSize + 1] = "color";
            *SkStrAppendU32(colorIdString + colorStringLen, i) = 0;

            namedColors.set(SkString(colorIdString), color);
        }
        pctx.fNamedColors = &namedColors;
    }

    constexpr const size_t glyphStringLen = sizeof("glyph") - 1;
    char glyphIdString[glyphStringLen + kSkStrAppendU32_MaxSize + 1] = "glyph";
    *SkStrAppendU32(glyphIdString + glyphStringLen, glyphId) = 0;

    fSkSvg->renderNode(&canvas, pctx, glyphIdString);
    return true;
}
