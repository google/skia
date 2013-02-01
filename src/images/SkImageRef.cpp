
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkImageRef.h"
#include "SkBitmap.h"
#include "SkFlattenableBuffers.h"
#include "SkImageDecoder.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkThread.h"

//#define DUMP_IMAGEREF_LIFECYCLE


///////////////////////////////////////////////////////////////////////////////

SkImageRef::SkImageRef(SkStream* stream, SkBitmap::Config config,
                       int sampleSize, SkBaseMutex* mutex)
        : SkPixelRef(mutex), fErrorInDecoding(false) {
    SkASSERT(stream);
    stream->ref();
    fStream = stream;
    fConfig = config;
    fSampleSize = sampleSize;
    fDoDither = true;
    fPrev = fNext = NULL;
    fFactory = NULL;

#ifdef DUMP_IMAGEREF_LIFECYCLE
    SkDebugf("add ImageRef %p [%d] data=%d\n",
              this, config, (int)stream->getLength());
#endif
}

SkImageRef::~SkImageRef() {

#ifdef DUMP_IMAGEREF_LIFECYCLE
    SkDebugf("delete ImageRef %p [%d] data=%d\n",
              this, fConfig, (int)fStream->getLength());
#endif

    fStream->unref();
    SkSafeUnref(fFactory);
}

bool SkImageRef::getInfo(SkBitmap* bitmap) {
    SkAutoMutexAcquire ac(this->mutex());

    if (!this->prepareBitmap(SkImageDecoder::kDecodeBounds_Mode)) {
        return false;
    }

    SkASSERT(SkBitmap::kNo_Config != fBitmap.config());
    if (bitmap) {
        bitmap->setConfig(fBitmap.config(), fBitmap.width(), fBitmap.height());
    }
    return true;
}

bool SkImageRef::isOpaque(SkBitmap* bitmap) {
    if (bitmap && bitmap->pixelRef() == this) {
        bitmap->lockPixels();
        bitmap->setIsOpaque(fBitmap.isOpaque());
        bitmap->unlockPixels();
        return true;
    }
    return false;
}

SkImageDecoderFactory* SkImageRef::setDecoderFactory(
                                                SkImageDecoderFactory* fact) {
    SkRefCnt_SafeAssign(fFactory, fact);
    return fact;
}

///////////////////////////////////////////////////////////////////////////////

bool SkImageRef::onDecode(SkImageDecoder* codec, SkStream* stream,
                          SkBitmap* bitmap, SkBitmap::Config config,
                          SkImageDecoder::Mode mode) {
    return codec->decode(stream, bitmap, config, mode);
}

bool SkImageRef::prepareBitmap(SkImageDecoder::Mode mode) {

    if (fErrorInDecoding) {
        return false;
    }

    /*  As soon as we really know our config, we record it, so that on
        subsequent calls to the codec, we are sure we will always get the same
        result.
    */
    if (SkBitmap::kNo_Config != fBitmap.config()) {
        fConfig = fBitmap.config();
    }

    if (NULL != fBitmap.getPixels() ||
            (SkBitmap::kNo_Config != fBitmap.config() &&
             SkImageDecoder::kDecodeBounds_Mode == mode)) {
        return true;
    }

    SkASSERT(fBitmap.getPixels() == NULL);

    fStream->rewind();

    SkImageDecoder* codec;
    if (fFactory) {
        codec = fFactory->newDecoder(fStream);
    } else {
        codec = SkImageDecoder::Factory(fStream);
    }

    if (codec) {
        SkAutoTDelete<SkImageDecoder> ad(codec);

        codec->setSampleSize(fSampleSize);
        codec->setDitherImage(fDoDither);
        if (this->onDecode(codec, fStream, &fBitmap, fConfig, mode)) {
            return true;
        }
    }

#ifdef DUMP_IMAGEREF_LIFECYCLE
    if (NULL == codec) {
        SkDebugf("--- ImageRef: <%s> failed to find codec\n", this->getURI());
    } else {
        SkDebugf("--- ImageRef: <%s> failed in codec for %d mode\n",
                 this->getURI(), mode);
    }
#endif
    fErrorInDecoding = true;
    fBitmap.reset();
    return false;
}

void* SkImageRef::onLockPixels(SkColorTable** ct) {
    if (NULL == fBitmap.getPixels()) {
        (void)this->prepareBitmap(SkImageDecoder::kDecodePixels_Mode);
    }

    if (ct) {
        *ct = fBitmap.getColorTable();
    }
    return fBitmap.getPixels();
}

size_t SkImageRef::ramUsed() const {
    size_t size = 0;

    if (fBitmap.getPixels()) {
        size = fBitmap.getSize();
        if (fBitmap.getColorTable()) {
            size += fBitmap.getColorTable()->count() * sizeof(SkPMColor);
        }
    }
    return size;
}

///////////////////////////////////////////////////////////////////////////////

SkImageRef::SkImageRef(SkFlattenableReadBuffer& buffer, SkBaseMutex* mutex)
        : INHERITED(buffer, mutex), fErrorInDecoding(false) {
    fConfig = (SkBitmap::Config)buffer.readUInt();
    fSampleSize = buffer.readInt();
    fDoDither = buffer.readBool();

    size_t length = buffer.getArrayCount();
    fStream = SkNEW_ARGS(SkMemoryStream, (length));
    buffer.readByteArray((void*)fStream->getMemoryBase());

    fPrev = fNext = NULL;
    fFactory = NULL;
}

void SkImageRef::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.writeUInt(fConfig);
    buffer.writeInt(fSampleSize);
    buffer.writeBool(fDoDither);
    fStream->rewind();
    buffer.writeStream(fStream, fStream->getLength());
}
