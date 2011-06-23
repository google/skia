/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
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

class SkImageDecoder_WIC : public SkImageDecoder {
protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm, Mode mode);
};

bool SkImageDecoder_WIC::onDecode(SkStream* stream, SkBitmap* bm, Mode mode) {
    //Initialize COM.
    AutoCoInitialize scopedCo;
    HRESULT hr = scopedCo.getHR();
    
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
        if (SkImageDecoder::kDecodeBounds_Mode == mode) {
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
    
    if (SUCCEEDED(hr)) {
        hr = piFormatConverter->Initialize(
            piBitmapSourceOriginal.get()      //Input bitmap to convert
            , GUID_WICPixelFormat32bppPBGRA   //Destination pixel format
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
        bm->lockPixels();
        bm->eraseColor(0);
        const int stride = bm->rowBytes();
        hr = piBitmapSourceConverted->CopyPixels(
            NULL,                             //Get all the pixels
            stride,
            stride * height,
            reinterpret_cast<BYTE *>(bm->getPixels())
        );
        bm->unlockPixels();
    }
    
    return SUCCEEDED(hr);
}

/////////////////////////////////////////////////////////////////////////

SkImageDecoder* SkImageDecoder::Factory(SkStream* stream) {
    return SkNEW(SkImageDecoder_WIC);
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
                                , const SkBitmap& bm
                                , int quality)
{
    GUID type;
    switch (fType) {
        case kJPEG_Type:
            type = GUID_ContainerFormatJpeg;
            break;
        case kPNG_Type:
            type = GUID_ContainerFormatPng;
            break;
        default:
            return false;
    }

    //Initialize COM.
    AutoCoInitialize scopedCo;
    HRESULT hr = scopedCo.getHR();
    
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
    const UINT width = bm.width();
    const UINT height = bm.height();
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
        hr = piBitmapFrameEncode->WritePixels(
            height
            , bm.rowBytes()
            , bm.rowBytes()*height
            , reinterpret_cast<BYTE*>(bm.getPixels()));
    }
    
    if (SUCCEEDED(hr)) {
        hr = piBitmapFrameEncode->Commit();
    }
    
    if (SUCCEEDED(hr)) {
        hr = piEncoder->Commit();
    }
    
    return SUCCEEDED(hr);
}

SkImageEncoder* SkImageEncoder::Create(Type t) {
    switch (t) {
        case kJPEG_Type:
        case kPNG_Type:
            break;
        default:
            return NULL;
    }
    return SkNEW_ARGS(SkImageEncoder_WIC, (t));
}

