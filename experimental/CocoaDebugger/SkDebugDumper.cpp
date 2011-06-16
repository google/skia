#include "SkDebugDumper.h"
#include "SkString.h"
#include "SkPaint.h"
#include "SkShader.h"
#include "SkPathEffect.h"
#include "SkXfermode.h"
#include "SkColorFilter.h"
#include "SkPathEffect.h"
#include "SkMaskFilter.h"
#include "SkGradientShader.h"
#include "SkDebuggerViews.h"

bool gNeverSetToTrueJustNeedToFoolLinker;
static void init_effects() {
    if (gNeverSetToTrueJustNeedToFoolLinker) {
        SkPoint p = SkPoint::Make(0,0);
        SkPoint q = SkPoint::Make(100,100);
        SkPoint pts[] = {p, q};
        SkColor colors[] = { SK_ColorRED, SK_ColorGREEN };
        SkScalar pos[] = { 0, 1.0};
        SkGradientShader::CreateLinear(pts, colors, pos, 2, 
                                       SkShader::kMirror_TileMode);
    }
}

SkDebugDumper::SkDebugDumper(SkEventSinkID cID, SkEventSinkID clID, 
                             SkEventSinkID ipID) {
    fContentID = cID;
    fCommandListID = clID;
    fInfoPanelID = ipID;
    fInit = false;
    fDisabled = false;
    fCount = 0;
    init_effects();
}

static void appendPtr(SkString* str, const void* ptr, const char name[]) {
    if (ptr) {
        str->appendf("$s: %p\t", name, ptr);
    }
}

static void appendFlattenable(SkString* str, const SkFlattenable* ptr,
                              const char name[]) {
    if (ptr) {
        SkString info;
        if (ptr->toDumpString(&info)) {
            str->appendf("%s", info.c_str());
        } else {
            str->appendf("%s: %p", name, ptr);
        }
    }
}

static SkString dumpMatrix(SkDumpCanvasM* canvas) {
    SkString str;
    SkMatrix m = canvas->getTotalMatrix();
    str.appendf("Matrix:");
    str.appendf("Translate (%0.4g, %0.4g) ", 
                 SkScalarToFloat(m.get(SkMatrix::kMTransX)), 
                 SkScalarToFloat(m.get(SkMatrix::kMTransY)));
    str.appendf("Scale (%0.4g, %0.4g) ", 
                 SkScalarToFloat(m.get(SkMatrix::kMScaleX)), 
                 SkScalarToFloat(m.get(SkMatrix::kMScaleY)));
    str.appendf("Skew (%0.4g, %0.4g) ", 
                 SkScalarToFloat(m.get(SkMatrix::kMSkewX)), 
                 SkScalarToFloat(m.get(SkMatrix::kMSkewY)));
    str.appendf("Perspective (%0.4g, %0.4g, %0.4g) ", 
                 SkScalarToFloat(m.get(SkMatrix::kMPersp0)), 
                 SkScalarToFloat(m.get(SkMatrix::kMPersp1)),
                 SkScalarToFloat(m.get(SkMatrix::kMPersp2)));
    return str;
}

static SkString dumpClip(SkDumpCanvasM* canvas) {
    SkString str;
    SkPath p;
    int maxPts = 50;
    if (canvas->getTotalClip().getBoundaryPath(&p)) {
        SkPoint pts[maxPts];
        int numPts = p.getPoints(pts, maxPts);
        
        str.appendf("Clip: [ ");
        for (int i = 0; i < numPts; ++i) {
            str.appendf("(%0.4g, %0.4g)", pts[i].x(), pts[i].y());
            if (i < numPts-1)
                str.appendf(" , ");
        }
        str.appendf(" ]");
    }
    return str;
}

static const char* gPaintFlags[] = {
    "AntiAliasing",
    "Bitmap Filtering",
    "Dithering",
    "Underline Text",
    "Strike-Through Text",
    "Fake Bold Text",
    "Linear Text",
    "Subpixel Positioned Text",
    "Device Kerning Text",
    "LCD/Subpixel Glyph Rendering",
    "Embedded Bitmap Text",
    "Freetype Autohinting",
    
    "ALL"
};


static SkString dumpPaint(SkDumpCanvasM* canvas, const SkPaint* p,
                      SkDumpCanvasM::Verb verb) {
    SkString str;
    str.appendf("Color: #%08X\n", p->getColor()); 
    str.appendf("Flags: %s\n", gPaintFlags[p->getFlags()]);
    appendFlattenable(&str, p->getShader(), "shader");
    appendFlattenable(&str, p->getXfermode(), "xfermode");
    appendFlattenable(&str, p->getPathEffect(), "pathEffect");
    appendFlattenable(&str, p->getMaskFilter(), "maskFilter");
    appendFlattenable(&str, p->getPathEffect(), "pathEffect");
    appendFlattenable(&str, p->getColorFilter(), "filter");
    
    if (SkDumpCanvasM::kDrawText_Verb == verb) {
        str.appendf("Text Size:%0.4g\n", SkScalarToFloat(p->getTextSize()));
        appendPtr(&str, p->getTypeface(), "typeface");
    }
    
    return str;
}

void SkDebugDumper::dump(SkDumpCanvasM* canvas, SkDumpCanvasM::Verb verb,
                          const char str[], const SkPaint* p) {      
    if (!fDisabled) {
        SkString msg, tab;
        
        const int level = canvas->getNestLevel() + canvas->getSaveCount() - 1;
        SkASSERT(level >= 0);
        for (int i = 0; i < level; i++) {
            tab.append("| ");
        }
        
        msg.appendf("%03d: %s%s\n", fCount, tab.c_str(), str);
        ++fCount;
        if (!fInit) {
            SkEvent* cmd = new SkEvent(SkDebugger_CommandType);
            cmd->setString(SkDebugger_Atom, msg);
            cmd->post(fCommandListID, 100);
        }
        else {
            SkEvent* state = new SkEvent(SkDebugger_StateType);
            state->setString(SkDebugger_Matrix, dumpMatrix(canvas));
            state->setString(SkDebugger_Clip, dumpClip(canvas));
            if (p) {
                state->setString(SkDebugger_PaintInfo, dumpPaint(canvas, p, verb));
                state->getMetaData().setPtr(SkDebugger_Paint, (void*)p, PaintProc);
            }
            state->post(fInfoPanelID);
        }
    }
}