
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
#include "SkDevice.h"
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
}

SkPicture::SkPicture(const SkPicture& src) {
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

        if (NULL != clone->fRecord) {
            clone->fRecord->unref();
            clone->fRecord = NULL;
        }
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

    SkBitmap bm;
    bm.setConfig(SkBitmap::kNo_Config, width, height);
    SkAutoTUnref<SkDevice> dev(SkNEW_ARGS(SkDevice, (bm)));

    // Must be set before calling createBBoxHierarchy
    fWidth = width;
    fHeight = height;

    if (recordingFlags & kOptimizeForClippedPlayback_RecordingFlag) {
        SkBBoxHierarchy* tree = this->createBBoxHierarchy();
        SkASSERT(NULL != tree);
        fRecord = SkNEW_ARGS(SkBBoxHierarchyRecord, (recordingFlags, tree, dev));
        tree->unref();
    } else {
        fRecord = SkNEW_ARGS(SkPictureRecord, (recordingFlags, dev));
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
    return SkRTree::Create(kRTreeMinChildren, kRTreeMaxChildren,
                           aspectRatio);
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

SkPicture::SkPicture(SkStream* stream) {
    this->initFromStream(stream, NULL, NULL);
}

SkPicture::SkPicture(SkStream* stream, bool* success, InstallPixelRefProc proc) {
    this->initFromStream(stream, success, proc);
}

void SkPicture::initFromStream(SkStream* stream, bool* success, InstallPixelRefProc proc) {
    if (success) {
        *success = false;
    }
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
        fPlayback = SkNEW_ARGS(SkPicturePlayback, (stream, info, proc));
    }

    // do this at the end, so that they will be zero if we hit an error.
    fWidth = info.fWidth;
    fHeight = info.fHeight;
    if (success) {
        *success = true;
    }
}

void SkPicture::serialize(SkWStream* stream, EncodeBitmap encoder) const {
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
        playback->serialize(stream, encoder);
        // delete playback if it is a local version (i.e. cons'd up just now)
        if (playback != fPlayback) {
            SkDELETE(playback);
        }
    } else {
        stream->writeBool(false);
    }
}

#ifdef SK_BUILD_FOR_ANDROID
void SkPicture::abortPlayback() {
    if (NULL == fPlayback) {
        return;
    }
    fPlayback->abort();
}
#endif
