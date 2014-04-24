
/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPictureFlat.h"
#include "SkPicturePlayback.h"
#include "SkPictureRecord.h"

#include "SkBBHFactory.h"
#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkChunkAlloc.h"
#include "SkPicture.h"
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

template <typename T> int SafeCount(const T* obj) {
    return obj ? obj->count() : 0;
}

#define DUMP_BUFFER_SIZE 65536

//#define ENABLE_TIME_DRAW    // dumps milliseconds for each draw


#ifdef SK_DEBUG
// enable SK_DEBUG_TRACE to trace DrawType elements when
//     recorded and played back
// #define SK_DEBUG_TRACE
// enable SK_DEBUG_SIZE to see the size of picture components
// #define SK_DEBUG_SIZE
// enable SK_DEBUG_DUMP to see the contents of recorded elements
// #define SK_DEBUG_DUMP
// enable SK_DEBUG_VALIDATE to check internal structures for consistency
// #define SK_DEBUG_VALIDATE
#endif

#if defined SK_DEBUG_TRACE || defined SK_DEBUG_DUMP
const char* DrawTypeToString(DrawType drawType) {
    switch (drawType) {
        case UNUSED: SkDebugf("DrawType UNUSED\n"); SkASSERT(0); break;
        case CLIP_PATH: return "CLIP_PATH";
        case CLIP_REGION: return "CLIP_REGION";
        case CLIP_RECT: return "CLIP_RECT";
        case CLIP_RRECT: return "CLIP_RRECT";
        case CONCAT: return "CONCAT";
        case DRAW_BITMAP: return "DRAW_BITMAP";
        case DRAW_BITMAP_MATRIX: return "DRAW_BITMAP_MATRIX";
        case DRAW_BITMAP_NINE: return "DRAW_BITMAP_NINE";
        case DRAW_BITMAP_RECT_TO_RECT: return "DRAW_BITMAP_RECT_TO_RECT";
        case DRAW_CLEAR: return "DRAW_CLEAR";
        case DRAW_DATA: return "DRAW_DATA";
        case DRAW_OVAL: return "DRAW_OVAL";
        case DRAW_PAINT: return "DRAW_PAINT";
        case DRAW_PATH: return "DRAW_PATH";
        case DRAW_PICTURE: return "DRAW_PICTURE";
        case DRAW_POINTS: return "DRAW_POINTS";
        case DRAW_POS_TEXT: return "DRAW_POS_TEXT";
        case DRAW_POS_TEXT_TOP_BOTTOM: return "DRAW_POS_TEXT_TOP_BOTTOM";
        case DRAW_POS_TEXT_H: return "DRAW_POS_TEXT_H";
        case DRAW_POS_TEXT_H_TOP_BOTTOM: return "DRAW_POS_TEXT_H_TOP_BOTTOM";
        case DRAW_RECT: return "DRAW_RECT";
        case DRAW_RRECT: return "DRAW_RRECT";
        case DRAW_SPRITE: return "DRAW_SPRITE";
        case DRAW_TEXT: return "DRAW_TEXT";
        case DRAW_TEXT_ON_PATH: return "DRAW_TEXT_ON_PATH";
        case DRAW_TEXT_TOP_BOTTOM: return "DRAW_TEXT_TOP_BOTTOM";
        case DRAW_VERTICES: return "DRAW_VERTICES";
        case RESTORE: return "RESTORE";
        case ROTATE: return "ROTATE";
        case SAVE: return "SAVE";
        case SAVE_LAYER: return "SAVE_LAYER";
        case SCALE: return "SCALE";
        case SET_MATRIX: return "SET_MATRIX";
        case SKEW: return "SKEW";
        case TRANSLATE: return "TRANSLATE";
        case NOOP: return "NOOP";
        default:
            SkDebugf("DrawType error 0x%08x\n", drawType);
            SkASSERT(0);
            break;
    }
    SkASSERT(0);
    return NULL;
}
#endif

#ifdef SK_DEBUG_VALIDATE
static void validateMatrix(const SkMatrix* matrix) {
    SkScalar scaleX = matrix->getScaleX();
    SkScalar scaleY = matrix->getScaleY();
    SkScalar skewX = matrix->getSkewX();
    SkScalar skewY = matrix->getSkewY();
    SkScalar perspX = matrix->getPerspX();
    SkScalar perspY = matrix->getPerspY();
    if (scaleX != 0 && skewX != 0)
        SkDebugf("scaleX != 0 && skewX != 0\n");
    SkASSERT(scaleX == 0 || skewX == 0);
    SkASSERT(scaleY == 0 || skewY == 0);
    SkASSERT(perspX == 0);
    SkASSERT(perspY == 0);
}
#endif


///////////////////////////////////////////////////////////////////////////////

SkPicture::SkPicture()
    : fAccelData(NULL) {
    this->needsNewGenID();
    fRecord = NULL;
    fPlayback = NULL;
    fWidth = fHeight = 0;
}

SkPicture::SkPicture(const SkPicture& src)
    : INHERITED()
    , fAccelData(NULL)
    , fContentInfo(src.fContentInfo) {
    this->needsNewGenID();
    fWidth = src.fWidth;
    fHeight = src.fHeight;
    fRecord = NULL;

    /*  We want to copy the src's playback. However, if that hasn't been built
        yet, we need to fake a call to endRecording() without actually calling
        it (since it is destructive, and we don't want to change src).
     */
    if (src.fPlayback) {
        fPlayback = SkNEW_ARGS(SkPicturePlayback, (this, *src.fPlayback));
        SkASSERT(NULL == src.fRecord);
        fUniqueID = src.uniqueID();     // need to call method to ensure != 0
    } else if (src.fRecord) {
        SkPictInfo info;
        this->createHeader(&info);
        // here we do a fake src.endRecording()
        fPlayback = SkNEW_ARGS(SkPicturePlayback, (this, *src.fRecord, info));
    } else {
        fPlayback = NULL;
    }

    fPathHeap.reset(SkSafeRef(src.fPathHeap.get()));
}

const SkPath& SkPicture::getPath(int index) const {
    return (*fPathHeap.get())[index];
}

int SkPicture::addPathToHeap(const SkPath& path) {
    if (NULL == fPathHeap) {
        fPathHeap.reset(SkNEW(SkPathHeap));
    }
#ifdef SK_DEDUP_PICTURE_PATHS
    return fPathHeap->insert(path);
#else
    return fPathHeap->append(path);
#endif
}

void SkPicture::initForPlayback() const {
    // ensure that the paths bounds are pre-computed
    if (NULL != fPathHeap.get()) {
        for (int i = 0; i < fPathHeap->count(); i++) {
            (*fPathHeap.get())[i].updateBoundsCache();
        }
    }
}

void SkPicture::dumpSize() const {
    SkDebugf("--- picture size: paths=%d\n",
             SafeCount(fPathHeap.get()));
}

SkPicture::~SkPicture() {
    SkSafeUnref(fRecord);
    SkDELETE(fPlayback);
    SkSafeUnref(fAccelData);
}

void SkPicture::internalOnly_EnableOpts(bool enableOpts) {
    if (NULL != fRecord) {
        fRecord->internalOnly_EnableOpts(enableOpts);
    }
}

void SkPicture::swap(SkPicture& other) {
    SkTSwap(fUniqueID, other.fUniqueID);
    SkTSwap(fRecord, other.fRecord);
    SkTSwap(fPlayback, other.fPlayback);
    SkTSwap(fAccelData, other.fAccelData);
    SkTSwap(fWidth, other.fWidth);
    SkTSwap(fHeight, other.fHeight);
    fPathHeap.swap(&other.fPathHeap);
    fContentInfo.swap(&other.fContentInfo);
}

SkPicture* SkPicture::clone() const {
    SkPicture* clonedPicture = SkNEW(SkPicture);
    this->clone(clonedPicture, 1);
    return clonedPicture;
}

void SkPicture::clone(SkPicture* pictures, int count) const {
    SkPictCopyInfo copyInfo;
    SkPictInfo info;
    this->createHeader(&info);

    for (int i = 0; i < count; i++) {
        SkPicture* clone = &pictures[i];

        clone->needsNewGenID();
        clone->fWidth = fWidth;
        clone->fHeight = fHeight;
        SkSafeSetNull(clone->fRecord);
        SkDELETE(clone->fPlayback);
        clone->fContentInfo.set(fContentInfo);

        /*  We want to copy the src's playback. However, if that hasn't been built
            yet, we need to fake a call to endRecording() without actually calling
            it (since it is destructive, and we don't want to change src).
         */
        if (fPlayback) {
            clone->fPlayback = SkNEW_ARGS(SkPicturePlayback, (clone, *fPlayback, &copyInfo));
            SkASSERT(NULL == fRecord);
            clone->fUniqueID = this->uniqueID(); // need to call method to ensure != 0
        } else if (fRecord) {
            // here we do a fake src.endRecording()
            clone->fPlayback = SkNEW_ARGS(SkPicturePlayback, (clone, *fRecord, info, true));
        } else {
            clone->fPlayback = NULL;
        }

        clone->fPathHeap.reset(SkSafeRef(fPathHeap.get()));
    }
}

SkPicture::AccelData::Domain SkPicture::AccelData::GenerateDomain() {
    static int32_t gNextID = 0;

    int32_t id = sk_atomic_inc(&gNextID);
    if (id >= 1 << (8 * sizeof(Domain))) {
        SK_CRASH();
    }

    return static_cast<Domain>(id);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_SUPPORT_LEGACY_DERIVED_PICTURE_CLASSES

SkCanvas* SkPicture::beginRecording(int width, int height,
                                    uint32_t recordingFlags) {
    if (fPlayback) {
        SkDELETE(fPlayback);
        fPlayback = NULL;
    }
    SkSafeUnref(fAccelData);
    SkSafeSetNull(fRecord);
    fContentInfo.reset();

    this->needsNewGenID();

    // Must be set before calling createBBoxHierarchy
    fWidth = width;
    fHeight = height;

    const SkISize size = SkISize::Make(width, height);

    if (recordingFlags & kOptimizeForClippedPlayback_RecordingFlag) {
        SkBBoxHierarchy* tree = this->createBBoxHierarchy();
        SkASSERT(NULL != tree);
        fRecord = SkNEW_ARGS(SkBBoxHierarchyRecord, (this, size, recordingFlags, tree));
        tree->unref();
    } else {
        fRecord = SkNEW_ARGS(SkPictureRecord, (this, size, recordingFlags));
    }
    fRecord->beginRecording();

    return fRecord;
}

#endif

SkCanvas* SkPicture::beginRecording(int width, int height,
                                    SkBBHFactory* bbhFactory,
                                    uint32_t recordingFlags) {
    if (fPlayback) {
        SkDELETE(fPlayback);
        fPlayback = NULL;
    }
    SkSafeUnref(fAccelData);
    SkSafeSetNull(fRecord);
    SkASSERT(NULL == fPathHeap);
    fContentInfo.reset();

    this->needsNewGenID();

    fWidth = width;
    fHeight = height;

    const SkISize size = SkISize::Make(width, height);

    if (NULL != bbhFactory) {
        SkAutoTUnref<SkBBoxHierarchy> tree((*bbhFactory)(width, height));
        SkASSERT(NULL != tree);
        fRecord = SkNEW_ARGS(SkBBoxHierarchyRecord, (this, size,
                                                     recordingFlags|
                                                     kOptimizeForClippedPlayback_RecordingFlag,
                                                     tree.get()));
    } else {
        fRecord = SkNEW_ARGS(SkPictureRecord, (this, size, recordingFlags));
    }
    fRecord->beginRecording();

    return fRecord;
}


#ifdef SK_SUPPORT_LEGACY_DERIVED_PICTURE_CLASSES

SkBBoxHierarchy* SkPicture::createBBoxHierarchy() const {
    // TODO: this code is now replicated in SkRTreePicture. Once all external
    // clients have been weaned off of kOptimizeForClippedPlayback_RecordingFlag,
    // this code can be removed.

    // These values were empirically determined to produce reasonable
    // performance in most cases.
    static const int kRTreeMinChildren = 6;
    static const int kRTreeMaxChildren = 11;

    SkScalar aspectRatio = SkScalarDiv(SkIntToScalar(fWidth),
                                       SkIntToScalar(fHeight));
    bool sortDraws = false;  // Do not sort draw calls when bulk loading.

    return SkRTree::Create(kRTreeMinChildren, kRTreeMaxChildren,
                           aspectRatio, sortDraws);
}

#endif

SkCanvas* SkPicture::getRecordingCanvas() const {
    // will be null if we are not recording
    return fRecord;
}

void SkPicture::endRecording() {
    if (NULL == fPlayback) {
        if (NULL != fRecord) {
            fRecord->endRecording();
            SkPictInfo info;
            this->createHeader(&info);
            fPlayback = SkNEW_ARGS(SkPicturePlayback, (this, *fRecord, info));
            SkSafeSetNull(fRecord);
        }
    }
    SkASSERT(NULL == fRecord);
}

const SkPicture::OperationList& SkPicture::OperationList::InvalidList() {
    static OperationList gInvalid;
    return gInvalid;
}

const SkPicture::OperationList& SkPicture::EXPERIMENTAL_getActiveOps(const SkIRect& queryRect) {
    this->endRecording();  // TODO: remove eventually
    if (NULL != fPlayback) {
        return fPlayback->getActiveOps(queryRect);
    }
    return OperationList::InvalidList();
}

size_t SkPicture::EXPERIMENTAL_curOpID() const {
    if (NULL != fPlayback) {
        return fPlayback->curOpID();
    }
    return 0;
}

void SkPicture::draw(SkCanvas* surface, SkDrawPictureCallback* callback) {
    this->endRecording(); // TODO: remove eventually
    if (NULL != fPlayback) {
        fPlayback->draw(*surface, callback);
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkStream.h"

static const char kMagic[] = { 's', 'k', 'i', 'a', 'p', 'i', 'c', 't' };

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

SkPicture::SkPicture(SkPicturePlayback* playback, int width, int height)
    : fPlayback(playback)
    , fRecord(NULL)
    , fWidth(width)
    , fHeight(height)
    , fAccelData(NULL) {
    this->needsNewGenID();
}

SkPicture* SkPicture::CreateFromStream(SkStream* stream, InstallPixelRefProc proc) {
    SkPictInfo info;

    if (!InternalOnly_StreamIsSKP(stream, &info)) {
        return NULL;
    }

    SkPicture* newPict = SkNEW_ARGS(SkPicture, (NULL, info.fWidth, info.fHeight));

    // Check to see if there is a playback to recreate.
    if (stream->readBool()) {
        SkPicturePlayback* playback = SkPicturePlayback::CreateFromStream(newPict, stream,
                                                                          info, proc);
        if (NULL == playback) {
            SkDELETE(newPict);
            return NULL;
        }
        newPict->fPlayback = playback;
    }

    return newPict;
}

SkPicture* SkPicture::CreateFromBuffer(SkReadBuffer& buffer) {
    SkPictInfo info;

    if (!InternalOnly_BufferIsSKP(buffer, &info)) {
        return NULL;
    }

    SkPicture* newPict = SkNEW_ARGS(SkPicture, (NULL, info.fWidth, info.fHeight));

    // Check to see if there is a playback to recreate.
    if (buffer.readBool()) {
        SkPicturePlayback* playback = SkPicturePlayback::CreateFromBuffer(newPict, buffer, info);
        if (NULL == playback) {
            SkDELETE(newPict);
            return NULL;
        }
        newPict->fPlayback = playback;
    }

    return newPict;
}

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

void SkPicture::serialize(SkWStream* stream, EncodeBitmap encoder) const {
    SkPicturePlayback* playback = fPlayback;

    SkPictInfo info;
    this->createHeader(&info);
    if (NULL == playback && fRecord) {
        playback = SkNEW_ARGS(SkPicturePlayback, (this, *fRecord, info));
    }

    stream->write(&info, sizeof(info));
    if (playback) {
        stream->writeBool(true);
        playback->serialize(stream, encoder);
        // delete playback if it is a local version (i.e. cons'd up just now)
        if (playback != fPlayback) {
            SkDELETE(playback);
        }
    } else {
        stream->writeBool(false);
    }
}

void SkPicture::WriteTagSize(SkWriteBuffer& buffer, uint32_t tag, size_t size) {
    buffer.writeUInt(tag);
    buffer.writeUInt(SkToU32(size));
}

void SkPicture::WriteTagSize(SkWStream* stream, uint32_t tag,  size_t size) {
    stream->write32(tag);
    stream->write32(SkToU32(size));
}

bool SkPicture::parseBufferTag(SkReadBuffer& buffer,
                               uint32_t tag,
                               uint32_t size) {
    switch (tag) {
        case SK_PICT_PATH_BUFFER_TAG:
            if (size > 0) {
                fPathHeap.reset(SkNEW_ARGS(SkPathHeap, (buffer)));
            }
            break;
        default:
            // The tag was invalid.
            return false;
    }

    return true;    // success
}

void SkPicture::flattenToBuffer(SkWriteBuffer& buffer) const {
    int n;

    if ((n = SafeCount(fPathHeap.get())) > 0) {
        WriteTagSize(buffer, SK_PICT_PATH_BUFFER_TAG, n);
        fPathHeap->flatten(buffer);
    }
}

void SkPicture::flatten(SkWriteBuffer& buffer) const {
    SkPicturePlayback* playback = fPlayback;

    SkPictInfo info;
    this->createHeader(&info);
    if (NULL == playback && fRecord) {
        playback = SkNEW_ARGS(SkPicturePlayback, (this, *fRecord, info));
    }

    buffer.writeByteArray(&info, sizeof(info));
    if (playback) {
        buffer.writeBool(true);
        playback->flatten(buffer);
        // delete playback if it is a local version (i.e. cons'd up just now)
        if (playback != fPlayback) {
            SkDELETE(playback);
        }
    } else {
        buffer.writeBool(false);
    }
}

#if SK_SUPPORT_GPU
bool SkPicture::suitableForGpuRasterization(GrContext* context) const {
    // TODO: the heuristic used here needs to be refined
    static const int kNumPaintWithPathEffectUsesTol = 1;
    static const int kNumAAConcavePaths = 5;

    SkASSERT(this->numAAHairlineConcavePaths() <= this->numAAConcavePaths());

    return this->numPaintWithPathEffectUses() < kNumPaintWithPathEffectUsesTol &&
           (this->numAAConcavePaths()-this->numAAHairlineConcavePaths()) < kNumAAConcavePaths;
}
#endif

bool SkPicture::willPlayBackBitmaps() const {
    if (!fPlayback) {
        return false;
    }
    return fPlayback->containsBitmaps();
}

#ifdef SK_BUILD_FOR_ANDROID
void SkPicture::abortPlayback() {
    if (NULL == fPlayback) {
        return;
    }
    fPlayback->abort();
}
#endif

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

uint32_t SkPicture::uniqueID() const {
    if (NULL != fRecord) {
        SkASSERT(NULL == fPlayback);
        return SK_InvalidGenID;
    }

    if (SK_InvalidGenID == fUniqueID) {
        fUniqueID = next_picture_generation_id();
    }
    return fUniqueID;
}
