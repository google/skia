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

SkImageDecoder::SkImageDecoder()
{
}

SkImageDecoder::~SkImageDecoder()
{
}

bool SkImageDecoder::DecodeFile(const char file[], SkBitmap* bm, SkBitmap::Config pref)
{
	SkASSERT(file);
	SkASSERT(bm);

	SkFILEStream	stream(file);
	return stream.isValid() && SkImageDecoder::DecodeStream(&stream, bm, pref);
}

bool SkImageDecoder::DecodeMemory(const void* buffer, size_t size, SkBitmap* bm, SkBitmap::Config pref)
{
	SkASSERT(buffer);

	SkMemoryStream	stream(buffer, size);
	return size && SkImageDecoder::DecodeStream(&stream, bm, pref);
}

bool SkImageDecoder::DecodeURL(const char url[], SkBitmap* bm, SkBitmap::Config pref) 
{
	SkASSERT(url);
	SkASSERT(bm);

	SkURLStream	stream(url);
	return SkImageDecoder::DecodeStream(&stream, bm, pref);
}

bool SkImageDecoder::DecodeStream(SkStream* stream, SkBitmap* bm, SkBitmap::Config pref)
{
	SkASSERT(stream);
	SkASSERT(bm);

	SkImageDecoder* codec = SkImageDecoder::Factory(stream);
    if (codec)
    {
        SkAutoTDelete<SkImageDecoder>	ad(codec);
        SkBitmap						tmp;

        if (codec->onDecode(stream, &tmp, pref))
        {
            /*	We operate on a tmp bitmap until we know we succeed. This way
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
	SkFILEWStream	stream(file);
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

	SkAutoMutexAcquire	ac(globals.fMutex);
	SkBitmapRef::Rec*	rec = globals.fCache;

	while (rec)
	{
		if (rec->fPath.equals(path))
			return SkNEW_ARGS(SkBitmapRef, (rec));
		rec = rec->fNext;
	}

	if (forceDecode)
	{
		SkAutoTDelete<Rec>	autoRec(rec = SkNEW_ARGS(Rec, (true)));
		
		if (SkImageDecoder::DecodeFile(path, &rec->fBM))
		{
			rec->fPath.set(path);
			rec->fRefCnt = 0;
			rec->fNext = globals.fCache;
			globals.fCache = rec;

			(void)autoRec.detach();	// detach from autoRec, so we don't delete it
			return SkNEW_ARGS(SkBitmapRef, (rec));
		}
	}
	return nil;
}

SkBitmapRef* SkBitmapRef::DecodeMemory(const void* bytes, size_t len)
{
    SkBitmapRef::Rec* rec;
    SkAutoTDelete<Rec>  autoRec(rec = SkNEW_ARGS(Rec, (false)));

    if (SkImageDecoder::DecodeMemory(bytes, len, &rec->fBM))
    {
        rec->fRefCnt = 0;
        (void)autoRec.detach();
        return SkNEW_ARGS(SkBitmapRef, (rec));
    }
    return nil;
}

SkBitmapRef* SkBitmapRef::DecodeStream(SkStream* stream)
{
    SkBitmapRef::Rec* rec;
    SkAutoTDelete<Rec>  autoRec(rec = SkNEW_ARGS(Rec, (false)));

    if (SkImageDecoder::DecodeStream(stream, &rec->fBM))
    {
        rec->fRefCnt = 0;
        (void)autoRec.detach();
        return SkNEW_ARGS(SkBitmapRef, (rec));
    }
    return nil;
}    

