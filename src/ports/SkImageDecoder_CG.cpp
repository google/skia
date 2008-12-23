/* Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include <Carbon/Carbon.h>
#include "SkImageDecoder.h"
#include "SkMovie.h"
#include "SkStream.h"
#include "SkTemplates.h"

static void malloc_release_proc(void* info, const void* data, size_t size) {
    sk_free(info);
}

static CGDataProviderRef SkStreamToDataProvider(SkStream* stream) {
    // TODO: use callbacks, so we don't have to load all the data into RAM
    size_t len = stream->getLength();
    void* data = sk_malloc_throw(len);
    stream->read(data, len);
    
    return CGDataProviderCreateWithData(data, data, len, malloc_release_proc);
}

static CGImageSourceRef SkStreamToCGImageSource(SkStream* stream) {
    CGDataProviderRef data = SkStreamToDataProvider(stream);
    CGImageSourceRef imageSrc = CGImageSourceCreateWithDataProvider(data, 0);
    CGDataProviderRelease(data);
    return imageSrc;
}

class SkImageDecoder_CG : public SkImageDecoder {
protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm,
                          SkBitmap::Config pref, Mode);
};

#define BITMAP_INFO (kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast)

bool SkImageDecoder_CG::onDecode(SkStream* stream, SkBitmap* bm,
                                 SkBitmap::Config pref, Mode mode) {
    CGImageSourceRef imageSrc = SkStreamToCGImageSource(stream);

    if (NULL == imageSrc) {
        return false;
    }

    SkAutoTCallVProc<const void, CFRelease> arsrc(imageSrc);
    
    CGImageRef image = CGImageSourceCreateImageAtIndex(imageSrc, 0, NULL);
    if (NULL == image) {
        return false;
    }
    
    SkAutoTCallVProc<CGImage, CGImageRelease> arimage(image);
    
    const int width = CGImageGetWidth(image);
    const int height = CGImageGetHeight(image);
    bm->setConfig(SkBitmap::kARGB_8888_Config, width, height);
    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        return true;
    }
    
    if (!this->allocPixelRef(bm, NULL)) {
        return false;
    }
    
    bm->lockPixels();
    bm->eraseColor(0);

    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGContextRef cg = CGBitmapContextCreate(bm->getPixels(), width, height,
                                            8, bm->rowBytes(), cs, BITMAP_INFO);
    CGContextDrawImage(cg, CGRectMake(0, 0, width, height), image);
    CGContextRelease(cg);
    CGColorSpaceRelease(cs);

    bm->unlockPixels();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

SkImageDecoder* SkImageDecoder::Factory(SkStream* stream) {
    return SkNEW(SkImageDecoder_CG);
}

bool SkImageDecoder::SupportsFormat(Format format) {
    return true;
}

/////////////////////////////////////////////////////////////////////////

SkMovie* SkMovie::DecodeStream(SkStream* stream) {
    return NULL;
}

/////////////////////////////////////////////////////////////////////////

#ifdef SK_SUPPORT_IMAGE_ENCODE

SkImageEncoder* SkImageEncoder::Create(Type t) {
#if 0
    switch (t) {
        case kJPEG_Type:
            return SkImageEncoder_JPEG_Factory();
        case kPNG_Type:
            return SkImageEncoder_PNG_Factory();
        default:
            return NULL;
    }
#else
    return NULL;
#endif
}

#endif
