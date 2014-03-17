
/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPictureFlat.h"
#include "SkPicturePlayback.h"
#include "SkPictureRecord.h"

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

SkPicture::SkPicture() {
    fRecord = NULL;
    fPlayback = NULL;
    fWidth = fHeight = 0;
    fAccelData = NULL;
}

SkPicture::SkPicture(const SkPicture& src)
    : INHERITED()
    , fAccelData(NULL) {
    fWidth = src.fWidth;
    fHeight = src.fHeight;
    fRecord = NULL;

    /*  We want to copy the src's playback. However, if that hasn't been built
        yet, we need to fake a call to endRecording() without actually calling
        it (since it is destructive, and we don't want to change src).
     */
    if (src.fPlayback) {
        fPlayback = SkNEW_ARGS(SkPicturePlayback, (*src.fPlayback));
    } else if (src.fRecord) {
        // here we do a fake src.endRecording()
        fPlayback = SkNEW_ARGS(SkPicturePlayback, (*src.fRecord));
    } else {
        fPlayback = NULL;
    }
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
    SkTSwap(fRecord, other.fRecord);
    SkTSwap(fPlayback, other.fPlayback);
    SkTSwap(fAccelData, other.fAccelData);
    SkTSwap(fWidth, other.fWidth);
    SkTSwap(fHeight, other.fHeight);
}

SkPicture* SkPicture::clone() const {
    SkPicture* clonedPicture = SkNEW(SkPicture);
    clone(clonedPicture, 1);
    return clonedPicture;
}

void SkPicture::clone(SkPicture* pictures, int count) const {
    SkPictCopyInfo copyInfo;

    for (int i = 0; i < count; i++) {
        SkPicture* clone = &pictures[i];

        clone->fWidth = fWidth;
        clone->fHeight = fHeight;
        SkSafeSetNull(clone->fRecord);
        SkDELETE(clone->fPlayback);

        /*  We want to copy the src's playback. However, if that hasn't been built
            yet, we need to fake a call to endRecording() without actually calling
            it (since it is destructive, and we don't want to change src).
         */
        if (fPlayback) {
            clone->fPlayback = SkNEW_ARGS(SkPicturePlayback, (*fPlayback, &copyInfo));
        } else if (fRecord) {
            // here we do a fake src.endRecording()
            clone->fPlayback = SkNEW_ARGS(SkPicturePlayback, (*fRecord, true));
        } else {
            clone->fPlayback = NULL;
        }
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

SkCanvas* SkPicture::beginRecording(int width, int height,
                                    uint32_t recordingFlags) {
    if (fPlayback) {
        SkDELETE(fPlayback);
        fPlayback = NULL;
    }
    SkSafeUnref(fAccelData);
    SkSafeSetNull(fRecord);

    // Must be set before calling createBBoxHierarchy
    fWidth = width;
    fHeight = height;

    const SkISize size = SkISize::Make(width, height);

    if (recordingFlags & kOptimizeForClippedPlayback_RecordingFlag) {
        SkBBoxHierarchy* tree = this->createBBoxHierarchy();
        SkASSERT(NULL != tree);
        fRecord = SkNEW_ARGS(SkBBoxHierarchyRecord, (size, recordingFlags, tree));
        tree->unref();
    } else {
        fRecord = SkNEW_ARGS(SkPictureRecord, (size, recordingFlags));
    }
    fRecord->beginRecording();

    return fRecord;
}

SkBBoxHierarchy* SkPicture::createBBoxHierarchy() const {
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

SkCanvas* SkPicture::getRecordingCanvas() const {
    // will be null if we are not recording
    return fRecord;
}

void SkPicture::endRecording() {
    if (NULL == fPlayback) {
        if (NULL != fRecord) {
            fRecord->endRecording();
            fPlayback = SkNEW_ARGS(SkPicturePlayback, (*fRecord));
            SkSafeSetNull(fRecord);
        }
    }
    SkASSERT(NULL == fRecord);
}

void SkPicture::draw(SkCanvas* surface, SkDrawPictureCallback* callback) {
    this->endRecording();
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
    , fAccelData(NULL) {}

SkPicture* SkPicture::CreateFromStream(SkStream* stream, InstallPixelRefProc proc) {
    SkPictInfo info;

    if (!InternalOnly_StreamIsSKP(stream, &info)) {
        return NULL;
    }

    SkPicturePlayback* playback;
    // Check to see if there is a playback to recreate.
    if (stream->readBool()) {
        playback = SkPicturePlayback::CreateFromStream(stream, info, proc);
        if (NULL == playback) {
            return NULL;
        }
    } else {
        playback = NULL;
    }

    return SkNEW_ARGS(SkPicture, (playback, info.fWidth, info.fHeight));
}

SkPicture* SkPicture::CreateFromBuffer(SkReadBuffer& buffer) {
    SkPictInfo info;

    if (!InternalOnly_BufferIsSKP(buffer, &info)) {
        return NULL;
    }

    SkPicturePlayback* playback;
    // Check to see if there is a playback to recreate.
    if (buffer.readBool()) {
        playback = SkPicturePlayback::CreateFromBuffer(buffer);
        if (NULL == playback) {
            return NULL;
        }
    } else {
        playback = NULL;
    }

    return SkNEW_ARGS(SkPicture, (playback, info.fWidth, info.fHeight));
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

    if (NULL == playback && fRecord) {
        playback = SkNEW_ARGS(SkPicturePlayback, (*fRecord));
    }

    SkPictInfo header;
    this->createHeader(&header);
    stream->write(&header, sizeof(header));
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

void SkPicture::flatten(SkWriteBuffer& buffer) const {
    SkPicturePlayback* playback = fPlayback;

    if (NULL == playback && fRecord) {
        playback = SkNEW_ARGS(SkPicturePlayback, (*fRecord));
    }

    SkPictInfo header;
    this->createHeader(&header);
    buffer.writeByteArray(&header, sizeof(header));
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
