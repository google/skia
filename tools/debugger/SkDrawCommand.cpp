/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDrawCommand.h"

#include <algorithm>
#include "SkAutoMalloc.h"
#include "SkCanvasPriv.h"
#include "SkClipOpPriv.h"
#include "SkColorFilter.h"
#include "SkDashPathEffect.h"
#include "SkDrawable.h"
#include "SkImageFilter.h"
#include "SkJsonWriteBuffer.h"
#include "SkLatticeIter.h"
#include "SkMaskFilterBase.h"
#include "SkPaintDefaults.h"
#include "SkPathEffect.h"
#include "SkPicture.h"
#include "SkPngEncoder.h"
#include "SkReadBuffer.h"
#include "SkRectPriv.h"
#include "SkShadowFlags.h"
#include "SkTHash.h"
#include "SkTextBlobPriv.h"
#include "SkTypeface.h"
#include "SkWriteBuffer.h"

#define SKDEBUGCANVAS_ATTRIBUTE_COMMAND           "command"
#define SKDEBUGCANVAS_ATTRIBUTE_VISIBLE           "visible"
#define SKDEBUGCANVAS_ATTRIBUTE_MATRIX            "matrix"
#define SKDEBUGCANVAS_ATTRIBUTE_DRAWDEPTHTRANS    "drawDepthTranslation"
#define SKDEBUGCANVAS_ATTRIBUTE_COORDS            "coords"
#define SKDEBUGCANVAS_ATTRIBUTE_EDGING            "edging"
#define SKDEBUGCANVAS_ATTRIBUTE_HINTING           "hinting"
#define SKDEBUGCANVAS_ATTRIBUTE_BOUNDS            "bounds"
#define SKDEBUGCANVAS_ATTRIBUTE_PAINT             "paint"
#define SKDEBUGCANVAS_ATTRIBUTE_OUTER             "outer"
#define SKDEBUGCANVAS_ATTRIBUTE_INNER             "inner"
#define SKDEBUGCANVAS_ATTRIBUTE_MODE              "mode"
#define SKDEBUGCANVAS_ATTRIBUTE_POINTS            "points"
#define SKDEBUGCANVAS_ATTRIBUTE_PATH              "path"
#define SKDEBUGCANVAS_ATTRIBUTE_TEXT              "text"
#define SKDEBUGCANVAS_ATTRIBUTE_COLOR             "color"
#define SKDEBUGCANVAS_ATTRIBUTE_ALPHA             "alpha"
#define SKDEBUGCANVAS_ATTRIBUTE_BLENDMODE         "blendMode"
#define SKDEBUGCANVAS_ATTRIBUTE_STYLE             "style"
#define SKDEBUGCANVAS_ATTRIBUTE_STROKEWIDTH       "strokeWidth"
#define SKDEBUGCANVAS_ATTRIBUTE_STROKEMITER       "strokeMiter"
#define SKDEBUGCANVAS_ATTRIBUTE_STROKEJOIN        "strokeJoin"
#define SKDEBUGCANVAS_ATTRIBUTE_CAP               "cap"
#define SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS         "antiAlias"
#define SKDEBUGCANVAS_ATTRIBUTE_DITHER            "dither"
#define SKDEBUGCANVAS_ATTRIBUTE_FAKEBOLDTEXT      "fakeBoldText"
#define SKDEBUGCANVAS_ATTRIBUTE_LINEARTEXT        "linearText"
#define SKDEBUGCANVAS_ATTRIBUTE_SUBPIXELTEXT      "subpixelText"
#define SKDEBUGCANVAS_ATTRIBUTE_DEVKERNTEXT       "devKernText"
#define SKDEBUGCANVAS_ATTRIBUTE_LCDRENDERTEXT     "lcdRenderText"
#define SKDEBUGCANVAS_ATTRIBUTE_EMBEDDEDBITMAPTEXT "embeddedBitmapText"
#define SKDEBUGCANVAS_ATTRIBUTE_AUTOHINTING       "forceAutoHinting"
#define SKDEBUGCANVAS_ATTRIBUTE_REGION            "region"
#define SKDEBUGCANVAS_ATTRIBUTE_REGIONOP          "op"
#define SKDEBUGCANVAS_ATTRIBUTE_EDGESTYLE         "edgeStyle"
#define SKDEBUGCANVAS_ATTRIBUTE_DEVICEREGION      "deviceRegion"
#define SKDEBUGCANVAS_ATTRIBUTE_BLUR              "blur"
#define SKDEBUGCANVAS_ATTRIBUTE_SIGMA             "sigma"
#define SKDEBUGCANVAS_ATTRIBUTE_QUALITY           "quality"
#define SKDEBUGCANVAS_ATTRIBUTE_TEXTSIZE          "textSize"
#define SKDEBUGCANVAS_ATTRIBUTE_TEXTSCALEX        "textScaleX"
#define SKDEBUGCANVAS_ATTRIBUTE_TEXTSKEWX         "textSkewX"
#define SKDEBUGCANVAS_ATTRIBUTE_DASHING           "dashing"
#define SKDEBUGCANVAS_ATTRIBUTE_INTERVALS         "intervals"
#define SKDEBUGCANVAS_ATTRIBUTE_PHASE             "phase"
#define SKDEBUGCANVAS_ATTRIBUTE_FILLTYPE          "fillType"
#define SKDEBUGCANVAS_ATTRIBUTE_VERBS             "verbs"
#define SKDEBUGCANVAS_ATTRIBUTE_NAME              "name"
#define SKDEBUGCANVAS_ATTRIBUTE_DATA              "data"
#define SKDEBUGCANVAS_ATTRIBUTE_VALUES            "values"
#define SKDEBUGCANVAS_ATTRIBUTE_SHADER            "shader"
#define SKDEBUGCANVAS_ATTRIBUTE_PATHEFFECT        "pathEffect"
#define SKDEBUGCANVAS_ATTRIBUTE_MASKFILTER        "maskFilter"
#define SKDEBUGCANVAS_ATTRIBUTE_XFERMODE          "xfermode"
#define SKDEBUGCANVAS_ATTRIBUTE_LOOPER            "looper"
#define SKDEBUGCANVAS_ATTRIBUTE_BACKDROP          "backdrop"
#define SKDEBUGCANVAS_ATTRIBUTE_COLORFILTER       "colorfilter"
#define SKDEBUGCANVAS_ATTRIBUTE_IMAGEFILTER       "imagefilter"
#define SKDEBUGCANVAS_ATTRIBUTE_IMAGE             "image"
#define SKDEBUGCANVAS_ATTRIBUTE_BITMAP            "bitmap"
#define SKDEBUGCANVAS_ATTRIBUTE_SRC               "src"
#define SKDEBUGCANVAS_ATTRIBUTE_DST               "dst"
#define SKDEBUGCANVAS_ATTRIBUTE_CENTER            "center"
#define SKDEBUGCANVAS_ATTRIBUTE_STRICT            "strict"
#define SKDEBUGCANVAS_ATTRIBUTE_DESCRIPTION       "description"
#define SKDEBUGCANVAS_ATTRIBUTE_X                 "x"
#define SKDEBUGCANVAS_ATTRIBUTE_Y                 "y"
#define SKDEBUGCANVAS_ATTRIBUTE_RUNS              "runs"
#define SKDEBUGCANVAS_ATTRIBUTE_POSITIONS         "positions"
#define SKDEBUGCANVAS_ATTRIBUTE_GLYPHS            "glyphs"
#define SKDEBUGCANVAS_ATTRIBUTE_FONT              "font"
#define SKDEBUGCANVAS_ATTRIBUTE_TYPEFACE          "typeface"
#define SKDEBUGCANVAS_ATTRIBUTE_CUBICS            "cubics"
#define SKDEBUGCANVAS_ATTRIBUTE_COLORS            "colors"
#define SKDEBUGCANVAS_ATTRIBUTE_TEXTURECOORDS     "textureCoords"
#define SKDEBUGCANVAS_ATTRIBUTE_FILTERQUALITY     "filterQuality"
#define SKDEBUGCANVAS_ATTRIBUTE_STARTANGLE        "startAngle"
#define SKDEBUGCANVAS_ATTRIBUTE_SWEEPANGLE        "sweepAngle"
#define SKDEBUGCANVAS_ATTRIBUTE_USECENTER         "useCenter"
#define SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC         "shortDesc"
#define SKDEBUGCANVAS_ATTRIBUTE_UNIQUE_ID         "uniqueID"
#define SKDEBUGCANVAS_ATTRIBUTE_WIDTH             "width"
#define SKDEBUGCANVAS_ATTRIBUTE_HEIGHT            "height"
#define SKDEBUGCANVAS_ATTRIBUTE_ALPHA             "alpha"
#define SKDEBUGCANVAS_ATTRIBUTE_LATTICE           "lattice"
#define SKDEBUGCANVAS_ATTRIBUTE_LATTICEXCOUNT     "xCount"
#define SKDEBUGCANVAS_ATTRIBUTE_LATTICEYCOUNT     "yCount"
#define SKDEBUGCANVAS_ATTRIBUTE_LATTICEXDIVS      "xDivs"
#define SKDEBUGCANVAS_ATTRIBUTE_LATTICEYDIVS      "yDivs"
#define SKDEBUGCANVAS_ATTRIBUTE_LATTICEFLAGS      "flags"
#define SKDEBUGCANVAS_ATTRIBUTE_ZPLANE            "zPlane"
#define SKDEBUGCANVAS_ATTRIBUTE_LIGHTPOSITION     "lightPositions"
#define SKDEBUGCANVAS_ATTRIBUTE_AMBIENTCOLOR      "ambientColor"
#define SKDEBUGCANVAS_ATTRIBUTE_SPOTCOLOR         "spotColor"
#define SKDEBUGCANVAS_ATTRIBUTE_LIGHTRADIUS       "lightRadius"

#define SKDEBUGCANVAS_VERB_MOVE                   "move"
#define SKDEBUGCANVAS_VERB_LINE                   "line"
#define SKDEBUGCANVAS_VERB_QUAD                   "quad"
#define SKDEBUGCANVAS_VERB_CUBIC                  "cubic"
#define SKDEBUGCANVAS_VERB_CONIC                  "conic"
#define SKDEBUGCANVAS_VERB_CLOSE                  "close"

#define SKDEBUGCANVAS_STYLE_FILL                  "fill"
#define SKDEBUGCANVAS_STYLE_STROKE                "stroke"
#define SKDEBUGCANVAS_STYLE_STROKEANDFILL         "strokeAndFill"

#define SKDEBUGCANVAS_POINTMODE_POINTS            "points"
#define SKDEBUGCANVAS_POINTMODE_LINES             "lines"
#define SKDEBUGCANVAS_POINTMODE_POLYGON           "polygon"

#define SKDEBUGCANVAS_REGIONOP_DIFFERENCE         "difference"
#define SKDEBUGCANVAS_REGIONOP_INTERSECT          "intersect"
#define SKDEBUGCANVAS_REGIONOP_UNION              "union"
#define SKDEBUGCANVAS_REGIONOP_XOR                "xor"
#define SKDEBUGCANVAS_REGIONOP_REVERSE_DIFFERENCE "reverseDifference"
#define SKDEBUGCANVAS_REGIONOP_REPLACE            "replace"

#define SKDEBUGCANVAS_BLURSTYLE_NORMAL            "normal"
#define SKDEBUGCANVAS_BLURSTYLE_SOLID             "solid"
#define SKDEBUGCANVAS_BLURSTYLE_OUTER             "outer"
#define SKDEBUGCANVAS_BLURSTYLE_INNER             "inner"

#define SKDEBUGCANVAS_BLURQUALITY_LOW             "low"
#define SKDEBUGCANVAS_BLURQUALITY_HIGH            "high"

#define SKDEBUGCANVAS_FILLTYPE_WINDING            "winding"
#define SKDEBUGCANVAS_FILLTYPE_EVENODD            "evenOdd"
#define SKDEBUGCANVAS_FILLTYPE_INVERSEWINDING     "inverseWinding"
#define SKDEBUGCANVAS_FILLTYPE_INVERSEEVENODD     "inverseEvenOdd"

#define SKDEBUGCANVAS_CAP_BUTT                    "butt"
#define SKDEBUGCANVAS_CAP_ROUND                   "round"
#define SKDEBUGCANVAS_CAP_SQUARE                  "square"

#define SKDEBUGCANVAS_MITER_JOIN                  "miter"
#define SKDEBUGCANVAS_ROUND_JOIN                  "round"
#define SKDEBUGCANVAS_BEVEL_JOIN                  "bevel"

#define SKDEBUGCANVAS_COLORTYPE_ARGB4444          "ARGB4444"
#define SKDEBUGCANVAS_COLORTYPE_RGBA8888          "RGBA8888"
#define SKDEBUGCANVAS_COLORTYPE_BGRA8888          "BGRA8888"
#define SKDEBUGCANVAS_COLORTYPE_565               "565"
#define SKDEBUGCANVAS_COLORTYPE_GRAY8             "Gray8"
#define SKDEBUGCANVAS_COLORTYPE_INDEX8            "Index8"
#define SKDEBUGCANVAS_COLORTYPE_ALPHA8            "Alpha8"

#define SKDEBUGCANVAS_ALPHATYPE_OPAQUE            "opaque"
#define SKDEBUGCANVAS_ALPHATYPE_PREMUL            "premul"
#define SKDEBUGCANVAS_ALPHATYPE_UNPREMUL          "unpremul"
#define SKDEBUGCANVAS_ALPHATYPE_UNKNOWN           "unknown"

#define SKDEBUGCANVAS_FILTERQUALITY_NONE          "none"
#define SKDEBUGCANVAS_FILTERQUALITY_LOW           "low"
#define SKDEBUGCANVAS_FILTERQUALITY_MEDIUM        "medium"
#define SKDEBUGCANVAS_FILTERQUALITY_HIGH          "high"

#define SKDEBUGCANVAS_HINTING_NONE                "none"
#define SKDEBUGCANVAS_HINTING_SLIGHT              "slight"
#define SKDEBUGCANVAS_HINTING_NORMAL              "normal"
#define SKDEBUGCANVAS_HINTING_FULL                "full"

#define SKDEBUGCANVAS_EDGING_ALIAS                "alias"
#define SKDEBUGCANVAS_EDGING_ANTIALIAS            "antialias"
#define SKDEBUGCANVAS_EDGING_SUBPIXELANTIALIAS    "subpixelantialias"

#define SKDEBUGCANVAS_SHADOWFLAG_TRANSPARENT_OCC  "transparentOccluder"
#define SKDEBUGCANVAS_SHADOWFLAG_GEOMETRIC_ONLY   "geometricOnly"

static SkString* str_append(SkString* str, const SkRect& r) {
    str->appendf(" [%g %g %g %g]", r.left(), r.top(), r.right(), r.bottom());
    return str;
}

SkDrawCommand::SkDrawCommand(OpType type)
    : fOpType(type)
    , fVisible(true) {
}

const char* SkDrawCommand::GetCommandString(OpType type) {
    switch (type) {
        case kBeginDrawPicture_OpType: return "BeginDrawPicture";
        case kClear_OpType: return "DrawClear";
        case kClipPath_OpType: return "ClipPath";
        case kClipRegion_OpType: return "ClipRegion";
        case kClipRect_OpType: return "ClipRect";
        case kClipRRect_OpType: return "ClipRRect";
        case kConcat_OpType: return "Concat";
        case kDrawAnnotation_OpType: return "DrawAnnotation";
        case kDrawBitmap_OpType: return "DrawBitmap";
        case kDrawBitmapLattice_OpType: return "DrawBitmapLattice";
        case kDrawBitmapNine_OpType: return "DrawBitmapNine";
        case kDrawBitmapRect_OpType: return "DrawBitmapRect";
        case kDrawDRRect_OpType: return "DrawDRRect";
        case kDrawImage_OpType: return "DrawImage";
        case kDrawImageLattice_OpType: return "DrawImageLattice";
        case kDrawImageNine_OpType: return "DrawImageNine";
        case kDrawImageRect_OpType: return "DrawImageRect";
        case kDrawOval_OpType: return "DrawOval";
        case kDrawPaint_OpType: return "DrawPaint";
        case kDrawPatch_OpType: return "DrawPatch";
        case kDrawPath_OpType: return "DrawPath";
        case kDrawArc_OpType: return "DrawArc";
        case kDrawPoints_OpType: return "DrawPoints";
        case kDrawRect_OpType: return "DrawRect";
        case kDrawRRect_OpType: return "DrawRRect";
        case kDrawRegion_OpType: return "DrawRegion";
        case kDrawShadow_OpType: return "DrawShadow";
        case kDrawTextBlob_OpType: return "DrawTextBlob";
        case kDrawVertices_OpType: return "DrawVertices";
        case kDrawAtlas_OpType: return "DrawAtlas";
        case kDrawDrawable_OpType: return "DrawDrawable";
        case kDrawEdgeAAQuad_OpType: return "DrawEdgeAAQuad";
        case kDrawEdgeAAImageSet_OpType: return "DrawEdgeAAImageSet";
        case kEndDrawPicture_OpType: return "EndDrawPicture";
        case kRestore_OpType: return "Restore";
        case kSave_OpType: return "Save";
        case kSaveLayer_OpType: return "SaveLayer";
        case kSetMatrix_OpType: return "SetMatrix";
        default:
            SkDebugf("OpType error 0x%08x\n", type);
            SkASSERT(0);
            break;
    }
    SkDEBUGFAIL("DrawType UNUSED\n");
    return nullptr;
}

void SkDrawCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_COMMAND, this->GetCommandString(fOpType));
    writer.appendBool(SKDEBUGCANVAS_ATTRIBUTE_VISIBLE, this->isVisible());
}

namespace {

void xlate_and_scale_to_bounds(SkCanvas* canvas, const SkRect& bounds) {
    const SkISize& size = canvas->getBaseLayerSize();

    static const SkScalar kInsetFrac = 0.9f; // Leave a border around object

    canvas->translate(size.fWidth/2.0f, size.fHeight/2.0f);
    if (bounds.width() > bounds.height()) {
        canvas->scale(SkDoubleToScalar((kInsetFrac*size.fWidth)/bounds.width()),
                      SkDoubleToScalar((kInsetFrac*size.fHeight)/bounds.width()));
    } else {
        canvas->scale(SkDoubleToScalar((kInsetFrac*size.fWidth)/bounds.height()),
                      SkDoubleToScalar((kInsetFrac*size.fHeight)/bounds.height()));
    }
    canvas->translate(-bounds.centerX(), -bounds.centerY());
}


void render_path(SkCanvas* canvas, const SkPath& path) {
    canvas->clear(0xFFFFFFFF);

    const SkRect& bounds = path.getBounds();
    if (bounds.isEmpty()) {
        return;
    }

    SkAutoCanvasRestore acr(canvas, true);
    xlate_and_scale_to_bounds(canvas, bounds);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawPath(path, p);
}

void render_region(SkCanvas* canvas, const SkRegion& region) {
    canvas->clear(0xFFFFFFFF);

    const SkIRect& bounds = region.getBounds();
    if (bounds.isEmpty()) {
        return;
    }

    SkAutoCanvasRestore acr(canvas, true);
    xlate_and_scale_to_bounds(canvas, SkRect::Make(bounds));

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawRegion(region, p);
}

void render_bitmap(SkCanvas* canvas, const SkBitmap& input, const SkRect* srcRect = nullptr) {
    const SkISize& size = canvas->getBaseLayerSize();

    SkScalar xScale = SkIntToScalar(size.fWidth-2) / input.width();
    SkScalar yScale = SkIntToScalar(size.fHeight-2) / input.height();

    if (input.width() > input.height()) {
        yScale *= input.height() / (float) input.width();
    } else {
        xScale *= input.width() / (float) input.height();
    }

    SkRect dst = SkRect::MakeXYWH(SK_Scalar1, SK_Scalar1,
                                  xScale * input.width(),
                                  yScale * input.height());

    static const int kNumBlocks = 8;

    canvas->clear(0xFFFFFFFF);
    SkISize block = {
        canvas->imageInfo().width()/kNumBlocks,
        canvas->imageInfo().height()/kNumBlocks
    };
    for (int y = 0; y < kNumBlocks; ++y) {
        for (int x = 0; x < kNumBlocks; ++x) {
            SkPaint paint;
            paint.setColor((x+y)%2 ? SK_ColorLTGRAY : SK_ColorDKGRAY);
            SkRect r = SkRect::MakeXYWH(SkIntToScalar(x*block.width()),
                                        SkIntToScalar(y*block.height()),
                                        SkIntToScalar(block.width()),
                                        SkIntToScalar(block.height()));
            canvas->drawRect(r, paint);
        }
    }

    canvas->drawBitmapRect(input, dst, nullptr);

    if (srcRect) {
        SkRect r = SkRect::MakeLTRB(srcRect->fLeft * xScale + SK_Scalar1,
                                    srcRect->fTop * yScale + SK_Scalar1,
                                    srcRect->fRight * xScale + SK_Scalar1,
                                    srcRect->fBottom * yScale + SK_Scalar1);
        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setStyle(SkPaint::kStroke_Style);

        canvas->drawRect(r, p);
    }
}

void render_rrect(SkCanvas* canvas, const SkRRect& rrect) {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    const SkRect& bounds = rrect.getBounds();

    xlate_and_scale_to_bounds(canvas, bounds);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawRRect(rrect, p);
    canvas->restore();
}

void render_drrect(SkCanvas* canvas, const SkRRect& outer, const SkRRect& inner) {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    const SkRect& bounds = outer.getBounds();

    xlate_and_scale_to_bounds(canvas, bounds);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawDRRect(outer, inner, p);
    canvas->restore();
}

void render_shadow(SkCanvas* canvas, const SkPath& path, SkDrawShadowRec rec) {
    canvas->clear(0xFFFFFFFF);

    const SkRect& bounds = path.getBounds();
    if (bounds.isEmpty()) {
        return;
    }

    SkAutoCanvasRestore acr(canvas, true);
    xlate_and_scale_to_bounds(canvas, bounds);

    rec.fAmbientColor = SK_ColorBLACK;
    rec.fSpotColor = SK_ColorBLACK;
    canvas->private_draw_shadow_rec(path, rec);
}

static const char* const gBlendModeMap[] = {
    "clear",
    "src",
    "dst",
    "srcOver",
    "dstOver",
    "srcIn",
    "dstIn",
    "srcOut",
    "dstOut",
    "srcATop",
    "dstATop",
    "xor",
    "plus",
    "modulate",

    "screen",

    "overlay",
    "darken",
    "lighten",
    "colorDodge",
    "colorBurn",
    "hardLight",
    "softLight",
    "difference",
    "exclusion",
    "multiply",

    "hue",
    "saturation",
    "color",
    "luminosity",
};

static_assert(SK_ARRAY_COUNT(gBlendModeMap) == static_cast<size_t>(SkBlendMode::kLastMode) + 1,
              "blendMode mismatch");
static_assert(SK_ARRAY_COUNT(gBlendModeMap) == static_cast<size_t>(SkBlendMode::kLuminosity) + 1,
              "blendMode mismatch");

void apply_paint_blend_mode(const SkPaint& paint, SkJSONWriter& writer) {
    const auto mode = paint.getBlendMode();
    if (mode != SkBlendMode::kSrcOver) {
        SkASSERT(static_cast<size_t>(mode) < SK_ARRAY_COUNT(gBlendModeMap));
        writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_BLENDMODE,
                            gBlendModeMap[static_cast<size_t>(mode)]);
    }
}

};

void SkDrawCommand::MakeJsonColor(SkJSONWriter& writer, const SkColor color) {
    writer.beginArray(nullptr, false);
    writer.appendS32(SkColorGetA(color));
    writer.appendS32(SkColorGetR(color));
    writer.appendS32(SkColorGetG(color));
    writer.appendS32(SkColorGetB(color));
    writer.endArray();
}

void SkDrawCommand::MakeJsonColor4f(SkJSONWriter& writer, const SkColor4f& color) {
    writer.beginArray(nullptr, false);
    writer.appendFloat(color.fA);
    writer.appendFloat(color.fR);
    writer.appendFloat(color.fG);
    writer.appendFloat(color.fB);
    writer.endArray();
}

void SkDrawCommand::MakeJsonPoint(SkJSONWriter& writer, const SkPoint& point) {
    writer.beginArray(nullptr, false);
    writer.appendFloat(point.x());
    writer.appendFloat(point.y());
    writer.endArray();
}

void SkDrawCommand::MakeJsonPoint(SkJSONWriter& writer, SkScalar x, SkScalar y) {
    writer.beginArray(nullptr, false);
    writer.appendFloat(x);
    writer.appendFloat(y);
    writer.endArray();
}

void SkDrawCommand::MakeJsonPoint3(SkJSONWriter& writer, const SkPoint3& point) {
    writer.beginArray(nullptr, false);
    writer.appendFloat(point.x());
    writer.appendFloat(point.y());
    writer.appendFloat(point.z());
    writer.endArray();
}

void SkDrawCommand::MakeJsonRect(SkJSONWriter& writer, const SkRect& rect) {
    writer.beginArray(nullptr, false);
    writer.appendFloat(rect.left());
    writer.appendFloat(rect.top());
    writer.appendFloat(rect.right());
    writer.appendFloat(rect.bottom());
    writer.endArray();
}

void SkDrawCommand::MakeJsonIRect(SkJSONWriter& writer, const SkIRect& rect) {
    writer.beginArray(nullptr, false);
    writer.appendS32(rect.left());
    writer.appendS32(rect.top());
    writer.appendS32(rect.right());
    writer.appendS32(rect.bottom());
    writer.endArray();
}

static void make_json_rrect(SkJSONWriter& writer, const SkRRect& rrect) {
    writer.beginArray(nullptr, false);
    SkDrawCommand::MakeJsonRect(writer, rrect.rect());
    SkDrawCommand::MakeJsonPoint(writer, rrect.radii(SkRRect::kUpperLeft_Corner));
    SkDrawCommand::MakeJsonPoint(writer, rrect.radii(SkRRect::kUpperRight_Corner));
    SkDrawCommand::MakeJsonPoint(writer, rrect.radii(SkRRect::kLowerRight_Corner));
    SkDrawCommand::MakeJsonPoint(writer, rrect.radii(SkRRect::kLowerLeft_Corner));
    writer.endArray();
}

void SkDrawCommand::MakeJsonMatrix(SkJSONWriter& writer, const SkMatrix& matrix) {
    writer.beginArray();
    for (int r = 0; r < 3; ++r) {
        writer.beginArray(nullptr, false);
        for (int c = 0; c < 3; ++c) {
            writer.appendFloat(matrix[r * 3 + c]);
        }
        writer.endArray();
    }
    writer.endArray();
}

void SkDrawCommand::MakeJsonPath(SkJSONWriter& writer, const SkPath& path) {
    writer.beginObject();
    switch (path.getFillType()) {
        case SkPath::kWinding_FillType:
            writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_FILLTYPE, SKDEBUGCANVAS_FILLTYPE_WINDING);
            break;
        case SkPath::kEvenOdd_FillType:
            writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_FILLTYPE, SKDEBUGCANVAS_FILLTYPE_EVENODD);
            break;
        case SkPath::kInverseWinding_FillType:
            writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_FILLTYPE, SKDEBUGCANVAS_FILLTYPE_INVERSEWINDING);
            break;
        case SkPath::kInverseEvenOdd_FillType:
            writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_FILLTYPE, SKDEBUGCANVAS_FILLTYPE_INVERSEEVENODD);
            break;
    }
    writer.beginArray(SKDEBUGCANVAS_ATTRIBUTE_VERBS);
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        if (verb == SkPath::kClose_Verb) {
            writer.appendString(SKDEBUGCANVAS_VERB_CLOSE);
            continue;
        }
        writer.beginObject(); // verb
        switch (verb) {
            case SkPath::kLine_Verb: {
                writer.appendName(SKDEBUGCANVAS_VERB_LINE);
                MakeJsonPoint(writer, pts[1]);
                break;
            }
            case SkPath::kQuad_Verb: {
                writer.beginArray(SKDEBUGCANVAS_VERB_QUAD);
                MakeJsonPoint(writer, pts[1]);
                MakeJsonPoint(writer, pts[2]);
                writer.endArray(); // quad coords
                break;
            }
            case SkPath::kCubic_Verb: {
                writer.beginArray(SKDEBUGCANVAS_VERB_CUBIC);
                MakeJsonPoint(writer, pts[1]);
                MakeJsonPoint(writer, pts[2]);
                MakeJsonPoint(writer, pts[3]);
                writer.endArray(); // cubic coords
                break;
            }
            case SkPath::kConic_Verb: {
                writer.beginArray(SKDEBUGCANVAS_VERB_CONIC);
                MakeJsonPoint(writer, pts[1]);
                MakeJsonPoint(writer, pts[2]);
                writer.appendFloat(iter.conicWeight());
                writer.endArray(); // conic coords
                break;
            }
            case SkPath::kMove_Verb: {
                writer.appendName(SKDEBUGCANVAS_VERB_MOVE);
                MakeJsonPoint(writer, pts[0]);
                break;
            }
            case SkPath::kClose_Verb:
            case SkPath::kDone_Verb:
                // Unreachable
                break;
        }
        writer.endObject(); // verb
    }
    writer.endArray(); // verbs
    writer.endObject(); // path
}

void SkDrawCommand::MakeJsonRegion(SkJSONWriter& writer, const SkRegion& region) {
    // TODO: Actually serialize the rectangles, rather than just devolving to path
    SkPath path;
    region.getBoundaryPath(&path);
    MakeJsonPath(writer, path);
}

static const char* regionop_name(SkClipOp op) {
    switch (op) {
        case kDifference_SkClipOp:
            return SKDEBUGCANVAS_REGIONOP_DIFFERENCE;
        case kIntersect_SkClipOp:
            return SKDEBUGCANVAS_REGIONOP_INTERSECT;
        case kUnion_SkClipOp:
            return SKDEBUGCANVAS_REGIONOP_UNION;
        case kXOR_SkClipOp:
            return SKDEBUGCANVAS_REGIONOP_XOR;
        case kReverseDifference_SkClipOp:
            return SKDEBUGCANVAS_REGIONOP_REVERSE_DIFFERENCE;
        case kReplace_SkClipOp:
            return SKDEBUGCANVAS_REGIONOP_REPLACE;
        default:
            SkASSERT(false);
            return "<invalid region op>";
    }
}

static const char* pointmode_name(SkCanvas::PointMode mode) {
    switch (mode) {
        case SkCanvas::kPoints_PointMode:
            return SKDEBUGCANVAS_POINTMODE_POINTS;
        case SkCanvas::kLines_PointMode:
            return SKDEBUGCANVAS_POINTMODE_LINES;
        case SkCanvas::kPolygon_PointMode:
            return SKDEBUGCANVAS_POINTMODE_POLYGON;
        default:
            SkASSERT(false);
            return "<invalid point mode>";
    }
}

static void store_scalar(SkJSONWriter& writer, const char* key, SkScalar value,
                         SkScalar defaultValue) {
    if (value != defaultValue) {
        writer.appendFloat(key, value);
    }
}

static void store_bool(SkJSONWriter& writer,const char* key, bool value, bool defaultValue) {
    if (value != defaultValue) {
        writer.appendBool(key, value);
    }
}

static SkString encode_data(const void* bytes, size_t count, const char* contentType,
                            UrlDataManager& urlDataManager) {
    sk_sp<SkData> data(SkData::MakeWithCopy(bytes, count));
    return urlDataManager.addData(data.get(), contentType);
}

void SkDrawCommand::flatten(const SkFlattenable* flattenable, SkJSONWriter& writer,
                            UrlDataManager& urlDataManager) {
    SkBinaryWriteBuffer buffer;
    flattenable->flatten(buffer);
    void* data = sk_malloc_throw(buffer.bytesWritten());
    buffer.writeToMemory(data);
    SkString url = encode_data(data, buffer.bytesWritten(), "application/octet-stream",
                               urlDataManager);
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_NAME, flattenable->getTypeName());
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_DATA, url.c_str());

    writer.beginObject(SKDEBUGCANVAS_ATTRIBUTE_VALUES);
    SkJsonWriteBuffer jsonBuffer(&writer, &urlDataManager);
    flattenable->flatten(jsonBuffer);
    writer.endObject(); // values

    sk_free(data);
}

void SkDrawCommand::WritePNG(SkBitmap bitmap, SkWStream& out) {
    SkPixmap pm;
    SkAssertResult(bitmap.peekPixels(&pm));

    SkPngEncoder::Options options;
    options.fZLibLevel = 1;
    options.fFilterFlags = SkPngEncoder::FilterFlag::kNone;
    SkPngEncoder::Encode(&out, pm, options);
}

bool SkDrawCommand::flatten(const SkImage& image, SkJSONWriter& writer,
                            UrlDataManager& urlDataManager) {
    size_t rowBytes = 4 * image.width();
    SkAutoMalloc buffer(rowBytes * image.height());
    SkImageInfo dstInfo = SkImageInfo::Make(image.width(), image.height(),
                                            kN32_SkColorType, kPremul_SkAlphaType);
    if (!image.readPixels(dstInfo, buffer.get(), rowBytes, 0, 0)) {
        SkDebugf("readPixels failed\n");
        return false;
    }

    SkBitmap bm;
    bm.installPixels(dstInfo, buffer.get(), rowBytes);

    SkDynamicMemoryWStream out;
    SkDrawCommand::WritePNG(bm, out);
    sk_sp<SkData> encoded = out.detachAsData();
    SkString url = encode_data(encoded->data(), encoded->size(), "image/png", urlDataManager);
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_DATA, url.c_str());
    return true;
}

static const char* color_type_name(SkColorType colorType) {
    switch (colorType) {
        case kARGB_4444_SkColorType:
            return SKDEBUGCANVAS_COLORTYPE_ARGB4444;
        case kRGBA_8888_SkColorType:
            return SKDEBUGCANVAS_COLORTYPE_RGBA8888;
        case kBGRA_8888_SkColorType:
            return SKDEBUGCANVAS_COLORTYPE_BGRA8888;
        case kRGB_565_SkColorType:
            return SKDEBUGCANVAS_COLORTYPE_565;
        case kGray_8_SkColorType:
            return SKDEBUGCANVAS_COLORTYPE_GRAY8;
        case kAlpha_8_SkColorType:
            return SKDEBUGCANVAS_COLORTYPE_ALPHA8;
        default:
            SkASSERT(false);
            return SKDEBUGCANVAS_COLORTYPE_RGBA8888;
    }
}

static const char* alpha_type_name(SkAlphaType alphaType) {
    switch (alphaType) {
        case kOpaque_SkAlphaType:
            return SKDEBUGCANVAS_ALPHATYPE_OPAQUE;
        case kPremul_SkAlphaType:
            return SKDEBUGCANVAS_ALPHATYPE_PREMUL;
        case kUnpremul_SkAlphaType:
            return SKDEBUGCANVAS_ALPHATYPE_UNPREMUL;
        default:
            SkASSERT(false);
            return SKDEBUGCANVAS_ALPHATYPE_OPAQUE;
    }
}

bool SkDrawCommand::flatten(const SkBitmap& bitmap, SkJSONWriter& writer,
                            UrlDataManager& urlDataManager) {
    sk_sp<SkImage> image(SkImage::MakeFromBitmap(bitmap));
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_COLOR, color_type_name(bitmap.colorType()));
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_ALPHA, alpha_type_name(bitmap.alphaType()));
    bool success = flatten(*image, writer, urlDataManager);
    return success;
}

static void apply_font_hinting(const SkFont& font, SkJSONWriter& writer) {
    SkFontHinting hinting = font.getHinting();
    if (hinting != SkPaintDefaults_Hinting) {
        switch (hinting) {
            case kNo_SkFontHinting:
                writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_HINTING, SKDEBUGCANVAS_HINTING_NONE);
                break;
            case kSlight_SkFontHinting:
                writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_HINTING, SKDEBUGCANVAS_HINTING_SLIGHT);
                break;
            case kNormal_SkFontHinting:
                writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_HINTING, SKDEBUGCANVAS_HINTING_NORMAL);
                break;
            case kFull_SkFontHinting:
                writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_HINTING, SKDEBUGCANVAS_HINTING_FULL);
                break;
        }
    }
}

static void apply_font_edging(const SkFont& font, SkJSONWriter& writer) {
    switch (font.getEdging()) {
        case SkFont::Edging::kAlias:
            writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_EDGING, SKDEBUGCANVAS_EDGING_ALIAS);
            break;
        case SkFont::Edging::kAntiAlias:
            writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_EDGING, SKDEBUGCANVAS_EDGING_ANTIALIAS);
            break;
        case SkFont::Edging::kSubpixelAntiAlias:
            writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_EDGING, SKDEBUGCANVAS_EDGING_SUBPIXELANTIALIAS);
            break;
    }
}

static void apply_paint_color(const SkPaint& paint, SkJSONWriter& writer) {
    SkColor color = paint.getColor();
    if (color != SK_ColorBLACK) {
        writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_COLOR);
        SkDrawCommand::MakeJsonColor(writer, color);
    }
}

static void apply_paint_style(const SkPaint& paint, SkJSONWriter& writer) {
    SkPaint::Style style = paint.getStyle();
    if (style != SkPaint::kFill_Style) {
        switch (style) {
            case SkPaint::kStroke_Style: {
                writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_STYLE, SKDEBUGCANVAS_STYLE_STROKE);
                break;
            }
            case SkPaint::kStrokeAndFill_Style: {
                writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_STYLE, SKDEBUGCANVAS_STYLE_STROKEANDFILL);
                break;
            }
            default: SkASSERT(false);
        }
    }
}

static void apply_paint_cap(const SkPaint& paint, SkJSONWriter& writer) {
    SkPaint::Cap cap = paint.getStrokeCap();
    if (cap != SkPaint::kDefault_Cap) {
        switch (cap) {
            case SkPaint::kButt_Cap:
                writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_CAP, SKDEBUGCANVAS_CAP_BUTT);
                break;
            case SkPaint::kRound_Cap:
                writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_CAP, SKDEBUGCANVAS_CAP_ROUND);
                break;
            case SkPaint::kSquare_Cap:
                writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_CAP, SKDEBUGCANVAS_CAP_SQUARE);
                break;
            default: SkASSERT(false);
        }
    }
}

static void apply_paint_join(const SkPaint& paint, SkJSONWriter& writer) {
    SkPaint::Join join = paint.getStrokeJoin();
    if (join != SkPaint::kDefault_Join) {
        switch (join) {
            case SkPaint::kMiter_Join:
                writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_STROKEJOIN, SKDEBUGCANVAS_MITER_JOIN);
                break;
            case SkPaint::kRound_Join:
                writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_STROKEJOIN, SKDEBUGCANVAS_ROUND_JOIN);
                break;
            case SkPaint::kBevel_Join:
                writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_STROKEJOIN, SKDEBUGCANVAS_BEVEL_JOIN);
                break;
            default: SkASSERT(false);
        }
    }
}

static void apply_paint_filterquality(const SkPaint& paint, SkJSONWriter& writer) {
    SkFilterQuality quality = paint.getFilterQuality();
    switch (quality) {
        case kNone_SkFilterQuality:
            break;
        case kLow_SkFilterQuality:
            writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_FILTERQUALITY,
                                SKDEBUGCANVAS_FILTERQUALITY_LOW);
            break;
        case kMedium_SkFilterQuality:
            writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_FILTERQUALITY,
                                SKDEBUGCANVAS_FILTERQUALITY_MEDIUM);
            break;
        case kHigh_SkFilterQuality:
            writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_FILTERQUALITY,
                                SKDEBUGCANVAS_FILTERQUALITY_HIGH);
            break;
    }
}

static void apply_paint_maskfilter(const SkPaint& paint, SkJSONWriter& writer,
                                   UrlDataManager& urlDataManager) {
    SkMaskFilter* maskFilter = paint.getMaskFilter();
    if (maskFilter != nullptr) {
        SkMaskFilterBase::BlurRec blurRec;
        if (as_MFB(maskFilter)->asABlur(&blurRec)) {
            writer.beginObject(SKDEBUGCANVAS_ATTRIBUTE_BLUR);
            writer.appendFloat(SKDEBUGCANVAS_ATTRIBUTE_SIGMA, blurRec.fSigma);
            switch (blurRec.fStyle) {
                case SkBlurStyle::kNormal_SkBlurStyle:
                    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_STYLE,
                                        SKDEBUGCANVAS_BLURSTYLE_NORMAL);
                    break;
                case SkBlurStyle::kSolid_SkBlurStyle:
                    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_STYLE,
                                        SKDEBUGCANVAS_BLURSTYLE_SOLID);
                    break;
                case SkBlurStyle::kOuter_SkBlurStyle:
                    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_STYLE,
                                        SKDEBUGCANVAS_BLURSTYLE_OUTER);
                    break;
                case SkBlurStyle::kInner_SkBlurStyle:
                    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_STYLE,
                                        SKDEBUGCANVAS_BLURSTYLE_INNER);
                    break;
                default:
                    SkASSERT(false);
            }
            writer.endObject(); // blur
        } else {
            writer.beginObject(SKDEBUGCANVAS_ATTRIBUTE_MASKFILTER);
            SkDrawCommand::flatten(maskFilter, writer, urlDataManager);
            writer.endObject(); // maskFilter
        }
    }
}

static void apply_paint_patheffect(const SkPaint& paint, SkJSONWriter& writer,
                                   UrlDataManager& urlDataManager) {
    SkPathEffect* pathEffect = paint.getPathEffect();
    if (pathEffect != nullptr) {
        SkPathEffect::DashInfo dashInfo;
        SkPathEffect::DashType dashType = pathEffect->asADash(&dashInfo);
        if (dashType == SkPathEffect::kDash_DashType) {
            dashInfo.fIntervals = (SkScalar*) sk_malloc_throw(dashInfo.fCount * sizeof(SkScalar));
            pathEffect->asADash(&dashInfo);
            writer.beginObject(SKDEBUGCANVAS_ATTRIBUTE_DASHING);
            writer.beginArray(SKDEBUGCANVAS_ATTRIBUTE_INTERVALS, false);
            for (int32_t i = 0; i < dashInfo.fCount; i++) {
                writer.appendFloat(dashInfo.fIntervals[i]);
            }
            writer.endArray(); // intervals
            sk_free(dashInfo.fIntervals);
            writer.appendFloat(SKDEBUGCANVAS_ATTRIBUTE_PHASE, dashInfo.fPhase);
            writer.endObject(); // dashing
        } else {
            writer.beginObject(SKDEBUGCANVAS_ATTRIBUTE_PATHEFFECT);
            SkDrawCommand::flatten(pathEffect, writer, urlDataManager);
            writer.endObject(); // pathEffect
        }
    }
}

static void apply_font_typeface(const SkFont& font, SkJSONWriter& writer,
                                 UrlDataManager& urlDataManager) {
    SkTypeface* typeface = font.getTypefaceOrDefault();
    if (typeface != nullptr) {
        writer.beginObject(SKDEBUGCANVAS_ATTRIBUTE_TYPEFACE);
        SkDynamicMemoryWStream buffer;
        typeface->serialize(&buffer);
        void* data = sk_malloc_throw(buffer.bytesWritten());
        buffer.copyTo(data);
        SkString url = encode_data(data, buffer.bytesWritten(), "application/octet-stream",
                                   urlDataManager);
        writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_DATA, url.c_str());
        sk_free(data);
        writer.endObject();
    }
}

static void apply_flattenable(const char* key, SkFlattenable* flattenable, SkJSONWriter& writer,
                              UrlDataManager& urlDataManager) {
    if (flattenable != nullptr) {
        writer.beginObject(key);
        SkDrawCommand::flatten(flattenable, writer, urlDataManager);
        writer.endObject();
    }
}

void SkDrawCommand::MakeJsonPaint(SkJSONWriter& writer, const SkPaint& paint,
                                  UrlDataManager& urlDataManager) {
    writer.beginObject();
    store_scalar(writer, SKDEBUGCANVAS_ATTRIBUTE_STROKEWIDTH, paint.getStrokeWidth(), 0.0f);
    store_scalar(writer, SKDEBUGCANVAS_ATTRIBUTE_STROKEMITER, paint.getStrokeMiter(),
                 SkPaintDefaults_MiterLimit);
    store_bool(writer, SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS, paint.isAntiAlias(), false);
    store_bool(writer, SKDEBUGCANVAS_ATTRIBUTE_DITHER, paint.isDither(), false);

    apply_paint_color(paint, writer);
    apply_paint_style(paint, writer);
    apply_paint_blend_mode(paint, writer);
    apply_paint_cap(paint, writer);
    apply_paint_join(paint, writer);
    apply_paint_filterquality(paint, writer);
    apply_paint_patheffect(paint, writer, urlDataManager);
    apply_paint_maskfilter(paint, writer, urlDataManager);
    apply_flattenable(SKDEBUGCANVAS_ATTRIBUTE_SHADER, paint.getShader(), writer, urlDataManager);
    apply_flattenable(SKDEBUGCANVAS_ATTRIBUTE_LOOPER, paint.getLooper(), writer, urlDataManager);
    apply_flattenable(SKDEBUGCANVAS_ATTRIBUTE_IMAGEFILTER, paint.getImageFilter(), writer,
                      urlDataManager);
    apply_flattenable(SKDEBUGCANVAS_ATTRIBUTE_COLORFILTER, paint.getColorFilter(), writer,
                      urlDataManager);
    writer.endObject(); // paint
}

static void MakeJsonFont(const SkFont& font, SkJSONWriter& writer, UrlDataManager& urlDataManager) {
    writer.beginObject();
    store_bool(writer, SKDEBUGCANVAS_ATTRIBUTE_FAKEBOLDTEXT, font.isEmbolden(), false);
    store_bool(writer, SKDEBUGCANVAS_ATTRIBUTE_LINEARTEXT, font.isLinearMetrics(), false);
    store_bool(writer, SKDEBUGCANVAS_ATTRIBUTE_SUBPIXELTEXT, font.isSubpixel(), false);
    store_bool(writer, SKDEBUGCANVAS_ATTRIBUTE_EMBEDDEDBITMAPTEXT, font.isEmbeddedBitmaps(), false);
    store_bool(writer, SKDEBUGCANVAS_ATTRIBUTE_AUTOHINTING, font.isForceAutoHinting(), false);

    store_scalar(writer, SKDEBUGCANVAS_ATTRIBUTE_TEXTSIZE, font.getSize(),
                 SkPaintDefaults_TextSize);
    store_scalar(writer, SKDEBUGCANVAS_ATTRIBUTE_TEXTSCALEX, font.getScaleX(), SK_Scalar1);
    store_scalar(writer, SKDEBUGCANVAS_ATTRIBUTE_TEXTSCALEX, font.getSkewX(), 0.0f);
    apply_font_edging(font, writer);
    apply_font_hinting(font, writer);
    apply_font_typeface(font, writer, urlDataManager);
    writer.endObject(); // font
}

void SkDrawCommand::MakeJsonLattice(SkJSONWriter& writer, const SkCanvas::Lattice& lattice) {
    writer.beginObject();
    writer.appendS32(SKDEBUGCANVAS_ATTRIBUTE_LATTICEXCOUNT, lattice.fXCount);
    writer.appendS32(SKDEBUGCANVAS_ATTRIBUTE_LATTICEYCOUNT, lattice.fYCount);
    if (nullptr != lattice.fBounds) {
        writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_BOUNDS);
        MakeJsonIRect(writer, *lattice.fBounds);
    }
    writer.beginArray(SKDEBUGCANVAS_ATTRIBUTE_LATTICEXDIVS);
    for (int i = 0; i < lattice.fXCount; i++) {
        writer.appendS32(lattice.fXDivs[i]);
    }
    writer.endArray(); // xdivs
    writer.beginArray(SKDEBUGCANVAS_ATTRIBUTE_LATTICEYDIVS);
    for (int i = 0; i < lattice.fYCount; i++) {
        writer.appendS32(lattice.fYDivs[i]);
    }
    writer.endArray(); // ydivs
    if (nullptr != lattice.fRectTypes) {
        writer.beginArray(SKDEBUGCANVAS_ATTRIBUTE_LATTICEFLAGS);
        int flagCount = 0;
        for (int row = 0; row < lattice.fYCount+1; row++) {
            writer.beginArray();
            for (int column = 0; column < lattice.fXCount+1; column++) {
                writer.appendS32(lattice.fRectTypes[flagCount++]);
            }
            writer.endArray(); // row
        }
        writer.endArray();
    }
    writer.endObject();
}

SkClearCommand::SkClearCommand(SkColor color) : INHERITED(kClear_OpType) {
    fColor = color;
}

void SkClearCommand::execute(SkCanvas* canvas) const {
    canvas->clear(fColor);
}

void SkClearCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_COLOR);
    MakeJsonColor(writer, fColor);
}

SkClipPathCommand::SkClipPathCommand(const SkPath& path, SkClipOp op, bool doAA)
    : INHERITED(kClipPath_OpType) {
    fPath = path;
    fOp = op;
    fDoAA = doAA;
}

void SkClipPathCommand::execute(SkCanvas* canvas) const {
    canvas->clipPath(fPath, fOp, fDoAA);
}

bool SkClipPathCommand::render(SkCanvas* canvas) const {
    render_path(canvas, fPath);
    return true;
}

void SkClipPathCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PATH); MakeJsonPath(writer, fPath);
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_REGIONOP, regionop_name(fOp));
    writer.appendBool(SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS, fDoAA);
}

SkClipRegionCommand::SkClipRegionCommand(const SkRegion& region, SkClipOp op)
    : INHERITED(kClipRegion_OpType) {
    fRegion = region;
    fOp = op;
}

void SkClipRegionCommand::execute(SkCanvas* canvas) const {
    canvas->clipRegion(fRegion, fOp);
}

void SkClipRegionCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_REGION); MakeJsonRegion(writer, fRegion);
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_REGIONOP, regionop_name(fOp));
}

SkClipRectCommand::SkClipRectCommand(const SkRect& rect, SkClipOp op, bool doAA)
    : INHERITED(kClipRect_OpType) {
    fRect = rect;
    fOp = op;
    fDoAA = doAA;
}

void SkClipRectCommand::execute(SkCanvas* canvas) const {
    canvas->clipRect(fRect, fOp, fDoAA);
}

void SkClipRectCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_COORDS); MakeJsonRect(writer, fRect);
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_REGIONOP, regionop_name(fOp));
    writer.appendBool(SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS, fDoAA);

    SkString desc;
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC, str_append(&desc, fRect)->c_str());
}

SkClipRRectCommand::SkClipRRectCommand(const SkRRect& rrect, SkClipOp op, bool doAA)
    : INHERITED(kClipRRect_OpType) {
    fRRect = rrect;
    fOp = op;
    fDoAA = doAA;
}

void SkClipRRectCommand::execute(SkCanvas* canvas) const {
    canvas->clipRRect(fRRect, fOp, fDoAA);
}

bool SkClipRRectCommand::render(SkCanvas* canvas) const {
    render_rrect(canvas, fRRect);
    return true;
}

void SkClipRRectCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_COORDS); make_json_rrect(writer, fRRect);
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_REGIONOP, regionop_name(fOp));
    writer.appendBool(SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS, fDoAA);
}

SkConcatCommand::SkConcatCommand(const SkMatrix& matrix)
    : INHERITED(kConcat_OpType) {
    fMatrix = matrix;
}

void SkConcatCommand::execute(SkCanvas* canvas) const {
    canvas->concat(fMatrix);
}

void SkConcatCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_MATRIX); MakeJsonMatrix(writer, fMatrix);
}

////

SkDrawAnnotationCommand::SkDrawAnnotationCommand(const SkRect& rect, const char key[],
                                                 sk_sp<SkData> value)
    : INHERITED(kDrawAnnotation_OpType)
    , fRect(rect)
    , fKey(key)
    , fValue(std::move(value))
{
}

void SkDrawAnnotationCommand::execute(SkCanvas* canvas) const {
    canvas->drawAnnotation(fRect, fKey.c_str(), fValue);
}

void SkDrawAnnotationCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);

    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_COORDS); MakeJsonRect(writer, fRect);
    writer.appendString("key", fKey.c_str());
    if (fValue.get()) {
        // TODO: dump out the "value"
    }

    SkString desc;
    str_append(&desc, fRect)->appendf(" %s", fKey.c_str());
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC, desc.c_str());
}

////

SkDrawBitmapCommand::SkDrawBitmapCommand(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                                         const SkPaint* paint)
    : INHERITED(kDrawBitmap_OpType)
    , fBitmap(bitmap)
    , fLeft(left)
    , fTop(top)
    , fPaint(paint) {}

void SkDrawBitmapCommand::execute(SkCanvas* canvas) const {
    canvas->drawBitmap(fBitmap, fLeft, fTop, fPaint.getMaybeNull());
}

bool SkDrawBitmapCommand::render(SkCanvas* canvas) const {
    render_bitmap(canvas, fBitmap);
    return true;
}

void SkDrawBitmapCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginObject(SKDEBUGCANVAS_ATTRIBUTE_BITMAP);
    flatten(fBitmap, writer, urlDataManager);
    writer.endObject();
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_COORDS); MakeJsonPoint(writer, fLeft, fTop);
    if (fPaint.isValid()) {
        writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }
}

SkDrawBitmapLatticeCommand::SkDrawBitmapLatticeCommand(const SkBitmap& bitmap,
                                                       const SkCanvas::Lattice& lattice,
                                                       const SkRect& dst, const SkPaint* paint)
    : INHERITED(kDrawBitmapLattice_OpType)
    , fBitmap(bitmap)
    , fLattice(lattice)
    , fDst(dst)
    , fPaint(paint) {}

void SkDrawBitmapLatticeCommand::execute(SkCanvas* canvas) const {
    canvas->drawBitmapLattice(fBitmap, fLattice, fDst, fPaint.getMaybeNull());
}

bool SkDrawBitmapLatticeCommand::render(SkCanvas* canvas) const {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clear(0xFFFFFFFF);

    xlate_and_scale_to_bounds(canvas, fDst);

    this->execute(canvas);
    return true;
}

void SkDrawBitmapLatticeCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginObject(SKDEBUGCANVAS_ATTRIBUTE_BITMAP);
    flatten(fBitmap, writer, urlDataManager);
    writer.endObject(); // bitmap

    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_LATTICE); MakeJsonLattice(writer, fLattice);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_DST); MakeJsonRect(writer, fDst);
    if (fPaint.isValid()) {
        writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }

    SkString desc;
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC, str_append(&desc, fDst)->c_str());
}

SkDrawBitmapNineCommand::SkDrawBitmapNineCommand(const SkBitmap& bitmap, const SkIRect& center,
                                                 const SkRect& dst, const SkPaint* paint)
    : INHERITED(kDrawBitmapNine_OpType)
    , fBitmap(bitmap)
    , fCenter(center)
    , fDst(dst)
    , fPaint(paint) {}

void SkDrawBitmapNineCommand::execute(SkCanvas* canvas) const {
    canvas->drawBitmapNine(fBitmap, fCenter, fDst, fPaint.getMaybeNull());
}

bool SkDrawBitmapNineCommand::render(SkCanvas* canvas) const {
    SkRect tmp = SkRect::Make(fCenter);
    render_bitmap(canvas, fBitmap, &tmp);
    return true;
}

void SkDrawBitmapNineCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginObject(SKDEBUGCANVAS_ATTRIBUTE_BITMAP);
    flatten(fBitmap, writer, urlDataManager);
    writer.endObject(); // bitmap

    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_CENTER); MakeJsonIRect(writer, fCenter);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_DST); MakeJsonRect(writer, fDst);
    if (fPaint.isValid()) {
        writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }
}

SkDrawBitmapRectCommand::SkDrawBitmapRectCommand(const SkBitmap& bitmap, const SkRect* src,
                                                 const SkRect& dst, const SkPaint* paint,
                                                 SkCanvas::SrcRectConstraint constraint)
    : INHERITED(kDrawBitmapRect_OpType)
    , fBitmap(bitmap)
    , fSrc(src)
    , fDst(dst)
    , fPaint(paint)
    , fConstraint(constraint) {}

void SkDrawBitmapRectCommand::execute(SkCanvas* canvas) const {
    canvas->legacy_drawBitmapRect(fBitmap, fSrc.getMaybeNull(), fDst, fPaint.getMaybeNull(),
                                  fConstraint);
}

bool SkDrawBitmapRectCommand::render(SkCanvas* canvas) const {
    render_bitmap(canvas, fBitmap, fSrc.getMaybeNull());
    return true;
}

void SkDrawBitmapRectCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginObject(SKDEBUGCANVAS_ATTRIBUTE_BITMAP);
    flatten(fBitmap, writer, urlDataManager);
    writer.endObject(); // bitmap

    if (fSrc.isValid()) {
        writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_SRC); MakeJsonRect(writer, *fSrc);
    }
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_DST); MakeJsonRect(writer, fDst);
    if (fPaint.isValid()) {
        writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }
    if (fConstraint == SkCanvas::kStrict_SrcRectConstraint) {
        writer.appendBool(SKDEBUGCANVAS_ATTRIBUTE_STRICT, true);
    }

    SkString desc;
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC, str_append(&desc, fDst)->c_str());
}

SkDrawImageCommand::SkDrawImageCommand(const SkImage* image, SkScalar left, SkScalar top,
                                       const SkPaint* paint)
    : INHERITED(kDrawImage_OpType)
    , fImage(SkRef(image))
    , fLeft(left)
    , fTop(top)
    , fPaint(paint) {}

void SkDrawImageCommand::execute(SkCanvas* canvas) const {
    canvas->drawImage(fImage.get(), fLeft, fTop, fPaint.getMaybeNull());
}

bool SkDrawImageCommand::render(SkCanvas* canvas) const {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clear(0xFFFFFFFF);

    xlate_and_scale_to_bounds(canvas, SkRect::MakeXYWH(fLeft, fTop,
                                                       SkIntToScalar(fImage->width()),
                                                       SkIntToScalar(fImage->height())));
    this->execute(canvas);
    return true;
}

void SkDrawImageCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginObject(SKDEBUGCANVAS_ATTRIBUTE_IMAGE);
    flatten(*fImage, writer, urlDataManager);
    writer.endObject(); // image

    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_COORDS); MakeJsonPoint(writer, fLeft, fTop);
    if (fPaint.isValid()) {
        writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }

    writer.appendU32(SKDEBUGCANVAS_ATTRIBUTE_UNIQUE_ID, fImage->uniqueID());
    writer.appendS32(SKDEBUGCANVAS_ATTRIBUTE_WIDTH, fImage->width());
    writer.appendS32(SKDEBUGCANVAS_ATTRIBUTE_HEIGHT, fImage->height());
    switch (fImage->alphaType()) {
        case kOpaque_SkAlphaType:
            writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_ALPHA, SKDEBUGCANVAS_ALPHATYPE_OPAQUE);
            break;
        case kPremul_SkAlphaType:
            writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_ALPHA, SKDEBUGCANVAS_ALPHATYPE_PREMUL);
            break;
        case kUnpremul_SkAlphaType:
            writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_ALPHA, SKDEBUGCANVAS_ALPHATYPE_UNPREMUL);
            break;
        default:
            writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_ALPHA, SKDEBUGCANVAS_ALPHATYPE_UNKNOWN);
            break;
    }
}

SkDrawImageLatticeCommand::SkDrawImageLatticeCommand(const SkImage* image,
                                                     const SkCanvas::Lattice& lattice,
                                                     const SkRect& dst, const SkPaint* paint)
    : INHERITED(kDrawImageLattice_OpType)
    , fImage(SkRef(image))
    , fLattice(lattice)
    , fDst(dst)
    , fPaint(paint) {}

void SkDrawImageLatticeCommand::execute(SkCanvas* canvas) const {
    canvas->drawImageLattice(fImage.get(), fLattice, fDst, fPaint.getMaybeNull());
}

bool SkDrawImageLatticeCommand::render(SkCanvas* canvas) const {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clear(0xFFFFFFFF);

    xlate_and_scale_to_bounds(canvas, fDst);

    this->execute(canvas);
    return true;
}

void SkDrawImageLatticeCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginObject(SKDEBUGCANVAS_ATTRIBUTE_IMAGE);
    flatten(*fImage, writer, urlDataManager);
    writer.endObject(); // image

    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_LATTICE); MakeJsonLattice(writer, fLattice);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_DST); MakeJsonRect(writer, fDst);
    if (fPaint.isValid()) {
        writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }

    SkString desc;
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC, str_append(&desc, fDst)->c_str());
}

SkDrawImageRectCommand::SkDrawImageRectCommand(const SkImage* image, const SkRect* src,
                                               const SkRect& dst, const SkPaint* paint,
                                               SkCanvas::SrcRectConstraint constraint)
    : INHERITED(kDrawImageRect_OpType)
    , fImage(SkRef(image))
    , fSrc(src)
    , fDst(dst)
    , fPaint(paint)
    , fConstraint(constraint) {}

void SkDrawImageRectCommand::execute(SkCanvas* canvas) const {
    canvas->legacy_drawImageRect(fImage.get(), fSrc.getMaybeNull(), fDst,
                                 fPaint.getMaybeNull(), fConstraint);
}

bool SkDrawImageRectCommand::render(SkCanvas* canvas) const {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clear(0xFFFFFFFF);

    xlate_and_scale_to_bounds(canvas, fDst);

    this->execute(canvas);
    return true;
}

void SkDrawImageRectCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginObject(SKDEBUGCANVAS_ATTRIBUTE_IMAGE);
    flatten(*fImage, writer, urlDataManager);
    writer.endObject(); // image

    if (fSrc.isValid()) {
        writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_SRC); MakeJsonRect(writer, *fSrc);
    }
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_DST); MakeJsonRect(writer, fDst);
    if (fPaint.isValid()) {
        writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }
    if (fConstraint == SkCanvas::kStrict_SrcRectConstraint) {
        writer.appendBool(SKDEBUGCANVAS_ATTRIBUTE_STRICT, true);
    }

    SkString desc;
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC, str_append(&desc, fDst)->c_str());
}

SkDrawImageNineCommand::SkDrawImageNineCommand(const SkImage* image, const SkIRect& center,
                                               const SkRect& dst, const SkPaint* paint)
    : INHERITED(kDrawImageNine_OpType)
    , fImage(SkRef(image))
    , fCenter(center)
    , fDst(dst)
    , fPaint(paint) {}

void SkDrawImageNineCommand::execute(SkCanvas* canvas) const {
    canvas->drawImageNine(fImage.get(), fCenter, fDst, fPaint.getMaybeNull());
}

bool SkDrawImageNineCommand::render(SkCanvas* canvas) const {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clear(0xFFFFFFFF);

    xlate_and_scale_to_bounds(canvas, fDst);

    this->execute(canvas);
    return true;
}

void SkDrawImageNineCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginObject(SKDEBUGCANVAS_ATTRIBUTE_IMAGE);
    flatten(*fImage, writer, urlDataManager);
    writer.endObject(); // image

    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_CENTER); MakeJsonIRect(writer, fCenter);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_DST); MakeJsonRect(writer, fDst);
    if (fPaint.isValid()) {
        writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }
}

SkDrawOvalCommand::SkDrawOvalCommand(const SkRect& oval, const SkPaint& paint)
    : INHERITED(kDrawOval_OpType) {
    fOval = oval;
    fPaint = paint;
}

void SkDrawOvalCommand::execute(SkCanvas* canvas) const {
    canvas->drawOval(fOval, fPaint);
}

bool SkDrawOvalCommand::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    xlate_and_scale_to_bounds(canvas, fOval);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawOval(fOval, p);
    canvas->restore();

    return true;
}

void SkDrawOvalCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_COORDS); MakeJsonRect(writer, fOval);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT); MakeJsonPaint(writer, fPaint, urlDataManager);
}

SkDrawArcCommand::SkDrawArcCommand(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                                   bool useCenter, const SkPaint& paint)
        : INHERITED(kDrawArc_OpType) {
    fOval = oval;
    fStartAngle = startAngle;
    fSweepAngle = sweepAngle;
    fUseCenter = useCenter;
    fPaint = paint;
}

void SkDrawArcCommand::execute(SkCanvas* canvas) const {
    canvas->drawArc(fOval, fStartAngle, fSweepAngle, fUseCenter, fPaint);
}

bool SkDrawArcCommand::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    xlate_and_scale_to_bounds(canvas, fOval);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawArc(fOval, fStartAngle, fSweepAngle, fUseCenter, p);
    canvas->restore();

    return true;
}

void SkDrawArcCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_COORDS); MakeJsonRect(writer, fOval);
    writer.appendFloat(SKDEBUGCANVAS_ATTRIBUTE_STARTANGLE, fStartAngle);
    writer.appendFloat(SKDEBUGCANVAS_ATTRIBUTE_SWEEPANGLE, fSweepAngle);
    writer.appendBool(SKDEBUGCANVAS_ATTRIBUTE_USECENTER, fUseCenter);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT); MakeJsonPaint(writer, fPaint, urlDataManager);
}

SkDrawPaintCommand::SkDrawPaintCommand(const SkPaint& paint)
    : INHERITED(kDrawPaint_OpType) {
    fPaint = paint;
}

void SkDrawPaintCommand::execute(SkCanvas* canvas) const {
    canvas->drawPaint(fPaint);
}

bool SkDrawPaintCommand::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->drawPaint(fPaint);
    return true;
}

void SkDrawPaintCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT); MakeJsonPaint(writer, fPaint, urlDataManager);
}

SkDrawPathCommand::SkDrawPathCommand(const SkPath& path, const SkPaint& paint)
    : INHERITED(kDrawPath_OpType) {
    fPath = path;
    fPaint = paint;
}

void SkDrawPathCommand::execute(SkCanvas* canvas) const {
    canvas->drawPath(fPath, fPaint);
}

bool SkDrawPathCommand::render(SkCanvas* canvas) const {
    render_path(canvas, fPath);
    return true;
}

void SkDrawPathCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PATH); MakeJsonPath(writer, fPath);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT); MakeJsonPaint(writer, fPaint, urlDataManager);
}

SkDrawRegionCommand::SkDrawRegionCommand(const SkRegion& region, const SkPaint& paint)
    : INHERITED(kDrawRegion_OpType) {
    fRegion = region;
    fPaint = paint;
}

void SkDrawRegionCommand::execute(SkCanvas* canvas) const {
    canvas->drawRegion(fRegion, fPaint);
}

bool SkDrawRegionCommand::render(SkCanvas* canvas) const {
    render_region(canvas, fRegion);
    return true;
}

void SkDrawRegionCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_REGION); MakeJsonRegion(writer, fRegion);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT); MakeJsonPaint(writer, fPaint, urlDataManager);
}

SkBeginDrawPictureCommand::SkBeginDrawPictureCommand(const SkPicture* picture,
                                                     const SkMatrix* matrix,
                                                     const SkPaint* paint)
    : INHERITED(kBeginDrawPicture_OpType)
    , fPicture(SkRef(picture))
    , fMatrix(matrix)
    , fPaint(paint) {}

void SkBeginDrawPictureCommand::execute(SkCanvas* canvas) const {
    if (fPaint.isValid()) {
        SkRect bounds = fPicture->cullRect();
        if (fMatrix.isValid()) {
            fMatrix->mapRect(&bounds);
        }
        canvas->saveLayer(&bounds, fPaint.get());
    }

    if (fMatrix.isValid()) {
        if (!fPaint.isValid()) {
            canvas->save();
        }
        canvas->concat(*fMatrix);
    }
}

bool SkBeginDrawPictureCommand::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    xlate_and_scale_to_bounds(canvas, fPicture->cullRect());

    canvas->drawPicture(fPicture.get());

    canvas->restore();

    return true;
}

SkEndDrawPictureCommand::SkEndDrawPictureCommand(bool restore)
    : INHERITED(kEndDrawPicture_OpType) , fRestore(restore) { }

void SkEndDrawPictureCommand::execute(SkCanvas* canvas) const {
    if (fRestore) {
        canvas->restore();
    }
}

SkDrawPointsCommand::SkDrawPointsCommand(SkCanvas::PointMode mode, size_t count,
                                         const SkPoint pts[], const SkPaint& paint)
    : INHERITED(kDrawPoints_OpType)
    , fMode(mode)
    , fPts(pts, count)
    , fPaint(paint) {}

void SkDrawPointsCommand::execute(SkCanvas* canvas) const {
    canvas->drawPoints(fMode, fPts.count(), fPts.begin(), fPaint);
}

bool SkDrawPointsCommand::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    SkRect bounds;

    bounds.setEmpty();
    for (int i = 0; i < fPts.count(); ++i) {
        SkRectPriv::GrowToInclude(&bounds, fPts[i]);
    }

    xlate_and_scale_to_bounds(canvas, bounds);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawPoints(fMode, fPts.count(), fPts.begin(), p);
    canvas->restore();

    return true;
}

void SkDrawPointsCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_MODE, pointmode_name(fMode));
    writer.beginArray(SKDEBUGCANVAS_ATTRIBUTE_POINTS);
    for (int i = 0; i < fPts.count(); i++) {
        MakeJsonPoint(writer, fPts[i]);
    }
    writer.endArray(); // points
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT); MakeJsonPaint(writer, fPaint, urlDataManager);
}

SkDrawTextBlobCommand::SkDrawTextBlobCommand(sk_sp<SkTextBlob> blob, SkScalar x, SkScalar y,
                                             const SkPaint& paint)
    : INHERITED(kDrawTextBlob_OpType)
    , fBlob(std::move(blob))
    , fXPos(x)
    , fYPos(y)
    , fPaint(paint) {}

void SkDrawTextBlobCommand::execute(SkCanvas* canvas) const {
    canvas->drawTextBlob(fBlob, fXPos, fYPos, fPaint);
}

bool SkDrawTextBlobCommand::render(SkCanvas* canvas) const {
    canvas->clear(SK_ColorWHITE);
    canvas->save();

    SkRect bounds = fBlob->bounds().makeOffset(fXPos, fYPos);
    xlate_and_scale_to_bounds(canvas, bounds);

    canvas->drawTextBlob(fBlob, fXPos, fYPos, fPaint);

    canvas->restore();

    return true;
}

void SkDrawTextBlobCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginArray(SKDEBUGCANVAS_ATTRIBUTE_RUNS);
    SkTextBlobRunIterator iter(fBlob.get());
    while (!iter.done()) {
        writer.beginObject(); // run
        writer.beginArray(SKDEBUGCANVAS_ATTRIBUTE_GLYPHS);
        for (uint32_t i = 0; i < iter.glyphCount(); i++) {
            writer.appendU32(iter.glyphs()[i]);
        }
        writer.endArray(); // glyphs
        if (iter.positioning() != SkTextBlobRunIterator::kDefault_Positioning) {
            writer.beginArray(SKDEBUGCANVAS_ATTRIBUTE_POSITIONS);
            const SkScalar* iterPositions = iter.pos();
            for (uint32_t i = 0; i < iter.glyphCount(); i++) {
                switch (iter.positioning()) {
                case SkTextBlobRunIterator::kFull_Positioning:
                    MakeJsonPoint(writer, iterPositions[i * 2], iterPositions[i * 2 + 1]);
                    break;
                case SkTextBlobRunIterator::kHorizontal_Positioning:
                    writer.appendFloat(iterPositions[i]);
                    break;
                case SkTextBlobRunIterator::kDefault_Positioning:
                    break;
                case SkTextBlobRunIterator::kRSXform_Positioning:
                    // TODO_RSXFORM_BLOB
                    break;
                }
            }
            writer.endArray(); // positions
        }
        writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_FONT);
            MakeJsonFont(iter.font(), writer, urlDataManager);
        writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_COORDS);
            MakeJsonPoint(writer, iter.offset());

        writer.endObject(); // run
        iter.next();
    }
    writer.endArray(); // runs
    writer.appendFloat(SKDEBUGCANVAS_ATTRIBUTE_X, fXPos);
    writer.appendFloat(SKDEBUGCANVAS_ATTRIBUTE_Y, fYPos);
    SkRect bounds = fBlob->bounds();
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_COORDS); MakeJsonRect(writer, bounds);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT); MakeJsonPaint(writer, fPaint, urlDataManager);

    SkString desc;
    // make the bounds local by applying the x,y
    bounds.offset(fXPos, fYPos);
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC, str_append(&desc, bounds)->c_str());
}

SkDrawPatchCommand::SkDrawPatchCommand(const SkPoint cubics[12], const SkColor colors[4],
                                       const SkPoint texCoords[4], SkBlendMode bmode,
                                       const SkPaint& paint)
    : INHERITED(kDrawPatch_OpType)
    , fBlendMode(bmode)
{
    memcpy(fCubics, cubics, sizeof(fCubics));
    if (colors != nullptr) {
        memcpy(fColors, colors, sizeof(fColors));
        fColorsPtr = fColors;
    } else {
        fColorsPtr = nullptr;
    }
    if (texCoords != nullptr) {
        memcpy(fTexCoords, texCoords, sizeof(fTexCoords));
        fTexCoordsPtr = fTexCoords;
    } else {
        fTexCoordsPtr = nullptr;
    }
    fPaint = paint;
}

void SkDrawPatchCommand::execute(SkCanvas* canvas) const {
    canvas->drawPatch(fCubics, fColorsPtr, fTexCoordsPtr, fBlendMode, fPaint);
}

void SkDrawPatchCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginArray(SKDEBUGCANVAS_ATTRIBUTE_CUBICS);
    for (int i = 0; i < 12; i++) {
        MakeJsonPoint(writer, fCubics[i]);
    }
    writer.endArray(); // cubics
    if (fColorsPtr != nullptr) {
        writer.beginArray(SKDEBUGCANVAS_ATTRIBUTE_COLORS);
        for (int i = 0; i < 4; i++) {
            MakeJsonColor(writer, fColorsPtr[i]);
        }
        writer.endArray(); // colors
    }
    if (fTexCoordsPtr != nullptr) {
        writer.beginArray(SKDEBUGCANVAS_ATTRIBUTE_TEXTURECOORDS);
        for (int i = 0; i < 4; i++) {
            MakeJsonPoint(writer, fTexCoords[i]);
        }
        writer.endArray(); // texCoords
    }
    // fBlendMode
}

SkDrawRectCommand::SkDrawRectCommand(const SkRect& rect, const SkPaint& paint)
    : INHERITED(kDrawRect_OpType) {
    fRect = rect;
    fPaint = paint;
}

void SkDrawRectCommand::execute(SkCanvas* canvas) const {
    canvas->drawRect(fRect, fPaint);
}

void SkDrawRectCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_COORDS); MakeJsonRect(writer, fRect);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT); MakeJsonPaint(writer, fPaint, urlDataManager);

    SkString desc;
    writer.appendString(SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC, str_append(&desc, fRect)->c_str());
}

SkDrawRRectCommand::SkDrawRRectCommand(const SkRRect& rrect, const SkPaint& paint)
    : INHERITED(kDrawRRect_OpType) {
    fRRect = rrect;
    fPaint = paint;
}

void SkDrawRRectCommand::execute(SkCanvas* canvas) const {
    canvas->drawRRect(fRRect, fPaint);
}

bool SkDrawRRectCommand::render(SkCanvas* canvas) const {
    render_rrect(canvas, fRRect);
    return true;
}

void SkDrawRRectCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_COORDS); make_json_rrect(writer, fRRect);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT); MakeJsonPaint(writer, fPaint, urlDataManager);
}

SkDrawDRRectCommand::SkDrawDRRectCommand(const SkRRect& outer,
                                         const SkRRect& inner,
                                         const SkPaint& paint)
    : INHERITED(kDrawDRRect_OpType) {
    fOuter = outer;
    fInner = inner;
    fPaint = paint;
}

void SkDrawDRRectCommand::execute(SkCanvas* canvas) const {
    canvas->drawDRRect(fOuter, fInner, fPaint);
}

bool SkDrawDRRectCommand::render(SkCanvas* canvas) const {
    render_drrect(canvas, fOuter, fInner);
    return true;
}

void SkDrawDRRectCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_OUTER); make_json_rrect(writer, fOuter);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_INNER); make_json_rrect(writer, fInner);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT); MakeJsonPaint(writer, fPaint, urlDataManager);
}

SkDrawShadowCommand::SkDrawShadowCommand(const SkPath& path, const SkDrawShadowRec& rec)
        : INHERITED(kDrawShadow_OpType) {
    fPath = path;
    fShadowRec = rec;
}

void SkDrawShadowCommand::execute(SkCanvas* canvas) const {
    canvas->private_draw_shadow_rec(fPath, fShadowRec);
}

bool SkDrawShadowCommand::render(SkCanvas* canvas) const {
    render_shadow(canvas, fPath, fShadowRec);
    return true;
}

void SkDrawShadowCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);

    bool geometricOnly = SkToBool(fShadowRec.fFlags & SkShadowFlags::kGeometricOnly_ShadowFlag);
    bool transparentOccluder =
            SkToBool(fShadowRec.fFlags & SkShadowFlags::kTransparentOccluder_ShadowFlag);

    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PATH); MakeJsonPath(writer, fPath);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_ZPLANE);
        MakeJsonPoint3(writer, fShadowRec.fZPlaneParams);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_LIGHTPOSITION);
        MakeJsonPoint3(writer, fShadowRec.fLightPos);
    writer.appendFloat(SKDEBUGCANVAS_ATTRIBUTE_LIGHTRADIUS, fShadowRec.fLightRadius);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_AMBIENTCOLOR);
        MakeJsonColor(writer, fShadowRec.fAmbientColor);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_SPOTCOLOR);
        MakeJsonColor(writer, fShadowRec.fSpotColor);
    store_bool(writer, SKDEBUGCANVAS_SHADOWFLAG_TRANSPARENT_OCC, transparentOccluder, false);
    store_bool(writer, SKDEBUGCANVAS_SHADOWFLAG_GEOMETRIC_ONLY, geometricOnly, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkDrawEdgeAAQuadCommand::SkDrawEdgeAAQuadCommand(const SkRect& rect, const SkPoint clip[],
                                                 SkCanvas::QuadAAFlags aa, SkColor color,
                                                 SkBlendMode mode)
        : INHERITED(kDrawEdgeAAQuad_OpType)
        , fRect(rect)
        , fHasClip(clip != nullptr)
        , fAA(aa)
        , fColor(color)
        , fMode(mode) {
    if (clip) {
        for (int i = 0; i < 4; ++i) {
            fClip[i] = clip[i];
        }
    }
}

void SkDrawEdgeAAQuadCommand::execute(SkCanvas* canvas) const {
    canvas->experimental_DrawEdgeAAQuad(fRect, fHasClip ? fClip : nullptr, fAA, fColor, fMode);
}

SkDrawEdgeAAImageSetCommand::SkDrawEdgeAAImageSetCommand(
        const SkCanvas::ImageSetEntry set[], int count, const SkPoint dstClips[],
        const SkMatrix preViewMatrices[], const SkPaint* paint,
        SkCanvas::SrcRectConstraint constraint)
        : INHERITED(kDrawEdgeAAImageSet_OpType)
        , fSet(count)
        , fCount(count)
        , fPaint(paint)
        , fConstraint(constraint) {
    int totalDstClipCount, totalMatrixCount;
    SkCanvasPriv::GetDstClipAndMatrixCounts(set, count, &totalDstClipCount, &totalMatrixCount);

    std::copy_n(set, count, fSet.get());
    fDstClips.reset(totalDstClipCount);
    std::copy_n(dstClips, totalDstClipCount, fDstClips.get());
    fPreViewMatrices.reset(totalMatrixCount);
    std::copy_n(preViewMatrices, totalMatrixCount, fPreViewMatrices.get());
}

void SkDrawEdgeAAImageSetCommand::execute(SkCanvas* canvas) const {
    canvas->experimental_DrawEdgeAAImageSet(fSet.get(), fCount, fDstClips.get(),
            fPreViewMatrices.get(), fPaint.getMaybeNull(), fConstraint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkDrawDrawableCommand::SkDrawDrawableCommand(SkDrawable* drawable, const SkMatrix* matrix)
    : INHERITED(kDrawDrawable_OpType)
    , fDrawable(SkRef(drawable))
    , fMatrix(matrix) {}

void SkDrawDrawableCommand::execute(SkCanvas* canvas) const {
    canvas->drawDrawable(fDrawable.get(), fMatrix.getMaybeNull());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkDrawVerticesCommand::SkDrawVerticesCommand(sk_sp<SkVertices> vertices, SkBlendMode bmode,
                                             const SkPaint& paint)
    : INHERITED(kDrawVertices_OpType)
    , fVertices(std::move(vertices))
    , fBlendMode(bmode)
    , fPaint(paint) {}

void SkDrawVerticesCommand::execute(SkCanvas* canvas) const {
    canvas->drawVertices(fVertices, fBlendMode, fPaint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkDrawAtlasCommand::SkDrawAtlasCommand(const SkImage* image, const SkRSXform xform[],
                                       const SkRect tex[], const SkColor colors[], int count,
                                       SkBlendMode bmode, const SkRect* cull,
                                       const SkPaint* paint)
    : INHERITED(kDrawAtlas_OpType)
    , fImage(SkRef(image))
    , fXform(xform, count)
    , fTex(tex, count)
    , fColors(colors, colors ? count : 0)
    , fBlendMode(bmode)
    , fCull(cull)
    , fPaint(paint) {}

void SkDrawAtlasCommand::execute(SkCanvas* canvas) const {
    canvas->drawAtlas(fImage.get(), fXform.begin(), fTex.begin(),
                      fColors.isEmpty() ? nullptr : fColors.begin(), fXform.count(), fBlendMode,
                      fCull.getMaybeNull(), fPaint.getMaybeNull());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkRestoreCommand::SkRestoreCommand()
    : INHERITED(kRestore_OpType) {}

void SkRestoreCommand::execute(SkCanvas* canvas) const {
    canvas->restore();
}

SkSaveCommand::SkSaveCommand()
    : INHERITED(kSave_OpType) {
}

void SkSaveCommand::execute(SkCanvas* canvas) const {
    canvas->save();
}

SkSaveLayerCommand::SkSaveLayerCommand(const SkCanvas::SaveLayerRec& rec)
    : INHERITED(kSaveLayer_OpType)
    , fBounds(rec.fBounds)
    , fPaint(rec.fPaint)
    , fBackdrop(SkSafeRef(rec.fBackdrop))
    , fSaveLayerFlags(rec.fSaveLayerFlags) {}

void SkSaveLayerCommand::execute(SkCanvas* canvas) const {
    canvas->saveLayer(SkCanvas::SaveLayerRec(fBounds.getMaybeNull(), fPaint.getMaybeNull(),
                                             fSaveLayerFlags));
}

void SkSaveLayerCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    if (fBounds.isValid()) {
        writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_BOUNDS); MakeJsonRect(writer, *fBounds);
    }
    if (fPaint.isValid()) {
        writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }
    if (fBackdrop != nullptr) {
        writer.beginObject(SKDEBUGCANVAS_ATTRIBUTE_BACKDROP);
        flatten(fBackdrop.get(), writer, urlDataManager);
        writer.endObject(); // backdrop
    }
    if (fSaveLayerFlags != 0) {
        SkDebugf("unsupported: saveLayer flags\n");
        SkASSERT(false);
    }
}

SkSetMatrixCommand::SkSetMatrixCommand(const SkMatrix& matrix)
    : INHERITED(kSetMatrix_OpType) {
    fMatrix = matrix;
}

void SkSetMatrixCommand::execute(SkCanvas* canvas) const {
    canvas->setMatrix(fMatrix);
}

void SkSetMatrixCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_MATRIX); MakeJsonMatrix(writer, fMatrix);
}
