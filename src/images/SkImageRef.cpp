
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkImageRef.h"
#include "SkBitmap.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkImageDecoder.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkThread.h"

//#define DUMP_IMAGEREF_LIFECYCLE

///////////////////////////////////////////////////////////////////////////////

SkImageRef::SkImageRef(const SkImageInfo& info, SkStreamRewindable* stream,
                       int sampleSize, SkBaseMutex* mutex)
        : INHERITED(info, mutex), fErrorInDecoding(false)
{
    SkASSERT(stream);
    stream->ref();
    fStream = stream;
    fSampleSize = sampleSize;
    fDoDither = true;
    fPrev = fNext = NULL;
    fFactory = NULL;

    // This sets the colortype/alphatype to exactly match our info, so that this
    // can get communicated down to the codec.
    fBitmap.setConfig(info);

#ifdef DUMP_IMAGEREF_LIFECYCLE
    SkDebugf("add ImageRef %p [%d] data=%d\n",
              this, this->info().fColorType, (int)stream->getLength());
#endif
}

SkImageRef::~SkImageRef() {

#ifdef DUMP_IMAGEREF_LIFECYCLE
    SkDebugf("delete ImageRef %p [%d] data=%d\n",
              this, this->info().fColorType, (int)fStream->getLength());
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
        // what about colortables??????
        bitmap->setAlphaType(fBitmap.alphaType());
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

bool SkImageRef::onDecode(SkImageDecoder* codec, SkStreamRewindable* stream,
                          SkBitmap* bitmap, SkBitmap::Config config,
                          SkImageDecoder::Mode mode) {
    return codec->decode(stream, bitmap, config, mode);
}

bool SkImageRef::prepareBitmap(SkImageDecoder::Mode mode) {

    if (fErrorInDecoding) {
        return false;
    }

    if (NULL != fBitmap.getPixels() ||
            (SkBitmap::kNo_Config != fBitmap.config() &&
             SkImageDecoder::kDecodeBounds_Mode == mode)) {
        return true;
    }

    SkASSERT(fBitmap.getPixels() == NULL);

    if (!fStream->rewind()) {
        SkDEBUGF(("Failed to rewind SkImageRef stream!"));
        return false;
    }

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
        codec->setRequireUnpremultipliedColors(this->info().fAlphaType == kUnpremul_SkAlphaType);
        if (this->onDecode(codec, fStream, &fBitmap, fBitmap.config(), mode)) {
            if (kOpaque_SkAlphaType == fBitmap.alphaType()) {
                this->changeAlphaType(kOpaque_SkAlphaType);
            }
            SkASSERT(this->info() == fBitmap.info());
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

bool SkImageRef::onNewLockPixels(LockRec* rec) {
    if (NULL == fBitmap.getPixels()) {
        (void)this->prepareBitmap(SkImageDecoder::kDecodePixels_Mode);
    }

    if (NULL == fBitmap.getPixels()) {
        return false;
    }
    rec->fPixels = fBitmap.getPixels();
    rec->fColorTable = NULL;
    rec->fRowBytes = fBitmap.rowBytes();
    return true;
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

SkImageRef::SkImageRef(SkReadBuffer& buffer, SkBaseMutex* mutex)
        : INHERITED(buffer, mutex), fErrorInDecoding(false) {
    fSampleSize = buffer.readInt();
    fDoDither = buffer.readBool();

    size_t length = buffer.getArrayCount();
    if (buffer.validateAvailable(length)) {
        fStream = SkNEW_ARGS(SkMemoryStream, (length));
        buffer.readByteArray((void*)fStream->getMemoryBase(), length);
    } else {
        fStream = NULL;
    }

    fPrev = fNext = NULL;
    fFactory = NULL;

    // This sets the colortype/alphatype to exactly match our info, so that this
    // can get communicated down to the codec.
    fBitmap.setConfig(this->info());
}

void SkImageRef::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.writeInt(fSampleSize);
    buffer.writeBool(fDoDither);
    // FIXME: Consider moving this logic should go into writeStream itself.
    // writeStream currently has no other callers, so this may be fine for
    // now.
    if (!fStream->rewind()) {
        SkDEBUGF(("Failed to rewind SkImageRef stream!"));
        buffer.write32(0);
    } else {
        // FIXME: Handle getLength properly here. Perhaps this class should
        // take an SkStreamAsset.
        buffer.writeStream(fStream, fStream->getLength());
    }
}
