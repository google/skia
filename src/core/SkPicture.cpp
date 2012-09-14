
/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPictureFlat.h"
#include "SkPicturePlayback.h"
#include "SkPictureRecord.h"

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

SK_DEFINE_INST_COUNT(SkPicture)

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
        case CONCAT: return "CONCAT";
        case DRAW_BITMAP: return "DRAW_BITMAP";
        case DRAW_BITMAP_MATRIX: return "DRAW_BITMAP_MATRIX";
        case DRAW_BITMAP_RECT: return "DRAW_BITMAP_RECT";
        case DRAW_PAINT: return "DRAW_PAINT";
        case DRAW_PATH: return "DRAW_PATH";
        case DRAW_PICTURE: return "DRAW_PICTURE";
        case DRAW_POINTS: return "DRAW_POINTS";
        case DRAW_POS_TEXT: return "DRAW_POS_TEXT";
        case DRAW_POS_TEXT_H: return "DRAW_POS_TEXT_H";
        case DRAW_RECT_GENERAL: return "DRAW_RECT_GENERAL";
        case DRAW_RECT_SIMPLE: return "DRAW_RECT_SIMPLE";
        case DRAW_SPRITE: return "DRAW_SPRITE";
        case DRAW_TEXT: return "DRAW_TEXT";
        case DRAW_TEXT_ON_PATH: return "DRAW_TEXT_ON_PATH";
        case RESTORE: return "RESTORE";
        case ROTATE: return "ROTATE";
        case SAVE: return "SAVE";
        case SAVE_LAYER: return "SAVE_LAYER";
        case SCALE: return "SCALE";
        case SKEW: return "SKEW";
        case TRANSLATE: return "TRANSLATE";
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
}

SkPicture::SkPicture(const SkPicture& src) : SkRefCnt() {
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
}

void SkPicture::swap(SkPicture& other) {
    SkTSwap(fRecord, other.fRecord);
    SkTSwap(fPlayback, other.fPlayback);
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
        clone->fRecord = NULL;

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

///////////////////////////////////////////////////////////////////////////////

SkCanvas* SkPicture::beginRecording(int width, int height,
                                    uint32_t recordingFlags) {
    if (fPlayback) {
        SkDELETE(fPlayback);
        fPlayback = NULL;
    }

    if (NULL != fRecord) {
        fRecord->unref();
        fRecord = NULL;
    }

    if (recordingFlags & kOptimizeForClippedPlayback_RecordingFlag) {
        SkScalar aspectRatio = SkScalarDiv(SkIntToScalar(width),
                                           SkIntToScalar(height));
        SkRTree* tree = SkRTree::Create(6, 11, aspectRatio);
        SkASSERT(NULL != tree);
        fRecord = SkNEW_ARGS(SkBBoxHierarchyRecord, (recordingFlags, tree));
        tree->unref();
    } else {
        fRecord = SkNEW_ARGS(SkPictureRecord, (recordingFlags));
    }

    fWidth = width;
    fHeight = height;

    SkBitmap bm;
    bm.setConfig(SkBitmap::kNo_Config, width, height);
    fRecord->setBitmapDevice(bm);

    return fRecord;
}

bool SkPicture::hasRecorded() const {
    return NULL != fRecord && fRecord->writeStream().size() > 0;
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
            fRecord->unref();
            fRecord = NULL;
        }
    }
    SkASSERT(NULL == fRecord);
}

void SkPicture::draw(SkCanvas* surface) {
    this->endRecording();
    if (fPlayback) {
        fPlayback->draw(*surface);
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkStream.h"

// V2 : adds SkPixelRef's generation ID.
// V3 : PictInfo tag at beginning, and EOF tag at the end
// V4 : move SkPictInfo to be the header
// V5 : don't read/write FunctionPtr on cross-process (we can detect that)
// V6 : added serialization of SkPath's bounds (and packed its flags tighter)
#define PICTURE_VERSION     6

SkPicture::SkPicture(SkStream* stream) : SkRefCnt() {
    fRecord = NULL;
    fPlayback = NULL;
    fWidth = fHeight = 0;

    SkPictInfo info;

    if (!stream->read(&info, sizeof(info))) {
        return;
    }
    if (PICTURE_VERSION != info.fVersion) {
        return;
    }

    if (stream->readBool()) {
        bool isValid = false;
        fPlayback = SkNEW_ARGS(SkPicturePlayback, (stream, info, &isValid));
        if (!isValid) {
            SkDELETE(fPlayback);
            fPlayback = NULL;
            return;
        }
    }

    // do this at the end, so that they will be zero if we hit an error.
    fWidth = info.fWidth;
    fHeight = info.fHeight;
}

void SkPicture::serialize(SkWStream* stream) const {
    SkPicturePlayback* playback = fPlayback;

    if (NULL == playback && fRecord) {
        playback = SkNEW_ARGS(SkPicturePlayback, (*fRecord));
    }

    SkPictInfo info;

    info.fVersion = PICTURE_VERSION;
    info.fWidth = fWidth;
    info.fHeight = fHeight;
    info.fFlags = SkPictInfo::kCrossProcess_Flag;
#ifdef SK_SCALAR_IS_FLOAT
    info.fFlags |= SkPictInfo::kScalarIsFloat_Flag;
#endif
    if (8 == sizeof(void*)) {
        info.fFlags |= SkPictInfo::kPtrIs64Bit_Flag;
    }

    stream->write(&info, sizeof(info));
    if (playback) {
        stream->writeBool(true);
        playback->serialize(stream);
        // delete playback if it is a local version (i.e. cons'd up just now)
        if (playback != fPlayback) {
            SkDELETE(playback);
        }
    } else {
        stream->writeBool(false);
    }
}

void SkPicture::abortPlayback() {
    if (NULL == fPlayback) {
        return;
    }
    fPlayback->abort();
}


