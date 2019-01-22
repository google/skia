/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDrawCommand.h"

#include <algorithm>
#include "SkAutoMalloc.h"
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
        case kDrawImageSet_OpType: return "DrawImageSet";
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

Json::Value SkDrawCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result;
    result[SKDEBUGCANVAS_ATTRIBUTE_COMMAND] = this->GetCommandString(fOpType);
    result[SKDEBUGCANVAS_ATTRIBUTE_VISIBLE] = Json::Value(this->isVisible());
    return result;
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

void apply_paint_blend_mode(const SkPaint& paint, Json::Value* target) {
    const auto mode = paint.getBlendMode();
    if (mode != SkBlendMode::kSrcOver) {
        SkASSERT(static_cast<size_t>(mode) < SK_ARRAY_COUNT(gBlendModeMap));
        (*target)[SKDEBUGCANVAS_ATTRIBUTE_BLENDMODE] =
            Json::Value(gBlendModeMap[static_cast<size_t>(mode)]);
    }
}

};

Json::Value SkDrawCommand::MakeJsonColor(const SkColor color) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(SkColorGetA(color)));
    result.append(Json::Value(SkColorGetR(color)));
    result.append(Json::Value(SkColorGetG(color)));
    result.append(Json::Value(SkColorGetB(color)));
    return result;
}

Json::Value SkDrawCommand::MakeJsonColor4f(const SkColor4f& color) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(color.fA));
    result.append(Json::Value(color.fR));
    result.append(Json::Value(color.fG));
    result.append(Json::Value(color.fB));
    return result;
}

Json::Value SkDrawCommand::MakeJsonPoint(const SkPoint& point) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(point.x()));
    result.append(Json::Value(point.y()));
    return result;
}

Json::Value SkDrawCommand::MakeJsonPoint(SkScalar x, SkScalar y) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(x));
    result.append(Json::Value(y));
    return result;
}

Json::Value SkDrawCommand::MakeJsonPoint3(const SkPoint3& point) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(point.x()));
    result.append(Json::Value(point.y()));
    result.append(Json::Value(point.z()));
    return result;
}

Json::Value SkDrawCommand::MakeJsonRect(const SkRect& rect) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(rect.left()));
    result.append(Json::Value(rect.top()));
    result.append(Json::Value(rect.right()));
    result.append(Json::Value(rect.bottom()));
    return result;
}

Json::Value SkDrawCommand::MakeJsonIRect(const SkIRect& rect) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(rect.left()));
    result.append(Json::Value(rect.top()));
    result.append(Json::Value(rect.right()));
    result.append(Json::Value(rect.bottom()));
    return result;
}

static Json::Value make_json_rrect(const SkRRect& rrect) {
    Json::Value result(Json::arrayValue);
    result.append(SkDrawCommand::MakeJsonRect(rrect.rect()));
    result.append(SkDrawCommand::MakeJsonPoint(rrect.radii(SkRRect::kUpperLeft_Corner)));
    result.append(SkDrawCommand::MakeJsonPoint(rrect.radii(SkRRect::kUpperRight_Corner)));
    result.append(SkDrawCommand::MakeJsonPoint(rrect.radii(SkRRect::kLowerRight_Corner)));
    result.append(SkDrawCommand::MakeJsonPoint(rrect.radii(SkRRect::kLowerLeft_Corner)));
    return result;
}

Json::Value SkDrawCommand::MakeJsonMatrix(const SkMatrix& matrix) {
    Json::Value result(Json::arrayValue);
    Json::Value row1(Json::arrayValue);
    row1.append(Json::Value(matrix[0]));
    row1.append(Json::Value(matrix[1]));
    row1.append(Json::Value(matrix[2]));
    result.append(row1);
    Json::Value row2(Json::arrayValue);
    row2.append(Json::Value(matrix[3]));
    row2.append(Json::Value(matrix[4]));
    row2.append(Json::Value(matrix[5]));
    result.append(row2);
    Json::Value row3(Json::arrayValue);
    row3.append(Json::Value(matrix[6]));
    row3.append(Json::Value(matrix[7]));
    row3.append(Json::Value(matrix[8]));
    result.append(row3);
    return result;
}

Json::Value SkDrawCommand::MakeJsonScalar(SkScalar z) {
    Json::Value result(z);
    return result;
}

Json::Value SkDrawCommand::MakeJsonPath(const SkPath& path) {
    Json::Value result(Json::objectValue);
    switch (path.getFillType()) {
        case SkPath::kWinding_FillType:
            result[SKDEBUGCANVAS_ATTRIBUTE_FILLTYPE] = SKDEBUGCANVAS_FILLTYPE_WINDING;
            break;
        case SkPath::kEvenOdd_FillType:
            result[SKDEBUGCANVAS_ATTRIBUTE_FILLTYPE] = SKDEBUGCANVAS_FILLTYPE_EVENODD;
            break;
        case SkPath::kInverseWinding_FillType:
            result[SKDEBUGCANVAS_ATTRIBUTE_FILLTYPE] = SKDEBUGCANVAS_FILLTYPE_INVERSEWINDING;
            break;
        case SkPath::kInverseEvenOdd_FillType:
            result[SKDEBUGCANVAS_ATTRIBUTE_FILLTYPE] = SKDEBUGCANVAS_FILLTYPE_INVERSEEVENODD;
            break;
    }
    Json::Value verbs(Json::arrayValue);
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kLine_Verb: {
                Json::Value line(Json::objectValue);
                line[SKDEBUGCANVAS_VERB_LINE] = MakeJsonPoint(pts[1]);
                verbs.append(line);
                break;
            }
            case SkPath::kQuad_Verb: {
                Json::Value quad(Json::objectValue);
                Json::Value coords(Json::arrayValue);
                coords.append(MakeJsonPoint(pts[1]));
                coords.append(MakeJsonPoint(pts[2]));
                quad[SKDEBUGCANVAS_VERB_QUAD] = coords;
                verbs.append(quad);
                break;
            }
            case SkPath::kCubic_Verb: {
                Json::Value cubic(Json::objectValue);
                Json::Value coords(Json::arrayValue);
                coords.append(MakeJsonPoint(pts[1]));
                coords.append(MakeJsonPoint(pts[2]));
                coords.append(MakeJsonPoint(pts[3]));
                cubic[SKDEBUGCANVAS_VERB_CUBIC] = coords;
                verbs.append(cubic);
                break;
            }
            case SkPath::kConic_Verb: {
                Json::Value conic(Json::objectValue);
                Json::Value coords(Json::arrayValue);
                coords.append(MakeJsonPoint(pts[1]));
                coords.append(MakeJsonPoint(pts[2]));
                coords.append(Json::Value(iter.conicWeight()));
                conic[SKDEBUGCANVAS_VERB_CONIC] = coords;
                verbs.append(conic);
                break;
            }
            case SkPath::kMove_Verb: {
                Json::Value move(Json::objectValue);
                move[SKDEBUGCANVAS_VERB_MOVE] = MakeJsonPoint(pts[0]);
                verbs.append(move);
                break;
            }
            case SkPath::kClose_Verb:
                verbs.append(Json::Value(SKDEBUGCANVAS_VERB_CLOSE));
                break;
            case SkPath::kDone_Verb:
                break;
        }
    }
    result[SKDEBUGCANVAS_ATTRIBUTE_VERBS] = verbs;
    return result;
}

Json::Value SkDrawCommand::MakeJsonRegion(const SkRegion& region) {
    // TODO: Actually serialize the rectangles, rather than just devolving to path
    SkPath path;
    region.getBoundaryPath(&path);
    return MakeJsonPath(path);
}

static Json::Value make_json_regionop(SkClipOp op) {
    switch (op) {
        case kDifference_SkClipOp:
            return Json::Value(SKDEBUGCANVAS_REGIONOP_DIFFERENCE);
        case kIntersect_SkClipOp:
            return Json::Value(SKDEBUGCANVAS_REGIONOP_INTERSECT);
        case kUnion_SkClipOp:
            return Json::Value(SKDEBUGCANVAS_REGIONOP_UNION);
        case kXOR_SkClipOp:
            return Json::Value(SKDEBUGCANVAS_REGIONOP_XOR);
        case kReverseDifference_SkClipOp:
            return Json::Value(SKDEBUGCANVAS_REGIONOP_REVERSE_DIFFERENCE);
        case kReplace_SkClipOp:
            return Json::Value(SKDEBUGCANVAS_REGIONOP_REPLACE);
        default:
            SkASSERT(false);
            return Json::Value("<invalid region op>");
    }
}

static Json::Value make_json_pointmode(SkCanvas::PointMode mode) {
    switch (mode) {
        case SkCanvas::kPoints_PointMode:
            return Json::Value(SKDEBUGCANVAS_POINTMODE_POINTS);
        case SkCanvas::kLines_PointMode:
            return Json::Value(SKDEBUGCANVAS_POINTMODE_LINES);
        case SkCanvas::kPolygon_PointMode:
            return Json::Value(SKDEBUGCANVAS_POINTMODE_POLYGON);
        default:
            SkASSERT(false);
            return Json::Value("<invalid point mode>");
    }
}

static void store_scalar(Json::Value* target, const char* key, SkScalar value,
                         SkScalar defaultValue) {
    if (value != defaultValue) {
        (*target)[key] = Json::Value(value);
    }
}

static void store_bool(Json::Value* target, const char* key, bool value, bool defaultValue) {
    if (value != defaultValue) {
        (*target)[key] = Json::Value(value);
    }
}

static void encode_data(const void* bytes, size_t count, const char* contentType,
                        UrlDataManager& urlDataManager, Json::Value* target) {
    sk_sp<SkData> data(SkData::MakeWithCopy(bytes, count));
    SkString url = urlDataManager.addData(data.get(), contentType);
    *target = Json::Value(url.c_str());
}

void SkDrawCommand::flatten(const SkFlattenable* flattenable, Json::Value* target,
                            UrlDataManager& urlDataManager) {
    SkBinaryWriteBuffer buffer;
    flattenable->flatten(buffer);
    void* data = sk_malloc_throw(buffer.bytesWritten());
    buffer.writeToMemory(data);
    Json::Value jsonData;
    encode_data(data, buffer.bytesWritten(), "application/octet-stream", urlDataManager, &jsonData);
    Json::Value jsonFlattenable;
    jsonFlattenable[SKDEBUGCANVAS_ATTRIBUTE_NAME] = Json::Value(flattenable->getTypeName());
    jsonFlattenable[SKDEBUGCANVAS_ATTRIBUTE_DATA] = jsonData;

    SkJsonWriteBuffer jsonBuffer(&urlDataManager);
    flattenable->flatten(jsonBuffer);
    jsonFlattenable[SKDEBUGCANVAS_ATTRIBUTE_VALUES] = jsonBuffer.getValue();

    (*target) = jsonFlattenable;
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

bool SkDrawCommand::flatten(const SkImage& image, Json::Value* target,
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
    Json::Value jsonData;
    encode_data(encoded->data(), encoded->size(), "image/png", urlDataManager, &jsonData);
    (*target)[SKDEBUGCANVAS_ATTRIBUTE_DATA] = jsonData;
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

bool SkDrawCommand::flatten(const SkBitmap& bitmap, Json::Value* target,
                            UrlDataManager& urlDataManager) {
    sk_sp<SkImage> image(SkImage::MakeFromBitmap(bitmap));
    (*target)[SKDEBUGCANVAS_ATTRIBUTE_COLOR] = Json::Value(color_type_name(bitmap.colorType()));
    (*target)[SKDEBUGCANVAS_ATTRIBUTE_ALPHA] = Json::Value(alpha_type_name(bitmap.alphaType()));
    bool success = flatten(*image, target, urlDataManager);
    return success;
}

static void apply_font_hinting(const SkFont& font, Json::Value* target) {
    SkFontHinting hinting = font.getHinting();
    if (hinting != SkPaintDefaults_Hinting) {
        switch (hinting) {
            case kNo_SkFontHinting:
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_HINTING] = SKDEBUGCANVAS_HINTING_NONE;
                break;
            case kSlight_SkFontHinting:
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_HINTING] = SKDEBUGCANVAS_HINTING_SLIGHT;
                break;
            case kNormal_SkFontHinting:
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_HINTING] = SKDEBUGCANVAS_HINTING_NORMAL;
                break;
            case kFull_SkFontHinting:
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_HINTING] = SKDEBUGCANVAS_HINTING_FULL;
                break;
        }
    }
}

static void apply_font_edging(const SkFont& font, Json::Value* target) {
    switch (font.getEdging()) {
        case SkFont::Edging::kAlias:
            (*target)[SKDEBUGCANVAS_ATTRIBUTE_EDGING] = SKDEBUGCANVAS_EDGING_ALIAS;
            break;
        case SkFont::Edging::kAntiAlias:
            (*target)[SKDEBUGCANVAS_ATTRIBUTE_EDGING] = SKDEBUGCANVAS_EDGING_ANTIALIAS;
            break;
        case SkFont::Edging::kSubpixelAntiAlias:
            (*target)[SKDEBUGCANVAS_ATTRIBUTE_EDGING] = SKDEBUGCANVAS_EDGING_SUBPIXELANTIALIAS;
            break;
    }
}

static void apply_paint_color(const SkPaint& paint, Json::Value* target) {
    SkColor color = paint.getColor();
    if (color != SK_ColorBLACK) {
        Json::Value colorValue(Json::arrayValue);
        colorValue.append(Json::Value(SkColorGetA(color)));
        colorValue.append(Json::Value(SkColorGetR(color)));
        colorValue.append(Json::Value(SkColorGetG(color)));
        colorValue.append(Json::Value(SkColorGetB(color)));
        (*target)[SKDEBUGCANVAS_ATTRIBUTE_COLOR] = colorValue;
    }
}

static void apply_paint_style(const SkPaint& paint, Json::Value* target) {
    SkPaint::Style style = paint.getStyle();
    if (style != SkPaint::kFill_Style) {
        switch (style) {
            case SkPaint::kStroke_Style: {
                Json::Value stroke(SKDEBUGCANVAS_STYLE_STROKE);
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_STYLE] = stroke;
                break;
            }
            case SkPaint::kStrokeAndFill_Style: {
                Json::Value strokeAndFill(SKDEBUGCANVAS_STYLE_STROKEANDFILL);
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_STYLE] = strokeAndFill;
                break;
            }
            default: SkASSERT(false);
        }
    }
}

static void apply_paint_cap(const SkPaint& paint, Json::Value* target) {
    SkPaint::Cap cap = paint.getStrokeCap();
    if (cap != SkPaint::kDefault_Cap) {
        switch (cap) {
            case SkPaint::kButt_Cap:
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_CAP] = Json::Value(SKDEBUGCANVAS_CAP_BUTT);
                break;
            case SkPaint::kRound_Cap:
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_CAP] = Json::Value(SKDEBUGCANVAS_CAP_ROUND);
                break;
            case SkPaint::kSquare_Cap:
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_CAP] = Json::Value(SKDEBUGCANVAS_CAP_SQUARE);
                break;
            default: SkASSERT(false);
        }
    }
}

static void apply_paint_join(const SkPaint& paint, Json::Value* target) {
    SkPaint::Join join = paint.getStrokeJoin();
    if (join != SkPaint::kDefault_Join) {
        switch (join) {
            case SkPaint::kMiter_Join:
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_STROKEJOIN] = Json::Value(
                                                                          SKDEBUGCANVAS_MITER_JOIN);
                break;
            case SkPaint::kRound_Join:
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_STROKEJOIN] = Json::Value(
                                                                          SKDEBUGCANVAS_ROUND_JOIN);
                break;
            case SkPaint::kBevel_Join:
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_STROKEJOIN] = Json::Value(
                                                                        SKDEBUGCANVAS_BEVEL_JOIN);
                break;
            default: SkASSERT(false);
        }
    }
}

static void apply_paint_filterquality(const SkPaint& paint, Json::Value* target) {
    SkFilterQuality quality = paint.getFilterQuality();
    switch (quality) {
        case kNone_SkFilterQuality:
            break;
        case kLow_SkFilterQuality:
            (*target)[SKDEBUGCANVAS_ATTRIBUTE_FILTERQUALITY] = Json::Value(
                                                                   SKDEBUGCANVAS_FILTERQUALITY_LOW);
            break;
        case kMedium_SkFilterQuality:
            (*target)[SKDEBUGCANVAS_ATTRIBUTE_FILTERQUALITY] = Json::Value(
                                                                SKDEBUGCANVAS_FILTERQUALITY_MEDIUM);
            break;
        case kHigh_SkFilterQuality:
            (*target)[SKDEBUGCANVAS_ATTRIBUTE_FILTERQUALITY] = Json::Value(
                                                                  SKDEBUGCANVAS_FILTERQUALITY_HIGH);
            break;
    }
}

static void apply_paint_maskfilter(const SkPaint& paint, Json::Value* target,
                                   UrlDataManager& urlDataManager) {
    SkMaskFilter* maskFilter = paint.getMaskFilter();
    if (maskFilter != nullptr) {
        SkMaskFilterBase::BlurRec blurRec;
        if (as_MFB(maskFilter)->asABlur(&blurRec)) {
            Json::Value blur(Json::objectValue);
            blur[SKDEBUGCANVAS_ATTRIBUTE_SIGMA] = Json::Value(blurRec.fSigma);
            switch (blurRec.fStyle) {
                case SkBlurStyle::kNormal_SkBlurStyle:
                    blur[SKDEBUGCANVAS_ATTRIBUTE_STYLE] = Json::Value(
                                                                    SKDEBUGCANVAS_BLURSTYLE_NORMAL);
                    break;
                case SkBlurStyle::kSolid_SkBlurStyle:
                    blur[SKDEBUGCANVAS_ATTRIBUTE_STYLE] = Json::Value(
                                                                     SKDEBUGCANVAS_BLURSTYLE_SOLID);
                    break;
                case SkBlurStyle::kOuter_SkBlurStyle:
                    blur[SKDEBUGCANVAS_ATTRIBUTE_STYLE] = Json::Value(
                                                                     SKDEBUGCANVAS_BLURSTYLE_OUTER);
                    break;
                case SkBlurStyle::kInner_SkBlurStyle:
                    blur[SKDEBUGCANVAS_ATTRIBUTE_STYLE] = Json::Value(
                                                                     SKDEBUGCANVAS_BLURSTYLE_INNER);
                    break;
                default:
                    SkASSERT(false);
            }
            (*target)[SKDEBUGCANVAS_ATTRIBUTE_BLUR] = blur;
        } else {
            Json::Value jsonMaskFilter;
            SkDrawCommand::flatten(maskFilter, &jsonMaskFilter, urlDataManager);
            (*target)[SKDEBUGCANVAS_ATTRIBUTE_MASKFILTER] = jsonMaskFilter;
        }
    }
}

static void apply_paint_patheffect(const SkPaint& paint, Json::Value* target,
                                   UrlDataManager& urlDataManager) {
    SkPathEffect* pathEffect = paint.getPathEffect();
    if (pathEffect != nullptr) {
        SkPathEffect::DashInfo dashInfo;
        SkPathEffect::DashType dashType = pathEffect->asADash(&dashInfo);
        if (dashType == SkPathEffect::kDash_DashType) {
            dashInfo.fIntervals = (SkScalar*) sk_malloc_throw(dashInfo.fCount * sizeof(SkScalar));
            pathEffect->asADash(&dashInfo);
            Json::Value dashing(Json::objectValue);
            Json::Value intervals(Json::arrayValue);
            for (int32_t i = 0; i < dashInfo.fCount; i++) {
                intervals.append(Json::Value(dashInfo.fIntervals[i]));
            }
            sk_free(dashInfo.fIntervals);
            dashing[SKDEBUGCANVAS_ATTRIBUTE_INTERVALS] = intervals;
            dashing[SKDEBUGCANVAS_ATTRIBUTE_PHASE] = dashInfo.fPhase;
            (*target)[SKDEBUGCANVAS_ATTRIBUTE_DASHING] = dashing;
        } else {
            Json::Value jsonPathEffect;
            SkDrawCommand::flatten(pathEffect, &jsonPathEffect, urlDataManager);
            (*target)[SKDEBUGCANVAS_ATTRIBUTE_PATHEFFECT] = jsonPathEffect;
        }
    }
}

static void apply_font_typeface(const SkFont& font, Json::Value* target,
                                 UrlDataManager& urlDataManager) {
    SkTypeface* typeface = font.getTypefaceOrDefault();
    if (typeface != nullptr) {
        Json::Value jsonTypeface;
        SkDynamicMemoryWStream buffer;
        typeface->serialize(&buffer);
        void* data = sk_malloc_throw(buffer.bytesWritten());
        buffer.copyTo(data);
        Json::Value jsonData;
        encode_data(data, buffer.bytesWritten(), "application/octet-stream", urlDataManager,
                    &jsonData);
        jsonTypeface[SKDEBUGCANVAS_ATTRIBUTE_DATA] = jsonData;
        sk_free(data);
        (*target)[SKDEBUGCANVAS_ATTRIBUTE_TYPEFACE] = jsonTypeface;
    }
}

static void apply_paint_shader(const SkPaint& paint, Json::Value* target,
                               UrlDataManager& urlDataManager) {
    SkFlattenable* shader = paint.getShader();
    if (shader != nullptr) {
        Json::Value jsonShader;
        SkDrawCommand::flatten(shader, &jsonShader, urlDataManager);
        (*target)[SKDEBUGCANVAS_ATTRIBUTE_SHADER] = jsonShader;
    }
}

static void apply_paint_imagefilter(const SkPaint& paint, Json::Value* target,
                                    UrlDataManager& urlDataManager) {
    SkFlattenable* imageFilter = paint.getImageFilter();
    if (imageFilter != nullptr) {
        Json::Value jsonImageFilter;
        SkDrawCommand::flatten(imageFilter, &jsonImageFilter, urlDataManager);
        (*target)[SKDEBUGCANVAS_ATTRIBUTE_IMAGEFILTER] = jsonImageFilter;
    }
}

static void apply_paint_colorfilter(const SkPaint& paint, Json::Value* target,
                                    UrlDataManager& urlDataManager) {
    SkFlattenable* colorFilter = paint.getColorFilter();
    if (colorFilter != nullptr) {
        Json::Value jsonColorFilter;
        SkDrawCommand::flatten(colorFilter, &jsonColorFilter, urlDataManager);
        (*target)[SKDEBUGCANVAS_ATTRIBUTE_COLORFILTER] = jsonColorFilter;
    }
}

static void apply_paint_looper(const SkPaint& paint, Json::Value* target,
                               UrlDataManager& urlDataManager) {
    SkFlattenable* looper = paint.getLooper();
    if (looper != nullptr) {
        Json::Value jsonLooper;
        SkDrawCommand::flatten(looper, &jsonLooper, urlDataManager);
        (*target)[SKDEBUGCANVAS_ATTRIBUTE_LOOPER] = jsonLooper;
    }
}

Json::Value SkDrawCommand::MakeJsonPaint(const SkPaint& paint, UrlDataManager& urlDataManager) {
    Json::Value result(Json::objectValue);
    store_scalar(&result, SKDEBUGCANVAS_ATTRIBUTE_STROKEWIDTH, paint.getStrokeWidth(), 0.0f);
    store_scalar(&result, SKDEBUGCANVAS_ATTRIBUTE_STROKEMITER, paint.getStrokeMiter(),
                 SkPaintDefaults_MiterLimit);
    store_bool(&result, SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS, paint.isAntiAlias(), false);
    store_bool(&result, SKDEBUGCANVAS_ATTRIBUTE_DITHER, paint.isDither(), false);

    apply_paint_color(paint, &result);
    apply_paint_style(paint, &result);
    apply_paint_blend_mode(paint, &result);
    apply_paint_cap(paint, &result);
    apply_paint_join(paint, &result);
    apply_paint_filterquality(paint, &result);
    apply_paint_patheffect(paint, &result, urlDataManager);
    apply_paint_maskfilter(paint, &result, urlDataManager);
    apply_paint_shader(paint, &result, urlDataManager);
    apply_paint_looper(paint, &result, urlDataManager);
    apply_paint_imagefilter(paint, &result, urlDataManager);
    apply_paint_colorfilter(paint, &result, urlDataManager);
    return result;
}

static Json::Value MakeJsonFont(const SkFont& font, UrlDataManager& urlDataManager) {
    Json::Value result(Json::objectValue);
    store_bool(&result, SKDEBUGCANVAS_ATTRIBUTE_FAKEBOLDTEXT, font.isEmbolden(), false);
    store_bool(&result, SKDEBUGCANVAS_ATTRIBUTE_LINEARTEXT, font.isLinearMetrics(), false);
    store_bool(&result, SKDEBUGCANVAS_ATTRIBUTE_SUBPIXELTEXT, font.isSubpixel(), false);
    store_bool(&result, SKDEBUGCANVAS_ATTRIBUTE_EMBEDDEDBITMAPTEXT, font.isEmbeddedBitmaps(), false);
    store_bool(&result, SKDEBUGCANVAS_ATTRIBUTE_AUTOHINTING, font.isForceAutoHinting(), false);

    store_scalar(&result, SKDEBUGCANVAS_ATTRIBUTE_TEXTSIZE, font.getSize(),
                 SkPaintDefaults_TextSize);
    store_scalar(&result, SKDEBUGCANVAS_ATTRIBUTE_TEXTSCALEX, font.getScaleX(), SK_Scalar1);
    store_scalar(&result, SKDEBUGCANVAS_ATTRIBUTE_TEXTSCALEX, font.getSkewX(), 0.0f);
    apply_font_edging(font, &result);
    apply_font_hinting(font, &result);
    apply_font_typeface(font, &result, urlDataManager);
    return result;
}

Json::Value SkDrawCommand::MakeJsonLattice(const SkCanvas::Lattice& lattice) {
    Json::Value result(Json::objectValue);
    result[SKDEBUGCANVAS_ATTRIBUTE_LATTICEXCOUNT] = Json::Value(lattice.fXCount);
    result[SKDEBUGCANVAS_ATTRIBUTE_LATTICEYCOUNT] = Json::Value(lattice.fYCount);
    if (nullptr != lattice.fBounds) {
        result[SKDEBUGCANVAS_ATTRIBUTE_BOUNDS] = MakeJsonIRect(*lattice.fBounds);
    }
    Json::Value XDivs(Json::arrayValue);
    for (int i = 0; i < lattice.fXCount; i++) {
        XDivs.append(Json::Value(lattice.fXDivs[i]));
    }
    result[SKDEBUGCANVAS_ATTRIBUTE_LATTICEXDIVS] = XDivs;
    Json::Value YDivs(Json::arrayValue);
    for (int i = 0; i < lattice.fYCount; i++) {
        YDivs.append(Json::Value(lattice.fYDivs[i]));
    }
    result[SKDEBUGCANVAS_ATTRIBUTE_LATTICEYDIVS] = YDivs;
    if (nullptr != lattice.fRectTypes) {
        Json::Value flags(Json::arrayValue);
        int flagCount = 0;
        for (int row = 0; row < lattice.fYCount+1; row++) {
            Json::Value flagsRow(Json::arrayValue);
            for (int column = 0; column < lattice.fXCount+1; column++) {
                flagsRow.append(Json::Value(lattice.fRectTypes[flagCount++]));
            }
            flags.append(flagsRow);
        }
        result[SKDEBUGCANVAS_ATTRIBUTE_LATTICEFLAGS] = flags;
    }
    return result;
}

SkClearCommand::SkClearCommand(SkColor color) : INHERITED(kClear_OpType) {
    fColor = color;
}

void SkClearCommand::execute(SkCanvas* canvas) const {
    canvas->clear(fColor);
}

Json::Value SkClearCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_COLOR] = MakeJsonColor(fColor);
    return result;
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

Json::Value SkClipPathCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_PATH] = MakeJsonPath(fPath);
    result[SKDEBUGCANVAS_ATTRIBUTE_REGIONOP] = make_json_regionop(fOp);
    result[SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS] = fDoAA;
    return result;
}

SkClipRegionCommand::SkClipRegionCommand(const SkRegion& region, SkClipOp op)
    : INHERITED(kClipRegion_OpType) {
    fRegion = region;
    fOp = op;
}

void SkClipRegionCommand::execute(SkCanvas* canvas) const {
    canvas->clipRegion(fRegion, fOp);
}

Json::Value SkClipRegionCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_REGION] = MakeJsonRegion(fRegion);
    result[SKDEBUGCANVAS_ATTRIBUTE_REGIONOP] = make_json_regionop(fOp);
    return result;
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

Json::Value SkClipRectCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = MakeJsonRect(fRect);
    result[SKDEBUGCANVAS_ATTRIBUTE_REGIONOP] = make_json_regionop(fOp);
    result[SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS] = Json::Value(fDoAA);

    SkString desc;
    result[SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC] = Json::Value(str_append(&desc, fRect)->c_str());

    return result;
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

Json::Value SkClipRRectCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = make_json_rrect(fRRect);
    result[SKDEBUGCANVAS_ATTRIBUTE_REGIONOP] = make_json_regionop(fOp);
    result[SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS] = Json::Value(fDoAA);
    return result;
}

SkConcatCommand::SkConcatCommand(const SkMatrix& matrix)
    : INHERITED(kConcat_OpType) {
    fMatrix = matrix;
}

void SkConcatCommand::execute(SkCanvas* canvas) const {
    canvas->concat(fMatrix);
}

Json::Value SkConcatCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_MATRIX] = MakeJsonMatrix(fMatrix);
    return result;
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

Json::Value SkDrawAnnotationCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);

    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = MakeJsonRect(fRect);
    result["key"] = Json::Value(fKey.c_str());
    if (fValue.get()) {
        // TODO: dump out the "value"
    }

    SkString desc;
    str_append(&desc, fRect)->appendf(" %s", fKey.c_str());
    result[SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC] = Json::Value(desc.c_str());

    return result;
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

Json::Value SkDrawBitmapCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    Json::Value encoded;
    if (flatten(fBitmap, &encoded, urlDataManager)) {
        Json::Value command(Json::objectValue);
        result[SKDEBUGCANVAS_ATTRIBUTE_BITMAP] = encoded;
        result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = MakeJsonPoint(fLeft, fTop);
        if (fPaint.isValid()) {
            result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(*fPaint.get(), urlDataManager);
        }
    }
    return result;
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

Json::Value SkDrawBitmapLatticeCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    Json::Value encoded;
    if (flatten(fBitmap, &encoded, urlDataManager)) {
        result[SKDEBUGCANVAS_ATTRIBUTE_BITMAP] = encoded;
        result[SKDEBUGCANVAS_ATTRIBUTE_LATTICE] = MakeJsonLattice(fLattice);
        result[SKDEBUGCANVAS_ATTRIBUTE_DST] = MakeJsonRect(fDst);
        if (fPaint.isValid()) {
            result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(*fPaint.get(), urlDataManager);
        }
    }

    SkString desc;
    result[SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC] = Json::Value(str_append(&desc, fDst)->c_str());

    return result;
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

Json::Value SkDrawBitmapNineCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    Json::Value encoded;
    if (flatten(fBitmap, &encoded, urlDataManager)) {
        result[SKDEBUGCANVAS_ATTRIBUTE_BITMAP] = encoded;
        result[SKDEBUGCANVAS_ATTRIBUTE_CENTER] = MakeJsonIRect(fCenter);
        result[SKDEBUGCANVAS_ATTRIBUTE_DST] = MakeJsonRect(fDst);
        if (fPaint.isValid()) {
            result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(*fPaint.get(), urlDataManager);
        }
    }
    return result;
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

Json::Value SkDrawBitmapRectCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    Json::Value encoded;
    if (flatten(fBitmap, &encoded, urlDataManager)) {
        result[SKDEBUGCANVAS_ATTRIBUTE_BITMAP] = encoded;
        if (fSrc.isValid()) {
            result[SKDEBUGCANVAS_ATTRIBUTE_SRC] = MakeJsonRect(*fSrc.get());
        }
        result[SKDEBUGCANVAS_ATTRIBUTE_DST] = MakeJsonRect(fDst);
        if (fPaint.isValid()) {
            result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(*fPaint.get(), urlDataManager);
        }
        if (fConstraint == SkCanvas::kStrict_SrcRectConstraint) {
            result[SKDEBUGCANVAS_ATTRIBUTE_STRICT] = Json::Value(true);
        }
    }

    SkString desc;
    result[SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC] = Json::Value(str_append(&desc, fDst)->c_str());

    return result;
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

Json::Value SkDrawImageCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    Json::Value encoded;
    if (flatten(*fImage, &encoded, urlDataManager)) {
        result[SKDEBUGCANVAS_ATTRIBUTE_IMAGE] = encoded;
        result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = MakeJsonPoint(fLeft, fTop);
        if (fPaint.isValid()) {
            result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(*fPaint.get(), urlDataManager);
        }

        result[SKDEBUGCANVAS_ATTRIBUTE_UNIQUE_ID] = fImage->uniqueID();
        result[SKDEBUGCANVAS_ATTRIBUTE_WIDTH] = fImage->width();
        result[SKDEBUGCANVAS_ATTRIBUTE_HEIGHT] = fImage->height();
        switch (fImage->alphaType()) {
            case kOpaque_SkAlphaType:
                result[SKDEBUGCANVAS_ATTRIBUTE_ALPHA] = SKDEBUGCANVAS_ALPHATYPE_OPAQUE;
                break;
            case kPremul_SkAlphaType:
                result[SKDEBUGCANVAS_ATTRIBUTE_ALPHA] = SKDEBUGCANVAS_ALPHATYPE_PREMUL;
                break;
            case kUnpremul_SkAlphaType:
                result[SKDEBUGCANVAS_ATTRIBUTE_ALPHA] = SKDEBUGCANVAS_ALPHATYPE_UNPREMUL;
                break;
            default:
                result[SKDEBUGCANVAS_ATTRIBUTE_ALPHA] = SKDEBUGCANVAS_ALPHATYPE_UNKNOWN;
                break;
        }
    }
    return result;
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

Json::Value SkDrawImageLatticeCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    Json::Value encoded;
    if (flatten(*fImage.get(), &encoded, urlDataManager)) {
        result[SKDEBUGCANVAS_ATTRIBUTE_IMAGE] = encoded;
        result[SKDEBUGCANVAS_ATTRIBUTE_LATTICE] = MakeJsonLattice(fLattice);
        result[SKDEBUGCANVAS_ATTRIBUTE_DST] = MakeJsonRect(fDst);
        if (fPaint.isValid()) {
            result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(*fPaint.get(), urlDataManager);
        }
    }

    SkString desc;
    result[SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC] = Json::Value(str_append(&desc, fDst)->c_str());

    return result;
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

Json::Value SkDrawImageRectCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    Json::Value encoded;
    if (flatten(*fImage.get(), &encoded, urlDataManager)) {
        result[SKDEBUGCANVAS_ATTRIBUTE_IMAGE] = encoded;
        if (fSrc.isValid()) {
            result[SKDEBUGCANVAS_ATTRIBUTE_SRC] = MakeJsonRect(*fSrc.get());
        }
        result[SKDEBUGCANVAS_ATTRIBUTE_DST] = MakeJsonRect(fDst);
        if (fPaint.isValid()) {
            result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(*fPaint.get(), urlDataManager);
        }
        if (fConstraint == SkCanvas::kStrict_SrcRectConstraint) {
            result[SKDEBUGCANVAS_ATTRIBUTE_STRICT] = Json::Value(true);
        }
    }

    SkString desc;
    result[SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC] = Json::Value(str_append(&desc, fDst)->c_str());

    return result;
}

SkDrawImageSetCommand::SkDrawImageSetCommand(const SkCanvas::ImageSetEntry set[], int count,
                                             SkFilterQuality filterQuality, SkBlendMode mode)
        : INHERITED(kDrawImageSet_OpType)
        , fSet(count)
        , fCount(count)
        , fFilterQuality(filterQuality)
        , fMode(mode) {
    std::copy_n(set, count, fSet.get());
}

void SkDrawImageSetCommand::execute(SkCanvas* canvas) const {
    canvas->experimental_DrawImageSetV1(fSet.get(), fCount, fFilterQuality, fMode);
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

Json::Value SkDrawImageNineCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    Json::Value encoded;
    if (flatten(*fImage.get(), &encoded, urlDataManager)) {
        result[SKDEBUGCANVAS_ATTRIBUTE_IMAGE] = encoded;
        result[SKDEBUGCANVAS_ATTRIBUTE_CENTER] = MakeJsonIRect(fCenter);
        result[SKDEBUGCANVAS_ATTRIBUTE_DST] = MakeJsonRect(fDst);
        if (fPaint.isValid()) {
            result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(*fPaint.get(), urlDataManager);
        }
    }
    return result;
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

Json::Value SkDrawOvalCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = MakeJsonRect(fOval);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(fPaint, urlDataManager);
    return result;
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

Json::Value SkDrawArcCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = MakeJsonRect(fOval);
    result[SKDEBUGCANVAS_ATTRIBUTE_STARTANGLE] = MakeJsonScalar(fStartAngle);
    result[SKDEBUGCANVAS_ATTRIBUTE_SWEEPANGLE] = MakeJsonScalar(fSweepAngle);
    result[SKDEBUGCANVAS_ATTRIBUTE_USECENTER] = fUseCenter;
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(fPaint, urlDataManager);
    return result;
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

Json::Value SkDrawPaintCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(fPaint, urlDataManager);
    return result;
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

Json::Value SkDrawPathCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_PATH] = MakeJsonPath(fPath);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(fPaint, urlDataManager);
    return result;
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

Json::Value SkDrawRegionCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_REGION] = MakeJsonRegion(fRegion);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(fPaint, urlDataManager);
    return result;
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
            fMatrix.get()->mapRect(&bounds);
        }
        canvas->saveLayer(&bounds, fPaint.get());
    }

    if (fMatrix.isValid()) {
        if (!fPaint.isValid()) {
            canvas->save();
        }
        canvas->concat(*fMatrix.get());
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

Json::Value SkDrawPointsCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_MODE] = make_json_pointmode(fMode);
    Json::Value points(Json::arrayValue);
    for (int i = 0; i < fPts.count(); i++) {
        points.append(MakeJsonPoint(fPts[i]));
    }
    result[SKDEBUGCANVAS_ATTRIBUTE_POINTS] = points;
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(fPaint, urlDataManager);
    return result;
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

Json::Value SkDrawTextBlobCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    Json::Value runs(Json::arrayValue);
    SkTextBlobRunIterator iter(fBlob.get());
    while (!iter.done()) {
        Json::Value run(Json::objectValue);
        Json::Value jsonPositions(Json::arrayValue);
        Json::Value jsonGlyphs(Json::arrayValue);
        const SkScalar* iterPositions = iter.pos();
        const uint16_t* iterGlyphs = iter.glyphs();
        for (uint32_t i = 0; i < iter.glyphCount(); i++) {
            switch (iter.positioning()) {
                case SkTextBlobRunIterator::kFull_Positioning:
                    jsonPositions.append(MakeJsonPoint(iterPositions[i * 2],
                                                       iterPositions[i * 2 + 1]));
                    break;
                case SkTextBlobRunIterator::kHorizontal_Positioning:
                    jsonPositions.append(Json::Value(iterPositions[i]));
                    break;
                case SkTextBlobRunIterator::kDefault_Positioning:
                    break;
                case SkTextBlobRunIterator::kRSXform_Positioning:
                    // TODO_RSXFORM_BLOB
                    break;
            }
            jsonGlyphs.append(Json::Value(iterGlyphs[i]));
        }
        if (iter.positioning() != SkTextBlobRunIterator::kDefault_Positioning) {
            run[SKDEBUGCANVAS_ATTRIBUTE_POSITIONS] = jsonPositions;
        }
        run[SKDEBUGCANVAS_ATTRIBUTE_GLYPHS] = jsonGlyphs;
        SkPaint fontPaint;
        run[SKDEBUGCANVAS_ATTRIBUTE_FONT] = MakeJsonFont(iter.font(), urlDataManager);
        run[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = MakeJsonPoint(iter.offset());
        runs.append(run);
        iter.next();
    }
    SkRect bounds = fBlob->bounds();
    result[SKDEBUGCANVAS_ATTRIBUTE_RUNS] = runs;
    result[SKDEBUGCANVAS_ATTRIBUTE_X] = Json::Value(fXPos);
    result[SKDEBUGCANVAS_ATTRIBUTE_Y] = Json::Value(fYPos);
    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = MakeJsonRect(bounds);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(fPaint, urlDataManager);

    SkString desc;
    // make the bounds local by applying the x,y
    bounds.offset(fXPos, fYPos);
    result[SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC] = Json::Value(str_append(&desc, bounds)->c_str());

    return result;
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

Json::Value SkDrawPatchCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    Json::Value cubics = Json::Value(Json::arrayValue);
    for (int i = 0; i < 12; i++) {
        cubics.append(MakeJsonPoint(fCubics[i]));
    }
    result[SKDEBUGCANVAS_ATTRIBUTE_CUBICS] = cubics;
    if (fColorsPtr != nullptr) {
        Json::Value colors = Json::Value(Json::arrayValue);
        for (int i = 0; i < 4; i++) {
            colors.append(MakeJsonColor(fColorsPtr[i]));
        }
        result[SKDEBUGCANVAS_ATTRIBUTE_COLORS] = colors;
    }
    if (fTexCoordsPtr != nullptr) {
        Json::Value texCoords = Json::Value(Json::arrayValue);
        for (int i = 0; i < 4; i++) {
            texCoords.append(MakeJsonPoint(fTexCoords[i]));
        }
        result[SKDEBUGCANVAS_ATTRIBUTE_TEXTURECOORDS] = texCoords;
    }
    // fBlendMode
    return result;
}

SkDrawRectCommand::SkDrawRectCommand(const SkRect& rect, const SkPaint& paint)
    : INHERITED(kDrawRect_OpType) {
    fRect = rect;
    fPaint = paint;
}

void SkDrawRectCommand::execute(SkCanvas* canvas) const {
    canvas->drawRect(fRect, fPaint);
}

Json::Value SkDrawRectCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = MakeJsonRect(fRect);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(fPaint, urlDataManager);

    SkString desc;
    result[SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC] = Json::Value(str_append(&desc, fRect)->c_str());

    return result;
}

SkDrawEdgeAARectCommand::SkDrawEdgeAARectCommand(const SkRect& rect, SkCanvas::QuadAAFlags aa,
                                                 SkColor color, SkBlendMode mode)
    : INHERITED(kDrawEdgeAARect_OpType) {
    fRect = rect;
    fAA = aa;
    fColor = color;
    fMode = mode;
}

void SkDrawEdgeAARectCommand::execute(SkCanvas* canvas) const {
    canvas->experimental_DrawEdgeAARectV1(fRect, fAA, fColor, fMode);
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

Json::Value SkDrawRRectCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = make_json_rrect(fRRect);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(fPaint, urlDataManager);
    return result;
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

Json::Value SkDrawDRRectCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_OUTER] = make_json_rrect(fOuter);
    result[SKDEBUGCANVAS_ATTRIBUTE_INNER] = make_json_rrect(fInner);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(fPaint, urlDataManager);
    return result;
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

Json::Value SkDrawShadowCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_PATH] = MakeJsonPath(fPath);

    bool geometricOnly = SkToBool(fShadowRec.fFlags & SkShadowFlags::kGeometricOnly_ShadowFlag);
    bool transparentOccluder =
            SkToBool(fShadowRec.fFlags & SkShadowFlags::kTransparentOccluder_ShadowFlag);

    result[SKDEBUGCANVAS_ATTRIBUTE_PATH] = MakeJsonPath(fPath);
    result[SKDEBUGCANVAS_ATTRIBUTE_ZPLANE] = MakeJsonPoint3(fShadowRec.fZPlaneParams);
    result[SKDEBUGCANVAS_ATTRIBUTE_LIGHTPOSITION] = MakeJsonPoint3(fShadowRec.fLightPos);
    result[SKDEBUGCANVAS_ATTRIBUTE_LIGHTRADIUS] = MakeJsonScalar(fShadowRec.fLightRadius);
    result[SKDEBUGCANVAS_ATTRIBUTE_AMBIENTCOLOR] = MakeJsonColor(fShadowRec.fAmbientColor);
    result[SKDEBUGCANVAS_ATTRIBUTE_SPOTCOLOR] = MakeJsonColor(fShadowRec.fSpotColor);
    store_bool(&result, SKDEBUGCANVAS_SHADOWFLAG_TRANSPARENT_OCC, transparentOccluder, false);
    store_bool(&result, SKDEBUGCANVAS_SHADOWFLAG_GEOMETRIC_ONLY, geometricOnly, false);
    return result;
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

Json::Value SkSaveLayerCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    if (fBounds.isValid()) {
        result[SKDEBUGCANVAS_ATTRIBUTE_BOUNDS] = MakeJsonRect(*fBounds.get());
    }
    if (fPaint.isValid()) {
        result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(*fPaint.get(), urlDataManager);
    }
    if (fBackdrop != nullptr) {
        Json::Value jsonBackdrop;
        flatten(fBackdrop.get(), &jsonBackdrop, urlDataManager);
        result[SKDEBUGCANVAS_ATTRIBUTE_BACKDROP] = jsonBackdrop;
    }
    if (fSaveLayerFlags != 0) {
        SkDebugf("unsupported: saveLayer flags\n");
        SkASSERT(false);
    }
    return result;
}

SkSetMatrixCommand::SkSetMatrixCommand(const SkMatrix& matrix)
    : INHERITED(kSetMatrix_OpType) {
    fMatrix = matrix;
}

void SkSetMatrixCommand::execute(SkCanvas* canvas) const {
    canvas->setMatrix(fMatrix);
}

Json::Value SkSetMatrixCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_MATRIX] = MakeJsonMatrix(fMatrix);
    return result;
}
