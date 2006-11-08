/* libs/graphics/images/SkImageDecoder.cpp
**
** Copyright 2006, Google Inc.
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

///////////////////////////////////////////////////////////////////////////////////////////

SkImageDecoder::SkImageDecoder() : fPeeker(NULL), fChooser(NULL)
{
}

SkImageDecoder::~SkImageDecoder()
{
    fPeeker->safeUnref();
    fChooser->safeUnref();
}

SkImageDecoder::Peeker* SkImageDecoder::setPeeker(Peeker* peeker)
{
    SkRefCnt_SafeAssign(fPeeker, peeker);
    return peeker;
}

SkImageDecoder::Chooser* SkImageDecoder::setChooser(Chooser* chooser)
{
    SkRefCnt_SafeAssign(fChooser, chooser);
    return chooser;
}

bool SkImageDecoder::chooseFromOneChoice(SkBitmap::Config config, int width, int height) const
{
    Chooser* chooser = fChooser;

    if (NULL == chooser)    // no chooser, we just say YES to decoding :)
        return true;

    chooser->begin(1);
    chooser->inspect(0, config, width, height);
    return chooser->choose() == 0;
}

bool SkImageDecoder::DecodeFile(const char file[], SkBitmap* bm, SkBitmap::Config pref)
{
    SkASSERT(file);
    SkASSERT(bm);

    SkFILEStream    stream(file);
    return stream.isValid() && SkImageDecoder::DecodeStream(&stream, bm, pref);
}

bool SkImageDecoder::DecodeMemory(const void* buffer, size_t size, SkBitmap* bm, SkBitmap::Config pref)
{
    SkASSERT(buffer);

    SkMemoryStream  stream(buffer, size);
    return size && SkImageDecoder::DecodeStream(&stream, bm, pref);
}

bool SkImageDecoder::DecodeURL(const char url[], SkBitmap* bm, SkBitmap::Config pref) 
{
    SkASSERT(url);
    SkASSERT(bm);

    SkURLStream stream(url);
    return SkImageDecoder::DecodeStream(&stream, bm, pref);
}

bool SkImageDecoder::DecodeStream(SkStream* stream, SkBitmap* bm, SkBitmap::Config pref)
{
    SkASSERT(stream);
    SkASSERT(bm);

    SkImageDecoder* codec = SkImageDecoder::Factory(stream);
    if (codec)
    {
        SkAutoTDelete<SkImageDecoder>   ad(codec);
        SkBitmap                        tmp;

        if (codec->onDecode(stream, &tmp, pref))
        {
            /*  We operate on a tmp bitmap until we know we succeed. This way
                we're sure we don't change the caller's bitmap and then later
                return false. Returning false must mean that their parameter
                is unchanged.
            */
            bm->swap(tmp);
            return true;
        }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////

#ifdef SK_SUPPORT_IMAGE_ENCODE

SkImageEncoder::~SkImageEncoder()
{
}

bool SkImageEncoder::encodeStream(SkWStream* stream, const SkBitmap& bm, int quality)
{
    quality = SkMin32(100, SkMax32(0, quality));
    return this->onEncode(stream, bm, quality);
}

bool SkImageEncoder::encodeFile(const char file[], const SkBitmap& bm, int quality)
{
    quality = SkMin32(100, SkMax32(0, quality));
    SkFILEWStream   stream(file);
    return this->onEncode(&stream, bm, quality);
}

extern SkImageEncoder* sk_create_jpeg_encoder();
extern SkImageEncoder* sk_create_png_encoder();

SkImageEncoder* SkImageEncoder::Create(Type t)
{
    if (kJPEG_Type == t)
        return sk_create_jpeg_encoder();
    if (kPNG_Type == t)
        return sk_create_png_encoder();
    return nil;
}

#endif

///////////////////////////////////////////////////////////////////////////////////

#include "SkBitmapRefPriv.h"

SkBitmapRef* SkBitmapRef::DecodeFile(const char path[], bool forceDecode)
{
    SkBitmapRef_Globals& globals = *(SkBitmapRef_Globals*)SkGlobals::Find(kBitmapRef_GlobalsTag,
                                                                          SkBitmapRef_Globals::Create);

    SkAutoMutexAcquire  ac(globals.fMutex);
    SkBitmapRef::Rec*   rec = globals.fCache;

    while (rec)
    {
        if (rec->fPath.equals(path))
            return SkNEW_ARGS(SkBitmapRef, (rec));
        rec = rec->fNext;
    }

    if (forceDecode)
    {
        SkAutoTDelete<Rec>  autoRec(rec = SkNEW_ARGS(Rec, (true)));
        
        if (SkImageDecoder::DecodeFile(path, &rec->fBM))
        {
            rec->fPath.set(path);
            rec->fRefCnt = 0;
            rec->fNext = globals.fCache;
            globals.fCache = rec;

            (void)autoRec.detach(); // detach from autoRec, so we don't delete it
            return SkNEW_ARGS(SkBitmapRef, (rec));
        }
    }
    return nil;
}

SkBitmapRef* SkBitmapRef::DecodeMemory(const void* bytes, size_t len, const char* name)
{
    SkBitmapRef_Globals& globals = *(SkBitmapRef_Globals*)SkGlobals::Find(kBitmapRef_GlobalsTag,
                                                                          SkBitmapRef_Globals::Create);

    SkAutoMutexAcquire  ac(globals.fMutex);
    SkBitmapRef::Rec*   rec = globals.fCache;

    if (name)
    {
        while (rec)
        {
            if (rec->fPath.equals(name))
                return SkNEW_ARGS(SkBitmapRef, (rec));
            rec = rec->fNext;
        }
    }
    
    SkAutoTDelete<Rec>  autoRec(rec = SkNEW_ARGS(Rec, (true)));

    if (SkImageDecoder::DecodeMemory(bytes, len, &rec->fBM))
    {
        if (name)
            rec->fPath.set(name);
        rec->fRefCnt = 0;
        rec->fNext = globals.fCache;
        globals.fCache = rec;
        
        (void)autoRec.detach();
        return SkNEW_ARGS(SkBitmapRef, (rec));
    }
    return nil;
}

SkBitmapRef* SkBitmapRef::DecodeStream(SkStream* stream, const char* name)
{
    SkBitmapRef_Globals& globals = *(SkBitmapRef_Globals*)SkGlobals::Find(kBitmapRef_GlobalsTag,
                                                                          SkBitmapRef_Globals::Create);

    SkAutoMutexAcquire  ac(globals.fMutex);
    SkBitmapRef::Rec*   rec = globals.fCache;

    if (name)
    {
        while (rec)
        {
            if (rec->fPath.equals(name))
                return SkNEW_ARGS(SkBitmapRef, (rec));
            rec = rec->fNext;
        }
    }
    
    SkAutoTDelete<Rec>  autoRec(rec = SkNEW_ARGS(Rec, (true)));

    if (SkImageDecoder::DecodeStream(stream, &rec->fBM))
    {
        if (name)
            rec->fPath.set(name);
        rec->fRefCnt = 0;
        rec->fNext = globals.fCache;
        globals.fCache = rec;
        
        (void)autoRec.detach();
        return SkNEW_ARGS(SkBitmapRef, (rec));
    }
    return nil;
}    

