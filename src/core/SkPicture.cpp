
/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPictureFlat.h"
#include "SkPictureData.h"
#include "SkPicturePlayback.h"
#include "SkPictureRecord.h"
#include "SkPictureRecorder.h"
#include "SkPictureStateTree.h"

#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkChunkAlloc.h"
#include "SkDrawPictureCallback.h"
#include "SkPaintPriv.h"
#include "SkPicture.h"
#include "SkRecordAnalysis.h"
#include "SkRegion.h"
#include "SkStream.h"
#include "SkTDArray.h"
#include "SkTSearch.h"
#include "SkTime.h"

#include "SkReader32.h"
#include "SkWriter32.h"
#include "SkRTree.h"
#include "SkBBoxHierarchyRecord.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

#include "SkRecord.h"
#include "SkRecordDraw.h"
#include "SkRecorder.h"

template <typename T> int SafeCount(const T* obj) {
    return obj ? obj->count() : 0;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_SUPPORT_LEGACY_DEFAULT_PICTURE_CTOR
// fRecord OK
SkPicture::SkPicture()
    : fWidth(0)
    , fHeight(0)
    , fRecordWillPlayBackBitmaps(false) {
    this->needsNewGenID();
}
#endif

// fRecord OK
SkPicture::SkPicture(int width, int height,
                     const SkPictureRecord& record,
                     bool deepCopyOps)
    : fWidth(width)
    , fHeight(height)
    , fRecordWillPlayBackBitmaps(false) {
    this->needsNewGenID();

    SkPictInfo info;
    this->createHeader(&info);
    fData.reset(SkNEW_ARGS(SkPictureData, (record, info, deepCopyOps)));
}

// Create an SkPictureData-backed SkPicture from an SkRecord.
// This for compatibility with serialization code only.  This is not cheap.
static SkPicture* backport(const SkRecord& src, int width, int height) {
    SkPictureRecorder recorder;
    SkRecordDraw(src, recorder.beginRecording(width, height), NULL/*bbh*/, NULL/*callback*/);
    return recorder.endRecording();
}

// fRecord OK
SkPicture::~SkPicture() {
    this->callDeletionListeners();
}

#ifdef SK_SUPPORT_LEGACY_PICTURE_CLONE
// fRecord TODO, fix by deleting this method
SkPicture* SkPicture::clone() const {
#ifdef SK_PICTURE_CLONE_NOOP
    return SkRef(const_cast<SkPicture*>(this));
#else
    SkAutoTDelete<SkPictureData> newData;

    if (fData.get()) {
        SkPictCopyInfo copyInfo;

        int paintCount = SafeCount(fData->fPaints);

        /* The alternative to doing this is to have a clone method on the paint and have it
         * make the deep copy of its internal structures as needed. The holdup to doing
         * that is at this point we would need to pass the SkBitmapHeap so that we don't
         * unnecessarily flatten the pixels in a bitmap shader.
         */
        copyInfo.paintData.setCount(paintCount);

        /* Use an SkBitmapHeap to avoid flattening bitmaps in shaders. If there already is
         * one, use it. If this SkPictureData was created from a stream, fBitmapHeap
         * will be NULL, so create a new one.
         */
        if (fData->fBitmapHeap.get() == NULL) {
            // FIXME: Put this on the stack inside SkPicture::clone.
            SkBitmapHeap* heap = SkNEW(SkBitmapHeap);
            copyInfo.controller.setBitmapStorage(heap);
            heap->unref();
        } else {
            copyInfo.controller.setBitmapStorage(fData->fBitmapHeap);
        }

        SkDEBUGCODE(int heapSize = SafeCount(fData->fBitmapHeap.get());)
        for (int i = 0; i < paintCount; i++) {
            if (NeedsDeepCopy(fData->fPaints->at(i))) {
                copyInfo.paintData[i] =
                    SkFlatData::Create<SkPaint::FlatteningTraits>(&copyInfo.controller,
                    fData->fPaints->at(i), 0);

            } else {
                // this is our sentinel, which we use in the unflatten loop
                copyInfo.paintData[i] = NULL;
            }
        }
        SkASSERT(SafeCount(fData->fBitmapHeap.get()) == heapSize);

        // needed to create typeface playback
        copyInfo.controller.setupPlaybacks();

        newData.reset(SkNEW_ARGS(SkPictureData, (*fData, &copyInfo)));
    }

    SkPicture* clone = SkNEW_ARGS(SkPicture, (newData.detach(), fWidth, fHeight));
    clone->fRecordWillPlayBackBitmaps = fRecordWillPlayBackBitmaps;
    clone->fUniqueID = this->uniqueID(); // need to call method to ensure != 0

    return clone;
#endif
}
#endif//SK_SUPPORT_LEGACY_PICTURE_CLONE

// fRecord OK
void SkPicture::EXPERIMENTAL_addAccelData(const SkPicture::AccelData* data) const {
    fAccelData.reset(SkRef(data));
}

// fRecord OK
const SkPicture::AccelData* SkPicture::EXPERIMENTAL_getAccelData(
        SkPicture::AccelData::Key key) const {
    if (NULL != fAccelData.get() && fAccelData->getKey() == key) {
        return fAccelData.get();
    }
    return NULL;
}

// fRecord OK
SkPicture::AccelData::Domain SkPicture::AccelData::GenerateDomain() {
    static int32_t gNextID = 0;

    int32_t id = sk_atomic_inc(&gNextID);
    if (id >= 1 << (8 * sizeof(Domain))) {
        SK_CRASH();
    }

    return static_cast<Domain>(id);
}

///////////////////////////////////////////////////////////////////////////////

uint32_t SkPicture::OperationList::offset(int index) const {
    SkASSERT(index < fOps.count());
    return ((SkPictureStateTree::Draw*)fOps[index])->fOffset;
}

const SkMatrix& SkPicture::OperationList::matrix(int index) const {
    SkASSERT(index < fOps.count());
    return *((SkPictureStateTree::Draw*)fOps[index])->fMatrix;
}

// fRecord TODO
const SkPicture::OperationList* SkPicture::EXPERIMENTAL_getActiveOps(const SkIRect& queryRect) const {
    SkASSERT(NULL != fData.get());
    if (NULL != fData.get()) {
        return fData->getActiveOps(queryRect);
    }
    return NULL;
}

// fRecord OK
void SkPicture::draw(SkCanvas* canvas, SkDrawPictureCallback* callback) const {
    SkASSERT(NULL != canvas);
    SkASSERT(NULL != fData.get() || NULL != fRecord.get());

    if (NULL != fData.get()) {
        SkPicturePlayback playback(this);
        playback.draw(canvas, callback);
    }
    if (NULL != fRecord.get()) {
        SkRecordDraw(*fRecord, canvas, fBBH.get(), callback);
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkStream.h"

static const char kMagic[] = { 's', 'k', 'i', 'a', 'p', 'i', 'c', 't' };

// fRecord OK
bool SkPicture::IsValidPictInfo(const SkPictInfo& info) {
    if (0 != memcmp(info.fMagic, kMagic, sizeof(kMagic))) {
        return false;
    }

    if (info.fVersion < MIN_PICTURE_VERSION ||
        info.fVersion > CURRENT_PICTURE_VERSION) {
        return false;
    }

    return true;
}

// fRecord OK
bool SkPicture::InternalOnly_StreamIsSKP(SkStream* stream, SkPictInfo* pInfo) {
    if (NULL == stream) {
        return false;
    }

    // Check magic bytes.
    SkPictInfo info;
    SkASSERT(sizeof(kMagic) == sizeof(info.fMagic));
    if (!stream->read(&info, sizeof(info)) || !IsValidPictInfo(info)) {
        return false;
    }

    if (pInfo != NULL) {
        *pInfo = info;
    }
    return true;
}

// fRecord OK
bool SkPicture::InternalOnly_BufferIsSKP(SkReadBuffer& buffer, SkPictInfo* pInfo) {
    // Check magic bytes.
    SkPictInfo info;
    SkASSERT(sizeof(kMagic) == sizeof(info.fMagic));
    if (!buffer.readByteArray(&info, sizeof(info)) || !IsValidPictInfo(info)) {
        return false;
    }

    if (pInfo != NULL) {
        *pInfo = info;
    }
    return true;
}

// fRecord OK
SkPicture::SkPicture(SkPictureData* data, int width, int height)
    : fData(data)
    , fWidth(width)
    , fHeight(height)
    , fRecordWillPlayBackBitmaps(false) {
    this->needsNewGenID();
}

// fRecord OK
SkPicture* SkPicture::CreateFromStream(SkStream* stream, InstallPixelRefProc proc) {
    SkPictInfo info;

    if (!InternalOnly_StreamIsSKP(stream, &info)) {
        return NULL;
    }

    // Check to see if there is a playback to recreate.
    if (stream->readBool()) {
        SkPictureData* data = SkPictureData::CreateFromStream(stream, info, proc);
        if (NULL == data) {
            return NULL;
        }

        return SkNEW_ARGS(SkPicture, (data, info.fWidth, info.fHeight));
    }

    return NULL;
}

// fRecord OK
SkPicture* SkPicture::CreateFromBuffer(SkReadBuffer& buffer) {
    SkPictInfo info;

    if (!InternalOnly_BufferIsSKP(buffer, &info)) {
        return NULL;
    }

    // Check to see if there is a playback to recreate.
    if (buffer.readBool()) {
        SkPictureData* data = SkPictureData::CreateFromBuffer(buffer, info);
        if (NULL == data) {
            return NULL;
        }

        return SkNEW_ARGS(SkPicture, (data, info.fWidth, info.fHeight));
    }

    return NULL;
}

// fRecord OK
void SkPicture::createHeader(SkPictInfo* info) const {
    // Copy magic bytes at the beginning of the header
    SkASSERT(sizeof(kMagic) == 8);
    SkASSERT(sizeof(kMagic) == sizeof(info->fMagic));
    memcpy(info->fMagic, kMagic, sizeof(kMagic));

    // Set picture info after magic bytes in the header
    info->fVersion = CURRENT_PICTURE_VERSION;
    info->fWidth = fWidth;
    info->fHeight = fHeight;
    info->fFlags = SkPictInfo::kCrossProcess_Flag;
    // TODO: remove this flag, since we're always float (now)
    info->fFlags |= SkPictInfo::kScalarIsFloat_Flag;

    if (8 == sizeof(void*)) {
        info->fFlags |= SkPictInfo::kPtrIs64Bit_Flag;
    }
}

// fRecord OK
void SkPicture::serialize(SkWStream* stream, EncodeBitmap encoder) const {
    const SkPictureData* data = fData.get();

    // If we're a new-format picture, backport to old format for serialization.
    SkAutoTDelete<SkPicture> oldFormat;
    if (NULL == data && NULL != fRecord.get()) {
        oldFormat.reset(backport(*fRecord, fWidth, fHeight));
        data = oldFormat->fData.get();
        SkASSERT(NULL != data);
    }

    SkPictInfo info;
    this->createHeader(&info);
    stream->write(&info, sizeof(info));

    if (NULL != data) {
        stream->writeBool(true);
        data->serialize(stream, encoder);
    } else {
        stream->writeBool(false);
    }
}

// fRecord OK
void SkPicture::flatten(SkWriteBuffer& buffer) const {
    const SkPictureData* data = fData.get();

    // If we're a new-format picture, backport to old format for serialization.
    SkAutoTDelete<SkPicture> oldFormat;
    if (NULL == data && NULL != fRecord.get()) {
        oldFormat.reset(backport(*fRecord, fWidth, fHeight));
        data = oldFormat->fData.get();
        SkASSERT(NULL != data);
    }

    SkPictInfo info;
    this->createHeader(&info);
    buffer.writeByteArray(&info, sizeof(info));

    if (NULL != data) {
        buffer.writeBool(true);
        data->flatten(buffer);
    } else {
        buffer.writeBool(false);
    }
}

#if SK_SUPPORT_GPU
// fRecord TODO
bool SkPicture::suitableForGpuRasterization(GrContext* context, const char **reason) const {
    if (NULL == fData.get()) {
        if (NULL != reason) {
            *reason = "Missing internal data.";
        }
        return false;
    }

    return fData->suitableForGpuRasterization(context, reason);
}
#endif

// fRecord OK
bool SkPicture::willPlayBackBitmaps() const {
    if (fRecord.get()) {
        return fRecordWillPlayBackBitmaps;
    }
    if (!fData.get()) {
        return false;
    }
    return fData->containsBitmaps();
}

// fRecord OK
static int32_t next_picture_generation_id() {
    static int32_t  gPictureGenerationID = 0;
    // do a loop in case our global wraps around, as we never want to
    // return a 0
    int32_t genID;
    do {
        genID = sk_atomic_inc(&gPictureGenerationID) + 1;
    } while (SK_InvalidGenID == genID);
    return genID;
}

// fRecord OK
uint32_t SkPicture::uniqueID() const {
    if (SK_InvalidGenID == fUniqueID) {
        fUniqueID = next_picture_generation_id();
    }
    return fUniqueID;
}

// fRecord OK
SkPicture::SkPicture(int width, int height, SkRecord* record, SkBBoxHierarchy* bbh)
    : fWidth(width)
    , fHeight(height)
    , fRecord(record)
    , fBBH(SkSafeRef(bbh))
    , fRecordWillPlayBackBitmaps(SkRecordWillPlaybackBitmaps(*record)) {
    // TODO: delay as much of this work until just before first playback?
    if (fBBH.get()) {
        SkRecordFillBounds(*record, fBBH.get());
    }
    this->needsNewGenID();
}

// Note that we are assuming that this entry point will only be called from
// one thread. Currently the only client of this method is
// SkGpuDevice::EXPERIMENTAL_optimize which should be only called from a single
// thread.
void SkPicture::addDeletionListener(DeletionListener* listener) const {
    SkASSERT(NULL != listener);

    *fDeletionListeners.append() = SkRef(listener);
}

void SkPicture::callDeletionListeners() {
    for (int i = 0; i < fDeletionListeners.count(); ++i) {
        fDeletionListeners[i]->onDeletion(this->uniqueID());
    }

    fDeletionListeners.unrefAll();
}

// fRecord OK
int SkPicture::approximateOpCount() const {
    SkASSERT(fRecord.get() || fData.get());
    if (fRecord.get()) {
        return fRecord->count();
    }
    if (fData.get()) {
        return fData->opCount();
    }
    return 0;
}
