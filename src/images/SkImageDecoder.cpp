/* libs/graphics/images/SkImageDecoder.cpp
**
** Copyright 2006, The Android Open Source Project
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

#include "SkImageDecoder.h"
#include "SkBitmap.h"
#include "SkPixelRef.h"
#include "SkStream.h"
#include "SkTemplates.h"

static SkBitmap::Config gDeviceConfig = SkBitmap::kNo_Config;

SkBitmap::Config SkImageDecoder::GetDeviceConfig()
{
    return gDeviceConfig;
}

void SkImageDecoder::SetDeviceConfig(SkBitmap::Config config)
{
    gDeviceConfig = config;
}

///////////////////////////////////////////////////////////////////////////////

SkImageDecoder::SkImageDecoder()
    : fPeeker(NULL), fChooser(NULL), fAllocator(NULL), fSampleSize(1),
      fDitherImage(true) {
}

SkImageDecoder::~SkImageDecoder() {
    fPeeker->safeUnref();
    fChooser->safeUnref();
    fAllocator->safeUnref();
}

SkImageDecoder::Format SkImageDecoder::getFormat() const {
    return kUnknown_Format;
}

SkImageDecoder::Peeker* SkImageDecoder::setPeeker(Peeker* peeker) {
    SkRefCnt_SafeAssign(fPeeker, peeker);
    return peeker;
}

SkImageDecoder::Chooser* SkImageDecoder::setChooser(Chooser* chooser) {
    SkRefCnt_SafeAssign(fChooser, chooser);
    return chooser;
}

SkBitmap::Allocator* SkImageDecoder::setAllocator(SkBitmap::Allocator* alloc) {
    SkRefCnt_SafeAssign(fAllocator, alloc);
    return alloc;
}

void SkImageDecoder::setSampleSize(int size) {
    if (size < 1) {
        size = 1;
    }
    fSampleSize = size;
}

bool SkImageDecoder::chooseFromOneChoice(SkBitmap::Config config, int width,
                                         int height) const {
    Chooser* chooser = fChooser;

    if (NULL == chooser) {    // no chooser, we just say YES to decoding :)
        return true;
    }
    chooser->begin(1);
    chooser->inspect(0, config, width, height);
    return chooser->choose() == 0;
}

bool SkImageDecoder::allocPixelRef(SkBitmap* bitmap,
                                   SkColorTable* ctable) const {
    return bitmap->allocPixels(fAllocator, ctable);
}

///////////////////////////////////////////////////////////////////////////////

/*  Technically, this should be 342, since that is the cutoff point between
    an index and 32bit bitmap (they take equal ram), but since 32bit is almost
    always faster, I bump up the value a bit.
*/
#define MIN_SIZE_FOR_INDEX  (512)

/*  Return the "optimal" config for this bitmap. In this case, we just look to
    promote index bitmaps to full-color, since those are a little faster to
    draw (fewer memory lookups).

    Seems like we could expose this to the caller through some exising or new
    proxy object, allowing them to decide (after sniffing some aspect of the
    original bitmap) what config they really want.
 */
static SkBitmap::Config optimal_config(const SkBitmap& bm,
                                       SkBitmap::Config pref) {
    if (bm.config() != pref) {
        if (bm.config() == SkBitmap::kIndex8_Config) {
            Sk64 size64 = bm.getSize64();
            if (size64.is32()) {
                int32_t size = size64.get32();
                if (size < MIN_SIZE_FOR_INDEX) {
                    return SkBitmap::kARGB_8888_Config;
                }
            }
        }
    }
    return bm.config();
}

bool SkImageDecoder::decode(SkStream* stream, SkBitmap* bm,
                            SkBitmap::Config pref, Mode mode) {
    // pass a temporary bitmap, so that if we return false, we are assured of
    // leaving the caller's bitmap untouched.
    SkBitmap    tmp;

    // we reset this to false before calling onDecode
    fShouldCancelDecode = false;

    if (!this->onDecode(stream, &tmp, pref, mode)) {
        return false;
    }

    SkBitmap::Config c = optimal_config(tmp, pref);
    if (c != tmp.config()) {
        if (mode == kDecodeBounds_Mode) {
            tmp.setConfig(c, tmp.width(), tmp.height());
        } else {
            SkBitmap tmp2;
            if (tmp.copyTo(&tmp2, c, this->getAllocator())) {
                tmp.swap(tmp2);
            }
        }
    }
    bm->swap(tmp);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool SkImageDecoder::DecodeFile(const char file[], SkBitmap* bm,
                                SkBitmap::Config pref,  Mode mode) {
    SkASSERT(file);
    SkASSERT(bm);

    SkFILEStream    stream(file);
    if (stream.isValid()) {
        if (SkImageDecoder::DecodeStream(&stream, bm, pref, mode)) {
            bm->pixelRef()->setURI(file);
        }
        return true;
    }
    return false;
}

bool SkImageDecoder::DecodeMemory(const void* buffer, size_t size, SkBitmap* bm,
                                  SkBitmap::Config pref, Mode mode) {
    if (0 == size) {
        return false;
    }
    SkASSERT(buffer);

    SkMemoryStream  stream(buffer, size);
    return SkImageDecoder::DecodeStream(&stream, bm, pref, mode);
}

bool SkImageDecoder::DecodeStream(SkStream* stream, SkBitmap* bm,
                                  SkBitmap::Config pref, Mode mode) {
    SkASSERT(stream);
    SkASSERT(bm);

    bool success = false;
    SkImageDecoder* codec = SkImageDecoder::Factory(stream);

    if (NULL != codec) {
        success = codec->decode(stream, bm, pref, mode);
        delete codec;
    }
    return success;
}

