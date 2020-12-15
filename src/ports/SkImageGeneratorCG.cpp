/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkEncodedOrigin.h"
#include "include/ports/SkImageGeneratorCG.h"
#include "include/private/SkTemplates.h"
#include "include/utils/mac/SkCGUtils.h"
#include "src/core/SkPixmapPriv.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#ifdef SK_BUILD_FOR_MAC
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <MobileCoreServices/MobileCoreServices.h>
#endif

namespace {
class ImageGeneratorCG : public SkImageGenerator {
public:
    ImageGeneratorCG(const SkImageInfo&, SkUniqueCFRef<CGImageSourceRef> imageSrc,
                     sk_sp<SkData> data, SkEncodedOrigin);

protected:
    sk_sp<SkData> onRefEncodedData() override;

    bool onGetPixels(const SkImageInfo&, void* pixels, size_t rowBytes, const Options&) override;

private:
    const SkUniqueCFRef<CGImageSourceRef> fImageSrc;
    const sk_sp<SkData> fData;
    const SkEncodedOrigin fOrigin;

    using INHERITED = SkImageGenerator;
};

static SkUniqueCFRef<CGImageSourceRef> data_to_CGImageSrc(SkData* data) {
    SkUniqueCFRef<CGDataProviderRef> cgData(
            CGDataProviderCreateWithData(data, data->data(), data->size(), nullptr));
    if (!cgData) {
        return nullptr;
    }
    return SkUniqueCFRef<CGImageSourceRef>(
            CGImageSourceCreateWithDataProvider(cgData.get(), nullptr));
}

}  // namespace

std::unique_ptr<SkImageGenerator> SkImageGeneratorCG::MakeFromEncodedCG(sk_sp<SkData> data) {
    SkUniqueCFRef<CGImageSourceRef> imageSrc = data_to_CGImageSrc(data.get());
    if (!imageSrc) {
        return nullptr;
    }

    SkUniqueCFRef<CFDictionaryRef> properties(
            CGImageSourceCopyPropertiesAtIndex(imageSrc.get(), 0, nullptr));
    if (!properties) {
        return nullptr;
    }

    CFNumberRef widthRef = static_cast<CFNumberRef>(
            CFDictionaryGetValue(properties.get(), kCGImagePropertyPixelWidth));
    CFNumberRef heightRef = static_cast<CFNumberRef>(
            CFDictionaryGetValue(properties.get(), kCGImagePropertyPixelHeight));
    if (nullptr == widthRef || nullptr == heightRef) {
        return nullptr;
    }

    int width, height;
    if (!CFNumberGetValue(widthRef , kCFNumberIntType, &width ) ||
        !CFNumberGetValue(heightRef, kCFNumberIntType, &height))
    {
        return nullptr;
    }

    bool hasAlpha = bool(CFDictionaryGetValue(properties.get(), kCGImagePropertyHasAlpha));
    SkAlphaType alphaType = hasAlpha ? kPremul_SkAlphaType : kOpaque_SkAlphaType;
    SkImageInfo info = SkImageInfo::MakeS32(width, height, alphaType);

    SkEncodedOrigin origin = kDefault_SkEncodedOrigin;
    CFNumberRef orientationRef = static_cast<CFNumberRef>(
            CFDictionaryGetValue(properties.get(), kCGImagePropertyOrientation));
    int originInt;
    if (orientationRef && CFNumberGetValue(orientationRef, kCFNumberIntType, &originInt)) {
        origin = (SkEncodedOrigin) originInt;
    }

    if (SkEncodedOriginSwapsWidthHeight(origin)) {
        info = SkPixmapPriv::SwapWidthHeight(info);
    }

    // FIXME: We have the opportunity to extract color space information here,
    //        though I think it makes sense to wait until we understand how
    //        we want to communicate it to the generator.

    return std::unique_ptr<SkImageGenerator>(new ImageGeneratorCG(info, std::move(imageSrc),
                                                                  std::move(data), origin));
}

ImageGeneratorCG::ImageGeneratorCG(const SkImageInfo& info, SkUniqueCFRef<CGImageSourceRef> src,
                                   sk_sp<SkData> data, SkEncodedOrigin origin)
    : INHERITED(info)
    , fImageSrc(std::move(src))
    , fData(std::move(data))
    , fOrigin(origin)
{}

sk_sp<SkData> ImageGeneratorCG::onRefEncodedData() {
    return fData;
}

bool ImageGeneratorCG::onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                   const Options&)
{
    if (kN32_SkColorType != info.colorType()) {
        // FIXME: Support other colorTypes.
        return false;
    }

    switch (info.alphaType()) {
        case kOpaque_SkAlphaType:
            if (kOpaque_SkAlphaType != this->getInfo().alphaType()) {
                return false;
            }
            break;
        case kPremul_SkAlphaType:
            break;
        default:
            return false;
    }

    SkUniqueCFRef<CGImageRef> image(CGImageSourceCreateImageAtIndex(fImageSrc.get(), 0, nullptr));
    if (!image) {
        return false;
    }

    SkPixmap dst(info, pixels, rowBytes);
    auto decode = [&image](const SkPixmap& pm) {
        // FIXME: Using SkCopyPixelsFromCGImage (as opposed to swizzling
        // ourselves) greatly restricts the color and alpha types that we
        // support.  If we swizzle ourselves, we can add support for:
        //     kUnpremul_SkAlphaType
        //     16-bit per component RGBA
        //     kGray_8_SkColorType
        // Additionally, it would be interesting to compare the performance
        // of SkSwizzler with CG's built in swizzler.
        return SkCopyPixelsFromCGImage(pm, image.get());
    };
    return SkPixmapPriv::Orient(dst, fOrigin, decode);
}
