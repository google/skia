/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDebugCanvas.h"
#include "SkDevice.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkPictureRecord.h"
#include "SkPictureRecorder.h"
#include "SkStream.h"
#include "picture_utils.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

static void usage() {
    SkDebugf("Usage: filter -i inFile [-o outFile] [--input-dir path] [--output-dir path]\n");
    SkDebugf("                        [-h|--help]\n\n");
    SkDebugf("    -i inFile  : file to filter.\n");
    SkDebugf("    -o outFile : result of filtering.\n");
    SkDebugf("    --input-dir : process all files in dir with .skp extension.\n");
    SkDebugf("    --output-dir : results of filtering the input dir.\n");
    SkDebugf("    -h|--help  : Show this help message.\n");
}

// Is the supplied paint simply a color?
static bool is_simple(const SkPaint& p) {
    return NULL == p.getPathEffect() &&
           NULL == p.getShader() &&
           NULL == p.getXfermode() &&
           NULL == p.getMaskFilter() &&
           NULL == p.getColorFilter() &&
           NULL == p.getRasterizer() &&
           NULL == p.getLooper() &&
           NULL == p.getImageFilter();
}


// Check for:
//    SAVE_LAYER
//        DRAW_BITMAP_RECT_TO_RECT
//    RESTORE
// where the saveLayer's color can be moved into the drawBitmapRect
static bool check_0(SkDebugCanvas* canvas, int curCommand) {
    if (SkDrawCommand::kSaveLayer_OpType != canvas->getDrawCommandAt(curCommand)->getType() ||
        canvas->getSize() <= curCommand+2 ||
        SkDrawCommand::kDrawBitmapRect_OpType != canvas->getDrawCommandAt(curCommand+1)->getType() ||
        SkDrawCommand::kRestore_OpType != canvas->getDrawCommandAt(curCommand+2)->getType()) {
        return false;
    }

    SkSaveLayerCommand* saveLayer =
        (SkSaveLayerCommand*) canvas->getDrawCommandAt(curCommand);
    SkDrawBitmapRectCommand* dbmr =
        (SkDrawBitmapRectCommand*) canvas->getDrawCommandAt(curCommand+1);

    const SkPaint* saveLayerPaint = saveLayer->paint();
    SkPaint* dbmrPaint = dbmr->paint();

    // For this optimization we only fold the saveLayer and drawBitmapRect
    // together if the saveLayer's draw is simple (i.e., no fancy effects)
    // and the only difference in the colors is their alpha value
    SkColor layerColor = saveLayerPaint->getColor() | 0xFF000000; // force opaque
    SkColor dbmrColor = dbmrPaint->getColor() | 0xFF000000;       // force opaque

    // If either operation lacks a paint then the collapse is trivial
    return NULL == saveLayerPaint ||
           NULL == dbmrPaint ||
           (is_simple(*saveLayerPaint) && dbmrColor == layerColor);
}

// Fold the saveLayer's alpha into the drawBitmapRect and remove the saveLayer
// and restore
static void apply_0(SkDebugCanvas* canvas, int curCommand) {
    SkSaveLayerCommand* saveLayer =
        (SkSaveLayerCommand*) canvas->getDrawCommandAt(curCommand);
    const SkPaint* saveLayerPaint = saveLayer->paint();

    // if (NULL == saveLayerPaint) the dbmr's paint doesn't need to be changed
    if (saveLayerPaint) {
        SkDrawBitmapRectCommand* dbmr =
            (SkDrawBitmapRectCommand*) canvas->getDrawCommandAt(curCommand+1);
        SkPaint* dbmrPaint = dbmr->paint();

        if (NULL == dbmrPaint) {
            // if the DBMR doesn't have a paint just use the saveLayer's
            dbmr->setPaint(*saveLayerPaint);
        } else if (saveLayerPaint) {
            // Both paints are present so their alphas need to be combined
            SkColor color = saveLayerPaint->getColor();
            int a0 = SkColorGetA(color);

            color = dbmrPaint->getColor();
            int a1 = SkColorGetA(color);

            int newA = SkMulDiv255Round(a0, a1);
            SkASSERT(newA <= 0xFF);

            SkColor newColor = SkColorSetA(color, newA);
            dbmrPaint->setColor(newColor);
        }
    }

    canvas->deleteDrawCommandAt(curCommand+2);  // restore
    canvas->deleteDrawCommandAt(curCommand);    // saveLayer
}

// Check for:
//    SAVE_LAYER
//        SAVE
//            CLIP_RECT
//            DRAW_BITMAP_RECT_TO_RECT
//        RESTORE
//    RESTORE
// where the saveLayer's color can be moved into the drawBitmapRect
static bool check_1(SkDebugCanvas* canvas, int curCommand) {
    if (SkDrawCommand::kSaveLayer_OpType != canvas->getDrawCommandAt(curCommand)->getType() ||
        canvas->getSize() <= curCommand+5 ||
        SkDrawCommand::kSave_OpType != canvas->getDrawCommandAt(curCommand+1)->getType() ||
        SkDrawCommand::kClipRect_OpType != canvas->getDrawCommandAt(curCommand+2)->getType() ||
        SkDrawCommand::kDrawBitmapRect_OpType != canvas->getDrawCommandAt(curCommand+3)->getType() ||
        SkDrawCommand::kRestore_OpType != canvas->getDrawCommandAt(curCommand+4)->getType() ||
        SkDrawCommand::kRestore_OpType != canvas->getDrawCommandAt(curCommand+5)->getType()) {
        return false;
    }

    SkSaveLayerCommand* saveLayer =
        (SkSaveLayerCommand*) canvas->getDrawCommandAt(curCommand);
    SkDrawBitmapRectCommand* dbmr =
        (SkDrawBitmapRectCommand*) canvas->getDrawCommandAt(curCommand+3);

    const SkPaint* saveLayerPaint = saveLayer->paint();
    SkPaint* dbmrPaint = dbmr->paint();

    // For this optimization we only fold the saveLayer and drawBitmapRect
    // together if the saveLayer's draw is simple (i.e., no fancy effects) and
    // and the only difference in the colors is that the saveLayer's can have
    // an alpha while the drawBitmapRect's is opaque.
    // TODO: it should be possible to fold them together even if they both
    // have different non-255 alphas but this is low priority since we have
    // never seen that case
    // If either operation lacks a paint then the collapse is trivial
    SkColor layerColor = saveLayerPaint->getColor() | 0xFF000000; // force opaque

    return NULL == saveLayerPaint ||
           NULL == dbmrPaint ||
           (is_simple(*saveLayerPaint) && dbmrPaint->getColor() == layerColor);
}

// Fold the saveLayer's alpha into the drawBitmapRect and remove the saveLayer
// and restore
static void apply_1(SkDebugCanvas* canvas, int curCommand) {
    SkSaveLayerCommand* saveLayer =
        (SkSaveLayerCommand*) canvas->getDrawCommandAt(curCommand);
    const SkPaint* saveLayerPaint = saveLayer->paint();

    // if (NULL == saveLayerPaint) the dbmr's paint doesn't need to be changed
    if (saveLayerPaint) {
        SkDrawBitmapRectCommand* dbmr =
            (SkDrawBitmapRectCommand*) canvas->getDrawCommandAt(curCommand+3);
        SkPaint* dbmrPaint = dbmr->paint();

        if (NULL == dbmrPaint) {
            dbmr->setPaint(*saveLayerPaint);
        } else {
            SkColor newColor = SkColorSetA(dbmrPaint->getColor(),
                                           SkColorGetA(saveLayerPaint->getColor()));
            dbmrPaint->setColor(newColor);
        }
    }

    canvas->deleteDrawCommandAt(curCommand+5);    // restore
    canvas->deleteDrawCommandAt(curCommand);      // saveLayer
}

// Check for:
//    SAVE
//        CLIP_RECT
//        DRAW_RECT
//    RESTORE
// where the rect is entirely within the clip and the clip is an intersect
static bool check_2(SkDebugCanvas* canvas, int curCommand) {
    if (SkDrawCommand::kSave_OpType != canvas->getDrawCommandAt(curCommand)->getType() ||
        canvas->getSize() <= curCommand+4 ||
        SkDrawCommand::kClipRect_OpType != canvas->getDrawCommandAt(curCommand+1)->getType() ||
        SkDrawCommand::kDrawRect_OpType != canvas->getDrawCommandAt(curCommand+2)->getType() ||
        SkDrawCommand::kRestore_OpType != canvas->getDrawCommandAt(curCommand+3)->getType()) {
        return false;
    }

    SkClipRectCommand* cr =
        (SkClipRectCommand*) canvas->getDrawCommandAt(curCommand+1);
    SkDrawRectCommand* dr =
        (SkDrawRectCommand*) canvas->getDrawCommandAt(curCommand+2);

    if (SkRegion::kIntersect_Op != cr->op()) {
        return false;
    }

    return cr->rect().contains(dr->rect());
}

// Remove everything but the drawRect
static void apply_2(SkDebugCanvas* canvas, int curCommand) {
    canvas->deleteDrawCommandAt(curCommand+3);   // restore
    // drawRect
    canvas->deleteDrawCommandAt(curCommand+1);   // clipRect
    canvas->deleteDrawCommandAt(curCommand);     // save
}

// Check for:
//    SAVE
//        CLIP_RRECT
//        DRAW_RECT
//    RESTORE
// where the rect entirely encloses the clip
static bool check_3(SkDebugCanvas* canvas, int curCommand) {
    if (SkDrawCommand::kSave_OpType != canvas->getDrawCommandAt(curCommand)->getType() ||
        canvas->getSize() <= curCommand+4 ||
        SkDrawCommand::kClipRRect_OpType != canvas->getDrawCommandAt(curCommand+1)->getType() ||
        SkDrawCommand::kDrawRect_OpType != canvas->getDrawCommandAt(curCommand+2)->getType() ||
        SkDrawCommand::kRestore_OpType != canvas->getDrawCommandAt(curCommand+3)->getType()) {
        return false;
    }

    SkClipRRectCommand* crr =
        (SkClipRRectCommand*) canvas->getDrawCommandAt(curCommand+1);
    SkDrawRectCommand* dr  =
        (SkDrawRectCommand*) canvas->getDrawCommandAt(curCommand+2);

    if (SkRegion::kIntersect_Op != crr->op()) {
        return false;
    }

    return dr->rect().contains(crr->rrect().rect());
}

// Replace everything with a drawRRect with the paint from the drawRect
// and the AA settings from the clipRRect
static void apply_3(SkDebugCanvas* canvas, int curCommand) {

    canvas->deleteDrawCommandAt(curCommand+3);    // restore

    SkClipRRectCommand* crr =
        (SkClipRRectCommand*) canvas->getDrawCommandAt(curCommand+1);
    SkDrawRectCommand* dr  =
        (SkDrawRectCommand*) canvas->getDrawCommandAt(curCommand+2);

    // TODO: could skip paint re-creation if the AA settings already match
    SkPaint newPaint = dr->paint();
    newPaint.setAntiAlias(crr->doAA());
    SkDrawRRectCommand* drr = new SkDrawRRectCommand(crr->rrect(), newPaint);
    canvas->setDrawCommandAt(curCommand+2, drr);

    canvas->deleteDrawCommandAt(curCommand+1);    // clipRRect
    canvas->deleteDrawCommandAt(curCommand);      // save
}

// Check for:
//    SAVE
//        CLIP_RECT
//        DRAW_BITMAP_RECT_TO_RECT
//    RESTORE
// where the rect and drawBitmapRect dst exactly match
static bool check_4(SkDebugCanvas* canvas, int curCommand) {
    if (SkDrawCommand::kSave_OpType != canvas->getDrawCommandAt(curCommand)->getType() ||
        canvas->getSize() <= curCommand+4 ||
        SkDrawCommand::kClipRect_OpType != canvas->getDrawCommandAt(curCommand+1)->getType() ||
        SkDrawCommand::kDrawBitmapRect_OpType != canvas->getDrawCommandAt(curCommand+2)->getType() ||
        SkDrawCommand::kRestore_OpType != canvas->getDrawCommandAt(curCommand+3)->getType()) {
        return false;
    }

    SkClipRectCommand* cr =
        (SkClipRectCommand*) canvas->getDrawCommandAt(curCommand+1);
    SkDrawBitmapRectCommand* dbmr  =
        (SkDrawBitmapRectCommand*) canvas->getDrawCommandAt(curCommand+2);

    if (SkRegion::kIntersect_Op != cr->op()) {
        return false;
    }

    return dbmr->dstRect() == cr->rect();
}

// Remove everything but the drawBitmapRect
static void apply_4(SkDebugCanvas* canvas, int curCommand) {
    canvas->deleteDrawCommandAt(curCommand+3);    // restore
    // drawBitmapRectToRect
    canvas->deleteDrawCommandAt(curCommand+1);    // clipRect
    canvas->deleteDrawCommandAt(curCommand);      // save
}

// Check for:
//  SAVE
//      CLIP_RECT
//      SAVE_LAYER
//          SAVE
//              CLIP_RECT
//              SAVE_LAYER
//                  SAVE
//                      CLIP_RECT
//                      DRAWBITMAPRECTTORECT
//                  RESTORE
//              RESTORE
//          RESTORE
//      RESTORE
//  RESTORE
// where:
//      all the clipRect's are BW, nested, intersections
//      the drawBitmapRectToRect is a 1-1 copy from src to dest
//      the last (smallest) clip rect is a subset of the drawBitmapRectToRect's dest rect
//      all the saveLayer's paints can be rolled into the drawBitmapRectToRect's paint
// This pattern is used by Google spreadsheet when drawing the toolbar buttons
static bool check_7(SkDebugCanvas* canvas, int curCommand) {
    if (SkDrawCommand::kSave_OpType != canvas->getDrawCommandAt(curCommand)->getType() ||
        canvas->getSize() <= curCommand+13 ||
        SkDrawCommand::kClipRect_OpType != canvas->getDrawCommandAt(curCommand+1)->getType() ||
        SkDrawCommand::kSaveLayer_OpType != canvas->getDrawCommandAt(curCommand+2)->getType() ||
        SkDrawCommand::kSave_OpType != canvas->getDrawCommandAt(curCommand+3)->getType() ||
        SkDrawCommand::kClipRect_OpType != canvas->getDrawCommandAt(curCommand+4)->getType() ||
        SkDrawCommand::kSaveLayer_OpType != canvas->getDrawCommandAt(curCommand+5)->getType() ||
        SkDrawCommand::kSave_OpType != canvas->getDrawCommandAt(curCommand+6)->getType() ||
        SkDrawCommand::kClipRect_OpType != canvas->getDrawCommandAt(curCommand+7)->getType() ||
        SkDrawCommand::kDrawBitmapRect_OpType != canvas->getDrawCommandAt(curCommand+8)->getType() ||
        SkDrawCommand::kRestore_OpType != canvas->getDrawCommandAt(curCommand+9)->getType() ||
        SkDrawCommand::kRestore_OpType != canvas->getDrawCommandAt(curCommand+10)->getType() ||
        SkDrawCommand::kRestore_OpType != canvas->getDrawCommandAt(curCommand+11)->getType() ||
        SkDrawCommand::kRestore_OpType != canvas->getDrawCommandAt(curCommand+12)->getType() ||
        SkDrawCommand::kRestore_OpType != canvas->getDrawCommandAt(curCommand+13)->getType()) {
        return false;
    }

    SkClipRectCommand* clip0 =
        (SkClipRectCommand*) canvas->getDrawCommandAt(curCommand+1);
    SkSaveLayerCommand* saveLayer0 =
        (SkSaveLayerCommand*) canvas->getDrawCommandAt(curCommand+2);
    SkClipRectCommand* clip1 =
        (SkClipRectCommand*) canvas->getDrawCommandAt(curCommand+4);
    SkSaveLayerCommand* saveLayer1 =
        (SkSaveLayerCommand*) canvas->getDrawCommandAt(curCommand+5);
    SkClipRectCommand* clip2 =
        (SkClipRectCommand*) canvas->getDrawCommandAt(curCommand+7);
    SkDrawBitmapRectCommand* dbmr =
        (SkDrawBitmapRectCommand*) canvas->getDrawCommandAt(curCommand+8);

    if (clip0->doAA() || clip1->doAA() || clip2->doAA()) {
        return false;
    }

    if (SkRegion::kIntersect_Op != clip0->op() ||
        SkRegion::kIntersect_Op != clip1->op() ||
        SkRegion::kIntersect_Op != clip2->op()) {
        return false;
    }

    if (!clip0->rect().contains(clip1->rect()) ||
        !clip1->rect().contains(clip2->rect())) {
        return false;
    }

    // The src->dest mapping needs to be 1-to-1
    if (NULL == dbmr->srcRect()) {
        if (dbmr->bitmap().width() != dbmr->dstRect().width() ||
            dbmr->bitmap().height() != dbmr->dstRect().height()) {
            return false;
        }
    } else {
        if (dbmr->srcRect()->width() != dbmr->dstRect().width() ||
            dbmr->srcRect()->height() != dbmr->dstRect().height()) {
            return false;
        }
    }

    if (!dbmr->dstRect().contains(clip2->rect())) {
        return false;
    }

    const SkPaint* saveLayerPaint0 = saveLayer0->paint();
    const SkPaint* saveLayerPaint1 = saveLayer1->paint();

    if ((saveLayerPaint0 && !is_simple(*saveLayerPaint0)) ||
        (saveLayerPaint1 && !is_simple(*saveLayerPaint1))) {
        return false;
    }

    SkPaint* dbmrPaint = dbmr->paint();

    if (NULL == dbmrPaint) {
        return true;
    }

    if (saveLayerPaint0) {
        SkColor layerColor0 = saveLayerPaint0->getColor() | 0xFF000000; // force opaque
        if (dbmrPaint->getColor() != layerColor0) {
            return false;
        }
    }

    if (saveLayerPaint1) {
        SkColor layerColor1 = saveLayerPaint1->getColor() | 0xFF000000; // force opaque
        if (dbmrPaint->getColor() != layerColor1) {
            return false;
        }
    }

    return true;
}

// Reduce to a single drawBitmapRectToRect call by folding the clipRect's into
// the src and dst Rects and the saveLayer paints into the drawBitmapRectToRect's
// paint.
static void apply_7(SkDebugCanvas* canvas, int curCommand) {
    SkSaveLayerCommand* saveLayer0 =
        (SkSaveLayerCommand*) canvas->getDrawCommandAt(curCommand+2);
    SkSaveLayerCommand* saveLayer1 =
        (SkSaveLayerCommand*) canvas->getDrawCommandAt(curCommand+5);
    SkClipRectCommand* clip2 =
        (SkClipRectCommand*) canvas->getDrawCommandAt(curCommand+7);
    SkDrawBitmapRectCommand* dbmr =
        (SkDrawBitmapRectCommand*) canvas->getDrawCommandAt(curCommand+8);

    SkScalar newSrcLeft = dbmr->srcRect()->fLeft + clip2->rect().fLeft - dbmr->dstRect().fLeft;
    SkScalar newSrcTop = dbmr->srcRect()->fTop + clip2->rect().fTop - dbmr->dstRect().fTop;

    SkRect newSrc = SkRect::MakeXYWH(newSrcLeft, newSrcTop,
                                     clip2->rect().width(), clip2->rect().height());

    dbmr->setSrcRect(newSrc);
    dbmr->setDstRect(clip2->rect());

    SkColor color = 0xFF000000;
    int a0, a1;

    const SkPaint* saveLayerPaint0 = saveLayer0->paint();
    if (saveLayerPaint0) {
        color = saveLayerPaint0->getColor();
        a0 = SkColorGetA(color);
    } else {
        a0 = 0xFF;
    }

    const SkPaint* saveLayerPaint1 = saveLayer1->paint();
    if (saveLayerPaint1) {
        color = saveLayerPaint1->getColor();
        a1 = SkColorGetA(color);
    } else {
        a1 = 0xFF;
    }

    int newA = SkMulDiv255Round(a0, a1);
    SkASSERT(newA <= 0xFF);

    SkPaint* dbmrPaint = dbmr->paint();

    if (dbmrPaint) {
        SkColor newColor = SkColorSetA(dbmrPaint->getColor(), newA);
        dbmrPaint->setColor(newColor);
    } else {
        SkColor newColor = SkColorSetA(color, newA);

        SkPaint newPaint;
        newPaint.setColor(newColor);
        dbmr->setPaint(newPaint);
    }

    // remove everything except the drawbitmaprect
    canvas->deleteDrawCommandAt(curCommand+13);   // restore
    canvas->deleteDrawCommandAt(curCommand+12);   // restore
    canvas->deleteDrawCommandAt(curCommand+11);   // restore
    canvas->deleteDrawCommandAt(curCommand+10);   // restore
    canvas->deleteDrawCommandAt(curCommand+9);    // restore
    canvas->deleteDrawCommandAt(curCommand+7);    // clipRect
    canvas->deleteDrawCommandAt(curCommand+6);    // save
    canvas->deleteDrawCommandAt(curCommand+5);    // saveLayer
    canvas->deleteDrawCommandAt(curCommand+4);    // clipRect
    canvas->deleteDrawCommandAt(curCommand+3);    // save
    canvas->deleteDrawCommandAt(curCommand+2);    // saveLayer
    canvas->deleteDrawCommandAt(curCommand+1);    // clipRect
    canvas->deleteDrawCommandAt(curCommand);      // save
}

// Check for:
//    SAVE
//       CLIP_RECT
//       DRAWBITMAPRECTTORECT
//    RESTORE
// where:
//      the drawBitmapRectToRect is a 1-1 copy from src to dest
//      the clip rect is BW and a subset of the drawBitmapRectToRect's dest rect
static bool check_8(SkDebugCanvas* canvas, int curCommand) {
    if (SkDrawCommand::kSave_OpType != canvas->getDrawCommandAt(curCommand)->getType() ||
        canvas->getSize() <= curCommand+4 ||
        SkDrawCommand::kClipRect_OpType != canvas->getDrawCommandAt(curCommand+1)->getType() ||
        SkDrawCommand::kDrawBitmapRect_OpType != canvas->getDrawCommandAt(curCommand+2)->getType() ||
        SkDrawCommand::kRestore_OpType != canvas->getDrawCommandAt(curCommand+3)->getType()) {
        return false;
    }

    SkClipRectCommand* clip =
        (SkClipRectCommand*) canvas->getDrawCommandAt(curCommand+1);
    SkDrawBitmapRectCommand* dbmr =
        (SkDrawBitmapRectCommand*) canvas->getDrawCommandAt(curCommand+2);

    if (clip->doAA() || SkRegion::kIntersect_Op != clip->op()) {
        return false;
    }

    // The src->dest mapping needs to be 1-to-1
    if (NULL == dbmr->srcRect()) {
        if (dbmr->bitmap().width() != dbmr->dstRect().width() ||
            dbmr->bitmap().height() != dbmr->dstRect().height()) {
            return false;
        }
    } else {
        if (dbmr->srcRect()->width() != dbmr->dstRect().width() ||
            dbmr->srcRect()->height() != dbmr->dstRect().height()) {
            return false;
        }
    }

    if (!dbmr->dstRect().contains(clip->rect())) {
        return false;
    }

    return true;
}

// Fold the clipRect into the drawBitmapRectToRect's src and dest rects
static void apply_8(SkDebugCanvas* canvas, int curCommand) {
    SkClipRectCommand* clip =
        (SkClipRectCommand*) canvas->getDrawCommandAt(curCommand+1);
    SkDrawBitmapRectCommand* dbmr =
        (SkDrawBitmapRectCommand*) canvas->getDrawCommandAt(curCommand+2);

    SkScalar newSrcLeft, newSrcTop;

    if (dbmr->srcRect()) {
        newSrcLeft = dbmr->srcRect()->fLeft + clip->rect().fLeft - dbmr->dstRect().fLeft;
        newSrcTop  = dbmr->srcRect()->fTop + clip->rect().fTop - dbmr->dstRect().fTop;
    } else {
        newSrcLeft = clip->rect().fLeft - dbmr->dstRect().fLeft;
        newSrcTop  = clip->rect().fTop - dbmr->dstRect().fTop;
    }

    SkRect newSrc = SkRect::MakeXYWH(newSrcLeft, newSrcTop,
                                     clip->rect().width(), clip->rect().height());

    dbmr->setSrcRect(newSrc);
    dbmr->setDstRect(clip->rect());

    // remove everything except the drawbitmaprect
    canvas->deleteDrawCommandAt(curCommand+3);
    canvas->deleteDrawCommandAt(curCommand+1);
    canvas->deleteDrawCommandAt(curCommand);
}

// Check for:
//  SAVE
//    CLIP_RECT
//    DRAWBITMAPRECTTORECT
//  RESTORE
// where:
//      clipRect is BW and encloses the DBMR2R's dest rect
static bool check_9(SkDebugCanvas* canvas, int curCommand) {
    if (SkDrawCommand::kSave_OpType != canvas->getDrawCommandAt(curCommand)->getType() ||
        canvas->getSize() <= curCommand+4 ||
        SkDrawCommand::kClipRect_OpType != canvas->getDrawCommandAt(curCommand+1)->getType() ||
        SkDrawCommand::kDrawBitmapRect_OpType != canvas->getDrawCommandAt(curCommand+2)->getType() ||
        SkDrawCommand::kRestore_OpType != canvas->getDrawCommandAt(curCommand+3)->getType()) {
        return false;
    }

    SkClipRectCommand* clip =
        (SkClipRectCommand*) canvas->getDrawCommandAt(curCommand+1);
    SkDrawBitmapRectCommand* dbmr =
        (SkDrawBitmapRectCommand*) canvas->getDrawCommandAt(curCommand+2);

    if (clip->doAA() || SkRegion::kIntersect_Op != clip->op()) {
        return false;
    }

    if (!clip->rect().contains(dbmr->dstRect())) {
        return false;
    }

    return true;
}

// remove everything except the drawbitmaprect
static void apply_9(SkDebugCanvas* canvas, int curCommand) {
    canvas->deleteDrawCommandAt(curCommand+3);   // restore
    // drawBitmapRectToRect
    canvas->deleteDrawCommandAt(curCommand+1);   // clipRect
    canvas->deleteDrawCommandAt(curCommand);     // save
}

typedef bool (*PFCheck)(SkDebugCanvas* canvas, int curCommand);
typedef void (*PFApply)(SkDebugCanvas* canvas, int curCommand);

struct OptTableEntry {
    PFCheck fCheck;
    PFApply fApply;
    int fNumTimesApplied;
} gOptTable[] = {
    { check_0, apply_0, 0 },
    { check_1, apply_1, 0 },
    { check_2, apply_2, 0 },
    { check_3, apply_3, 0 },
    { check_4, apply_4, 0 },
    { check_7, apply_7, 0 },
    { check_8, apply_8, 0 },
    { check_9, apply_9, 0 },
};


static int filter_picture(const SkString& inFile, const SkString& outFile) {
    SkAutoTUnref<SkPicture> inPicture;

    SkFILEStream inStream(inFile.c_str());
    if (inStream.isValid()) {
        inPicture.reset(SkPicture::CreateFromStream(&inStream));
    }

    if (NULL == inPicture.get()) {
        SkDebugf("Could not read file %s\n", inFile.c_str());
        return -1;
    }

    int localCount[SK_ARRAY_COUNT(gOptTable)];

    memset(localCount, 0, sizeof(localCount));

    SkDebugCanvas debugCanvas(SkScalarCeilToInt(inPicture->cullRect().width()),
                              SkScalarCeilToInt(inPicture->cullRect().height()));
    inPicture->playback(&debugCanvas);

    // delete the initial save and restore since replaying the commands will
    // re-add them
    if (debugCanvas.getSize() > 1) {
        debugCanvas.deleteDrawCommandAt(0);
        debugCanvas.deleteDrawCommandAt(debugCanvas.getSize()-1);
    }

    bool changed = true;
    int numBefore = debugCanvas.getSize();

    while (changed) {
        changed = false;
        for (int i = 0; i < debugCanvas.getSize(); ++i) {
            for (size_t opt = 0; opt < SK_ARRAY_COUNT(gOptTable); ++opt) {
                if ((*gOptTable[opt].fCheck)(&debugCanvas, i)) {
                    (*gOptTable[opt].fApply)(&debugCanvas, i);

                    ++gOptTable[opt].fNumTimesApplied;
                    ++localCount[opt];

                    if (debugCanvas.getSize() == i) {
                        // the optimization removed all the remaining operations
                        break;
                    }

                    opt = 0;          // try all the opts all over again
                    changed = true;
                }
            }
        }
    }

    int numAfter = debugCanvas.getSize();

    if (!outFile.isEmpty()) {
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(inPicture->cullRect().width(),
                                                   inPicture->cullRect().height(),
                                                   NULL, 0);
        debugCanvas.draw(canvas);
        SkAutoTUnref<SkPicture> outPicture(recorder.endRecording());

        SkFILEWStream outStream(outFile.c_str());

        outPicture->serialize(&outStream);
    }

    bool someOptFired = false;
    for (size_t opt = 0; opt < SK_ARRAY_COUNT(gOptTable); ++opt) {
        if (0 != localCount[opt]) {
            SkDebugf("%d: %d ", opt, localCount[opt]);
            someOptFired = true;
        }
    }

    if (!someOptFired) {
        SkDebugf("No opts fired\n");
    } else {
        SkDebugf("\t before: %d after: %d delta: %d\n",
                 numBefore, numAfter, numBefore-numAfter);
    }

    return 0;
}

// This function is not marked as 'static' so it can be referenced externally
// in the iOS build.
int tool_main(int argc, char** argv); // suppress a warning on mac

int tool_main(int argc, char** argv) {
    SkGraphics::Init();

    if (argc < 3) {
        usage();
        return -1;
    }

    SkString inFile, outFile, inDir, outDir;

    char* const* stop = argv + argc;
    for (++argv; argv < stop; ++argv) {
        if (strcmp(*argv, "-i") == 0) {
            argv++;
            if (argv < stop && **argv) {
                inFile.set(*argv);
            } else {
                SkDebugf("missing arg for -i\n");
                usage();
                return -1;
            }
        } else if (strcmp(*argv, "--input-dir") == 0) {
            argv++;
            if (argv < stop && **argv) {
                inDir.set(*argv);
            } else {
                SkDebugf("missing arg for --input-dir\n");
                usage();
                return -1;
            }
        } else if (strcmp(*argv, "--output-dir") == 0) {
            argv++;
            if (argv < stop && **argv) {
                outDir.set(*argv);
            } else {
                SkDebugf("missing arg for --output-dir\n");
                usage();
                return -1;
            }
        } else if (strcmp(*argv, "-o") == 0) {
            argv++;
            if (argv < stop && **argv) {
                outFile.set(*argv);
            } else {
                SkDebugf("missing arg for -o\n");
                usage();
                return -1;
            }
        } else if (strcmp(*argv, "--help") == 0 || strcmp(*argv, "-h") == 0) {
            usage();
            return 0;
        } else {
            SkDebugf("unknown arg %s\n", *argv);
            usage();
            return -1;
        }
    }

    SkOSFile::Iter iter(inDir.c_str(), "skp");

    SkString inputFilename, outputFilename;
    if (iter.next(&inputFilename)) {

        do {
            inFile = SkOSPath::Join(inDir.c_str(), inputFilename.c_str());
            if (!outDir.isEmpty()) {
                outFile = SkOSPath::Join(outDir.c_str(), inputFilename.c_str());
            }
            SkDebugf("Executing %s\n", inputFilename.c_str());
            filter_picture(inFile, outFile);
        } while(iter.next(&inputFilename));

    } else if (!inFile.isEmpty()) {
        filter_picture(inFile, outFile);
    } else {
        usage();
        return -1;
    }

    for (size_t opt = 0; opt < SK_ARRAY_COUNT(gOptTable); ++opt) {
        SkDebugf("opt %d: %d\n", opt, gOptTable[opt].fNumTimesApplied);
    }

    SkGraphics::Term();
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
