/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkDebugCanvas.h"
#include "SkPictureFlat.h"

#define WARN(msg)                                           \
    SkDebugf("%s:%d: %s\n", __FILE__, __LINE__, msg);

// Do the commands in 'input' match the supplied pattern? Note: this is a pretty
// heavy-weight operation since we are drawing the picture into a debug canvas
// to extract the commands.
static bool check_pattern(SkPicture& input, const SkTDArray<DrawType> &pattern) {
    SkDebugCanvas debugCanvas(input.width(), input.height());
    debugCanvas.setBounds(input.width(), input.height());
    input.draw(&debugCanvas);

    if (pattern.count() != debugCanvas.getSize()) {
        return false;
    }

    for (int i = 0; i < pattern.count(); ++i) {
        if (pattern[i] != debugCanvas.getDrawCommandAt(i)->getType()) {
            return false;
        }
    }

    return true;
}

// construct the pattern removed by the SkPictureRecord::remove_save_layer1
// optimization, i.e.:
//   SAVE_LAYER
//       DRAW_BITMAP|DRAW_BITMAP_MATRIX|DRAW_BITMAP_NINE|DRAW_BITMAP_RECT_TO_RECT
//   RESTORE
//
// saveLayerHasPaint - control if the saveLayer has a paint (the optimization
//                     takes a different path if this is false)
// dbmr2rHasPaint    - control if the dbmr2r has a paint (the optimization
//                     takes a different path if this is false)
// colorsMatch       - control if the saveLayer and dbmr2r paint colors
//                     match (the optimization will fail if they do not)
static SkPicture* create_save_layer_opt_1(SkTDArray<DrawType>* preOptPattern,
                                          SkTDArray<DrawType>* postOptPattern,
                                          const SkBitmap& checkerBoard,
                                          bool saveLayerHasPaint,
                                          bool dbmr2rHasPaint,
                                          bool colorsMatch)  {
    // Create the pattern that should trigger the optimization
    preOptPattern->setCount(5);
    (*preOptPattern)[0] = SAVE;
    (*preOptPattern)[1] = SAVE_LAYER;
    (*preOptPattern)[2] = DRAW_BITMAP_RECT_TO_RECT;
    (*preOptPattern)[3] = RESTORE;
    (*preOptPattern)[4] = RESTORE;

    if (colorsMatch) {
        // Create the pattern that should appear after the optimization
        postOptPattern->setCount(5);
        (*postOptPattern)[0] = SAVE; // extra save/restore added by extra draw
        (*postOptPattern)[1] = SAVE;
        (*postOptPattern)[2] = DRAW_BITMAP_RECT_TO_RECT;
        (*postOptPattern)[3] = RESTORE;
        (*postOptPattern)[4] = RESTORE;
    } else {
        // Create the pattern that appears if the optimization doesn't fire
        postOptPattern->setCount(7);
        (*postOptPattern)[0] = SAVE; // extra save/restore added by extra draw
        (*postOptPattern)[1] = SAVE;
        (*postOptPattern)[2] = SAVE_LAYER;
        (*postOptPattern)[3] = DRAW_BITMAP_RECT_TO_RECT;
        (*postOptPattern)[4] = RESTORE;
        (*postOptPattern)[5] = RESTORE;
        (*postOptPattern)[6] = RESTORE;
    }

    SkPicture* result = new SkPicture;

    // have to disable the optimizations while generating the picture
    SkCanvas* canvas = result->beginRecording(100, 100,
                         SkPicture::kDisableRecordOptimizations_RecordingFlag);

    SkPaint saveLayerPaint;
    saveLayerPaint.setColor(0xCC000000);

    // saveLayer's 'bounds' parameter must be NULL for this optimization
    if (saveLayerHasPaint) {
        canvas->saveLayer(NULL, &saveLayerPaint);
    } else {
        canvas->saveLayer(NULL, NULL);
    }

    SkRect rect = { 10, 10, 90, 90 };

    // The dbmr2r's paint must be opaque
    SkPaint dbmr2rPaint;
    if (colorsMatch) {
        dbmr2rPaint.setColor(0xFF000000);
    } else {
        dbmr2rPaint.setColor(0xFFFF0000);
    }

    if (dbmr2rHasPaint) {
        canvas->drawBitmapRectToRect(checkerBoard, NULL, rect, &dbmr2rPaint);
    } else {
        canvas->drawBitmapRectToRect(checkerBoard, NULL, rect, NULL);
    }
    canvas->restore();

    result->endRecording();

    return result;
}

// straight-ahead version that is seen in the skps
static SkPicture* create_save_layer_opt_1_v1(SkTDArray<DrawType>* preOptPattern,
                                             SkTDArray<DrawType>* postOptPattern,
                                             const SkBitmap& checkerBoard) {
    return create_save_layer_opt_1(preOptPattern, postOptPattern, checkerBoard,
                                   true,   // saveLayer has a paint
                                   true,   // dbmr2r has a paint
                                   true);  // and the colors match
}

// alternate version that should still succeed
static SkPicture* create_save_layer_opt_1_v2(SkTDArray<DrawType>* preOptPattern,
                                             SkTDArray<DrawType>* postOptPattern,
                                             const SkBitmap& checkerBoard) {
    return create_save_layer_opt_1(preOptPattern, postOptPattern, checkerBoard,
                                   false,  // saveLayer doesn't have a paint!
                                   true,   // dbmr2r has a paint
                                   true);  // color matching not really applicable
}

// alternate version that should still succeed
static SkPicture* create_save_layer_opt_1_v3(SkTDArray<DrawType>* preOptPattern,
                                             SkTDArray<DrawType>* postOptPattern,
                                             const SkBitmap& checkerBoard) {
    return create_save_layer_opt_1(preOptPattern, postOptPattern, checkerBoard,
                                   true,   // saveLayer has a paint
                                   false,  // dbmr2r doesn't have a paint!
                                   true);  // color matching not really applicable
}

// version in which the optimization fails b.c. the colors don't match
static SkPicture* create_save_layer_opt_1_v4(SkTDArray<DrawType>* preOptPattern,
                                             SkTDArray<DrawType>* postOptPattern,
                                             const SkBitmap& checkerBoard) {
    return create_save_layer_opt_1(preOptPattern, postOptPattern, checkerBoard,
                                   true,   // saveLayer has a paint
                                   true,   // dbmr2r has a paint
                                   false); // and the colors don't match!
}

// construct the pattern removed by the SkPictureRecord::remove_save_layer2
// optimization, i.e.:
//   SAVE_LAYER (with NULL == bounds)
//      SAVE
//         CLIP_RECT
//         DRAW_BITMAP|DRAW_BITMAP_MATRIX|DRAW_BITMAP_NINE|DRAW_BITMAP_RECT_TO_RECT
//      RESTORE
//   RESTORE
//
// saveLayerHasPaint - control if the saveLayer has a paint (the optimization
//                     takes a different path if this is false)
// dbmr2rHasPaint    - control if the dbmr2r has a paint (the optimization
//                     takes a different path if this is false)
// colorsMatch       - control if the saveLayer and dbmr2r paint colors
//                     match (the optimization will fail if they do not)
static SkPicture* create_save_layer_opt_2(SkTDArray<DrawType>* preOptPattern,
                                          SkTDArray<DrawType>* postOptPattern,
                                          const SkBitmap& checkerBoard,
                                          bool saveLayerHasPaint,
                                          bool dbmr2rHasPaint,
                                          bool colorsMatch)  {
    // Create the pattern that should trigger the optimization
    preOptPattern->setCount(8);
    (*preOptPattern)[0] = SAVE;
    (*preOptPattern)[1] = SAVE_LAYER;
    (*preOptPattern)[2] = SAVE;
    (*preOptPattern)[3] = CLIP_RECT;
    (*preOptPattern)[4] = DRAW_BITMAP_RECT_TO_RECT;
    (*preOptPattern)[5] = RESTORE;
    (*preOptPattern)[6] = RESTORE;
    (*preOptPattern)[7] = RESTORE;

    if (colorsMatch) {
        // Create the pattern that should appear after the optimization
        postOptPattern->setCount(8);
        (*postOptPattern)[0] = SAVE; // extra save/restore added by extra draw
        (*postOptPattern)[1] = SAVE;
        (*postOptPattern)[2] = SAVE;
        (*postOptPattern)[3] = CLIP_RECT;
        (*postOptPattern)[4] = DRAW_BITMAP_RECT_TO_RECT;
        (*postOptPattern)[5] = RESTORE;
        (*postOptPattern)[6] = RESTORE;
        (*postOptPattern)[7] = RESTORE;
    } else {
        // Create the pattern that appears if the optimization doesn't fire
        postOptPattern->setCount(10);
        (*postOptPattern)[0] = SAVE; // extra save/restore added by extra draw
        (*postOptPattern)[1] = SAVE;
        (*postOptPattern)[2] = SAVE_LAYER;
        (*postOptPattern)[3] = SAVE;
        (*postOptPattern)[4] = CLIP_RECT;
        (*postOptPattern)[5] = DRAW_BITMAP_RECT_TO_RECT;
        (*postOptPattern)[6] = RESTORE;
        (*postOptPattern)[7] = RESTORE;
        (*postOptPattern)[8] = RESTORE;
        (*postOptPattern)[9] = RESTORE;
    }

    SkPicture* result = new SkPicture;

    // have to disable the optimizations while generating the picture
    SkCanvas* canvas = result->beginRecording(100, 100,
                         SkPicture::kDisableRecordOptimizations_RecordingFlag);

    SkPaint saveLayerPaint;
    saveLayerPaint.setColor(0xCC000000);

    // saveLayer's 'bounds' parameter must be NULL for this optimization
    if (saveLayerHasPaint) {
        canvas->saveLayer(NULL, &saveLayerPaint);
    } else {
        canvas->saveLayer(NULL, NULL);
    }

    canvas->save();

    SkRect rect = { 10, 10, 90, 90 };
    canvas->clipRect(rect);

    // The dbmr2r's paint must be opaque
    SkPaint dbmr2rPaint;
    if (colorsMatch) {
        dbmr2rPaint.setColor(0xFF000000);
    } else {
        dbmr2rPaint.setColor(0xFFFF0000);
    }

    if (dbmr2rHasPaint) {
        canvas->drawBitmapRectToRect(checkerBoard, NULL, rect, &dbmr2rPaint);
    } else {
        canvas->drawBitmapRectToRect(checkerBoard, NULL, rect, NULL);
    }
    canvas->restore();
    canvas->restore();

    result->endRecording();

    return result;
}

// straight-ahead version that is seen in the skps
static SkPicture* create_save_layer_opt_2_v1(SkTDArray<DrawType>* preOptPattern,
                                             SkTDArray<DrawType>* postOptPattern,
                                             const SkBitmap& checkerBoard) {
    return create_save_layer_opt_2(preOptPattern, postOptPattern, checkerBoard,
                                   true,   // saveLayer has a paint
                                   true,   // dbmr2r has a paint
                                   true);  // and the colors match
}

// alternate version that should still succeed
static SkPicture* create_save_layer_opt_2_v2(SkTDArray<DrawType>* preOptPattern,
                                             SkTDArray<DrawType>* postOptPattern,
                                             const SkBitmap& checkerBoard) {
    return create_save_layer_opt_2(preOptPattern, postOptPattern, checkerBoard,
                                   false,  // saveLayer doesn't have a paint!
                                   true,   // dbmr2r has a paint
                                   true);  // color matching not really applicable
}

// alternate version that should still succeed
static SkPicture* create_save_layer_opt_2_v3(SkTDArray<DrawType>* preOptPattern,
                                             SkTDArray<DrawType>* postOptPattern,
                                             const SkBitmap& checkerBoard) {
    return create_save_layer_opt_2(preOptPattern, postOptPattern, checkerBoard,
                                   true,   // saveLayer has a paint
                                   false,  // dbmr2r doesn't have a paint!
                                   true);  // color matching not really applicable
}

// version in which the optimization fails b.c. the colors don't match
static SkPicture* create_save_layer_opt_2_v4(SkTDArray<DrawType>* preOptPattern,
                                             SkTDArray<DrawType>* postOptPattern,
                                             const SkBitmap& checkerBoard) {
    return create_save_layer_opt_2(preOptPattern, postOptPattern, checkerBoard,
                                   true,   // saveLayer has a paint
                                   true,   // dbmr2r has a paint
                                   false); // and the colors don't match!
}

// As our .skp optimizations get folded into the captured skps our code will
// no longer be locally exercised. This GM manually constructs the patterns
// our optimizations will remove to test them. It acts as both a GM and a unit
// test
class OptimizationsGM : public skiagm::GM {
public:
    OptimizationsGM() {
        this->makeCheckerboard();
    }

    static const int kWidth = 800;
    static const int kHeight = 800;

protected:
    SkString onShortName() {
        return SkString("optimizations");
    }

    SkISize onISize() { return SkISize::Make(kWidth, kHeight); }

    typedef SkPicture* (*PFCreateOpt)(SkTDArray<DrawType> *preOptPattern,
                                      SkTDArray<DrawType> *postOptPattern,
                                      const SkBitmap& checkerBoard);

    virtual void onDraw(SkCanvas* canvas) {

        PFCreateOpt gOpts[] = {
            create_save_layer_opt_1_v1,
            create_save_layer_opt_1_v2,
            create_save_layer_opt_1_v3,
            create_save_layer_opt_1_v4,
            create_save_layer_opt_2_v1,
            create_save_layer_opt_2_v2,
            create_save_layer_opt_2_v3,
            create_save_layer_opt_2_v4,
        };

        SkTDArray<DrawType> prePattern, postPattern;
        int xPos = 0, yPos = 0;

        for (size_t i = 0; i < SK_ARRAY_COUNT(gOpts); ++i) {
            SkAutoTUnref<SkPicture> pre((*gOpts[i])(&prePattern, &postPattern, fCheckerboard));

            if (!(check_pattern(*pre, prePattern))) {
                WARN("Pre optimization pattern mismatch");
                SkASSERT(0);
            }

            canvas->save();
                canvas->translate(SkIntToScalar(xPos), SkIntToScalar(yPos));
                pre->draw(canvas);
                xPos += pre->width();
            canvas->restore();

            // re-render the 'pre' picture and thus 'apply' the optimization
            SkAutoTUnref<SkPicture> post(new SkPicture);

            SkCanvas* recordCanvas = post->beginRecording(pre->width(), pre->height());

            pre->draw(recordCanvas);

            post->endRecording();

            if (!(check_pattern(*post, postPattern))) {
                WARN("Post optimization pattern mismatch");
                SkASSERT(0);
            }

            canvas->save();
                canvas->translate(SkIntToScalar(xPos), SkIntToScalar(yPos));
                post->draw(canvas);
                xPos += post->width();
            canvas->restore();

            if (xPos >= kWidth) {
                // start a new line
                xPos = 0;
                yPos += post->height();
            }

            // TODO: we could also render the pre and post pictures to bitmaps
            // and manually compare them in this method
        }
    }

private:
    void makeCheckerboard() {
        static const unsigned int kCheckerboardWidth = 16;
        static const unsigned int kCheckerboardHeight = 16;

        fCheckerboard.setConfig(SkBitmap::kARGB_8888_Config,
                                kCheckerboardWidth, kCheckerboardHeight);
        fCheckerboard.allocPixels();
        SkAutoLockPixels lock(fCheckerboard);
        for (unsigned int y = 0; y < kCheckerboardHeight; y += 2) {
            SkPMColor* scanline = fCheckerboard.getAddr32(0, y);
            for (unsigned int x = 0; x < kCheckerboardWidth; x += 2) {
                *scanline++ = 0xFFFFFFFF;
                *scanline++ = 0xFF000000;
            }
            scanline = fCheckerboard.getAddr32(0, y + 1);
            for (unsigned int x = 0; x < kCheckerboardWidth; x += 2) {
                *scanline++ = 0xFF000000;
                *scanline++ = 0xFFFFFFFF;
            }
        }
    }

    SkBitmap fCheckerboard;

    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new OptimizationsGM; )
