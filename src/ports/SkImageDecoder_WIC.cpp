
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wincodec.h>
#include "SkAutoCoInitialize.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkIStream.h"
#include "SkMovie.h"
#include "SkStream.h"
#include "SkTScopedComPtr.h"
#include "SkUnPreMultiply.h"

//All Windows SDKs back to XPSP2 export the CLSID_WICImagingFactory symbol.
//In the Windows8 SDK the CLSID_WICImagingFactory symbol is still exported
//but CLSID_WICImagingFactory is then #defined to CLSID_WICImagingFactory2.
//Undo this #define if it has been done so that we link against the symbols
//we intended to link against on all SDKs.
#if defined(CLSID_WICImagingFactory)
#undef CLSID_WICImagingFactory
#endif

class SkImageDecoder_WIC : public SkImageDecoder {
public:
    // Decoding modes corresponding to SkImageDecoder::Mode, plus an extra mode for decoding
    // only the format.
    enum WICModes {
        kDecodeFormat_WICMode,
        kDecodeBounds_WICMode,
        kDecodePixels_WICMode,
    };

    /**
     *  Helper function to decode an SkStream.
     *  @param stream SkStream to decode. Must be at the beginning.
     *  @param bm   SkBitmap to decode into. Only used if wicMode is kDecodeBounds_WICMode or
     *      kDecodePixels_WICMode, in which case it must not be NULL.
     *  @param format Out parameter for the SkImageDecoder::Format of the SkStream. Only used if
     *      wicMode is kDecodeFormat_WICMode.
     */
    bool decodeStream(SkStream* stream, SkBitmap* bm, WICModes wicMode, Format* format) const;

protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm, Mode mode) SK_OVERRIDE;
};

struct FormatConversion {
    GUID                    fGuidFormat;
    SkImageDecoder::Format  fFormat;
};

static const FormatConversion gFormatConversions[] = {
    { GUID_ContainerFormatBmp, SkImageDecoder::kBMP_Format },
    { GUID_ContainerFormatGif, SkImageDecoder::kGIF_Format },
    { GUID_ContainerFormatIco, SkImageDecoder::kICO_Format },
    { GUID_ContainerFormatJpeg, SkImageDecoder::kJPEG_Format },
    { GUID_ContainerFormatPng, SkImageDecoder::kPNG_Format },
};

static SkImageDecoder::Format GuidContainerFormat_to_Format(REFGUID guid) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gFormatConversions); i++) {
        if (IsEqualGUID(guid, gFormatConversions[i].fGuidFormat)) {
            return gFormatConversions[i].fFormat;
        }
    }
    return SkImageDecoder::kUnknown_Format;
}

bool SkImageDecoder_WIC::onDecode(SkStream* stream, SkBitmap* bm, Mode mode) {
    WICModes wicMode;
    switch (mode) {
        case SkImageDecoder::kDecodeBounds_Mode:
            wicMode = kDecodeBounds_WICMode;
            break;
        case SkImageDecoder::kDecodePixels_Mode:
            wicMode = kDecodePixels_WICMode;
            break;
    }
    return this->decodeStream(stream, bm, wicMode, NULL);
}

bool SkImageDecoder_WIC::decodeStream(SkStream* stream, SkBitmap* bm, WICModes wicMode,
                                      Format* format) const {
    //Initialize COM.
    SkAutoCoInitialize scopedCo;
    if (!scopedCo.succeeded()) {
        return false;
    }

    HRESULT hr = S_OK;

    //Create Windows Imaging Component ImagingFactory.
    SkTScopedComPtr<IWICImagingFactory> piImagingFactory;
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(
            CLSID_WICImagingFactory
            , NULL
            , CLSCTX_INPROC_SERVER
            , IID_PPV_ARGS(&piImagingFactory)
        );
    }

    //Convert SkStream to IStream.
    SkTScopedComPtr<IStream> piStream;
    if (SUCCEEDED(hr)) {
        hr = SkIStream::CreateFromSkStream(stream, false, &piStream);
    }

    //Make sure we're at the beginning of the stream.
    if (SUCCEEDED(hr)) {
        LARGE_INTEGER liBeginning = { 0 };
        hr = piStream->Seek(liBeginning, STREAM_SEEK_SET, NULL);
    }

    //Create the decoder from the stream content.
    SkTScopedComPtr<IWICBitmapDecoder> piBitmapDecoder;
    if (SUCCEEDED(hr)) {
        hr = piImagingFactory->CreateDecoderFromStream(
            piStream.get()                    //Image to be decoded
            , NULL                            //No particular vendor
            , WICDecodeMetadataCacheOnDemand  //Cache metadata when needed
            , &piBitmapDecoder                //Pointer to the decoder
        );
    }

    if (kDecodeFormat_WICMode == wicMode) {
        SkASSERT(format != NULL);
        //Get the format
        if (SUCCEEDED(hr)) {
            GUID guidFormat;
            hr = piBitmapDecoder->GetContainerFormat(&guidFormat);
            if (SUCCEEDED(hr)) {
                *format = GuidContainerFormat_to_Format(guidFormat);
                return true;
            }
        }
        return false;
    }

    //Get the first frame from the decoder.
    SkTScopedComPtr<IWICBitmapFrameDecode> piBitmapFrameDecode;
    if (SUCCEEDED(hr)) {
        hr = piBitmapDecoder->GetFrame(0, &piBitmapFrameDecode);
    }

    //Get the BitmapSource interface of the frame.
    SkTScopedComPtr<IWICBitmapSource> piBitmapSourceOriginal;
    if (SUCCEEDED(hr)) {
        hr = piBitmapFrameDecode->QueryInterface(
            IID_PPV_ARGS(&piBitmapSourceOriginal)
        );
    }

    //Get the size of the bitmap.
    UINT width;
    UINT height;
    if (SUCCEEDED(hr)) {
        hr = piBitmapSourceOriginal->GetSize(&width, &height);
    }

    //Exit early if we're only looking for the bitmap bounds.
    if (SUCCEEDED(hr)) {
        bm->setConfig(SkBitmap::kARGB_8888_Config, width, height);
        if (kDecodeBounds_WICMode == wicMode) {
            return true;
        }
        if (!this->allocPixelRef(bm, NULL)) {
            return false;
        }
    }

    //Create a format converter.
    SkTScopedComPtr<IWICFormatConverter> piFormatConverter;
    if (SUCCEEDED(hr)) {
        hr = piImagingFactory->CreateFormatConverter(&piFormatConverter);
    }

    GUID destinationPixelFormat;
    if (this->getRequireUnpremultipliedColors()) {
        destinationPixelFormat = GUID_WICPixelFormat32bppBGRA;
    } else {
        destinationPixelFormat = GUID_WICPixelFormat32bppPBGRA;
    }

    if (SUCCEEDED(hr)) {
        hr = piFormatConverter->Initialize(
            piBitmapSourceOriginal.get()      //Input bitmap to convert
            , destinationPixelFormat          //Destination pixel format
            , WICBitmapDitherTypeNone         //Specified dither patterm
            , NULL                            //Specify a particular palette
            , 0.f                             //Alpha threshold
            , WICBitmapPaletteTypeCustom      //Palette translation type
        );
    }

    //Get the BitmapSource interface of the format converter.
    SkTScopedComPtr<IWICBitmapSource> piBitmapSourceConverted;
    if (SUCCEEDED(hr)) {
        hr = piFormatConverter->QueryInterface(
            IID_PPV_ARGS(&piBitmapSourceConverted)
        );
    }

    //Copy the pixels into the bitmap.
    if (SUCCEEDED(hr)) {
        SkAutoLockPixels alp(*bm);
        bm->eraseColor(SK_ColorTRANSPARENT);
        const UINT stride = bm->rowBytes();
        hr = piBitmapSourceConverted->CopyPixels(
            NULL,                             //Get all the pixels
            stride,
            stride * height,
            reinterpret_cast<BYTE *>(bm->getPixels())
        );

        // Note: we don't need to premultiply here since we specified PBGRA
        bm->computeAndSetOpaquePredicate();
    }

    return SUCCEEDED(hr);
}

/////////////////////////////////////////////////////////////////////////

extern SkImageDecoder* image_decoder_from_stream(SkStream*);

SkImageDecoder* SkImageDecoder::Factory(SkStream* stream) {
    SkImageDecoder* decoder = image_decoder_from_stream(stream);
    if (NULL == decoder) {
        // If no image decoder specific to the stream exists, use SkImageDecoder_WIC.
        return SkNEW(SkImageDecoder_WIC);
    } else {
        return decoder;
    }
}

/////////////////////////////////////////////////////////////////////////

SkMovie* SkMovie::DecodeStream(SkStream* stream) {
    return NULL;
}

/////////////////////////////////////////////////////////////////////////

class SkImageEncoder_WIC : public SkImageEncoder {
public:
    SkImageEncoder_WIC(Type t) : fType(t) {}

protected:
    virtual bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality);

private:
    Type fType;
};

bool SkImageEncoder_WIC::onEncode(SkWStream* stream
                                , const SkBitmap& bitmapOrig
                                , int quality)
{
    GUID type;
    switch (fType) {
        case kBMP_Type:
            type = GUID_ContainerFormatBmp;
            break;
        case kICO_Type:
            type = GUID_ContainerFormatIco;
            break;
        case kJPEG_Type:
            type = GUID_ContainerFormatJpeg;
            break;
        case kPNG_Type:
            type = GUID_ContainerFormatPng;
            break;
        default:
            return false;
    }

    //Convert to 8888 if needed.
    const SkBitmap* bitmap;
    SkBitmap bitmapCopy;
    if (SkBitmap::kARGB_8888_Config == bitmapOrig.config() && bitmapOrig.isOpaque()) {
        bitmap = &bitmapOrig;
    } else {
        if (!bitmapOrig.copyTo(&bitmapCopy, SkBitmap::kARGB_8888_Config)) {
            return false;
        }
        bitmap = &bitmapCopy;
    }

    // We cannot use PBGRA so we need to unpremultiply ourselves
    if (!bitmap->isOpaque()) {
        SkAutoLockPixels alp(*bitmap);

        uint8_t* pixels = reinterpret_cast<uint8_t*>(bitmap->getPixels());
        for (int y = 0; y < bitmap->height(); ++y) {
            for (int x = 0; x < bitmap->width(); ++x) {
                uint8_t* bytes = pixels + y * bitmap->rowBytes() + x * bitmap->bytesPerPixel();

                SkPMColor* src = reinterpret_cast<SkPMColor*>(bytes);
                SkColor* dst = reinterpret_cast<SkColor*>(bytes);

                *dst = SkUnPreMultiply::PMColorToColor(*src);
            }
        }
    }

    //Initialize COM.
    SkAutoCoInitialize scopedCo;
    if (!scopedCo.succeeded()) {
        return false;
    }

    HRESULT hr = S_OK;

    //Create Windows Imaging Component ImagingFactory.
    SkTScopedComPtr<IWICImagingFactory> piImagingFactory;
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(
            CLSID_WICImagingFactory
            , NULL
            , CLSCTX_INPROC_SERVER
            , IID_PPV_ARGS(&piImagingFactory)
        );
    }

    //Convert the SkWStream to an IStream.
    SkTScopedComPtr<IStream> piStream;
    if (SUCCEEDED(hr)) {
        hr = SkWIStream::CreateFromSkWStream(stream, &piStream);
    }

    //Create an encode of the appropriate type.
    SkTScopedComPtr<IWICBitmapEncoder> piEncoder;
    if (SUCCEEDED(hr)) {
        hr = piImagingFactory->CreateEncoder(type, NULL, &piEncoder);
    }

    if (SUCCEEDED(hr)) {
        hr = piEncoder->Initialize(piStream.get(), WICBitmapEncoderNoCache);
    }

    //Create a the frame.
    SkTScopedComPtr<IWICBitmapFrameEncode> piBitmapFrameEncode;
    SkTScopedComPtr<IPropertyBag2> piPropertybag;
    if (SUCCEEDED(hr)) {
        hr = piEncoder->CreateNewFrame(&piBitmapFrameEncode, &piPropertybag);
    }

    if (SUCCEEDED(hr)) {
        PROPBAG2 name = { 0 };
        name.dwType = PROPBAG2_TYPE_DATA;
        name.vt = VT_R4;
        name.pstrName = L"ImageQuality";

        VARIANT value;
        VariantInit(&value);
        value.vt = VT_R4;
        value.fltVal = (FLOAT)(quality / 100.0);

        //Ignore result code.
        //  This returns E_FAIL if the named property is not in the bag.
        //TODO(bungeman) enumerate the properties,
        //  write and set hr iff property exists.
        piPropertybag->Write(1, &name, &value);
    }
    if (SUCCEEDED(hr)) {
        hr = piBitmapFrameEncode->Initialize(piPropertybag.get());
    }

    //Set the size of the frame.
    const UINT width = bitmap->width();
    const UINT height = bitmap->height();
    if (SUCCEEDED(hr)) {
        hr = piBitmapFrameEncode->SetSize(width, height);
    }

    //Set the pixel format of the frame.
    const WICPixelFormatGUID formatDesired = GUID_WICPixelFormat32bppBGRA;
    WICPixelFormatGUID formatGUID = formatDesired;
    if (SUCCEEDED(hr)) {
        hr = piBitmapFrameEncode->SetPixelFormat(&formatGUID);
    }
    if (SUCCEEDED(hr)) {
        //Be sure the image format is the one requested.
        hr = IsEqualGUID(formatGUID, formatDesired) ? S_OK : E_FAIL;
    }

    //Write the pixels into the frame.
    if (SUCCEEDED(hr)) {
        SkAutoLockPixels alp(*bitmap);
        const UINT stride = bitmap->rowBytes();
        hr = piBitmapFrameEncode->WritePixels(
            height
            , stride
            , stride * height
            , reinterpret_cast<BYTE*>(bitmap->getPixels()));
    }

    if (SUCCEEDED(hr)) {
        hr = piBitmapFrameEncode->Commit();
    }

    if (SUCCEEDED(hr)) {
        hr = piEncoder->Commit();
    }

    return SUCCEEDED(hr);
}

///////////////////////////////////////////////////////////////////////////////

#include "SkTRegistry.h"

static SkImageEncoder* sk_imageencoder_wic_factory(SkImageEncoder::Type t) {
    switch (t) {
        case SkImageEncoder::kBMP_Type:
        case SkImageEncoder::kICO_Type:
        case SkImageEncoder::kJPEG_Type:
        case SkImageEncoder::kPNG_Type:
            break;
        default:
            return NULL;
    }
    return SkNEW_ARGS(SkImageEncoder_WIC, (t));
}

static SkTRegistry<SkImageEncoder*, SkImageEncoder::Type> gEReg(sk_imageencoder_wic_factory);

static SkImageDecoder::Format get_format_wic(SkStream* stream) {
    SkImageDecoder::Format format;
    SkImageDecoder_WIC codec;
    if (!codec.decodeStream(stream, NULL, SkImageDecoder_WIC::kDecodeFormat_WICMode, &format)) {
        format = SkImageDecoder::kUnknown_Format;
    }
    return format;
}

static SkTRegistry<SkImageDecoder::Format, SkStream*> gFormatReg(get_format_wic);
