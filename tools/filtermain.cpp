/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDebugCanvas.h"
#include "SkDevice.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkPicturePlayback.h"
#include "SkPictureRecord.h"
#include "SkStream.h"
#include "picture_utils.h"
#include "path_utils.h"

static void usage() {
    SkDebugf("Usage: filter -i inFile [-o outFile] [--input-dir path] [--output-dir path]\n");
    SkDebugf("                        [-h|--help]\n\n");
    SkDebugf("    -i inFile  : file to file.\n");
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
static bool check_0(const SkTDArray<SkDrawCommand*>& commands, int curCommand) {
    if (SAVE_LAYER != commands[curCommand]->getType() ||
        commands.count() <= curCommand+2 ||
        DRAW_BITMAP_RECT_TO_RECT != commands[curCommand+1]->getType() ||
        RESTORE != commands[curCommand+2]->getType())
        return false;

    SaveLayer* saveLayer = (SaveLayer*) commands[curCommand];
    DrawBitmapRect* dbmr = (DrawBitmapRect*) commands[curCommand+1];

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
static void apply_0(SkTDArray<SkDrawCommand*>& commands, int curCommand) {
    SaveLayer* saveLayer = (SaveLayer*) commands[curCommand];
    DrawBitmapRect* dbmr = (DrawBitmapRect*) commands[curCommand+1];
    Restore* restore = (Restore*) commands[curCommand+2];

    const SkPaint* saveLayerPaint = saveLayer->paint();
    SkPaint* dbmrPaint = dbmr->paint();

    if (NULL == saveLayerPaint) {
        saveLayer->setVisible(false);
        restore->setVisible(false);
    } else if (NULL == dbmrPaint) {
        saveLayer->setVisible(false);
        dbmr->setPaint(*saveLayerPaint);
        restore->setVisible(false);
    } else {
        saveLayer->setVisible(false);
        SkColor newColor = SkColorSetA(dbmrPaint->getColor(),
                                       SkColorGetA(saveLayerPaint->getColor()));
        dbmrPaint->setColor(newColor);
        restore->setVisible(false);
    }
}

// Check for:
//    SAVE_LAYER
//        SAVE
//            CLIP_RECT
//            DRAW_BITMAP_RECT_TO_RECT
//        RESTORE
//    RESTORE
// where the saveLayer's color can be moved into the drawBitmapRect
static bool check_1(const SkTDArray<SkDrawCommand*>& commands, int curCommand) {
    if (SAVE_LAYER != commands[curCommand]->getType() ||
        commands.count() <= curCommand+5 ||
        SAVE != commands[curCommand+1]->getType() ||
        CLIP_RECT != commands[curCommand+2]->getType() ||
        DRAW_BITMAP_RECT_TO_RECT != commands[curCommand+3]->getType() ||
        RESTORE != commands[curCommand+4]->getType() ||
        RESTORE != commands[curCommand+5]->getType())
        return false;

    SaveLayer* saveLayer = (SaveLayer*) commands[curCommand];
    DrawBitmapRect* dbmr = (DrawBitmapRect*) commands[curCommand+3];

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
static void apply_1(SkTDArray<SkDrawCommand*>& commands, int curCommand) {
    SaveLayer* saveLayer = (SaveLayer*) commands[curCommand];
    DrawBitmapRect* dbmr = (DrawBitmapRect*) commands[curCommand+3];
    Restore* restore = (Restore*) commands[curCommand+5];

    const SkPaint* saveLayerPaint = saveLayer->paint();
    SkPaint* dbmrPaint = dbmr->paint();

    if (NULL == saveLayerPaint) {
        saveLayer->setVisible(false);
        restore->setVisible(false);
    } else if (NULL == dbmrPaint) {
        saveLayer->setVisible(false);
        dbmr->setPaint(*saveLayerPaint);
        restore->setVisible(false);
    } else {
        saveLayer->setVisible(false);
        SkColor newColor = SkColorSetA(dbmrPaint->getColor(),
                                       SkColorGetA(saveLayerPaint->getColor()));
        dbmrPaint->setColor(newColor);
        restore->setVisible(false);
    }
}

// Check for:
//    SAVE
//        CLIP_RECT
//        DRAW_RECT
//    RESTORE
// where the rect is entirely within the clip and the clip is an intersect
static bool check_2(const SkTDArray<SkDrawCommand*>& commands, int curCommand) {
    if (SAVE != commands[curCommand]->getType() ||
        commands.count() <= curCommand+4 ||
        CLIP_RECT != commands[curCommand+1]->getType() ||
        DRAW_RECT != commands[curCommand+2]->getType() ||
        RESTORE != commands[curCommand+3]->getType())
        return false;

    ClipRect* cr = (ClipRect*) commands[curCommand+1];
    DrawRectC* dr = (DrawRectC*) commands[curCommand+2];

    if (SkRegion::kIntersect_Op != cr->op()) {
        return false;
    }

    return cr->rect().contains(dr->rect());
}

// Remove everything but the drawRect
static void apply_2(SkTDArray<SkDrawCommand*>& commands, int curCommand) {
    Save* save = (Save*) commands[curCommand];
    ClipRect* cr = (ClipRect*) commands[curCommand+1];
    Restore* restore = (Restore*) commands[curCommand+3];

    save->setVisible(false);
    cr->setVisible(false);
    // leave the drawRect alone
    restore->setVisible(false);
}

// Check for:
//    SAVE
//        CLIP_RRECT
//        DRAW_RECT
//    RESTORE
// where the rect entirely encloses the clip
static bool check_3(const SkTDArray<SkDrawCommand*>& commands, int curCommand) {
    if (SAVE != commands[curCommand]->getType() ||
        commands.count() <= curCommand+4 ||
        CLIP_RRECT != commands[curCommand+1]->getType() ||
        DRAW_RECT != commands[curCommand+2]->getType() ||
        RESTORE != commands[curCommand+3]->getType())
        return false;

    ClipRRect* crr = (ClipRRect*) commands[curCommand+1];
    DrawRectC* dr  = (DrawRectC*) commands[curCommand+2];

    if (SkRegion::kIntersect_Op != crr->op()) {
        return false;
    }

    return dr->rect().contains(crr->rrect().rect());
}

// Replace everything with a drawRRect with the paint from the drawRect
// and the AA settings from the clipRRect
static void apply_3(SkTDArray<SkDrawCommand*>& commands, int curCommand) {
    Save* save = (Save*) commands[curCommand];
    ClipRRect* crr = (ClipRRect*) commands[curCommand+1];
    DrawRectC* dr = (DrawRectC*) commands[curCommand+2];
    Restore* restore = (Restore*) commands[curCommand+3];

    save->setVisible(false);
    crr->setVisible(false);
    dr->setVisible(false);
    restore->setVisible(false);

    // TODO: could skip paint re-creation if the AA settings already match
    SkPaint newPaint = dr->paint();
    newPaint.setAntiAlias(crr->doAA());
    DrawRRect* drr = new DrawRRect(crr->rrect(), newPaint);
    commands[curCommand+2] = drr;
}

// Check for:
//    SAVE
//        CLIP_RECT
//        DRAW_BITMAP_RECT_TO_RECT
//    RESTORE
// where the rect and drawBitmapRect dst exactly match
static bool check_4(const SkTDArray<SkDrawCommand*>& commands, int curCommand) {
    if (SAVE != commands[curCommand]->getType() ||
        commands.count() <= curCommand+4 ||
        CLIP_RECT != commands[curCommand+1]->getType() ||
        DRAW_BITMAP_RECT_TO_RECT != commands[curCommand+2]->getType() ||
        RESTORE != commands[curCommand+3]->getType())
        return false;

    ClipRect* cr = (ClipRect*) commands[curCommand+1];
    DrawBitmapRect* dbmr  = (DrawBitmapRect*) commands[curCommand+2];

    if (SkRegion::kIntersect_Op != cr->op()) {
        return false;
    }

    return dbmr->dstRect() == cr->rect();
}

// Remove everything but the drawBitmapRect
static void apply_4(SkTDArray<SkDrawCommand*>& commands, int curCommand) {
    Save* save = (Save*) commands[curCommand];
    ClipRect* cr = (ClipRect*) commands[curCommand+1];
    Restore* restore = (Restore*) commands[curCommand+3];

    save->setVisible(false);
    cr->setVisible(false);
    // leave drawBitmapRect alone
    restore->setVisible(false);
}

// Check for:
//    TRANSLATE
// where the translate is zero
static bool check_5(const SkTDArray<SkDrawCommand*>& commands, int curCommand) {
    if (TRANSLATE != commands[curCommand]->getType()) {
        return false;
    }

    Translate* t = (Translate*) commands[curCommand];

    return 0 == t->x() && 0 == t->y();
}

// Just remove the translate
static void apply_5(SkTDArray<SkDrawCommand*>& commands, int curCommand) {
    Translate* t = (Translate*) commands[curCommand];

    t->setVisible(false);
}

// Check for:
//    SCALE
// where the scale is 1,1
static bool check_6(const SkTDArray<SkDrawCommand*>& commands, int curCommand) {
    if (SCALE != commands[curCommand]->getType()) {
        return false;
    }

    Scale* s = (Scale*) commands[curCommand];

    return SK_Scalar1 == s->x() && SK_Scalar1 == s->y();
}

// Just remove the scale
static void apply_6(SkTDArray<SkDrawCommand*>& commands, int curCommand) {
    Scale* s = (Scale*) commands[curCommand];

    s->setVisible(false);
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
static bool check_7(const SkTDArray<SkDrawCommand*>& commands, int curCommand) {
    if (SAVE != commands[curCommand]->getType() ||
        commands.count() <= curCommand+13 ||
        CLIP_RECT != commands[curCommand+1]->getType() ||
        SAVE_LAYER != commands[curCommand+2]->getType() ||
        SAVE != commands[curCommand+3]->getType() ||
        CLIP_RECT != commands[curCommand+4]->getType() ||
        SAVE_LAYER != commands[curCommand+5]->getType() ||
        SAVE != commands[curCommand+6]->getType() ||
        CLIP_RECT != commands[curCommand+7]->getType() ||
        DRAW_BITMAP_RECT_TO_RECT != commands[curCommand+8]->getType() ||
        RESTORE != commands[curCommand+9]->getType() ||
        RESTORE != commands[curCommand+10]->getType() ||
        RESTORE != commands[curCommand+11]->getType() ||
        RESTORE != commands[curCommand+12]->getType() ||
        RESTORE != commands[curCommand+13]->getType())
        return false;

    ClipRect* clip0 = (ClipRect*) commands[curCommand+1];
    SaveLayer* saveLayer0 = (SaveLayer*) commands[curCommand+2];
    ClipRect* clip1 = (ClipRect*) commands[curCommand+4];
    SaveLayer* saveLayer1 = (SaveLayer*) commands[curCommand+5];
    ClipRect* clip2 = (ClipRect*) commands[curCommand+7];
    DrawBitmapRect* dbmr = (DrawBitmapRect*) commands[curCommand+8];

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

    if ((NULL != saveLayerPaint0 && !is_simple(*saveLayerPaint0)) ||
        (NULL != saveLayerPaint1 && !is_simple(*saveLayerPaint1))) {
        return false;
    }

    SkPaint* dbmrPaint = dbmr->paint();

    if (NULL == dbmrPaint) {
        return true;    
    }
    
    if (NULL != saveLayerPaint0) {
        SkColor layerColor0 = saveLayerPaint0->getColor() | 0xFF000000; // force opaque
        if (dbmrPaint->getColor() != layerColor0) {
            return false;
        }
    }

    if (NULL != saveLayerPaint1) {
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
static void apply_7(SkTDArray<SkDrawCommand*>& commands, int curCommand) {
    Save* save0 = (Save*) commands[curCommand];
    ClipRect* clip0 = (ClipRect*) commands[curCommand+1];
    SaveLayer* saveLayer0 = (SaveLayer*) commands[curCommand+2];
    Save* save1 = (Save*) commands[curCommand+3];
    ClipRect* clip1 = (ClipRect*) commands[curCommand+4];
    SaveLayer* saveLayer1 = (SaveLayer*) commands[curCommand+5];
    Save* save2 = (Save*) commands[curCommand+6];
    ClipRect* clip2 = (ClipRect*) commands[curCommand+7];
    DrawBitmapRect* dbmr = (DrawBitmapRect*) commands[curCommand+8];
    Restore* restore0 = (Restore*) commands[curCommand+9];
    Restore* restore1 = (Restore*) commands[curCommand+10];
    Restore* restore2 = (Restore*) commands[curCommand+11];
    Restore* restore3 = (Restore*) commands[curCommand+12];
    Restore* restore4 = (Restore*) commands[curCommand+13];

    SkScalar newSrcLeft = dbmr->srcRect()->fLeft + clip2->rect().fLeft - dbmr->dstRect().fLeft;
    SkScalar newSrcTop = dbmr->srcRect()->fTop + clip2->rect().fTop - dbmr->dstRect().fTop;

    SkRect newSrc = SkRect::MakeXYWH(newSrcLeft, newSrcTop, 
                                     clip2->rect().width(), clip2->rect().height());

    dbmr->setSrcRect(newSrc);
    dbmr->setDstRect(clip2->rect());

    SkColor color = 0xFF000000;
    int a0, a1;

    const SkPaint* saveLayerPaint0 = saveLayer0->paint();
    if (NULL != saveLayerPaint0) {
        color = saveLayerPaint0->getColor();
        a0 = SkColorGetA(color);
    } else {
        a0 = 0xFF;
    }

    const SkPaint* saveLayerPaint1 = saveLayer1->paint();
    if (NULL != saveLayerPaint1) {
        color = saveLayerPaint1->getColor();
        a1 = SkColorGetA(color);
    } else {
        a1 = 0xFF;
    }

    int newA = (a0 * a1) / 255;
    SkASSERT(newA <= 0xFF);

    SkPaint* dbmrPaint = dbmr->paint();

    if (NULL != dbmrPaint) {
        SkColor newColor = SkColorSetA(dbmrPaint->getColor(), newA);
        dbmrPaint->setColor(newColor);
    } else {
        SkColor newColor = SkColorSetA(color, newA);

        SkPaint newPaint;
        newPaint.setColor(newColor);
        dbmr->setPaint(newPaint);
    }

    // remove everything except the drawbitmaprect
    save0->setVisible(false);
    clip0->setVisible(false);
    saveLayer0->setVisible(false);
    save1->setVisible(false);
    clip1->setVisible(false);
    saveLayer1->setVisible(false);
    save2->setVisible(false);
    clip2->setVisible(false);
    restore0->setVisible(false);
    restore1->setVisible(false);
    restore2->setVisible(false);
    restore3->setVisible(false);
    restore4->setVisible(false);
}

typedef bool (*PFCheck)(const SkTDArray<SkDrawCommand*>& commands, int curCommand);
typedef void (*PFApply)(SkTDArray<SkDrawCommand*>& commands, int curCommand);

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
    { check_5, apply_5, 0 },
    { check_6, apply_6, 0 },
    { check_7, apply_7, 0 },
};

static int filter_picture(const SkString& inFile, const SkString& outFile) {
    SkPicture* inPicture = NULL;

    SkFILEStream inStream(inFile.c_str());
    if (inStream.isValid()) {
        inPicture = SkNEW_ARGS(SkPicture, (&inStream, NULL, &SkImageDecoder::DecodeMemory));
    }

    if (NULL == inPicture) {
        SkDebugf("Could not read file %s\n", inFile.c_str());
        return -1;
    }

    int localCount[SK_ARRAY_COUNT(gOptTable)];

    memset(localCount, 0, sizeof(localCount));

    SkDebugCanvas debugCanvas(inPicture->width(), inPicture->height());
    debugCanvas.setBounds(inPicture->width(), inPicture->height());
    inPicture->draw(&debugCanvas);

    SkTDArray<SkDrawCommand*>& commands = debugCanvas.getDrawCommands();

    // hide the initial save and restore since replaying the commands will
    // re-add them
    if (commands.count() > 0) {
        commands[0]->setVisible(false);
        commands[commands.count()-1]->setVisible(false);
    }

    for (int i = 0; i < commands.count(); ++i) {
        for (size_t opt = 0; opt < SK_ARRAY_COUNT(gOptTable); ++opt) {
            if ((*gOptTable[opt].fCheck)(commands, i)) {
                (*gOptTable[opt].fApply)(commands, i);
                ++gOptTable[opt].fNumTimesApplied;
                ++localCount[opt];
            }
        }
    }

    if (!outFile.isEmpty()) {
        SkPicture outPicture;

        SkCanvas* canvas = outPicture.beginRecording(inPicture->width(), inPicture->height());
        debugCanvas.draw(canvas);
        outPicture.endRecording();

        SkFILEWStream outStream(outFile.c_str());

        outPicture.serialize(&outStream);
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
        SkDebugf("\n");
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
            sk_tools::make_filepath(&inFile, inDir, inputFilename);
            if (!outDir.isEmpty()) {
                sk_tools::make_filepath(&outFile, outDir, inputFilename);
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
