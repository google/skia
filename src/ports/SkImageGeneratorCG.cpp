/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/ports/SkImageGeneratorCG.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "include/utils/mac/SkCGUtils.h"
#include "src/codec/SkCodecImageGenerator.h"
#include "src/codec/SkFrameHolder.h"
#include "src/ports/SkCGCodec.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#ifdef SK_BUILD_FOR_MAC
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <MobileCoreServices/MobileCoreServices.h>
#endif

#include <vector>

namespace {
class CGCodec : public SkCodec {
public:
    CGCodec(SkEncodedInfo&&, SkUniqueCFRef<CGImageSourceRef> imageSrc, SkEncodedOrigin);
    ~CGCodec() override;

protected:
    Result onGetPixels(const SkImageInfo&, void* pixels, size_t rowBytes, const Options&,
                       int* rowsDecoded) override;

    SkEncodedImageFormat onGetEncodedFormat() const override;

    bool usesColorXform() const override { return false; }

    int onGetFrameCount() override;

    bool onGetFrameInfo(int, FrameInfo*) const override;

    int onGetRepetitionCount() override;

    const SkFrameHolder* getFrameHolder() const override {
        return &fFrameHolder;
    }

private:
    const SkUniqueCFRef<CGImageSourceRef> fImageSrc;

    class Frame : public SkFrame {
    public:
        Frame(int id) : SkFrame(id) {
            this->setRequiredFrame(kNoFrame); // CG handles composing frames.
            // So a client will never think they should cache a frame to compose with a future one.
            this->setDisposalMethod(SkCodecAnimation::DisposalMethod::kRestorePrevious);
        }
    private:
        SkEncodedInfo::Alpha onReportedAlpha() const {
            return this->hasAlpha() ? SkEncodedInfo::kUnpremul_Alpha : SkEncodedInfo::kOpaque_Alpha;
        }
    };

    class FrameHolder : public SkFrameHolder {
    public:
        int frameCount() const { return SkTo<int>(fFrames.size()); }

        void init(const SkISize& dims, std::vector<Frame>&& frames) {
            SkASSERT(!fFrames.size());

            fScreenWidth  = dims.width();
            fScreenHeight = dims.height();
            fFrames       = std::move(frames);
        }
    private:
        std::vector<Frame> fFrames;
        const SkFrame* onGetFrame(int i) const {
            if (!SkTFitsIn<size_t>(i) || SkTo<size_t>(i) >= fFrames.size()) {
                return nullptr;
            }
            return &(fFrames[i]);
        }
    };

    FrameHolder fFrameHolder;

    typedef SkCodec INHERITED;
};

static void release_data(void* info, const void* data, size_t size) {
    auto skdata = reinterpret_cast<SkData*>(info);
    SkASSERT(data == skdata->data());
    SkASSERT(size == skdata->size());
    skdata->unref();
}

static SkUniqueCFRef<CGImageSourceRef> data_to_CGImageSrc(SkData* data) {
    SkUniqueCFRef<CGDataProviderRef> cgData(
            CGDataProviderCreateWithData(SkRef(data), data->data(), data->size(), &release_data));
    if (!cgData) {
        return nullptr;
    }

    return SkUniqueCFRef<CGImageSourceRef>(
            CGImageSourceCreateWithDataProvider(cgData.get(), nullptr));
}

static const struct {
    const char*          uniformTypeIdentifier;
    SkEncodedImageFormat format;
} gFormatTable[] = {
    { "public.png",          SkEncodedImageFormat::kPNG  },
    { "public.jpeg",         SkEncodedImageFormat::kJPEG },
    { "com.compuserve.gif",  SkEncodedImageFormat::kGIF  },
    { "com.microsoft.ico",   SkEncodedImageFormat::kICO  },
    { "com.microsoft.bmp",   SkEncodedImageFormat::kBMP  },
    { "public.heic",         SkEncodedImageFormat::kHEIF },
    // Disable DNG for now, but CG supports it if desired.
  //{ "com.adobe.raw-image", SkEncodedImageFormat::kDNG  },
};
}  // namespace

static bool get_format(CGImageSourceRef image, SkEncodedImageFormat* outFormat) {

    SkUniqueCFRef<CFStringRef> uniformTypeIdentifier(CGImageSourceGetType(image));
    if (const char* c_str = CFStringGetCStringPtr(uniformTypeIdentifier.get(),
                                                  kCFStringEncodingUTF8)) {
        for (const auto& entry : gFormatTable) {
            if (0 == strcmp(c_str, entry.uniformTypeIdentifier)) {
                *outFormat = entry.format;
                return true;
            }
        }
    }
    return false;
}

SkUniqueCFRef<CFDictionaryRef> copy_properties(CGImageSourceRef imageSrc, int index) {
    return SkUniqueCFRef<CFDictionaryRef>(CGImageSourceCopyPropertiesAtIndex(imageSrc, index,
                                                                             nullptr));
}

std::unique_ptr<SkCodec> SkCGCodec::MakeFromEncoded(sk_sp<SkData> data) {
    if (!data) return nullptr;

    SkUniqueCFRef<CGImageSourceRef> imageSrc = data_to_CGImageSrc(data.get());

    // Only support image types we're prepared for.
    SkEncodedImageFormat placeholder;
    if (!imageSrc || !get_format(imageSrc.get(), &placeholder)) {
        return nullptr;
    }

    auto properties = copy_properties(imageSrc.get(), 0);
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

    // TODO: Get actual color/alpha info from the image.
    auto info = SkEncodedInfo::Make(width, height, SkEncodedInfo::kRGBA_Color,
                hasAlpha ? SkEncodedInfo::kUnpremul_Alpha : SkEncodedInfo::kOpaque_Alpha, 8);

    SkEncodedOrigin origin = kDefault_SkEncodedOrigin;
    CFNumberRef orientationRef = static_cast<CFNumberRef>(
            CFDictionaryGetValue(properties.get(), kCGImagePropertyOrientation));
    int originInt;
    if (orientationRef && CFNumberGetValue(orientationRef, kCFNumberIntType, &originInt)) {
        origin = (SkEncodedOrigin) originInt;
    }

    // TODO: Extract color space information.
    return std::unique_ptr<SkCodec>(new CGCodec(std::move(info), std::move(imageSrc), origin));
}

std::unique_ptr<SkImageGenerator> SkImageGeneratorCG::MakeFromEncodedCG(sk_sp<SkData> data) {
    auto codec = SkCGCodec::MakeFromEncoded(data);
    return SkCodecImageGenerator::MakeFromCodec(std::move(codec), std::move(data));
}

CGCodec::CGCodec(SkEncodedInfo&& info, SkUniqueCFRef<CGImageSourceRef> src, SkEncodedOrigin origin)
    : INHERITED(std::move(info), XformFormat(), nullptr, origin)
    , fImageSrc(std::move(src))
{}

CGCodec::~CGCodec() = default;

SkEncodedImageFormat CGCodec::onGetEncodedFormat() const {
    SkEncodedImageFormat format;
    SkAssertResult(get_format(fImageSrc.get(), &format));
    return format;
}

SkCodec::Result CGCodec::onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                     const Options& options, int* rowsDecoded) {
    if (kN32_SkColorType != info.colorType()) {
        // FIXME: Support other colorTypes.
        return kUnimplemented;
    }

    SkUniqueCFRef<CGImageRef> image(CGImageSourceCreateImageAtIndex(fImageSrc.get(),
                                                                    options.fFrameIndex, nullptr));
    if (!image) {
        return kInternalError;
    }

    SkPixmap dst(info, pixels, rowBytes);

    // Defensively clear to transparent. SkCopyPixelsFromCGImage returns true even if it did not
    // initialize all pixels. There doesn't seem to be a way to know whether the image is complete,
    // at least when using non-incremental sources. CGImageSourceGetStatusAtIndex returns
    // kCGImageStatusComplete even if nothing was decoded.
    if (options.fZeroInitialized == kNo_ZeroInitialized) {
        dst.erase(SK_ColorTRANSPARENT);
    }
    if (rowsDecoded) {
        *rowsDecoded = info.height();
    }

    return SkCopyPixelsFromCGImage(dst, image.get()) ? kSuccess : kInternalError;
}

// Returns an unowned pointer.
static CFDictionaryRef get_gif_properties(CFDictionaryRef properties) {
    return static_cast<CFDictionaryRef>(CFDictionaryGetValue(properties,
                                                             kCGImagePropertyGIFDictionary));
}

int CGCodec::onGetFrameCount() {
    switch (this->getEncodedFormat()) {
        case SkEncodedImageFormat::kGIF:
            break;
        case SkEncodedImageFormat::kPNG:
            // TODO: Support APNG? No clients have asked for it, but CG supports it.
        case SkEncodedImageFormat::kHEIF:
            // TODO: Support HEIF.
        case SkEncodedImageFormat::kICO:
            // This is method is intended for animation but ICO is not typically used in that way.
            // Return 1 to signify no animation.
        default:
            return 1;
    }

    if (int count = fFrameHolder.frameCount()) {
        return count;
    }

    const size_t count = CGImageSourceGetCount(fImageSrc.get());
    if (!SkTFitsIn<int>(count) || count <= 1) {
        // Not animated.
        return 1;
    }

    const auto dimensions = this->dimensions();
    std::vector<Frame> frames;
    frames.reserve(count);
    for (size_t i = 0; i < count; i++) {
        auto properties = copy_properties(fImageSrc.get(), i);
        if (!properties) {
            if (i == 0) return 1;
            break;
        }
        frames.emplace_back(frames.size());
        Frame& frame = frames.back();
        frame.setHasAlpha(bool(CFDictionaryGetValue(properties.get(), kCGImagePropertyHasAlpha)));

        // This is intended to be the size of the updated portion of the frame, which we would use,
        // along with the disposal method and alpha, to determine the required frame and final
        // alpha. CG does not give us this information, so use the image dimensions.
        frame.setXYWH(0, 0, dimensions.width(), dimensions.height());

        // CG reports the delay time in seconds (at least for GIF). The dictionary will only be
        // present if there is a GraphicControlExtension. Otherwise, use default values, which
        // matches the existing GIF decoders.
        if (auto gifProperties = get_gif_properties(properties.get())) {
            double delayTime {0};
            auto delayTimeRef = static_cast<CFNumberRef>(
                    CFDictionaryGetValue(gifProperties, kCGImagePropertyGIFUnclampedDelayTime));
            if (delayTimeRef && CFNumberGetValue(delayTimeRef , kCFNumberDoubleType, &delayTime)) {
                frame.setDuration(delayTime * 1000);
            }
        }
    }

    fFrameHolder.init(dimensions, std::move(frames));
    return fFrameHolder.frameCount();
}

bool CGCodec::onGetFrameInfo(int i, FrameInfo* info) const {
    const auto* frame = fFrameHolder.getFrame(i);
    if (!frame) {
        return false;
    }

    if (info) {
        info->fRequiredFrame  = frame->getRequiredFrame();
        info->fDuration       = frame->getDuration();
        info->fFullyReceived  = true; // CGImageSourceGetStatusAtIndex is documented to give the
                                      // answer here, but it seems that CG does not even report a
                                      // GIF frame unless it is complete.
        info->fAlphaType      = frame->hasAlpha() ? kUnpremul_SkAlphaType : kOpaque_SkAlphaType;
        info->fDisposalMethod = frame->getDisposalMethod();
    }
    return true;
}

int CGCodec::onGetRepetitionCount() {
    SkUniqueCFRef<CFDictionaryRef> props(CGImageSourceCopyProperties(fImageSrc.get(), nullptr));
    if (auto gifProps = get_gif_properties(props.get())) {
        // loopCountRef will be null if the encoded image does not contain the netscape extension.
        CFNumberRef loopCountRef = static_cast<CFNumberRef>(
                CFDictionaryGetValue(gifProps, kCGImagePropertyGIFLoopCount));
        int loopCount;
        if (loopCountRef && CFNumberGetValue(loopCountRef , kCFNumberIntType, &loopCount)) {
            if (loopCount == 0) {
                return kRepetitionCountInfinite;
            }

            return loopCount;
        }
    }
    return 0;
}

