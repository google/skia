/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkCanvas.h"
#include "SkDebugCanvas.h"
#include "SkPicture.h"
#include "SkPictureFlat.h"
#include "SkPictureRecord.h"

// This test exercises the Matrix/Clip State collapsing system. It generates
// example skps and the compares the actual stored operations to the expected
// operations. The test works by emitting canvas operations at three levels:
// overall structure, bodies that draw something and model/clip state changes.
//
// Structure methods only directly emit save and restores but call the
// ModelClip and Body helper methods to fill in the structure. Since they only
// emit saves and restores the operations emitted by the structure methods will
// be completely removed by the matrix/clip collapse. Note: every save in
// a structure method is followed by a call to a ModelClip helper.
//
// Body methods only directly emit draw ops and saveLayer/restore pairs but call
// the ModelClip helper methods. Since the body methods emit the ops that cannot
// be collapsed (i.e., draw ops, saveLayer/restore) they also generate the
// expected result information. Note: every saveLayer in a body method is
// followed by a call to a ModelClip helper.
//
// The ModelClip methods output matrix and clip ops in various orders and
// combinations. They contribute to the expected result by outputting the
// expected matrix & clip ops. Note that, currently, the entire clip stack
// is output for each MC state so the clip operations accumulate down the
// save/restore stack.

// TODOs:
//   check on clip offsets
//      - not sure if this is possible. The desire is to verify that the clip
//        operations' offsets point to the correct follow-on operations. This
//        could be difficult since there is no good way to communicate the
//        offset stored in the SkPicture to the debugger's clip objects
//   add comparison of rendered before & after images?
//      - not sure if this would be useful since it somewhat duplicates the
//        correctness test of running render_pictures in record mode and
//        rendering before and after images. Additionally the matrix/clip collapse
//        is sure to cause some small differences so an automated test might
//        yield too many false positives.
//   run the matrix/clip collapse system on the 10K skp set
//      - this should give us warm fuzzies that the matrix clip collapse
//        system is ready for prime time
//   bench the recording times with/without matrix/clip collapsing

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE

// Enable/disable debugging helper code
//#define TEST_COLLAPSE_MATRIX_CLIP_STATE 1

// Extract the command ops from the input SkPicture
static void gets_ops(SkPicture& input, SkTDArray<DrawType>* ops) {
    SkDebugCanvas debugCanvas(input.width(), input.height());
    debugCanvas.setBounds(input.width(), input.height());
    input.draw(&debugCanvas);

    ops->setCount(debugCanvas.getSize());
    for (int i = 0; i < debugCanvas.getSize(); ++i) {
        (*ops)[i] = debugCanvas.getDrawCommandAt(i)->getType();
    }
}

enum ClipType {
    kNone_ClipType,
    kRect_ClipType,
    kRRect_ClipType,
    kPath_ClipType,
    kRegion_ClipType,

    kLast_ClipType = kRRect_ClipType
};

static const int kClipTypeCount = kLast_ClipType + 1;

enum MatType {
    kNone_MatType,
    kTranslate_MatType,
    kScale_MatType,
    kSkew_MatType,
    kRotate_MatType,
    kConcat_MatType,
    kSetMatrix_MatType,

    kLast_MatType = kScale_MatType
};

static const int kMatTypeCount = kLast_MatType + 1;

// TODO: implement the rest of the draw ops
enum DrawOpType {
    kNone_DrawOpType,
#if 0
    kBitmap_DrawOpType,
    kBitmapMatrix_DrawOpType,
    kBitmapNone_DrawOpType,
    kBitmapRectToRect_DrawOpType,
#endif
    kClear_DrawOpType,
#if 0
    kData_DrawOpType,
#endif
    kOval_DrawOpType,
#if 0
    kPaint_DrawOpType,
    kPath_DrawOpType,
    kPicture_DrawOpType,
    kPoints_DrawOpType,
    kPosText_DrawOpType,
    kPosTextTopBottom_DrawOpType,
    kPosTextH_DrawOpType,
    kPosTextHTopBottom_DrawOpType,
#endif
    kRect_DrawOpType,
    kRRect_DrawOpType,
#if 0
    kSprite_DrawOpType,
    kText_DrawOpType,
    kTextOnPath_DrawOpType,
    kTextTopBottom_DrawOpType,
    kDrawVertices_DrawOpType,
#endif

    kLastNonSaveLayer_DrawOpType = kRect_DrawOpType,

    // saveLayer's have to handled apart from the other draw operations
    // since they also alter the save/restore structure.
    kSaveLayer_DrawOpType,
};

static const int kNonSaveLayerDrawOpTypeCount = kLastNonSaveLayer_DrawOpType + 1;

typedef void (*PFEmitMC)(SkCanvas* canvas, MatType mat, ClipType clip,
                         DrawOpType draw, SkTDArray<DrawType>* expected,
                         int accumulatedClips);
typedef void (*PFEmitBody)(SkCanvas* canvas, PFEmitMC emitMC, MatType mat,
                           ClipType clip, DrawOpType draw,
                           SkTDArray<DrawType>* expected, int accumulatedClips);
typedef void (*PFEmitStruct)(SkCanvas* canvas, PFEmitMC emitMC, MatType mat,
                             ClipType clip, PFEmitBody emitBody, DrawOpType draw,
                             SkTDArray<DrawType>* expected);

//////////////////////////////////////////////////////////////////////////////

// TODO: expand the testing to include the different ops & AA types!
static void emit_clip(SkCanvas* canvas, ClipType clip) {
    switch (clip) {
        case kNone_ClipType:
            break;
        case kRect_ClipType: {
            SkRect r = SkRect::MakeLTRB(10, 10, 90, 90);
            canvas->clipRect(r, SkRegion::kIntersect_Op, true);
            break;
        }
        case kRRect_ClipType: {
            SkRect r = SkRect::MakeLTRB(10, 10, 90, 90);
            SkRRect rr;
            rr.setRectXY(r, 10, 10);
            canvas->clipRRect(rr, SkRegion::kIntersect_Op, true);
            break;
        }
        case kPath_ClipType: {
            SkPath p;
            p.moveTo(5.0f, 5.0f);
            p.lineTo(50.0f, 50.0f);
            p.lineTo(100.0f, 5.0f);
            p.close();
            canvas->clipPath(p, SkRegion::kIntersect_Op, true);
            break;
        }
        case kRegion_ClipType: {
            SkIRect rects[2] = {
                { 1, 1, 55, 55 },
                { 45, 45, 99, 99 },
            };
            SkRegion r;
            r.setRects(rects, 2);
            canvas->clipRegion(r, SkRegion::kIntersect_Op);
            break;
        }
        default:
            SkASSERT(0);
    }
}

static void add_clip(ClipType clip, MatType mat, SkTDArray<DrawType>* expected) {
    if (nullptr == expected) {
        // expected is nullptr if this clip will be fused into later clips
        return;
    }

    switch (clip) {
        case kNone_ClipType:
            break;
        case kRect_ClipType:
            *expected->append() = CONCAT;
            *expected->append() = CLIP_RECT;
            break;
        case kRRect_ClipType:
            *expected->append() = CONCAT;
            *expected->append() = CLIP_RRECT;
            break;
        case kPath_ClipType:
            *expected->append() = CONCAT;
            *expected->append() = CLIP_PATH;
            break;
        case kRegion_ClipType:
            *expected->append() = CONCAT;
            *expected->append() = CLIP_REGION;
            break;
        default:
            SkASSERT(0);
    }
}

static void emit_mat(SkCanvas* canvas, MatType mat) {
    switch (mat) {
    case kNone_MatType:
        break;
    case kTranslate_MatType:
        canvas->translate(5.0f, 5.0f);
        break;
    case kScale_MatType:
        canvas->scale(1.1f, 1.1f);
        break;
    case kSkew_MatType:
        canvas->skew(1.1f, 1.1f);
        break;
    case kRotate_MatType:
        canvas->rotate(1.0f);
        break;
    case kConcat_MatType: {
        SkMatrix m;
        m.setTranslate(1.0f, 1.0f);
        canvas->concat(m);
        break;
    }
    case kSetMatrix_MatType: {
        SkMatrix m;
        m.setTranslate(1.0f, 1.0f);
        canvas->setMatrix(m);
        break;
    }
    default:
        SkASSERT(0);
    }
}

static void add_mat(MatType mat, SkTDArray<DrawType>* expected) {
    if (nullptr == expected) {
        // expected is nullptr if this matrix call will be fused into later ones
        return;
    }

    switch (mat) {
    case kNone_MatType:
        break;
    case kTranslate_MatType:    // fall thru
    case kScale_MatType:        // fall thru
    case kSkew_MatType:         // fall thru
    case kRotate_MatType:       // fall thru
    case kConcat_MatType:       // fall thru
    case kSetMatrix_MatType:
        // TODO: this system currently converts a setMatrix to concat. If we wanted to
        // really preserve the setMatrix semantics we should keep it a setMatrix. I'm
        // not sure if this is a good idea though since this would keep things like pinch
        // zoom from working.
        *expected->append() = CONCAT;
        break;
    default:
        SkASSERT(0);
    }
}

static void emit_draw(SkCanvas* canvas, DrawOpType draw, SkTDArray<DrawType>* expected) {
    switch (draw) {
        case kNone_DrawOpType:
            break;
        case kClear_DrawOpType:
            canvas->clear(SK_ColorRED);
            *expected->append() = DRAW_CLEAR;
            break;
        case kOval_DrawOpType: {
            SkRect r = SkRect::MakeLTRB(10, 10, 90, 90);
            SkPaint p;
            canvas->drawOval(r, p);
            *expected->append() = DRAW_OVAL;
            break;
        }
        case kRect_DrawOpType: {
            SkRect r = SkRect::MakeLTRB(10, 10, 90, 90);
            SkPaint p;
            canvas->drawRect(r, p);
            *expected->append() = DRAW_RECT;
            break;
        }
        case kRRect_DrawOpType: {
            SkRect r = SkRect::MakeLTRB(10.0f, 10.0f, 90.0f, 90.0f);
            SkRRect rr;
            rr.setRectXY(r, 5.0f, 5.0f);
            SkPaint p;
            canvas->drawRRect(rr, p);
            *expected->append() = DRAW_RRECT;
            break;
        }
        default:
            SkASSERT(0);
    }
}

//////////////////////////////////////////////////////////////////////////////

// Emit:
//  clip
//  matrix
// Simple case - the clip isn't effect by the matrix
static void emit_clip_and_mat(SkCanvas* canvas, MatType mat, ClipType clip,
                              DrawOpType draw, SkTDArray<DrawType>* expected,
                              int accumulatedClips) {
    emit_clip(canvas, clip);
    emit_mat(canvas, mat);

    if (kNone_DrawOpType == draw) {
        return;
    }

    for (int i = 0; i < accumulatedClips; ++i) {
        add_clip(clip, mat, expected);
    }
    add_mat(mat, expected);
}

// Emit:
//  matrix
//  clip
// Emitting the matrix first is more challenging since the matrix has to be
// pushed across (i.e., applied to) the clip.
static void emit_mat_and_clip(SkCanvas* canvas, MatType mat, ClipType clip,
                              DrawOpType draw, SkTDArray<DrawType>* expected,
                              int accumulatedClips) {
    emit_mat(canvas, mat);
    emit_clip(canvas, clip);

    if (kNone_DrawOpType == draw) {
        return;
    }

    // the matrix & clip order will be reversed once collapsed!
    for (int i = 0; i < accumulatedClips; ++i) {
        add_clip(clip, mat, expected);
    }
    add_mat(mat, expected);
}

// Emit:
//  matrix
//  clip
//  matrix
//  clip
// This tests that the matrices and clips coalesce when collapsed
static void emit_double_mat_and_clip(SkCanvas* canvas, MatType mat, ClipType clip,
                                     DrawOpType draw, SkTDArray<DrawType>* expected,
                                     int accumulatedClips) {
    emit_mat(canvas, mat);
    emit_clip(canvas, clip);
    emit_mat(canvas, mat);
    emit_clip(canvas, clip);

    if (kNone_DrawOpType == draw) {
        return;
    }

    for (int i = 0; i < accumulatedClips; ++i) {
        add_clip(clip, mat, expected);
        add_clip(clip, mat, expected);
    }
    add_mat(mat, expected);
}

// Emit:
//  matrix
//  clip
//  clip
// This tests accumulation of clips in same transform state. It also tests pushing
// of the matrix across both the clips.
static void emit_mat_clip_clip(SkCanvas* canvas, MatType mat, ClipType clip,
                               DrawOpType draw, SkTDArray<DrawType>* expected,
                               int accumulatedClips) {
    emit_mat(canvas, mat);
    emit_clip(canvas, clip);
    emit_clip(canvas, clip);

    if (kNone_DrawOpType == draw) {
        return;
    }

    for (int i = 0; i < accumulatedClips; ++i) {
        add_clip(clip, mat, expected);
        add_clip(clip, mat, expected);
    }
    add_mat(mat, expected);
}

//////////////////////////////////////////////////////////////////////////////

// Emit:
//  matrix & clip calls
//  draw op
static void emit_body0(SkCanvas* canvas, PFEmitMC emitMC, MatType mat,
                       ClipType clip, DrawOpType draw,
                       SkTDArray<DrawType>* expected, int accumulatedClips) {
    bool needsSaveRestore = kNone_DrawOpType != draw &&
                            (kNone_MatType != mat || kNone_ClipType != clip);

    if (needsSaveRestore) {
        *expected->append() = SAVE;
    }
    (*emitMC)(canvas, mat, clip, draw, expected, accumulatedClips+1);
    emit_draw(canvas, draw, expected);
    if (needsSaveRestore) {
        *expected->append() = RESTORE;
    }
}

// Emit:
//  matrix & clip calls
//  draw op
//  matrix & clip calls
//  draw op
static void emit_body1(SkCanvas* canvas, PFEmitMC emitMC, MatType mat,
                       ClipType clip, DrawOpType draw,
                       SkTDArray<DrawType>* expected, int accumulatedClips) {
    bool needsSaveRestore = kNone_DrawOpType != draw &&
                            (kNone_MatType != mat || kNone_ClipType != clip);

    if (needsSaveRestore) {
        *expected->append() = SAVE;
    }
    (*emitMC)(canvas, mat, clip, draw, expected, accumulatedClips+1);
    emit_draw(canvas, draw, expected);
    if (needsSaveRestore) {
        *expected->append() = RESTORE;
        *expected->append() = SAVE;
    }
    (*emitMC)(canvas, mat, clip, draw, expected, accumulatedClips+2);
    emit_draw(canvas, draw, expected);
    if (needsSaveRestore) {
        *expected->append() = RESTORE;
    }
}

// Emit:
//  matrix & clip calls
//  SaveLayer
//      matrix & clip calls
//      draw op
//  Restore
static void emit_body2(SkCanvas* canvas, PFEmitMC emitMC, MatType mat,
                       ClipType clip, DrawOpType draw,
                       SkTDArray<DrawType>* expected, int accumulatedClips) {
    bool needsSaveRestore = kNone_DrawOpType != draw &&
                            (kNone_MatType != mat || kNone_ClipType != clip);

    if (kNone_MatType != mat || kNone_ClipType != clip) {
        *expected->append() = SAVE;
    }
    (*emitMC)(canvas, mat, clip, kSaveLayer_DrawOpType, expected, accumulatedClips+1);
    *expected->append() = SAVE_LAYER;
    // TODO: widen testing to exercise saveLayer's parameters
    canvas->saveLayer(nullptr, nullptr);
        if (needsSaveRestore) {
            *expected->append() = SAVE;
        }
        (*emitMC)(canvas, mat, clip, draw, expected, 1);
        emit_draw(canvas, draw, expected);
        if (needsSaveRestore) {
            *expected->append() = RESTORE;
        }
    canvas->restore();
    *expected->append() = RESTORE;
    if (kNone_MatType != mat || kNone_ClipType != clip) {
        *expected->append() = RESTORE;
    }
}

// Emit:
//  matrix & clip calls
//  SaveLayer
//      matrix & clip calls
//      SaveLayer
//          matrix & clip calls
//          draw op
//      Restore
//      matrix & clip calls (will be ignored)
//  Restore
static void emit_body3(SkCanvas* canvas, PFEmitMC emitMC, MatType mat,
                       ClipType clip, DrawOpType draw,
                       SkTDArray<DrawType>* expected, int accumulatedClips) {
    bool needsSaveRestore = kNone_DrawOpType != draw &&
                            (kNone_MatType != mat || kNone_ClipType != clip);

    if (kNone_MatType != mat || kNone_ClipType != clip) {
        *expected->append() = SAVE;
    }
    (*emitMC)(canvas, mat, clip, kSaveLayer_DrawOpType, expected, accumulatedClips+1);
    *expected->append() = SAVE_LAYER;
    // TODO: widen testing to exercise saveLayer's parameters
    canvas->saveLayer(nullptr, nullptr);
        (*emitMC)(canvas, mat, clip, kSaveLayer_DrawOpType, expected, 1);
        if (kNone_MatType != mat || kNone_ClipType != clip) {
            *expected->append() = SAVE;
        }
        *expected->append() = SAVE_LAYER;
        // TODO: widen testing to exercise saveLayer's parameters
        canvas->saveLayer(nullptr, nullptr);
            if (needsSaveRestore) {
                *expected->append() = SAVE;
            }
            (*emitMC)(canvas, mat, clip, draw, expected, 1);
            emit_draw(canvas, draw, expected);
            if (needsSaveRestore) {
                *expected->append() = RESTORE;
            }
        canvas->restore();             // for saveLayer
        *expected->append() = RESTORE; // for saveLayer
        if (kNone_MatType != mat || kNone_ClipType != clip) {
            *expected->append() = RESTORE;
        }
    canvas->restore();
    // required to match forced SAVE_LAYER
    *expected->append() = RESTORE;
    if (kNone_MatType != mat || kNone_ClipType != clip) {
        *expected->append() = RESTORE;
    }
}

//////////////////////////////////////////////////////////////////////////////

// Emit:
//  Save
//      some body
//  Restore
// Note: the outer save/restore are provided by beginRecording/endRecording
static void emit_struct0(SkCanvas* canvas,
                         PFEmitMC emitMC, MatType mat, ClipType clip,
                         PFEmitBody emitBody, DrawOpType draw,
                         SkTDArray<DrawType>* expected) {
    (*emitBody)(canvas, emitMC, mat, clip, draw, expected, 0);
}

// Emit:
//  Save
//      matrix & clip calls
//      Save
//          some body
//      Restore
//      matrix & clip calls (will be ignored)
//  Restore
// Note: the outer save/restore are provided by beginRecording/endRecording
static void emit_struct1(SkCanvas* canvas,
                         PFEmitMC emitMC, MatType mat, ClipType clip,
                         PFEmitBody emitBody, DrawOpType draw,
                         SkTDArray<DrawType>* expected) {
    (*emitMC)(canvas, mat, clip, draw, nullptr, 0); // these get fused into later ops
    canvas->save();
        (*emitBody)(canvas, emitMC, mat, clip, draw, expected, 1);
    canvas->restore();
    (*emitMC)(canvas, mat, clip, draw, nullptr, 0); // these will get removed
}

// Emit:
//  Save
//      matrix & clip calls
//      Save
//          some body
//      Restore
//      Save
//          some body
//      Restore
//      matrix & clip calls (will be ignored)
//  Restore
// Note: the outer save/restore are provided by beginRecording/endRecording
static void emit_struct2(SkCanvas* canvas,
                         PFEmitMC emitMC, MatType mat, ClipType clip,
                         PFEmitBody emitBody, DrawOpType draw,
                         SkTDArray<DrawType>* expected) {
    (*emitMC)(canvas, mat, clip, draw, nullptr, 1); // these will get fused into later ops
    canvas->save();
        (*emitBody)(canvas, emitMC, mat, clip, draw, expected, 1);
    canvas->restore();
    canvas->save();
        (*emitBody)(canvas, emitMC, mat, clip, draw, expected, 1);
    canvas->restore();
    (*emitMC)(canvas, mat, clip, draw, nullptr, 1); // these will get removed
}

// Emit:
//  Save
//      matrix & clip calls
//      Save
//          some body
//      Restore
//      Save
//          matrix & clip calls
//          Save
//              some body
//          Restore
//      Restore
//      matrix & clip calls (will be ignored)
//  Restore
// Note: the outer save/restore are provided by beginRecording/endRecording
static void emit_struct3(SkCanvas* canvas,
                         PFEmitMC emitMC, MatType mat, ClipType clip,
                         PFEmitBody emitBody, DrawOpType draw,
                         SkTDArray<DrawType>* expected) {
    (*emitMC)(canvas, mat, clip, draw, nullptr, 0); // these will get fused into later ops
    canvas->save();
        (*emitBody)(canvas, emitMC, mat, clip, draw, expected, 1);
    canvas->restore();
    canvas->save();
        (*emitMC)(canvas, mat, clip, draw, nullptr, 1); // these will get fused into later ops
        canvas->save();
            (*emitBody)(canvas, emitMC, mat, clip, draw, expected, 2);
        canvas->restore();
    canvas->restore();
    (*emitMC)(canvas, mat, clip, draw, nullptr, 0); // these will get removed
}

//////////////////////////////////////////////////////////////////////////////

#ifdef SK_COLLAPSE_MATRIX_CLIP_STATE
static void print(const SkTDArray<DrawType>& expected, const SkTDArray<DrawType>& actual) {
    SkDebugf("\n\nexpected %d --- actual %d\n", expected.count(), actual.count());
    int max = SkMax32(expected.count(), actual.count());

    for (int i = 0; i < max; ++i) {
        if (i < expected.count()) {
            SkDebugf("%16s,    ", SkDrawCommand::GetCommandString(expected[i]));
        } else {
            SkDebugf("%16s,    ", " ");
        }

        if (i < actual.count()) {
            SkDebugf("%s\n", SkDrawCommand::GetCommandString(actual[i]));
        } else {
            SkDebugf("\n");
        }
    }
    SkDebugf("\n\n");
    SkASSERT(0);
}
#endif

static void test_collapse(skiatest::Reporter* reporter) {
    PFEmitStruct gStructure[] = { emit_struct0, emit_struct1, emit_struct2, emit_struct3 };
    PFEmitBody gBody[] = { emit_body0, emit_body1, emit_body2, emit_body3 };
    PFEmitMC gMCs[] = { emit_clip_and_mat, emit_mat_and_clip,
                        emit_double_mat_and_clip, emit_mat_clip_clip };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gStructure); ++i) {
        for (size_t j = 0; j < SK_ARRAY_COUNT(gBody); ++j) {
            for (size_t k = 0; k < SK_ARRAY_COUNT(gMCs); ++k) {
                for (int l = 0; l < kMatTypeCount; ++l) {
                    for (int m = 0; m < kClipTypeCount; ++m) {
                        for (int n = 0; n < kNonSaveLayerDrawOpTypeCount; ++n) {
#ifdef TEST_COLLAPSE_MATRIX_CLIP_STATE
                            static int testID = -1;
                            ++testID;
                            if (testID < -1) {
                                continue;
                            }
                            SkDebugf("test: %d\n", testID);
#endif

                            SkTDArray<DrawType> expected, actual;

                            SkPicture picture;

                            // Note: beginRecording/endRecording add a save/restore pair
                            SkCanvas* canvas = picture.beginRecording(100, 100);
                            (*gStructure[i])(canvas,
                                             gMCs[k],
                                             (MatType) l,
                                             (ClipType) m,
                                             gBody[j],
                                             (DrawOpType) n,
                                             &expected);
                            picture.endRecording();

                            gets_ops(picture, &actual);

                            REPORTER_ASSERT(reporter, expected.count() == actual.count());

                            if (expected.count() != actual.count()) {
#ifdef TEST_COLLAPSE_MATRIX_CLIP_STATE
                                print(expected, actual);
#endif
                                continue;
                            }

                            for (int i = 0; i < expected.count(); ++i) {
                                REPORTER_ASSERT(reporter, expected[i] == actual[i]);
#ifdef TEST_COLLAPSE_MATRIX_CLIP_STATE
                                if (expected[i] != actual[i]) {
                                    print(expected, actual);
                                }
#endif
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

DEF_TEST(MatrixClipCollapse, reporter) {
    test_collapse(reporter);
}

#endif
