/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDrawCommand.h"

#include "png.h"

#include "SkAutoMalloc.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilter.h"
#include "SkDashPathEffect.h"
#include "SkImageFilter.h"
#include "SkJsonWriteBuffer.h"
#include "SkMaskFilter.h"
#include "SkObjectParser.h"
#include "SkPaintDefaults.h"
#include "SkPathEffect.h"
#include "SkPicture.h"
#include "SkTextBlob.h"
#include "SkTextBlobRunIterator.h"
#include "SkTHash.h"
#include "SkTypeface.h"
#include "SkValidatingReadBuffer.h"
#include "SkWriteBuffer.h"
#include "picture_utils.h"
#include "SkClipOpPriv.h"
#include <SkLatticeIter.h>

#define SKDEBUGCANVAS_ATTRIBUTE_COMMAND           "command"
#define SKDEBUGCANVAS_ATTRIBUTE_VISIBLE           "visible"
#define SKDEBUGCANVAS_ATTRIBUTE_MATRIX            "matrix"
#define SKDEBUGCANVAS_ATTRIBUTE_DRAWDEPTHTRANS    "drawDepthTranslation"
#define SKDEBUGCANVAS_ATTRIBUTE_COORDS            "coords"
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
#define SKDEBUGCANVAS_ATTRIBUTE_VERTICALTEXT      "verticalText"
#define SKDEBUGCANVAS_ATTRIBUTE_REGION            "region"
#define SKDEBUGCANVAS_ATTRIBUTE_REGIONOP          "op"
#define SKDEBUGCANVAS_ATTRIBUTE_EDGESTYLE         "edgeStyle"
#define SKDEBUGCANVAS_ATTRIBUTE_DEVICEREGION      "deviceRegion"
#define SKDEBUGCANVAS_ATTRIBUTE_BLUR              "blur"
#define SKDEBUGCANVAS_ATTRIBUTE_SIGMA             "sigma"
#define SKDEBUGCANVAS_ATTRIBUTE_QUALITY           "quality"
#define SKDEBUGCANVAS_ATTRIBUTE_TEXTALIGN         "textAlign"
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

#define SKDEBUGCANVAS_ALIGN_LEFT                  "left"
#define SKDEBUGCANVAS_ALIGN_CENTER                "center"
#define SKDEBUGCANVAS_ALIGN_RIGHT                 "right"

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

typedef SkDrawCommand* (*FROM_JSON)(Json::Value&, UrlDataManager&);

static SkString* str_append(SkString* str, const SkRect& r) {
    str->appendf(" [%g %g %g %g]", r.left(), r.top(), r.right(), r.bottom());
    return str;
}

// TODO(chudy): Refactor into non subclass model.

SkDrawCommand::SkDrawCommand(OpType type)
    : fOpType(type)
    , fVisible(true) {
}

SkDrawCommand::~SkDrawCommand() {
    fInfo.deleteAll();
}

const char* SkDrawCommand::GetCommandString(OpType type) {
    switch (type) {
        case kBeginDrawPicture_OpType: return "BeginDrawPicture";
        case kClipPath_OpType: return "ClipPath";
        case kClipRegion_OpType: return "ClipRegion";
        case kClipRect_OpType: return "ClipRect";
        case kClipRRect_OpType: return "ClipRRect";
        case kConcat_OpType: return "Concat";
        case kDrawAnnotation_OpType: return "DrawAnnotation";
        case kDrawBitmap_OpType: return "DrawBitmap";
        case kDrawBitmapNine_OpType: return "DrawBitmapNine";
        case kDrawBitmapRect_OpType: return "DrawBitmapRect";
        case kDrawClear_OpType: return "DrawClear";
        case kDrawDRRect_OpType: return "DrawDRRect";
        case kDrawImage_OpType: return "DrawImage";
        case kDrawImageLattice_OpType: return "DrawImageLattice";
        case kDrawImageRect_OpType: return "DrawImageRect";
        case kDrawOval_OpType: return "DrawOval";
        case kDrawPaint_OpType: return "DrawPaint";
        case kDrawPatch_OpType: return "DrawPatch";
        case kDrawPath_OpType: return "DrawPath";
        case kDrawPoints_OpType: return "DrawPoints";
        case kDrawPosText_OpType: return "DrawPosText";
        case kDrawPosTextH_OpType: return "DrawPosTextH";
        case kDrawRect_OpType: return "DrawRect";
        case kDrawRRect_OpType: return "DrawRRect";
        case kDrawText_OpType: return "DrawText";
        case kDrawTextBlob_OpType: return "DrawTextBlob";
        case kDrawTextOnPath_OpType: return "DrawTextOnPath";
        case kDrawTextRSXform_OpType: return "DrawTextRSXform";
        case kDrawVertices_OpType: return "DrawVertices";
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

SkString SkDrawCommand::toString() const {
    return SkString(GetCommandString(fOpType));
}

Json::Value SkDrawCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result;
    result[SKDEBUGCANVAS_ATTRIBUTE_COMMAND] = this->GetCommandString(fOpType);
    result[SKDEBUGCANVAS_ATTRIBUTE_VISIBLE] = Json::Value(this->isVisible());
    return result;
}

#define INSTALL_FACTORY(name) factories.set(SkString(GetCommandString(k ## name ##_OpType)), \
                                            (FROM_JSON) Sk ## name ## Command::fromJSON)
SkDrawCommand* SkDrawCommand::fromJSON(Json::Value& command, UrlDataManager& urlDataManager) {
    static SkTHashMap<SkString, FROM_JSON> factories;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        INSTALL_FACTORY(Restore);
        INSTALL_FACTORY(ClipPath);
        INSTALL_FACTORY(ClipRegion);
        INSTALL_FACTORY(ClipRect);
        INSTALL_FACTORY(ClipRRect);
        INSTALL_FACTORY(Concat);
        INSTALL_FACTORY(DrawAnnotation);
        INSTALL_FACTORY(DrawBitmap);
        INSTALL_FACTORY(DrawBitmapRect);
        INSTALL_FACTORY(DrawBitmapNine);
        INSTALL_FACTORY(DrawImage);
        INSTALL_FACTORY(DrawImageRect);
        INSTALL_FACTORY(DrawOval);
        INSTALL_FACTORY(DrawPaint);
        INSTALL_FACTORY(DrawPath);
        INSTALL_FACTORY(DrawPoints);
        INSTALL_FACTORY(DrawText);
        INSTALL_FACTORY(DrawPosText);
        INSTALL_FACTORY(DrawPosTextH);
        INSTALL_FACTORY(DrawTextOnPath);
        INSTALL_FACTORY(DrawTextRSXform);
        INSTALL_FACTORY(DrawTextBlob);

        INSTALL_FACTORY(DrawRect);
        INSTALL_FACTORY(DrawRRect);
        INSTALL_FACTORY(DrawDRRect);
        INSTALL_FACTORY(DrawPatch);
        INSTALL_FACTORY(Save);
        INSTALL_FACTORY(SaveLayer);
        INSTALL_FACTORY(SetMatrix);
    }
    SkString name = SkString(command[SKDEBUGCANVAS_ATTRIBUTE_COMMAND].asCString());
    FROM_JSON* factory = factories.find(name);
    if (factory == nullptr) {
        SkDebugf("no JSON factory for '%s'\n", name.c_str());
        return nullptr;
    }
    return (*factory)(command, urlDataManager);
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

void extract_json_paint_blend_mode(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_BLENDMODE)) {
        const char* mode = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_BLENDMODE].asCString();

        for (size_t i = 0; i < SK_ARRAY_COUNT(gBlendModeMap); ++i) {
            if (!strcmp(mode, gBlendModeMap[i])) {
                target->setBlendMode(static_cast<SkBlendMode>(i));
                break;
            }
        }
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
    return Json::Value("<unimplemented>");
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
    };
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
    };
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

static void write_png_callback(png_structp png_ptr, png_bytep data, png_size_t length) {
    SkWStream* out = (SkWStream*) png_get_io_ptr(png_ptr);
    out->write(data, length);
}

void SkDrawCommand::WritePNG(const uint8_t* rgba, unsigned width, unsigned height,
                             SkWStream& out, bool isOpaque) {
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    SkASSERT(png != nullptr);
    png_infop info_ptr = png_create_info_struct(png);
    SkASSERT(info_ptr != nullptr);
    if (setjmp(png_jmpbuf(png))) {
        SkFAIL("png encode error");
    }
    png_set_write_fn(png, &out, write_png_callback, NULL);
    int colorType = isOpaque ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGBA;
    png_set_IHDR(png, info_ptr, width, height, 8, colorType, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_compression_level(png, 1);
    png_bytepp rows = (png_bytepp) sk_malloc_throw(height * sizeof(png_byte*));
    png_bytep pixels = (png_bytep) sk_malloc_throw(width * height * 4);
    for (png_size_t y = 0; y < height; ++y) {
        const uint8_t* src = rgba + y * width * 4;
        rows[y] = pixels + y * width * 4;
        for (png_size_t x = 0; x < width; ++x) {
            rows[y][x * 4] = src[x * 4];
            rows[y][x * 4 + 1] = src[x * 4 + 1];
            rows[y][x * 4 + 2] = src[x * 4 + 2];
            rows[y][x * 4 + 3] = src[x * 4 + 3];
        }
    }
    png_write_info(png, info_ptr);
    if (isOpaque) {
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    }
    png_set_filter(png, 0, PNG_NO_FILTERS);
    png_write_image(png, &rows[0]);
    png_destroy_write_struct(&png, NULL);
    sk_free(rows);
    sk_free(pixels);
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
    sk_sp<SkData> encodedBitmap = sk_tools::encode_bitmap_for_png(bm);

    SkDynamicMemoryWStream out;
    SkDrawCommand::WritePNG(encodedBitmap->bytes(), image.width(), image.height(),
                            out, false);
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

static Json::ArrayIndex decode_data(Json::Value data, UrlDataManager& urlDataManager,
                                    const void** target) {
    UrlDataManager::UrlData* urlData = urlDataManager.getDataFromUrl(SkString(data.asCString()));
    if (urlData == nullptr) {
        SkASSERT(false);
        *target = nullptr;
        return 0;
    }
    *target = urlData->fData->data();
    // cast should be safe for any reasonably-sized object...
    return (Json::ArrayIndex) urlData->fData->size();
}

static SkFlattenable* load_flattenable(Json::Value jsonFlattenable,
                                       UrlDataManager& urlDataManager) {
    if (!jsonFlattenable.isMember(SKDEBUGCANVAS_ATTRIBUTE_NAME)) {
        return nullptr;
    }
    const char* name = jsonFlattenable[SKDEBUGCANVAS_ATTRIBUTE_NAME].asCString();
    SkFlattenable::Factory factory = SkFlattenable::NameToFactory(name);
    if (factory == nullptr) {
        SkDebugf("no factory for loading '%s'\n", name);
        return nullptr;
    }
    const void* data;
    int size = decode_data(jsonFlattenable[SKDEBUGCANVAS_ATTRIBUTE_DATA], urlDataManager, &data);
    SkValidatingReadBuffer buffer(data, size);
    sk_sp<SkFlattenable> result = factory(buffer);
    if (!buffer.isValid()) {
        SkDebugf("invalid buffer loading flattenable\n");
        return nullptr;
    }
    return result.release();
}

static SkColorType colortype_from_name(const char* name) {
    if (!strcmp(name, SKDEBUGCANVAS_COLORTYPE_ARGB4444)) {
        return kARGB_4444_SkColorType;
    }
    else if (!strcmp(name, SKDEBUGCANVAS_COLORTYPE_RGBA8888)) {
        return kRGBA_8888_SkColorType;
    }
    else if (!strcmp(name, SKDEBUGCANVAS_COLORTYPE_BGRA8888)) {
        return kBGRA_8888_SkColorType;
    }
    else if (!strcmp(name, SKDEBUGCANVAS_COLORTYPE_565)) {
        return kRGB_565_SkColorType;
    }
    else if (!strcmp(name, SKDEBUGCANVAS_COLORTYPE_GRAY8)) {
        return kGray_8_SkColorType;
    }
    else if (!strcmp(name, SKDEBUGCANVAS_COLORTYPE_ALPHA8)) {
        return kAlpha_8_SkColorType;
    }
    SkASSERT(false);
    return kN32_SkColorType;
}

static SkBitmap* convert_colortype(SkBitmap* bitmap, SkColorType colorType) {
    if (bitmap->colorType() == colorType  ) {
        return bitmap;
    }
    SkBitmap* dst = new SkBitmap();
    if (dst->tryAllocPixels(bitmap->info().makeColorType(colorType)) &&
        bitmap->readPixels(dst->info(), dst->getPixels(), dst->rowBytes(), 0, 0))
    {
        delete bitmap;
        return dst;
    }
    SkASSERT(false);
    delete dst;
    return bitmap;
}

// caller is responsible for freeing return value
static SkBitmap* load_bitmap(const Json::Value& jsonBitmap, UrlDataManager& urlDataManager) {
    if (!jsonBitmap.isMember(SKDEBUGCANVAS_ATTRIBUTE_DATA)) {
        SkDebugf("invalid bitmap\n");
        return nullptr;
    }
    const void* data;
    int size = decode_data(jsonBitmap[SKDEBUGCANVAS_ATTRIBUTE_DATA], urlDataManager, &data);
    sk_sp<SkData> encoded(SkData::MakeWithoutCopy(data, size));
    sk_sp<SkImage> image(SkImage::MakeFromEncoded(std::move(encoded), nullptr));

    std::unique_ptr<SkBitmap> bitmap(new SkBitmap());
    if (nullptr != image) {
        if (!image->asLegacyBitmap(bitmap.get(), SkImage::kRW_LegacyBitmapMode)) {
            SkDebugf("image decode failed\n");
            return nullptr;
        }

        if (jsonBitmap.isMember(SKDEBUGCANVAS_ATTRIBUTE_COLOR)) {
            const char* ctName = jsonBitmap[SKDEBUGCANVAS_ATTRIBUTE_COLOR].asCString();
            SkColorType ct = colortype_from_name(ctName);
            bitmap.reset(convert_colortype(bitmap.release(), ct));
        }
        return bitmap.release();
    }
    SkDebugf("image decode failed\n");
    return nullptr;
}

static sk_sp<SkImage> load_image(const Json::Value& jsonImage, UrlDataManager& urlDataManager) {
    SkBitmap* bitmap = load_bitmap(jsonImage, urlDataManager);
    if (bitmap == nullptr) {
        return nullptr;
    }
    auto result = SkImage::MakeFromBitmap(*bitmap);
    delete bitmap;
    return result;
}

bool SkDrawCommand::flatten(const SkBitmap& bitmap, Json::Value* target,
                            UrlDataManager& urlDataManager) {
    sk_sp<SkImage> image(SkImage::MakeFromBitmap(bitmap));
    (*target)[SKDEBUGCANVAS_ATTRIBUTE_COLOR] = Json::Value(color_type_name(bitmap.colorType()));
    (*target)[SKDEBUGCANVAS_ATTRIBUTE_ALPHA] = Json::Value(alpha_type_name(bitmap.alphaType()));
    bool success = flatten(*image, target, urlDataManager);
    return success;
}

static void apply_paint_hinting(const SkPaint& paint, Json::Value* target) {
    SkPaint::Hinting hinting = paint.getHinting();
    if (hinting != SkPaintDefaults_Hinting) {
        switch (hinting) {
            case SkPaint::kNo_Hinting:
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_HINTING] = SKDEBUGCANVAS_HINTING_NONE;
                break;
            case SkPaint::kSlight_Hinting:
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_HINTING] = SKDEBUGCANVAS_HINTING_SLIGHT;
                break;
            case SkPaint::kNormal_Hinting:
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_HINTING] = SKDEBUGCANVAS_HINTING_NORMAL;
                break;
            case SkPaint::kFull_Hinting:
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_HINTING] = SKDEBUGCANVAS_HINTING_FULL;
                break;
        }
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
        (*target)[SKDEBUGCANVAS_ATTRIBUTE_COLOR] = colorValue;;
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
        SkMaskFilter::BlurRec blurRec;
        if (maskFilter->asABlur(&blurRec)) {
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
            switch (blurRec.fQuality) {
                case SkBlurQuality::kLow_SkBlurQuality:
                    blur[SKDEBUGCANVAS_ATTRIBUTE_QUALITY] = Json::Value(
                                                                     SKDEBUGCANVAS_BLURQUALITY_LOW);
                    break;
                case SkBlurQuality::kHigh_SkBlurQuality:
                    blur[SKDEBUGCANVAS_ATTRIBUTE_QUALITY] = Json::Value(
                                                                    SKDEBUGCANVAS_BLURQUALITY_HIGH);
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

static void apply_paint_textalign(const SkPaint& paint, Json::Value* target) {
    SkPaint::Align textAlign = paint.getTextAlign();
    if (textAlign != SkPaint::kLeft_Align) {
        switch (textAlign) {
            case SkPaint::kCenter_Align: {
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_TEXTALIGN] = SKDEBUGCANVAS_ALIGN_CENTER;
                break;
            }
            case SkPaint::kRight_Align: {
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_TEXTALIGN] = SKDEBUGCANVAS_ALIGN_RIGHT;
                break;
            }
            default: SkASSERT(false);
        }
    }
}

static void apply_paint_typeface(const SkPaint& paint, Json::Value* target,
                                 UrlDataManager& urlDataManager) {
    SkTypeface* typeface = paint.getTypeface();
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
    store_bool(&result, SKDEBUGCANVAS_ATTRIBUTE_FAKEBOLDTEXT, paint.isFakeBoldText(), false);
    store_bool(&result, SKDEBUGCANVAS_ATTRIBUTE_LINEARTEXT, paint.isLinearText(), false);
    store_bool(&result, SKDEBUGCANVAS_ATTRIBUTE_SUBPIXELTEXT, paint.isSubpixelText(), false);
    store_bool(&result, SKDEBUGCANVAS_ATTRIBUTE_DEVKERNTEXT, paint.isDevKernText(), false);
    store_bool(&result, SKDEBUGCANVAS_ATTRIBUTE_LCDRENDERTEXT, paint.isLCDRenderText(), false);
    store_bool(&result, SKDEBUGCANVAS_ATTRIBUTE_EMBEDDEDBITMAPTEXT, paint.isEmbeddedBitmapText(), false);
    store_bool(&result, SKDEBUGCANVAS_ATTRIBUTE_AUTOHINTING, paint.isAutohinted(), false);
    store_bool(&result, SKDEBUGCANVAS_ATTRIBUTE_VERTICALTEXT, paint.isVerticalText(), false);
    //kGenA8FromLCD_Flag

    store_scalar(&result, SKDEBUGCANVAS_ATTRIBUTE_TEXTSIZE, paint.getTextSize(),
                 SkPaintDefaults_TextSize);
    store_scalar(&result, SKDEBUGCANVAS_ATTRIBUTE_TEXTSCALEX, paint.getTextScaleX(), SK_Scalar1);
    store_scalar(&result, SKDEBUGCANVAS_ATTRIBUTE_TEXTSCALEX, paint.getTextSkewX(), 0.0f);
    apply_paint_hinting(paint, &result);
    apply_paint_color(paint, &result);
    apply_paint_style(paint, &result);
    apply_paint_blend_mode(paint, &result);
    apply_paint_cap(paint, &result);
    apply_paint_join(paint, &result);
    apply_paint_filterquality(paint, &result);
    apply_paint_textalign(paint, &result);
    apply_paint_patheffect(paint, &result, urlDataManager);
    apply_paint_maskfilter(paint, &result, urlDataManager);
    apply_paint_shader(paint, &result, urlDataManager);
    apply_paint_looper(paint, &result, urlDataManager);
    apply_paint_imagefilter(paint, &result, urlDataManager);
    apply_paint_colorfilter(paint, &result, urlDataManager);
    apply_paint_typeface(paint, &result, urlDataManager);
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
    if (nullptr != lattice.fFlags) {
        Json::Value flags(Json::arrayValue);
        int flagCount = 0;
        for (int row = 0; row < lattice.fYCount+1; row++) {
            Json::Value flagsRow(Json::arrayValue);
            for (int column = 0; column < lattice.fXCount+1; column++) {
                flagsRow.append(Json::Value(lattice.fFlags[flagCount++]));
            }
            flags.append(flagsRow);
        }
        result[SKDEBUGCANVAS_ATTRIBUTE_LATTICEFLAGS] = flags;
    }
    return result;
}

static SkPoint get_json_point(Json::Value point) {
    return SkPoint::Make(point[0].asFloat(), point[1].asFloat());
}

static SkColor get_json_color(Json::Value color) {
    return SkColorSetARGB(color[0].asInt(), color[1].asInt(), color[2].asInt(), color[3].asInt());
}

static void extract_json_paint_color(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_COLOR)) {
        target->setColor(get_json_color(jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_COLOR]));
    }
}

static void extract_json_paint_shader(Json::Value& jsonPaint, UrlDataManager& urlDataManager,
                                      SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_SHADER)) {
        Json::Value jsonShader = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_SHADER];
        SkShader* shader = (SkShader*) load_flattenable(jsonShader, urlDataManager);
        if (shader != nullptr) {
            target->setShader(sk_ref_sp(shader));
        }
    }
}

static void extract_json_paint_patheffect(Json::Value& jsonPaint, UrlDataManager& urlDataManager,
                                          SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_PATHEFFECT)) {
        Json::Value jsonPathEffect = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_PATHEFFECT];
        sk_sp<SkPathEffect> pathEffect((SkPathEffect*)load_flattenable(jsonPathEffect,
                                                                       urlDataManager));
        if (pathEffect != nullptr) {
            target->setPathEffect(pathEffect);
        }
    }
}

static void extract_json_paint_maskfilter(Json::Value& jsonPaint, UrlDataManager& urlDataManager,
                                          SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_MASKFILTER)) {
        Json::Value jsonMaskFilter = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_MASKFILTER];
        sk_sp<SkMaskFilter> maskFilter((SkMaskFilter*)load_flattenable(jsonMaskFilter,
                                                                       urlDataManager));
        if (maskFilter) {
            target->setMaskFilter(std::move(maskFilter));
        }
    }
}

static void extract_json_paint_colorfilter(Json::Value& jsonPaint, UrlDataManager& urlDataManager,
                                           SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_COLORFILTER)) {
        Json::Value jsonColorFilter = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_COLORFILTER];
        sk_sp<SkColorFilter> colorFilter((SkColorFilter*)load_flattenable(jsonColorFilter,
                                                                          urlDataManager));
        if (colorFilter != nullptr) {
            target->setColorFilter(colorFilter);
        }
    }
}

static void extract_json_paint_looper(Json::Value& jsonPaint, UrlDataManager& urlDataManager,
                                      SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_LOOPER)) {
        Json::Value jsonLooper = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_LOOPER];
        sk_sp<SkDrawLooper> looper((SkDrawLooper*) load_flattenable(jsonLooper, urlDataManager));
        if (looper != nullptr) {
            target->setLooper(std::move(looper));
        }
    }
}

static void extract_json_paint_imagefilter(Json::Value& jsonPaint, UrlDataManager& urlDataManager,
                                           SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_IMAGEFILTER)) {
        Json::Value jsonImageFilter = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_IMAGEFILTER];
        sk_sp<SkImageFilter> imageFilter((SkImageFilter*) load_flattenable(jsonImageFilter,
                                                                           urlDataManager));
        if (imageFilter != nullptr) {
            target->setImageFilter(imageFilter);
        }
    }
}

static void extract_json_paint_typeface(Json::Value& jsonPaint, UrlDataManager& urlDataManager,
                                        SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_TYPEFACE)) {
        Json::Value jsonTypeface = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_TYPEFACE];
        Json::Value jsonData = jsonTypeface[SKDEBUGCANVAS_ATTRIBUTE_DATA];
        const void* data;
        Json::ArrayIndex length = decode_data(jsonData, urlDataManager, &data);
        SkMemoryStream buffer(data, length);
        target->setTypeface(SkTypeface::MakeDeserialize(&buffer));
    }
}

static void extract_json_paint_hinting(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_HINTING)) {
        const char* hinting = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_HINTING].asCString();
        if (!strcmp(hinting, SKDEBUGCANVAS_HINTING_NONE)) {
            target->setHinting(SkPaint::kNo_Hinting);
        } else if (!strcmp(hinting, SKDEBUGCANVAS_HINTING_SLIGHT)) {
            target->setHinting(SkPaint::kSlight_Hinting);
        } else if (!strcmp(hinting, SKDEBUGCANVAS_HINTING_NORMAL)) {
            target->setHinting(SkPaint::kNormal_Hinting);
        } else if (!strcmp(hinting, SKDEBUGCANVAS_HINTING_FULL)) {
            target->setHinting(SkPaint::kFull_Hinting);
        }
    }
}

static void extract_json_paint_style(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_STYLE)) {
        const char* style = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_STYLE].asCString();
        if (!strcmp(style, SKDEBUGCANVAS_STYLE_FILL)) {
            target->setStyle(SkPaint::kFill_Style);
        }
        else if (!strcmp(style, SKDEBUGCANVAS_STYLE_STROKE)) {
            target->setStyle(SkPaint::kStroke_Style);
        }
        else if (!strcmp(style, SKDEBUGCANVAS_STYLE_STROKEANDFILL)) {
            target->setStyle(SkPaint::kStrokeAndFill_Style);
        }
    }
}

static void extract_json_paint_strokewidth(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_STROKEWIDTH)) {
        float strokeWidth = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_STROKEWIDTH].asFloat();
        target->setStrokeWidth(strokeWidth);
    }
}

static void extract_json_paint_strokemiter(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_STROKEMITER)) {
        float strokeMiter = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_STROKEMITER].asFloat();
        target->setStrokeMiter(strokeMiter);
    }
}

static void extract_json_paint_strokejoin(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_STROKEJOIN)) {
        const char* join = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_STROKEJOIN].asCString();
        if (!strcmp(join, SKDEBUGCANVAS_MITER_JOIN)) {
            target->setStrokeJoin(SkPaint::kMiter_Join);
        }
        else if (!strcmp(join, SKDEBUGCANVAS_ROUND_JOIN)) {
            target->setStrokeJoin(SkPaint::kRound_Join);
        }
        else if (!strcmp(join, SKDEBUGCANVAS_BEVEL_JOIN)) {
            target->setStrokeJoin(SkPaint::kBevel_Join);
        }
        else {
            SkASSERT(false);
        }
    }
}

static void extract_json_paint_cap(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_CAP)) {
        const char* cap = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_CAP].asCString();
        if (!strcmp(cap, SKDEBUGCANVAS_CAP_BUTT)) {
            target->setStrokeCap(SkPaint::kButt_Cap);
        }
        else if (!strcmp(cap, SKDEBUGCANVAS_CAP_ROUND)) {
            target->setStrokeCap(SkPaint::kRound_Cap);
        }
        else if (!strcmp(cap, SKDEBUGCANVAS_CAP_SQUARE)) {
            target->setStrokeCap(SkPaint::kSquare_Cap);
        }
    }
}

static void extract_json_paint_filterquality(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_FILTERQUALITY)) {
        const char* quality = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_FILTERQUALITY].asCString();
        if (!strcmp(quality, SKDEBUGCANVAS_FILTERQUALITY_NONE)) {
            target->setFilterQuality(kNone_SkFilterQuality);
        }
        else if (!strcmp(quality, SKDEBUGCANVAS_FILTERQUALITY_LOW)) {
            target->setFilterQuality(kLow_SkFilterQuality);
        }
        else if (!strcmp(quality, SKDEBUGCANVAS_FILTERQUALITY_MEDIUM)) {
            target->setFilterQuality(kMedium_SkFilterQuality);
        }
        else if (!strcmp(quality, SKDEBUGCANVAS_FILTERQUALITY_HIGH)) {
            target->setFilterQuality(kHigh_SkFilterQuality);
        }
    }
}

static void extract_json_paint_antialias(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS)) {
        target->setAntiAlias(jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS].asBool());
    }
}

static void extract_json_paint_dither(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_DITHER)) {
        target->setDither(jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_DITHER].asBool());
    }
}

static void extract_json_paint_fakeboldtext(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_FAKEBOLDTEXT)) {
        target->setFakeBoldText(jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_FAKEBOLDTEXT].asBool());
    }
}

static void extract_json_paint_lineartext(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_LINEARTEXT)) {
        target->setLinearText(jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_LINEARTEXT].asBool());
    }
}

static void extract_json_paint_subpixeltext(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_SUBPIXELTEXT)) {
        target->setSubpixelText(jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_SUBPIXELTEXT].asBool());
    }
}

static void extract_json_paint_devkerntext(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_DEVKERNTEXT)) {
        target->setDevKernText(jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_DEVKERNTEXT].asBool());
    }
}

static void extract_json_paint_lcdrendertext(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_LCDRENDERTEXT)) {
        target->setLCDRenderText(jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_LCDRENDERTEXT].asBool());
    }
}

static void extract_json_paint_embeddedbitmaptext(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_EMBEDDEDBITMAPTEXT)) {
        target->setEmbeddedBitmapText(jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_EMBEDDEDBITMAPTEXT].asBool());
    }
}

static void extract_json_paint_autohinting(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_AUTOHINTING)) {
        target->setAutohinted(jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_AUTOHINTING].asBool());
    }
}

static void extract_json_paint_verticaltext(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_VERTICALTEXT)) {
        target->setVerticalText(jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_VERTICALTEXT].asBool());
    }
}

static void extract_json_paint_blur(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_BLUR)) {
        Json::Value blur = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_BLUR];
        SkScalar sigma = blur[SKDEBUGCANVAS_ATTRIBUTE_SIGMA].asFloat();
        SkBlurStyle style;
        const char* jsonStyle = blur[SKDEBUGCANVAS_ATTRIBUTE_STYLE].asCString();
        if (!strcmp(jsonStyle, SKDEBUGCANVAS_BLURSTYLE_NORMAL)) {
            style = SkBlurStyle::kNormal_SkBlurStyle;
        }
        else if (!strcmp(jsonStyle, SKDEBUGCANVAS_BLURSTYLE_SOLID)) {
            style = SkBlurStyle::kSolid_SkBlurStyle;
        }
        else if (!strcmp(jsonStyle, SKDEBUGCANVAS_BLURSTYLE_OUTER)) {
            style = SkBlurStyle::kOuter_SkBlurStyle;
        }
        else if (!strcmp(jsonStyle, SKDEBUGCANVAS_BLURSTYLE_INNER)) {
            style = SkBlurStyle::kInner_SkBlurStyle;
        }
        else {
            SkASSERT(false);
            style = SkBlurStyle::kNormal_SkBlurStyle;
        }
        SkBlurMaskFilter::BlurFlags flags;
        const char* jsonQuality = blur[SKDEBUGCANVAS_ATTRIBUTE_QUALITY].asCString();
        if (!strcmp(jsonQuality, SKDEBUGCANVAS_BLURQUALITY_LOW)) {
            flags = SkBlurMaskFilter::BlurFlags::kNone_BlurFlag;
        }
        else if (!strcmp(jsonQuality, SKDEBUGCANVAS_BLURQUALITY_HIGH)) {
            flags = SkBlurMaskFilter::BlurFlags::kHighQuality_BlurFlag;
        }
        else {
            SkASSERT(false);
            flags = SkBlurMaskFilter::BlurFlags::kNone_BlurFlag;
        }
        target->setMaskFilter(SkBlurMaskFilter::Make(style, sigma, flags));
    }
}

static void extract_json_paint_dashing(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_DASHING)) {
        Json::Value dash = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_DASHING];
        Json::Value jsonIntervals = dash[SKDEBUGCANVAS_ATTRIBUTE_INTERVALS];
        Json::ArrayIndex count = jsonIntervals.size();
        SkScalar* intervals = (SkScalar*) sk_malloc_throw(count * sizeof(SkScalar));
        for (Json::ArrayIndex i = 0; i < count; i++) {
            intervals[i] = jsonIntervals[i].asFloat();
        }
        SkScalar phase = dash[SKDEBUGCANVAS_ATTRIBUTE_PHASE].asFloat();
        target->setPathEffect(SkDashPathEffect::Make(intervals, count, phase));
        sk_free(intervals);
    }
}

static void extract_json_paint_textalign(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_TEXTALIGN)) {
        SkPaint::Align textAlign;
        const char* jsonAlign = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_TEXTALIGN].asCString();
        if (!strcmp(jsonAlign, SKDEBUGCANVAS_ALIGN_LEFT)) {
            textAlign = SkPaint::kLeft_Align;
        }
        else if (!strcmp(jsonAlign, SKDEBUGCANVAS_ALIGN_CENTER)) {
            textAlign = SkPaint::kCenter_Align;
        }
        else if (!strcmp(jsonAlign, SKDEBUGCANVAS_ALIGN_RIGHT)) {
            textAlign = SkPaint::kRight_Align;
        }
        else {
            SkASSERT(false);
            textAlign = SkPaint::kLeft_Align;
        }
        target->setTextAlign(textAlign);
    }
}

static void extract_json_paint_textsize(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_TEXTSIZE)) {
        float textSize = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_TEXTSIZE].asFloat();
        target->setTextSize(textSize);
    }
}

static void extract_json_paint_textscalex(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_TEXTSCALEX)) {
        float textScaleX = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_TEXTSCALEX].asFloat();
        target->setTextScaleX(textScaleX);
    }
}

static void extract_json_paint_textskewx(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_TEXTSKEWX)) {
        float textSkewX = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_TEXTSKEWX].asFloat();
        target->setTextSkewX(textSkewX);
    }
}

static void extract_json_paint(Json::Value& paint, UrlDataManager& urlDataManager,
                               SkPaint* result) {
    extract_json_paint_hinting(paint, result);
    extract_json_paint_color(paint, result);
    extract_json_paint_shader(paint, urlDataManager, result);
    extract_json_paint_patheffect(paint, urlDataManager, result);
    extract_json_paint_maskfilter(paint, urlDataManager, result);
    extract_json_paint_colorfilter(paint, urlDataManager, result);
    extract_json_paint_looper(paint, urlDataManager, result);
    extract_json_paint_imagefilter(paint, urlDataManager, result);
    extract_json_paint_typeface(paint, urlDataManager, result);
    extract_json_paint_style(paint, result);
    extract_json_paint_blend_mode(paint, result);
    extract_json_paint_strokewidth(paint, result);
    extract_json_paint_strokemiter(paint, result);
    extract_json_paint_strokejoin(paint, result);
    extract_json_paint_cap(paint, result);
    extract_json_paint_filterquality(paint, result);
    extract_json_paint_antialias(paint, result);
    extract_json_paint_dither(paint, result);
    extract_json_paint_fakeboldtext(paint, result);
    extract_json_paint_lineartext(paint, result);
    extract_json_paint_subpixeltext(paint, result);
    extract_json_paint_devkerntext(paint, result);
    extract_json_paint_lcdrendertext(paint, result);
    extract_json_paint_embeddedbitmaptext(paint, result);
    extract_json_paint_autohinting(paint, result);
    extract_json_paint_verticaltext(paint, result);
    extract_json_paint_blur(paint, result);
    extract_json_paint_dashing(paint, result);
    extract_json_paint_textalign(paint, result);
    extract_json_paint_textsize(paint, result);
    extract_json_paint_textscalex(paint, result);
    extract_json_paint_textskewx(paint, result);
}

static void extract_json_rect(Json::Value& rect, SkRect* result) {
    result->set(rect[0].asFloat(), rect[1].asFloat(), rect[2].asFloat(), rect[3].asFloat());
}

static void extract_json_irect(Json::Value& rect, SkIRect* result) {
    result->set(rect[0].asInt(), rect[1].asInt(), rect[2].asInt(), rect[3].asInt());
}

static void extract_json_rrect(Json::Value& rrect, SkRRect* result) {
    SkVector radii[4] = {
                            { rrect[1][0].asFloat(), rrect[1][1].asFloat() },
                            { rrect[2][0].asFloat(), rrect[2][1].asFloat() },
                            { rrect[3][0].asFloat(), rrect[3][1].asFloat() },
                            { rrect[4][0].asFloat(), rrect[4][1].asFloat() }
                        };
    result->setRectRadii(SkRect::MakeLTRB(rrect[0][0].asFloat(), rrect[0][1].asFloat(),
                                          rrect[0][2].asFloat(), rrect[0][3].asFloat()),
                                          radii);
}

static void extract_json_matrix(Json::Value& matrix, SkMatrix* result) {
    SkScalar values[] = {
        matrix[0][0].asFloat(), matrix[0][1].asFloat(), matrix[0][2].asFloat(),
        matrix[1][0].asFloat(), matrix[1][1].asFloat(), matrix[1][2].asFloat(),
        matrix[2][0].asFloat(), matrix[2][1].asFloat(), matrix[2][2].asFloat()
    };
    result->set9(values);
}

static void extract_json_path(Json::Value& path, SkPath* result) {
    const char* fillType = path[SKDEBUGCANVAS_ATTRIBUTE_FILLTYPE].asCString();
    if (!strcmp(fillType, SKDEBUGCANVAS_FILLTYPE_WINDING)) {
        result->setFillType(SkPath::kWinding_FillType);
    }
    else if (!strcmp(fillType, SKDEBUGCANVAS_FILLTYPE_EVENODD)) {
        result->setFillType(SkPath::kEvenOdd_FillType);
    }
    else if (!strcmp(fillType, SKDEBUGCANVAS_FILLTYPE_INVERSEWINDING)) {
        result->setFillType(SkPath::kInverseWinding_FillType);
    }
    else if (!strcmp(fillType, SKDEBUGCANVAS_FILLTYPE_INVERSEEVENODD)) {
        result->setFillType(SkPath::kInverseEvenOdd_FillType);
    }
    Json::Value verbs = path[SKDEBUGCANVAS_ATTRIBUTE_VERBS];
    for (Json::ArrayIndex i = 0; i < verbs.size(); i++) {
        Json::Value verb = verbs[i];
        if (verb.isString()) {
            SkASSERT(!strcmp(verb.asCString(), SKDEBUGCANVAS_VERB_CLOSE));
            result->close();
        }
        else {
            if (verb.isMember(SKDEBUGCANVAS_VERB_MOVE)) {
                Json::Value move = verb[SKDEBUGCANVAS_VERB_MOVE];
                result->moveTo(move[0].asFloat(), move[1].asFloat());
            }
            else if (verb.isMember(SKDEBUGCANVAS_VERB_LINE)) {
                Json::Value line = verb[SKDEBUGCANVAS_VERB_LINE];
                result->lineTo(line[0].asFloat(), line[1].asFloat());
            }
            else if (verb.isMember(SKDEBUGCANVAS_VERB_QUAD)) {
                Json::Value quad = verb[SKDEBUGCANVAS_VERB_QUAD];
                result->quadTo(quad[0][0].asFloat(), quad[0][1].asFloat(),
                               quad[1][0].asFloat(), quad[1][1].asFloat());
            }
            else if (verb.isMember(SKDEBUGCANVAS_VERB_CUBIC)) {
                Json::Value cubic = verb[SKDEBUGCANVAS_VERB_CUBIC];
                result->cubicTo(cubic[0][0].asFloat(), cubic[0][1].asFloat(),
                                cubic[1][0].asFloat(), cubic[1][1].asFloat(),
                                cubic[2][0].asFloat(), cubic[2][1].asFloat());
            }
            else if (verb.isMember(SKDEBUGCANVAS_VERB_CONIC)) {
                Json::Value conic = verb[SKDEBUGCANVAS_VERB_CONIC];
                result->conicTo(conic[0][0].asFloat(), conic[0][1].asFloat(),
                                conic[1][0].asFloat(), conic[1][1].asFloat(),
                                conic[2].asFloat());
            }
            else {
                SkASSERT(false);
            }
        }
    }
}

SkClipOp get_json_clipop(Json::Value& jsonOp) {
    const char* op = jsonOp.asCString();
    if (!strcmp(op, SKDEBUGCANVAS_REGIONOP_DIFFERENCE)) {
        return kDifference_SkClipOp;
    }
    else if (!strcmp(op, SKDEBUGCANVAS_REGIONOP_INTERSECT)) {
        return kIntersect_SkClipOp;
    }
    else if (!strcmp(op, SKDEBUGCANVAS_REGIONOP_UNION)) {
        return kUnion_SkClipOp;
    }
    else if (!strcmp(op, SKDEBUGCANVAS_REGIONOP_XOR)) {
        return kXOR_SkClipOp;
    }
    else if (!strcmp(op, SKDEBUGCANVAS_REGIONOP_REVERSE_DIFFERENCE)) {
        return kReverseDifference_SkClipOp;
    }
    else if (!strcmp(op, SKDEBUGCANVAS_REGIONOP_REPLACE)) {
        return kReplace_SkClipOp;
    }
    SkASSERT(false);
    return kIntersect_SkClipOp;
}

SkClearCommand::SkClearCommand(SkColor color) : INHERITED(kDrawClear_OpType) {
    fColor = color;
    fInfo.push(SkObjectParser::CustomTextToString("No Parameters"));
}

void SkClearCommand::execute(SkCanvas* canvas) const {
    canvas->clear(fColor);
}

Json::Value SkClearCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_COLOR] = MakeJsonColor(fColor);
    return result;
}

 SkClearCommand* SkClearCommand::fromJSON(Json::Value& command, UrlDataManager& urlDataManager) {
    Json::Value color = command[SKDEBUGCANVAS_ATTRIBUTE_COLOR];
    return new SkClearCommand(get_json_color(color));
}

SkClipPathCommand::SkClipPathCommand(const SkPath& path, SkClipOp op, bool doAA)
    : INHERITED(kClipPath_OpType) {
    fPath = path;
    fOp = op;
    fDoAA = doAA;

    fInfo.push(SkObjectParser::PathToString(path));
    fInfo.push(SkObjectParser::ClipOpToString(op));
    fInfo.push(SkObjectParser::BoolToString(doAA));
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

SkClipPathCommand* SkClipPathCommand::fromJSON(Json::Value& command,
                                               UrlDataManager& urlDataManager) {
    SkPath path;
    extract_json_path(command[SKDEBUGCANVAS_ATTRIBUTE_PATH], &path);
    return new SkClipPathCommand(path, get_json_clipop(command[SKDEBUGCANVAS_ATTRIBUTE_REGIONOP]),
                                 command[SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS].asBool());
}

SkClipRegionCommand::SkClipRegionCommand(const SkRegion& region, SkClipOp op)
    : INHERITED(kClipRegion_OpType) {
    fRegion = region;
    fOp = op;

    fInfo.push(SkObjectParser::RegionToString(region));
    fInfo.push(SkObjectParser::ClipOpToString(op));
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

SkClipRegionCommand* SkClipRegionCommand::fromJSON(Json::Value& command,
                                                   UrlDataManager& urlDataManager) {
    SkASSERT(false);
    return nullptr;
}

SkClipRectCommand::SkClipRectCommand(const SkRect& rect, SkClipOp op, bool doAA)
    : INHERITED(kClipRect_OpType) {
    fRect = rect;
    fOp = op;
    fDoAA = doAA;

    fInfo.push(SkObjectParser::RectToString(rect));
    fInfo.push(SkObjectParser::ClipOpToString(op));
    fInfo.push(SkObjectParser::BoolToString(doAA));
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

SkClipRectCommand* SkClipRectCommand::fromJSON(Json::Value& command,
                                               UrlDataManager& urlDataManager) {
    SkRect rect;
    extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_COORDS], &rect);
    return new SkClipRectCommand(rect, get_json_clipop(command[SKDEBUGCANVAS_ATTRIBUTE_REGIONOP]),
                                 command[SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS].asBool());
}

SkClipRRectCommand::SkClipRRectCommand(const SkRRect& rrect, SkClipOp op, bool doAA)
    : INHERITED(kClipRRect_OpType) {
    fRRect = rrect;
    fOp = op;
    fDoAA = doAA;

    fInfo.push(SkObjectParser::RRectToString(rrect));
    fInfo.push(SkObjectParser::ClipOpToString(op));
    fInfo.push(SkObjectParser::BoolToString(doAA));
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

SkClipRRectCommand* SkClipRRectCommand::fromJSON(Json::Value& command,
                                                 UrlDataManager& urlDataManager) {
    SkRRect rrect;
    extract_json_rrect(command[SKDEBUGCANVAS_ATTRIBUTE_COORDS], &rrect);
    return new SkClipRRectCommand(rrect,
                                  get_json_clipop(command[SKDEBUGCANVAS_ATTRIBUTE_REGIONOP]),
                                  command[SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS].asBool());
}

SkConcatCommand::SkConcatCommand(const SkMatrix& matrix)
    : INHERITED(kConcat_OpType) {
    fMatrix = matrix;

    fInfo.push(SkObjectParser::MatrixToString(matrix));
}

void SkConcatCommand::execute(SkCanvas* canvas) const {
    canvas->concat(fMatrix);
}

Json::Value SkConcatCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_MATRIX] = MakeJsonMatrix(fMatrix);
    return result;
}

SkConcatCommand* SkConcatCommand::fromJSON(Json::Value& command, UrlDataManager& urlDataManager) {
    SkMatrix matrix;
    extract_json_matrix(command[SKDEBUGCANVAS_ATTRIBUTE_MATRIX], &matrix);
    return new SkConcatCommand(matrix);
}

////

SkDrawAnnotationCommand::SkDrawAnnotationCommand(const SkRect& rect, const char key[],
                                                 sk_sp<SkData> value)
    : INHERITED(kDrawAnnotation_OpType)
    , fRect(rect)
    , fKey(key)
    , fValue(std::move(value))
{
    SkString str;
    str.appendf("Key: %s Value: ", key);
    if (fValue && fValue->size()) {
        str.append((const char*) fValue->bytes(), fValue->size());
    } else {
        str.appendf("no value");
    }
    str.appendf("\n");
    fInfo.push(new SkString(str));
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

SkDrawAnnotationCommand* SkDrawAnnotationCommand::fromJSON(Json::Value& command,
                                                           UrlDataManager& urlDataManager) {
    SkRect rect;
    extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_COORDS], &rect);
    sk_sp<SkData> data(nullptr); // TODO: extract "value" from the Json
    return new SkDrawAnnotationCommand(rect, command["key"].asCString(), data);
}

////

SkDrawBitmapCommand::SkDrawBitmapCommand(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                                         const SkPaint* paint)
    : INHERITED(kDrawBitmap_OpType) {
    fBitmap = bitmap;
    fLeft = left;
    fTop = top;
    if (paint) {
        fPaint = *paint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = nullptr;
    }

    fInfo.push(SkObjectParser::BitmapToString(bitmap));
    fInfo.push(SkObjectParser::ScalarToString(left, "SkScalar left: "));
    fInfo.push(SkObjectParser::ScalarToString(top, "SkScalar top: "));
    if (paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
}

void SkDrawBitmapCommand::execute(SkCanvas* canvas) const {
    canvas->drawBitmap(fBitmap, fLeft, fTop, fPaintPtr);
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
        if (fPaintPtr != nullptr) {
            result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(*fPaintPtr, urlDataManager);
        }
    }
    return result;
}

SkDrawBitmapCommand* SkDrawBitmapCommand::fromJSON(Json::Value& command,
                                                   UrlDataManager& urlDataManager) {
    SkBitmap* bitmap = load_bitmap(command[SKDEBUGCANVAS_ATTRIBUTE_BITMAP], urlDataManager);
    if (bitmap == nullptr) {
        return nullptr;
    }
    Json::Value point = command[SKDEBUGCANVAS_ATTRIBUTE_COORDS];
    SkPaint* paintPtr;
    SkPaint paint;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_PAINT)) {
        extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
        paintPtr = &paint;
    }
    else {
        paintPtr = nullptr;
    }
    SkDrawBitmapCommand* result = new SkDrawBitmapCommand(*bitmap, point[0].asFloat(),
                                                          point[1].asFloat(), paintPtr);
    delete bitmap;
    return result;
}

SkDrawBitmapNineCommand::SkDrawBitmapNineCommand(const SkBitmap& bitmap, const SkIRect& center,
                                                 const SkRect& dst, const SkPaint* paint)
    : INHERITED(kDrawBitmapNine_OpType) {
    fBitmap = bitmap;
    fCenter = center;
    fDst = dst;
    if (paint) {
        fPaint = *paint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = nullptr;
    }

    fInfo.push(SkObjectParser::BitmapToString(bitmap));
    fInfo.push(SkObjectParser::IRectToString(center));
    fInfo.push(SkObjectParser::RectToString(dst, "Dst: "));
    if (paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
}

void SkDrawBitmapNineCommand::execute(SkCanvas* canvas) const {
    canvas->drawBitmapNine(fBitmap, fCenter, fDst, fPaintPtr);
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
        if (fPaintPtr != nullptr) {
            result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(*fPaintPtr, urlDataManager);
        }
    }
    return result;
}

SkDrawBitmapNineCommand* SkDrawBitmapNineCommand::fromJSON(Json::Value& command,
                                                           UrlDataManager& urlDataManager) {
    SkBitmap* bitmap = load_bitmap(command[SKDEBUGCANVAS_ATTRIBUTE_BITMAP], urlDataManager);
    if (bitmap == nullptr) {
        return nullptr;
    }
    SkIRect center;
    extract_json_irect(command[SKDEBUGCANVAS_ATTRIBUTE_CENTER], &center);
    SkRect dst;
    extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_DST], &dst);
    SkPaint* paintPtr;
    SkPaint paint;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_PAINT)) {
        extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
        paintPtr = &paint;
    }
    else {
        paintPtr = nullptr;
    }
    SkDrawBitmapNineCommand* result = new SkDrawBitmapNineCommand(*bitmap, center, dst, paintPtr);
    delete bitmap;
    return result;
}

SkDrawBitmapRectCommand::SkDrawBitmapRectCommand(const SkBitmap& bitmap, const SkRect* src,
                                                 const SkRect& dst, const SkPaint* paint,
                                                 SkCanvas::SrcRectConstraint constraint)
    : INHERITED(kDrawBitmapRect_OpType) {
    fBitmap = bitmap;
    if (src) {
        fSrc = *src;
    } else {
        fSrc.setEmpty();
    }
    fDst = dst;

    if (paint) {
        fPaint = *paint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = nullptr;
    }
    fConstraint = constraint;

    fInfo.push(SkObjectParser::BitmapToString(bitmap));
    if (src) {
        fInfo.push(SkObjectParser::RectToString(*src, "Src: "));
    }
    fInfo.push(SkObjectParser::RectToString(dst, "Dst: "));
    if (paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
    fInfo.push(SkObjectParser::IntToString(fConstraint, "Constraint: "));
}

void SkDrawBitmapRectCommand::execute(SkCanvas* canvas) const {
    canvas->legacy_drawBitmapRect(fBitmap, this->srcRect(), fDst, fPaintPtr, fConstraint);
}

bool SkDrawBitmapRectCommand::render(SkCanvas* canvas) const {
    render_bitmap(canvas, fBitmap, this->srcRect());
    return true;
}

Json::Value SkDrawBitmapRectCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    Json::Value encoded;
    if (flatten(fBitmap, &encoded, urlDataManager)) {
        result[SKDEBUGCANVAS_ATTRIBUTE_BITMAP] = encoded;
        if (!fSrc.isEmpty()) {
            result[SKDEBUGCANVAS_ATTRIBUTE_SRC] = MakeJsonRect(fSrc);
        }
        result[SKDEBUGCANVAS_ATTRIBUTE_DST] = MakeJsonRect(fDst);
        if (fPaintPtr != nullptr) {
            result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(*fPaintPtr, urlDataManager);
        }
        if (fConstraint == SkCanvas::kStrict_SrcRectConstraint) {
            result[SKDEBUGCANVAS_ATTRIBUTE_STRICT] = Json::Value(true);
        }
    }

    SkString desc;
    result[SKDEBUGCANVAS_ATTRIBUTE_SHORTDESC] = Json::Value(str_append(&desc, fDst)->c_str());

    return result;
}

SkDrawBitmapRectCommand* SkDrawBitmapRectCommand::fromJSON(Json::Value& command,
                                                           UrlDataManager& urlDataManager) {
    SkBitmap* bitmap = load_bitmap(command[SKDEBUGCANVAS_ATTRIBUTE_BITMAP], urlDataManager);
    if (bitmap == nullptr) {
        return nullptr;
    }
    SkRect dst;
    extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_DST], &dst);
    SkPaint* paintPtr;
    SkPaint paint;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_PAINT)) {
        extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
        paintPtr = &paint;
    }
    else {
        paintPtr = nullptr;
    }
    SkCanvas::SrcRectConstraint constraint;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_STRICT) &&
        command[SKDEBUGCANVAS_ATTRIBUTE_STRICT].asBool()) {
        constraint = SkCanvas::kStrict_SrcRectConstraint;
    }
    else {
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }
    SkRect* srcPtr;
    SkRect src;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_SRC)) {
        extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_SRC], &src);
        srcPtr = &src;
    }
    else {
        srcPtr = nullptr;
    }
    SkDrawBitmapRectCommand* result = new SkDrawBitmapRectCommand(*bitmap, srcPtr, dst, paintPtr,
                                                                  constraint);
    delete bitmap;
    return result;
}

SkDrawImageCommand::SkDrawImageCommand(const SkImage* image, SkScalar left, SkScalar top,
                                       const SkPaint* paint)
    : INHERITED(kDrawImage_OpType)
    , fImage(SkRef(image))
    , fLeft(left)
    , fTop(top) {

    fInfo.push(SkObjectParser::ImageToString(image));
    fInfo.push(SkObjectParser::ScalarToString(left, "Left: "));
    fInfo.push(SkObjectParser::ScalarToString(top, "Top: "));

    if (paint) {
        fPaint.set(*paint);
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
}

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

SkDrawImageCommand* SkDrawImageCommand::fromJSON(Json::Value& command,
                                                 UrlDataManager& urlDataManager) {
    sk_sp<SkImage> image = load_image(command[SKDEBUGCANVAS_ATTRIBUTE_IMAGE], urlDataManager);
    if (image == nullptr) {
        return nullptr;
    }
    Json::Value point = command[SKDEBUGCANVAS_ATTRIBUTE_COORDS];
    SkPaint* paintPtr;
    SkPaint paint;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_PAINT)) {
        extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
        paintPtr = &paint;
    }
    else {
        paintPtr = nullptr;
    }
    SkDrawImageCommand* result = new SkDrawImageCommand(image.get(), point[0].asFloat(),
                                                        point[1].asFloat(), paintPtr);
    return result;
}

SkDrawImageLatticeCommand::SkDrawImageLatticeCommand(const SkImage* image,
                                                     const SkCanvas::Lattice& lattice,
                                                     const SkRect& dst, const SkPaint* paint)
    : INHERITED(kDrawImageLattice_OpType)
    , fImage(SkRef(image))
    , fLattice(lattice)
    , fDst(dst) {

      fInfo.push(SkObjectParser::ImageToString(image));
      fInfo.push(SkObjectParser::LatticeToString(lattice));
      fInfo.push(SkObjectParser::RectToString(dst, "Dst: "));
      if (paint) {
          fPaint.set(*paint);
          fInfo.push(SkObjectParser::PaintToString(*paint));
      }
}

void SkDrawImageLatticeCommand::execute(SkCanvas* canvas) const {
    SkLatticeIter iter(fLattice, fDst);
    SkRect srcR, dstR;
    while (iter.next(&srcR, &dstR)) {
        canvas->legacy_drawImageRect(fImage.get(), &srcR, dstR,
                                     fPaint.getMaybeNull(), SkCanvas::kStrict_SrcRectConstraint);
    }
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

SkDrawImageRectCommand::SkDrawImageRectCommand(const SkImage* image, const SkRect* src,
                                               const SkRect& dst, const SkPaint* paint,
                                               SkCanvas::SrcRectConstraint constraint)
    : INHERITED(kDrawImageRect_OpType)
    , fImage(SkRef(image))
    , fDst(dst)
    , fConstraint(constraint) {

    if (src) {
        fSrc.set(*src);
    }

    if (paint) {
        fPaint.set(*paint);
    }

    fInfo.push(SkObjectParser::ImageToString(image));
    if (src) {
        fInfo.push(SkObjectParser::RectToString(*src, "Src: "));
    }
    fInfo.push(SkObjectParser::RectToString(dst, "Dst: "));
    if (paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
    fInfo.push(SkObjectParser::IntToString(fConstraint, "Constraint: "));
}

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

SkDrawImageRectCommand* SkDrawImageRectCommand::fromJSON(Json::Value& command,
                                                         UrlDataManager& urlDataManager) {
    sk_sp<SkImage> image = load_image(command[SKDEBUGCANVAS_ATTRIBUTE_IMAGE], urlDataManager);
    if (image == nullptr) {
        return nullptr;
    }
    SkRect dst;
    extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_DST], &dst);
    SkPaint* paintPtr;
    SkPaint paint;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_PAINT)) {
        extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
        paintPtr = &paint;
    }
    else {
        paintPtr = nullptr;
    }
    SkCanvas::SrcRectConstraint constraint;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_STRICT) &&
        command[SKDEBUGCANVAS_ATTRIBUTE_STRICT].asBool()) {
        constraint = SkCanvas::kStrict_SrcRectConstraint;
    }
    else {
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }
    SkRect* srcPtr;
    SkRect src;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_SRC)) {
        extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_SRC], &src);
        srcPtr = &src;
    }
    else {
        srcPtr = nullptr;
    }
    SkDrawImageRectCommand* result = new SkDrawImageRectCommand(image.get(), srcPtr, dst, paintPtr,
                                                                constraint);
    return result;
}

SkDrawOvalCommand::SkDrawOvalCommand(const SkRect& oval, const SkPaint& paint)
    : INHERITED(kDrawOval_OpType) {
    fOval = oval;
    fPaint = paint;

    fInfo.push(SkObjectParser::RectToString(oval));
    fInfo.push(SkObjectParser::PaintToString(paint));
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

SkDrawOvalCommand* SkDrawOvalCommand::fromJSON(Json::Value& command,
                                               UrlDataManager& urlDataManager) {
    SkRect coords;
    extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_COORDS], &coords);
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
    return new SkDrawOvalCommand(coords, paint);
}

SkDrawArcCommand::SkDrawArcCommand(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                                   bool useCenter, const SkPaint& paint)
        : INHERITED(kDrawOval_OpType) {
    fOval = oval;
    fStartAngle = startAngle;
    fSweepAngle = sweepAngle;
    fUseCenter = useCenter;
    fPaint = paint;

    fInfo.push(SkObjectParser::RectToString(oval));
    fInfo.push(SkObjectParser::ScalarToString(startAngle, "StartAngle: "));
    fInfo.push(SkObjectParser::ScalarToString(sweepAngle, "SweepAngle: "));
    fInfo.push(SkObjectParser::BoolToString(useCenter));
    fInfo.push(SkObjectParser::PaintToString(paint));
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

SkDrawArcCommand* SkDrawArcCommand::fromJSON(Json::Value& command,
                                             UrlDataManager& urlDataManager) {
    SkRect coords;
    extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_COORDS], &coords);
    SkScalar startAngle = command[SKDEBUGCANVAS_ATTRIBUTE_STARTANGLE].asFloat();
    SkScalar sweepAngle = command[SKDEBUGCANVAS_ATTRIBUTE_SWEEPANGLE].asFloat();
    bool useCenter = command[SKDEBUGCANVAS_ATTRIBUTE_USECENTER].asBool();
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
    return new SkDrawArcCommand(coords, startAngle, sweepAngle, useCenter, paint);
}

SkDrawPaintCommand::SkDrawPaintCommand(const SkPaint& paint)
    : INHERITED(kDrawPaint_OpType) {
    fPaint = paint;

    fInfo.push(SkObjectParser::PaintToString(paint));
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

SkDrawPaintCommand* SkDrawPaintCommand::fromJSON(Json::Value& command,
                                                 UrlDataManager& urlDataManager) {
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
    return new SkDrawPaintCommand(paint);
}

SkDrawPathCommand::SkDrawPathCommand(const SkPath& path, const SkPaint& paint)
    : INHERITED(kDrawPath_OpType) {
    fPath = path;
    fPaint = paint;

    fInfo.push(SkObjectParser::PathToString(path));
    fInfo.push(SkObjectParser::PaintToString(paint));
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

SkDrawPathCommand* SkDrawPathCommand::fromJSON(Json::Value& command,
                                               UrlDataManager& urlDataManager) {
    SkPath path;
    extract_json_path(command[SKDEBUGCANVAS_ATTRIBUTE_PATH], &path);
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
    return new SkDrawPathCommand(path, paint);
}

SkBeginDrawPictureCommand::SkBeginDrawPictureCommand(const SkPicture* picture,
                                                     const SkMatrix* matrix,
                                                     const SkPaint* paint)
    : INHERITED(kBeginDrawPicture_OpType)
    , fPicture(SkRef(picture)) {

    SkString* str = new SkString;
    str->appendf("SkPicture: L: %f T: %f R: %f B: %f",
                 picture->cullRect().fLeft, picture->cullRect().fTop,
                 picture->cullRect().fRight, picture->cullRect().fBottom);
    fInfo.push(str);

    if (matrix) {
        fMatrix.set(*matrix);
        fInfo.push(SkObjectParser::MatrixToString(*matrix));
    }

    if (paint) {
        fPaint.set(*paint);
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }

}

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
    : INHERITED(kDrawPoints_OpType) {
    fMode = mode;
    fCount = count;
    fPts = new SkPoint[count];
    memcpy(fPts, pts, count * sizeof(SkPoint));
    fPaint = paint;

    fInfo.push(SkObjectParser::PointsToString(pts, count));
    fInfo.push(SkObjectParser::ScalarToString(SkIntToScalar((unsigned int)count),
                                              "Points: "));
    fInfo.push(SkObjectParser::PointModeToString(mode));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawPointsCommand::execute(SkCanvas* canvas) const {
    canvas->drawPoints(fMode, fCount, fPts, fPaint);
}

bool SkDrawPointsCommand::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    SkRect bounds;

    bounds.setEmpty();
    for (unsigned int i = 0; i < fCount; ++i) {
        bounds.growToInclude(fPts[i].fX, fPts[i].fY);
    }

    xlate_and_scale_to_bounds(canvas, bounds);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawPoints(fMode, fCount, fPts, p);
    canvas->restore();

    return true;
}

Json::Value SkDrawPointsCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_MODE] = make_json_pointmode(fMode);
    Json::Value points(Json::arrayValue);
    for (size_t i = 0; i < fCount; i++) {
        points.append(MakeJsonPoint(fPts[i]));
    }
    result[SKDEBUGCANVAS_ATTRIBUTE_POINTS] = points;
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(fPaint, urlDataManager);
    return result;
}

SkDrawPointsCommand* SkDrawPointsCommand::fromJSON(Json::Value& command,
                                                   UrlDataManager& urlDataManager) {
    SkCanvas::PointMode mode;
    const char* jsonMode = command[SKDEBUGCANVAS_ATTRIBUTE_MODE].asCString();
    if (!strcmp(jsonMode, SKDEBUGCANVAS_POINTMODE_POINTS)) {
        mode = SkCanvas::kPoints_PointMode;
    }
    else if (!strcmp(jsonMode, SKDEBUGCANVAS_POINTMODE_LINES)) {
        mode = SkCanvas::kLines_PointMode;
    }
    else if (!strcmp(jsonMode, SKDEBUGCANVAS_POINTMODE_POLYGON)) {
        mode = SkCanvas::kPolygon_PointMode;
    }
    else {
        SkASSERT(false);
        return nullptr;
    }
    Json::Value jsonPoints = command[SKDEBUGCANVAS_ATTRIBUTE_POINTS];
    int count = (int) jsonPoints.size();
    SkPoint* points = (SkPoint*) sk_malloc_throw(count * sizeof(SkPoint));
    for (int i = 0; i < count; i++) {
        points[i] = SkPoint::Make(jsonPoints[i][0].asFloat(), jsonPoints[i][1].asFloat());
    }
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
    SkDrawPointsCommand* result = new SkDrawPointsCommand(mode, count, points, paint);
    sk_free(points);
    return result;
}

SkDrawPosTextCommand::SkDrawPosTextCommand(const void* text, size_t byteLength,
                                           const SkPoint pos[], const SkPaint& paint)
    : INHERITED(kDrawPosText_OpType) {
    size_t numPts = paint.countText(text, byteLength);

    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;

    fPos = new SkPoint[numPts];
    memcpy(fPos, pos, numPts * sizeof(SkPoint));

    fPaint = paint;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    // TODO(chudy): Test that this works.
    fInfo.push(SkObjectParser::PointsToString(pos, 1));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawPosTextCommand::execute(SkCanvas* canvas) const {
    canvas->drawPosText(fText, fByteLength, fPos, fPaint);
}

Json::Value SkDrawPosTextCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_TEXT] = Json::Value((const char*) fText,
                                                       ((const char*) fText) + fByteLength);
    Json::Value coords(Json::arrayValue);
    size_t numCoords = fPaint.textToGlyphs(fText, fByteLength, nullptr);
    for (size_t i = 0; i < numCoords; i++) {
        coords.append(MakeJsonPoint(fPos[i]));
    }
    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = coords;
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(fPaint, urlDataManager);
    return result;
}

SkDrawPosTextCommand* SkDrawPosTextCommand::fromJSON(Json::Value& command,
                                                     UrlDataManager& urlDataManager) {
    const char* text = command[SKDEBUGCANVAS_ATTRIBUTE_TEXT].asCString();
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
    Json::Value coords = command[SKDEBUGCANVAS_ATTRIBUTE_COORDS];
    int count = (int) coords.size();
    SkPoint* points = (SkPoint*) sk_malloc_throw(count * sizeof(SkPoint));
    for (int i = 0; i < count; i++) {
        points[i] = SkPoint::Make(coords[i][0].asFloat(), coords[i][1].asFloat());
    }
    return new SkDrawPosTextCommand(text, strlen(text), points, paint);
}

SkDrawPosTextHCommand::SkDrawPosTextHCommand(const void* text, size_t byteLength,
                                             const SkScalar xpos[], SkScalar constY,
                                             const SkPaint& paint)
    : INHERITED(kDrawPosTextH_OpType) {
    size_t numPts = paint.countText(text, byteLength);

    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;

    fXpos = new SkScalar[numPts];
    memcpy(fXpos, xpos, numPts * sizeof(SkScalar));

    fConstY = constY;
    fPaint = paint;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    fInfo.push(SkObjectParser::ScalarToString(xpos[0], "XPOS: "));
    fInfo.push(SkObjectParser::ScalarToString(constY, "SkScalar constY: "));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawPosTextHCommand::execute(SkCanvas* canvas) const {
    canvas->drawPosTextH(fText, fByteLength, fXpos, fConstY, fPaint);
}

Json::Value SkDrawPosTextHCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_TEXT] = Json::Value((const char*) fText,
                                                       ((const char*) fText) + fByteLength);
    result[SKDEBUGCANVAS_ATTRIBUTE_Y] = Json::Value(fConstY);
    Json::Value xpos(Json::arrayValue);
    size_t numXpos = fPaint.textToGlyphs(fText, fByteLength, nullptr);
    for (size_t i = 0; i < numXpos; i++) {
        xpos.append(Json::Value(fXpos[i]));
    }
    result[SKDEBUGCANVAS_ATTRIBUTE_POSITIONS] = xpos;
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(fPaint, urlDataManager);
    return result;
}

SkDrawPosTextHCommand* SkDrawPosTextHCommand::fromJSON(Json::Value& command,
                                                       UrlDataManager& urlDataManager) {
    const char* text = command[SKDEBUGCANVAS_ATTRIBUTE_TEXT].asCString();
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
    Json::Value jsonXpos = command[SKDEBUGCANVAS_ATTRIBUTE_POSITIONS];
    int count = (int) jsonXpos.size();
    SkScalar* xpos = (SkScalar*) sk_malloc_throw(count * sizeof(SkScalar));
    for (int i = 0; i < count; i++) {
        xpos[i] = jsonXpos[i].asFloat();
    }
    SkScalar y = command[SKDEBUGCANVAS_ATTRIBUTE_Y].asFloat();
    return new SkDrawPosTextHCommand(text, strlen(text), xpos, y, paint);
}

static const char* gPositioningLabels[] = {
    "kDefault_Positioning",
    "kHorizontal_Positioning",
    "kFull_Positioning",
};

SkDrawTextBlobCommand::SkDrawTextBlobCommand(sk_sp<SkTextBlob> blob, SkScalar x, SkScalar y,
                                             const SkPaint& paint)
    : INHERITED(kDrawTextBlob_OpType)
    , fBlob(std::move(blob))
    , fXPos(x)
    , fYPos(y)
    , fPaint(paint) {

    std::unique_ptr<SkString> runsStr(new SkString);
    fInfo.push(SkObjectParser::ScalarToString(x, "XPOS: "));
    fInfo.push(SkObjectParser::ScalarToString(y, "YPOS: "));
    fInfo.push(SkObjectParser::RectToString(fBlob->bounds(), "Bounds: "));
    fInfo.push(runsStr.get());
    fInfo.push(SkObjectParser::PaintToString(paint));

    unsigned runs = 0;
    SkPaint runPaint(paint);
    SkTextBlobRunIterator iter(fBlob.get());
    while (!iter.done()) {
        std::unique_ptr<SkString> tmpStr(new SkString);
        tmpStr->printf("==== Run [%d] ====", runs++);
        fInfo.push(tmpStr.release());

        fInfo.push(SkObjectParser::IntToString(iter.glyphCount(), "GlyphCount: "));
        tmpStr.reset(new SkString("GlyphPositioning: "));
        tmpStr->append(gPositioningLabels[iter.positioning()]);
        fInfo.push(tmpStr.release());

        iter.applyFontToPaint(&runPaint);
        fInfo.push(SkObjectParser::PaintToString(runPaint));

        iter.next();
    }

    runsStr->printf("Runs: %d", runs);
    // runStr is owned by fInfo at this point.
    runsStr.release();
}

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
                case SkTextBlob::kFull_Positioning:
                    jsonPositions.append(MakeJsonPoint(iterPositions[i * 2],
                                                       iterPositions[i * 2 + 1]));
                    break;
                case SkTextBlob::kHorizontal_Positioning:
                    jsonPositions.append(Json::Value(iterPositions[i]));
                    break;
                case SkTextBlob::kDefault_Positioning:
                    break;
            }
            jsonGlyphs.append(Json::Value(iterGlyphs[i]));
        }
        if (iter.positioning() != SkTextBlob::kDefault_Positioning) {
            run[SKDEBUGCANVAS_ATTRIBUTE_POSITIONS] = jsonPositions;
        }
        run[SKDEBUGCANVAS_ATTRIBUTE_GLYPHS] = jsonGlyphs;
        SkPaint fontPaint;
        iter.applyFontToPaint(&fontPaint);
        run[SKDEBUGCANVAS_ATTRIBUTE_FONT] = MakeJsonPaint(fontPaint, urlDataManager);
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

SkDrawTextBlobCommand* SkDrawTextBlobCommand::fromJSON(Json::Value& command,
                                                       UrlDataManager& urlDataManager) {
    SkTextBlobBuilder builder;
    Json::Value runs = command[SKDEBUGCANVAS_ATTRIBUTE_RUNS];
    for (Json::ArrayIndex i = 0 ; i < runs.size(); i++) {
        Json::Value run = runs[i];
        SkPaint font;
        font.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        extract_json_paint(run[SKDEBUGCANVAS_ATTRIBUTE_FONT], urlDataManager, &font);
        Json::Value glyphs = run[SKDEBUGCANVAS_ATTRIBUTE_GLYPHS];
        int count = glyphs.size();
        Json::Value coords = run[SKDEBUGCANVAS_ATTRIBUTE_COORDS];
        SkScalar x = coords[0].asFloat();
        SkScalar y = coords[1].asFloat();
        SkRect bounds;
        extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_COORDS], &bounds);

        if (run.isMember(SKDEBUGCANVAS_ATTRIBUTE_POSITIONS)) {
            Json::Value positions = run[SKDEBUGCANVAS_ATTRIBUTE_POSITIONS];
            if (positions.size() > 0 && positions[0].isNumeric()) {
                SkTextBlobBuilder::RunBuffer buffer = builder.allocRunPosH(font, count, y, &bounds);
                for (int j = 0; j < count; j++) {
                    buffer.glyphs[j] = glyphs[j].asUInt();
                    buffer.pos[j] = positions[j].asFloat();
                }
            }
            else {
                SkTextBlobBuilder::RunBuffer buffer = builder.allocRunPos(font, count, &bounds);
                for (int j = 0; j < count; j++) {
                    buffer.glyphs[j] = glyphs[j].asUInt();
                    buffer.pos[j * 2] = positions[j][0].asFloat();
                    buffer.pos[j * 2 + 1] = positions[j][1].asFloat();
                }
            }
        }
        else {
            SkTextBlobBuilder::RunBuffer buffer = builder.allocRun(font, count, x, y, &bounds);
            for (int j = 0; j < count; j++) {
                buffer.glyphs[j] = glyphs[j].asUInt();
            }
        }
    }
    SkScalar x = command[SKDEBUGCANVAS_ATTRIBUTE_X].asFloat();
    SkScalar y = command[SKDEBUGCANVAS_ATTRIBUTE_Y].asFloat();
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
    return new SkDrawTextBlobCommand(builder.make(), x, y, paint);
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

    fInfo.push(SkObjectParser::PaintToString(paint));
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

SkDrawPatchCommand* SkDrawPatchCommand::fromJSON(Json::Value& command,
                                                 UrlDataManager& urlDataManager) {
    Json::Value jsonCubics = command[SKDEBUGCANVAS_ATTRIBUTE_CUBICS];
    SkPoint cubics[12];
    for (int i = 0; i < 12; i++) {
        cubics[i] = get_json_point(jsonCubics[i]);
    }
    SkColor* colorsPtr;
    SkColor colors[4];
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_COLORS)) {
        Json::Value jsonColors = command[SKDEBUGCANVAS_ATTRIBUTE_COLORS];
        for (int i = 0; i < 4; i++) {
            colors[i] = get_json_color(jsonColors[i]);
        }
        colorsPtr = colors;
    }
    else {
        colorsPtr = nullptr;
    }
    SkPoint* texCoordsPtr;
    SkPoint texCoords[4];
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_TEXTURECOORDS)) {
        Json::Value jsonTexCoords = command[SKDEBUGCANVAS_ATTRIBUTE_TEXTURECOORDS];
        for (int i = 0; i < 4; i++) {
            texCoords[i] = get_json_point(jsonTexCoords[i]);
        }
        texCoordsPtr = texCoords;
    }
    else {
        texCoordsPtr = nullptr;
    }

    SkBlendMode bmode = SkBlendMode::kSrcOver; // TODO: extract from json

    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
    return new SkDrawPatchCommand(cubics, colorsPtr, texCoordsPtr, bmode, paint);
}

SkDrawRectCommand::SkDrawRectCommand(const SkRect& rect, const SkPaint& paint)
    : INHERITED(kDrawRect_OpType) {
    fRect = rect;
    fPaint = paint;

    fInfo.push(SkObjectParser::RectToString(rect));
    fInfo.push(SkObjectParser::PaintToString(paint));
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

SkDrawRectCommand* SkDrawRectCommand::fromJSON(Json::Value& command,
                                               UrlDataManager& urlDataManager) {
    SkRect coords;
    extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_COORDS], &coords);
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
    return new SkDrawRectCommand(coords, paint);
}

SkDrawRRectCommand::SkDrawRRectCommand(const SkRRect& rrect, const SkPaint& paint)
    : INHERITED(kDrawRRect_OpType) {
    fRRect = rrect;
    fPaint = paint;

    fInfo.push(SkObjectParser::RRectToString(rrect));
    fInfo.push(SkObjectParser::PaintToString(paint));
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

SkDrawRRectCommand* SkDrawRRectCommand::fromJSON(Json::Value& command,
                                                 UrlDataManager& urlDataManager) {
    SkRRect coords;
    extract_json_rrect(command[SKDEBUGCANVAS_ATTRIBUTE_COORDS], &coords);
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
    return new SkDrawRRectCommand(coords, paint);
}

SkDrawDRRectCommand::SkDrawDRRectCommand(const SkRRect& outer,
                                         const SkRRect& inner,
                                         const SkPaint& paint)
    : INHERITED(kDrawDRRect_OpType) {
    fOuter = outer;
    fInner = inner;
    fPaint = paint;

    fInfo.push(SkObjectParser::RRectToString(outer));
    fInfo.push(SkObjectParser::RRectToString(inner));
    fInfo.push(SkObjectParser::PaintToString(paint));
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

SkDrawDRRectCommand* SkDrawDRRectCommand::fromJSON(Json::Value& command,
                                                   UrlDataManager& urlDataManager) {
    SkRRect outer;
    extract_json_rrect(command[SKDEBUGCANVAS_ATTRIBUTE_INNER], &outer);
    SkRRect inner;
    extract_json_rrect(command[SKDEBUGCANVAS_ATTRIBUTE_INNER], &inner);
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
    return new SkDrawDRRectCommand(outer, inner, paint);
}

SkDrawTextCommand::SkDrawTextCommand(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                                     const SkPaint& paint)
    : INHERITED(kDrawText_OpType) {
    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;
    fX = x;
    fY = y;
    fPaint = paint;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    fInfo.push(SkObjectParser::ScalarToString(x, "SkScalar x: "));
    fInfo.push(SkObjectParser::ScalarToString(y, "SkScalar y: "));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawTextCommand::execute(SkCanvas* canvas) const {
    canvas->drawText(fText, fByteLength, fX, fY, fPaint);
}

Json::Value SkDrawTextCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_TEXT] = Json::Value((const char*) fText,
                                                       ((const char*) fText) + fByteLength);
    Json::Value coords(Json::arrayValue);
    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = MakeJsonPoint(fX, fY);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(fPaint, urlDataManager);
    return result;
}

SkDrawTextCommand* SkDrawTextCommand::fromJSON(Json::Value& command,
                                               UrlDataManager& urlDataManager) {
    const char* text = command[SKDEBUGCANVAS_ATTRIBUTE_TEXT].asCString();
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
    Json::Value coords = command[SKDEBUGCANVAS_ATTRIBUTE_COORDS];
    return new SkDrawTextCommand(text, strlen(text), coords[0].asFloat(), coords[1].asFloat(),
                                 paint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkDrawTextOnPathCommand::SkDrawTextOnPathCommand(const void* text, size_t byteLength,
                                                 const SkPath& path, const SkMatrix* matrix,
                                                 const SkPaint& paint)
    : INHERITED(kDrawTextOnPath_OpType) {
    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;
    fPath = path;
    if (matrix) {
        fMatrix = *matrix;
    } else {
        fMatrix.setIdentity();
    }
    fPaint = paint;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    fInfo.push(SkObjectParser::PathToString(path));
    if (matrix) {
        fInfo.push(SkObjectParser::MatrixToString(*matrix));
    }
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawTextOnPathCommand::execute(SkCanvas* canvas) const {
    canvas->drawTextOnPath(fText, fByteLength, fPath,
                           fMatrix.isIdentity() ? nullptr : &fMatrix,
                           fPaint);
}

Json::Value SkDrawTextOnPathCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_TEXT] = Json::Value((const char*) fText,
                                                       ((const char*) fText) + fByteLength);
    Json::Value coords(Json::arrayValue);
    result[SKDEBUGCANVAS_ATTRIBUTE_PATH] = MakeJsonPath(fPath);
    if (!fMatrix.isIdentity()) {
        result[SKDEBUGCANVAS_ATTRIBUTE_MATRIX] = MakeJsonMatrix(fMatrix);
    }
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(fPaint, urlDataManager);
    return result;
}

SkDrawTextOnPathCommand* SkDrawTextOnPathCommand::fromJSON(Json::Value& command,
                                                           UrlDataManager& urlDataManager) {
    const char* text = command[SKDEBUGCANVAS_ATTRIBUTE_TEXT].asCString();
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
    SkPath path;
    extract_json_path(command[SKDEBUGCANVAS_ATTRIBUTE_PATH], &path);
    SkMatrix* matrixPtr;
    SkMatrix matrix;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_MATRIX)) {
        extract_json_matrix(command[SKDEBUGCANVAS_ATTRIBUTE_MATRIX], &matrix);
        matrixPtr = &matrix;
    }
    else {
        matrixPtr = nullptr;
    }
    return new SkDrawTextOnPathCommand(text, strlen(text), path, matrixPtr, paint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkDrawTextRSXformCommand::SkDrawTextRSXformCommand(const void* text, size_t byteLength,
                                                   const SkRSXform xform[], const SkRect* cull,
                                                   const SkPaint& paint)
    : INHERITED(kDrawTextRSXform_OpType)
{
    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;
    int count = paint.countText(text, byteLength);
    fXform = new SkRSXform[count];
    memcpy(fXform, xform, count * sizeof(SkRSXform));
    if (cull) {
        fCullStorage = *cull;
        fCull = &fCullStorage;
    } else {
        fCull = nullptr;
    }
    fPaint = paint;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawTextRSXformCommand::execute(SkCanvas* canvas) const {
    canvas->drawTextRSXform(fText, fByteLength, fXform, fCull, fPaint);
}

Json::Value SkDrawTextRSXformCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_TEXT] = Json::Value((const char*) fText,
                                                       ((const char*) fText) + fByteLength);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(fPaint, urlDataManager);
    return result;
}

SkDrawTextRSXformCommand* SkDrawTextRSXformCommand::fromJSON(Json::Value& command,
                                                             UrlDataManager& urlDataManager) {
    const char* text = command[SKDEBUGCANVAS_ATTRIBUTE_TEXT].asCString();
    size_t byteLength = strlen(text);
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);

    // TODO: handle xform and cull
    int count = paint.countText(text, byteLength);
    SkAutoTArray<SkRSXform> xform(count);
    for (int i = 0; i < count; ++i) {
        xform[i].fSCos = 1;
        xform[i].fSSin = xform[i].fTx = xform[i].fTy = 0;
    }
    return new SkDrawTextRSXformCommand(text, byteLength, &xform[0], nullptr, paint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkDrawVerticesCommand::SkDrawVerticesCommand(sk_sp<SkVertices> vertices, SkBlendMode bmode,
                                             const SkPaint& paint)
    : INHERITED(kDrawVertices_OpType)
    , fVertices(std::move(vertices))
    , fBlendMode(bmode)
    , fPaint(paint)
{
    // TODO(chudy)
    fInfo.push(SkObjectParser::CustomTextToString("To be implemented."));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawVerticesCommand::execute(SkCanvas* canvas) const {
    canvas->drawVertices(fVertices, fBlendMode, fPaint);
}

SkRestoreCommand::SkRestoreCommand()
    : INHERITED(kRestore_OpType) {
    fInfo.push(SkObjectParser::CustomTextToString("No Parameters"));
}

void SkRestoreCommand::execute(SkCanvas* canvas) const {
    canvas->restore();
}

SkRestoreCommand* SkRestoreCommand::fromJSON(Json::Value& command, UrlDataManager& urlDataManager) {
    return new SkRestoreCommand();
}

SkSaveCommand::SkSaveCommand()
    : INHERITED(kSave_OpType) {
}

void SkSaveCommand::execute(SkCanvas* canvas) const {
    canvas->save();
}

SkSaveCommand* SkSaveCommand::fromJSON(Json::Value& command, UrlDataManager& urlDataManager) {
    return new SkSaveCommand();
}

SkSaveLayerCommand::SkSaveLayerCommand(const SkCanvas::SaveLayerRec& rec)
    : INHERITED(kSaveLayer_OpType) {
    if (rec.fBounds) {
        fBounds = *rec.fBounds;
    } else {
        fBounds.setEmpty();
    }

    if (rec.fPaint) {
        fPaint = *rec.fPaint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = nullptr;
    }
    fSaveLayerFlags = rec.fSaveLayerFlags;

    if (rec.fBackdrop) {
        fBackdrop = rec.fBackdrop;
        fBackdrop->ref();
    } else {
        fBackdrop = nullptr;
    }

    if (rec.fBounds) {
        fInfo.push(SkObjectParser::RectToString(*rec.fBounds, "Bounds: "));
    }
    if (rec.fPaint) {
        fInfo.push(SkObjectParser::PaintToString(*rec.fPaint));
    }
    fInfo.push(SkObjectParser::SaveLayerFlagsToString(fSaveLayerFlags));
}

SkSaveLayerCommand::~SkSaveLayerCommand() {
    if (fBackdrop != nullptr) {
        fBackdrop->unref();
    }
}

void SkSaveLayerCommand::execute(SkCanvas* canvas) const {
    canvas->saveLayer(SkCanvas::SaveLayerRec(fBounds.isEmpty() ? nullptr : &fBounds,
                                             fPaintPtr,
                                             fSaveLayerFlags));
}

void SkSaveLayerCommand::vizExecute(SkCanvas* canvas) const {
    canvas->save();
}

Json::Value SkSaveLayerCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    if (!fBounds.isEmpty()) {
        result[SKDEBUGCANVAS_ATTRIBUTE_BOUNDS] = MakeJsonRect(fBounds);
    }
    if (fPaintPtr != nullptr) {
        result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = MakeJsonPaint(*fPaintPtr,
                                                                urlDataManager);
    }
    if (fBackdrop != nullptr) {
        Json::Value jsonBackdrop;
        flatten(fBackdrop, &jsonBackdrop, urlDataManager);
        result[SKDEBUGCANVAS_ATTRIBUTE_BACKDROP] = jsonBackdrop;
    }
    if (fSaveLayerFlags != 0) {
        SkDebugf("unsupported: saveLayer flags\n");
        SkASSERT(false);
    }
    return result;
}

SkSaveLayerCommand* SkSaveLayerCommand::fromJSON(Json::Value& command,
                                                 UrlDataManager& urlDataManager) {
    SkCanvas::SaveLayerRec rec;
    SkRect bounds;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_BOUNDS)) {
        extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_BOUNDS], &bounds);
        rec.fBounds = &bounds;
    }
    SkPaint paint;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_PAINT)) {
        extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], urlDataManager, &paint);
        rec.fPaint = &paint;
    }
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_BACKDROP)) {
        Json::Value backdrop = command[SKDEBUGCANVAS_ATTRIBUTE_BACKDROP];
        rec.fBackdrop = (SkImageFilter*) load_flattenable(backdrop, urlDataManager);
    }
    SkSaveLayerCommand* result = new SkSaveLayerCommand(rec);
    if (rec.fBackdrop != nullptr) {
        rec.fBackdrop->unref();
    }
    return result;
}

SkSetMatrixCommand::SkSetMatrixCommand(const SkMatrix& matrix)
    : INHERITED(kSetMatrix_OpType) {
    fUserMatrix.reset();
    fMatrix = matrix;
    fInfo.push(SkObjectParser::MatrixToString(matrix));
}

void SkSetMatrixCommand::setUserMatrix(const SkMatrix& userMatrix) {
    fUserMatrix = userMatrix;
}

void SkSetMatrixCommand::execute(SkCanvas* canvas) const {
    SkMatrix temp = SkMatrix::Concat(fUserMatrix, fMatrix);
    canvas->setMatrix(temp);
}

Json::Value SkSetMatrixCommand::toJSON(UrlDataManager& urlDataManager) const {
    Json::Value result = INHERITED::toJSON(urlDataManager);
    result[SKDEBUGCANVAS_ATTRIBUTE_MATRIX] = MakeJsonMatrix(fMatrix);
    return result;
}

SkSetMatrixCommand* SkSetMatrixCommand::fromJSON(Json::Value& command,
                                                 UrlDataManager& urlDataManager) {
    SkMatrix matrix;
    extract_json_matrix(command[SKDEBUGCANVAS_ATTRIBUTE_MATRIX], &matrix);
    return new SkSetMatrixCommand(matrix);
}
