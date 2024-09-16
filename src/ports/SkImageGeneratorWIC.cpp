/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkStream.h"
#include "include/ports/SkImageGeneratorWIC.h"
#include "include/private/base/SkTemplates.h"
#include "src/codec/SkPixmapUtilsPriv.h"
#include "src/utils/win/SkIStream.h"
#include "src/utils/win/SkTScopedComPtr.h"

#include <wincodec.h>

// All Windows SDKs back to XPSP2 export the CLSID_WICImagingFactory symbol.
// In the Windows8 SDK the CLSID_WICImagingFactory symbol is still exported
// but CLSID_WICImagingFactory is then #defined to CLSID_WICImagingFactory2.
// Undo this #define if it has been done so that we link against the symbols
// we intended to link against on all SDKs.
#if defined(CLSID_WICImagingFactory)
    #undef CLSID_WICImagingFactory
#endif

namespace {
class ImageGeneratorWIC : public SkImageGenerator {
public:
    /*
     * Takes ownership of the imagingFactory
     * Takes ownership of the imageSource
     */
    ImageGeneratorWIC(const SkImageInfo& info, IWICImagingFactory* imagingFactory,
            IWICBitmapSource* imageSource, sk_sp<SkData>, SkEncodedOrigin);
protected:
    sk_sp<SkData> onRefEncodedData() override;

    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, const Options&)
    override;

private:
    SkTScopedComPtr<IWICImagingFactory> fImagingFactory;
    SkTScopedComPtr<IWICBitmapSource>   fImageSource;
    sk_sp<SkData>                       fData;
    SkEncodedOrigin                     fOrigin;

    using INHERITED = SkImageGenerator;
};
}  // namespace

std::unique_ptr<SkImageGenerator> SkImageGeneratorWIC::MakeFromEncodedWIC(sk_sp<SkData> data) {
    // Create Windows Imaging Component ImagingFactory.
    SkTScopedComPtr<IWICImagingFactory> imagingFactory;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&imagingFactory));
    if (FAILED(hr)) {
        return nullptr;
    }

    // Create an IStream.
    SkTScopedComPtr<IStream> iStream;
    // Note that iStream will take ownership of the new memory stream because
    // we set |deleteOnRelease| to true.
    hr = SkIStream::CreateFromSkStream(std::make_unique<SkMemoryStream>(data), &iStream);
    if (FAILED(hr)) {
        return nullptr;
    }

    // Create the decoder from the stream.
    SkTScopedComPtr<IWICBitmapDecoder> decoder;
    hr = imagingFactory->CreateDecoderFromStream(iStream.get(), nullptr,
            WICDecodeMetadataCacheOnDemand, &decoder);
    if (FAILED(hr)) {
        return nullptr;
    }

    // Select the first frame from the decoder.
    SkTScopedComPtr<IWICBitmapFrameDecode> imageFrame;
    hr = decoder->GetFrame(0, &imageFrame);
    if (FAILED(hr)) {
        return nullptr;
    }

    // Get the metadata query reader from the frame.
    SkEncodedOrigin origin = kDefault_SkEncodedOrigin;
    SkTScopedComPtr<IWICMetadataQueryReader> queryReader;
    hr = imageFrame->GetMetadataQueryReader(&queryReader);

    // WIC decoder doesn't support BMP and ICO metadata, so we only continue
    // reading metadata if GetMetadataQueryReader didn't return an error code.
    if (SUCCEEDED(hr)) {
      // Query for the orientation metadata (assuming JPEG policy).
      PROPVARIANT propValue;
      PropVariantInit(&propValue);
      hr = queryReader->GetMetadataByName(L"/app1/ifd/{ushort=274}", &propValue);
      if (SUCCEEDED(hr) && propValue.vt == VT_UI2) {
          SkEncodedOrigin originValue = static_cast<SkEncodedOrigin>(propValue.uiVal);
          if (originValue >= kTopLeft_SkEncodedOrigin && originValue <= kLast_SkEncodedOrigin) {
            origin = originValue;
          }
      }
    }

    // Treat the frame as an image source.
    SkTScopedComPtr<IWICBitmapSource> imageSource;
    hr = imageFrame->QueryInterface(IID_PPV_ARGS(&imageSource));
    if (FAILED(hr)) {
        return nullptr;
    }

    // Get the size of the image.
    UINT width;
    UINT height;
    hr = imageSource->GetSize(&width, &height);
    if (FAILED(hr)) {
        return nullptr;
    }

    // Get the encoded pixel format.
    WICPixelFormatGUID format;
    hr = imageSource->GetPixelFormat(&format);
    if (FAILED(hr)) {
        return nullptr;
    }

    // Recommend kOpaque if the image is opaque and kPremul otherwise.
    // FIXME: We are stuck recommending kPremul for all indexed formats
    //        (Ex: GUID_WICPixelFormat8bppIndexed) because we don't have
    //        a way to check if the image has alpha.
    SkAlphaType alphaType = kPremul_SkAlphaType;

    if (GUID_WICPixelFormat16bppBGR555 == format ||
        GUID_WICPixelFormat16bppBGR565 == format ||
        GUID_WICPixelFormat32bppBGR101010 == format ||
        GUID_WICPixelFormatBlackWhite == format ||
        GUID_WICPixelFormat2bppGray == format ||
        GUID_WICPixelFormat4bppGray == format ||
        GUID_WICPixelFormat8bppGray == format ||
        GUID_WICPixelFormat16bppGray == format ||
        GUID_WICPixelFormat16bppGrayFixedPoint == format ||
        GUID_WICPixelFormat16bppGrayHalf == format ||
        GUID_WICPixelFormat32bppGrayFloat == format ||
        GUID_WICPixelFormat32bppGrayFixedPoint == format ||
        GUID_WICPixelFormat32bppRGBE == format ||
        GUID_WICPixelFormat24bppRGB == format ||
        GUID_WICPixelFormat24bppBGR == format ||
        GUID_WICPixelFormat32bppBGR == format ||
        GUID_WICPixelFormat48bppRGB == format ||
        GUID_WICPixelFormat48bppBGR == format ||
        GUID_WICPixelFormat48bppRGBFixedPoint == format ||
        GUID_WICPixelFormat48bppBGRFixedPoint == format ||
        GUID_WICPixelFormat48bppRGBHalf == format ||
        GUID_WICPixelFormat64bppRGBFixedPoint == format ||
        GUID_WICPixelFormat64bppRGBHalf == format ||
        GUID_WICPixelFormat96bppRGBFixedPoint == format ||
        GUID_WICPixelFormat128bppRGBFloat == format ||
        GUID_WICPixelFormat128bppRGBFixedPoint == format ||
        GUID_WICPixelFormat32bppRGB == format ||
        GUID_WICPixelFormat64bppRGB == format ||
        GUID_WICPixelFormat96bppRGBFloat == format ||
        GUID_WICPixelFormat32bppCMYK == format ||
        GUID_WICPixelFormat64bppCMYK == format ||
        GUID_WICPixelFormat8bppY == format ||
        GUID_WICPixelFormat8bppCb == format ||
        GUID_WICPixelFormat8bppCr == format ||
        GUID_WICPixelFormat16bppCbCr == format)
    {
        alphaType = kOpaque_SkAlphaType;
    }

    // FIXME: If we change the implementation to handle swizzling ourselves,
    //        we can support more output formats.
    SkImageInfo info = SkImageInfo::MakeS32(width, height, alphaType);
    if (SkEncodedOriginSwapsWidthHeight(origin)) {
        info = SkPixmapUtils::SwapWidthHeight(info);
    }
    return std::unique_ptr<SkImageGenerator>(
            new ImageGeneratorWIC(info, imagingFactory.release(), imageSource.release(),
                                    std::move(data), origin));
}

ImageGeneratorWIC::ImageGeneratorWIC(const SkImageInfo& info,
        IWICImagingFactory* imagingFactory, IWICBitmapSource* imageSource, sk_sp<SkData> data, SkEncodedOrigin origin)
    : INHERITED(info)
    , fImagingFactory(imagingFactory)
    , fImageSource(imageSource)
    , fData(std::move(data))
    , fOrigin(origin)
{}

sk_sp<SkData> ImageGeneratorWIC::onRefEncodedData() {
    return fData;
}

bool ImageGeneratorWIC::onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
        const Options&) {
    if (kN32_SkColorType != info.colorType()) {
        return false;
    }

    // Create a format converter.
    SkTScopedComPtr<IWICFormatConverter> formatConverter;
    HRESULT hr = fImagingFactory->CreateFormatConverter(&formatConverter);
    if (FAILED(hr)) {
        return false;
    }

    GUID format = GUID_WICPixelFormat32bppPBGRA;
    if (kUnpremul_SkAlphaType == info.alphaType()) {
        format = GUID_WICPixelFormat32bppBGRA;
    }

    hr = formatConverter->Initialize(fImageSource.get(), format, WICBitmapDitherTypeNone, nullptr,
            0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) {
        return false;
    }

    // Treat the format converter as an image source.
    SkTScopedComPtr<IWICBitmapSource> formatConverterSrc;
    hr = formatConverter->QueryInterface(IID_PPV_ARGS(&formatConverterSrc));
    if (FAILED(hr)) {
        return false;
    }

    SkPixmap dst(info, pixels, rowBytes);
    auto decode = [&formatConverterSrc](const SkPixmap& pm) {
        // Get the destination pixels.
        void* pixelsAddr = pm.writable_addr();
        size_t rowBytes = pm.rowBytes();
        const SkImageInfo& info = pm.info();

        // Set the destination pixels.
        HRESULT hr = formatConverterSrc->CopyPixels(nullptr, (UINT) rowBytes, (UINT) rowBytes * info.height(),
            (BYTE*) pixelsAddr);
        return SUCCEEDED(hr);
    };
    return SkPixmapUtils::Orient(dst, fOrigin, decode);
}
