#include "SkImageDecoder.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkMath.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkUtils.h"

extern void ValidateHeap();

class SkPVJPEGImageDecoder : public SkImageDecoder {
protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm,
                          SkBitmap::Config pref, Mode);

private:
    enum {
        STORAGE_SIZE = 8 * 1024
    };
    char    fStorage[STORAGE_SIZE];
};

SkImageDecoder* SkImageDecoder_PVJPEG_Factory(SkStream* stream)
{
    return SkNEW(SkPVJPEGImageDecoder);
}

#include "pvjpgdecoderinterface.h"
#include "pvjpgdecoder_factory.h"

class AutoPVDelete {
public:
    AutoPVDelete(PVJpgDecoderInterface* codec) : fCodec(codec) {}
    ~AutoPVDelete() {
        fCodec->Reset();
        PVJpgDecoderFactory::DeletePVJpgDecoder(fCodec);
    }
private:    
    PVJpgDecoderInterface* fCodec;
};

class MyObserver : public MPVJpegDecObserver {
public:
    MyObserver() : fCount(0) {}
    ~MyObserver() {
        if (fCount != 0) {
            SkDebugf("--- pvjpeg left %d allocations\n", fCount);
        }
    }

	virtual void allocateBuffer(uint8* &buffer, int32 buffersize) {
        ++fCount;
        // we double the allocation to work around bug when height is odd
        buffer = (uint8*)sk_malloc_throw(buffersize << 1);
        SkDebugf("---  pvjpeg alloc [%d] %d addr=%p\n", fCount, buffersize, buffer);
    }
    
	virtual void deallocateBuffer(uint8 *buffer) {
        SkDebugf("--- pvjpeg free [%d] addr=%p\n", fCount, buffer);
        --fCount;
        sk_free(buffer);
    }

private:
    int fCount;
};

static void check_status(TPvJpgDecStatus status) {
    if (TPVJPGDEC_SUCCESS != status) {
        SkDEBUGF(("--- pvjpeg status %d\n", status));
    }
}

static bool getFrame(PVJpgDecoderInterface* codec, SkBitmap* bitmap,
                     SkBitmap::Config prefConfig, SkImageDecoder::Mode mode) {
    TPvJpgDecInfo info;
    TPvJpgDecStatus status = codec->GetInfo(&info);
    if (status != TPVJPGDEC_SUCCESS)
        return false;

    int width = info.iWidth[0];
    int height = info.iHeight[0];

    bitmap->setConfig(SkBitmap::kRGB_565_Config, width, height);
    bitmap->setIsOpaque(true);

    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        return true;
    }
    
    SkASSERT(info.iNumComponent == 3);

    TPvJpgDecOutputFmt  format;
    format.iColorFormat = TPV_COLORFMT_RGB16;
    format.iCropped.topLeftX = 0;
    format.iCropped.topLeftY = 0;
    format.iCropped.bottomRightX = width - 1;
    format.iCropped.bottomRightY = height - 1;
    format.iOutputPitch = bitmap->rowBytes() >> 1;
    status = codec->SetOutput(&format);
    if (status != TPVJPGDEC_SUCCESS) {
        SkDebugf("--- PV SetOutput failed %d\n", status);
        return false;
    }

    TPvJpgDecFrame frame;
    uint8*         ptrs[3];
    int32          widths[3], heights[3];
    bzero(ptrs, sizeof(ptrs));
    frame.ptr = ptrs;
    frame.iWidth = widths;
    frame.iHeight = heights;
    
    status = codec->GetFrame(&frame);
    if (status != TPVJPGDEC_SUCCESS) {
        SkDebugf("--- PV GetFrame failed %d\n", status);
        return false;
    }

    bitmap->allocPixels();
    memcpy(bitmap->getPixels(), ptrs[0], bitmap->getSize());
    return true;
}

class OsclCleanupper {
public:
    OsclCleanupper() {
        OsclBase::Init();
        OsclErrorTrap::Init();
        OsclMem::Init();
    }
    ~OsclCleanupper() {
        OsclMem::Cleanup();
        OsclErrorTrap::Cleanup();
        OsclBase::Cleanup();
    }
};

bool SkPVJPEGImageDecoder::onDecode(SkStream* stream, SkBitmap* decodedBitmap,
                                    SkBitmap::Config prefConfig, Mode mode)
{
    // do I need this guy?
    OsclCleanupper oc;
    
    PVJpgDecoderInterface*  codec = PVJpgDecoderFactory::CreatePVJpgDecoder();
    TPvJpgDecStatus         status = codec->Init();
    check_status(status);

    MyObserver      observer;   // must create before autopvdelete
    AutoPVDelete    ad(codec);
    
    status = codec->SetObserver(&observer);
    check_status(status);
    
    char*   storage = fStorage;
    int32   bytesInStorage = 0;
    for (;;)
    {
        int32 bytesRead = stream->read(storage + bytesInStorage,
                                       STORAGE_SIZE - bytesInStorage);
        if (bytesRead <= 0) {
            SkDEBUGF(("SkPVJPEGImageDecoder: stream read returned %d\n", bytesRead));
            return false;
        }
        
        // update bytesInStorage to account for the read()
        bytesInStorage += bytesRead;
        SkASSERT(bytesInStorage <= STORAGE_SIZE);
        
        // now call Decode to eat some of the bytes
        int32 consumed = bytesInStorage;
        status = codec->Decode((uint8*)storage, &consumed);

        SkASSERT(bytesInStorage >= consumed);
        bytesInStorage -= consumed;
        // now bytesInStorage is the remaining unread bytes
        if (bytesInStorage > 0) { // slide the leftovers to the beginning
            SkASSERT(storage == fStorage);
            SkASSERT(consumed >= 0 && bytesInStorage >= 0);
            SkASSERT((size_t)(consumed + bytesInStorage) <= sizeof(fStorage));
            SkASSERT(sizeof(fStorage) == STORAGE_SIZE);
       //     SkDebugf("-- memmov srcOffset=%d, numBytes=%d\n", consumed, bytesInStorage);
            memmove(storage, storage + consumed, bytesInStorage);
        }
        
        switch (status) {
        case TPVJPGDEC_SUCCESS:
            SkDEBUGF(("SkPVJPEGImageDecoder::Decode returned success?\n");)
            return false;
        case TPVJPGDEC_FRAME_READY:
        case TPVJPGDEC_DONE:
            return getFrame(codec, decodedBitmap, prefConfig, mode);
        case TPVJPGDEC_FAIL:
        case TPVJPGDEC_INVALID_MEMORY:
        case TPVJPGDEC_INVALID_PARAMS:
        case TPVJPGDEC_NO_IMAGE_DATA:
            SkDEBUGF(("SkPVJPEGImageDecoder: failed to decode err=%d\n", status);)
            return false;
        case TPVJPGDEC_WAITING_FOR_INPUT:
            break;  // loop around and eat more from the stream
        }
    }
    return false;
}

