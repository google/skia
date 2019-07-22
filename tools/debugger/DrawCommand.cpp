/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/debugger/DrawCommand.h"

#include <algorithm>
#include "include/core/SkColorFilter.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPicture.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/encode/SkPngEncoder.h"
#include "include/private/SkShadowFlags.h"
#include "include/private/SkTHash.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkClipOpPriv.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkPaintDefaults.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "tools/debugger/JsonWriteBuffer.h"

#define DEBUGCANVAS_ATTRIBUTE_COMMAND "command"
#define DEBUGCANVAS_ATTRIBUTE_VISIBLE "visible"
#define DEBUGCANVAS_ATTRIBUTE_MATRIX "matrix"
#define DEBUGCANVAS_ATTRIBUTE_DRAWDEPTHTRANS "drawDepthTranslation"
#define DEBUGCANVAS_ATTRIBUTE_COORDS "coords"
#define DEBUGCANVAS_ATTRIBUTE_EDGING "edging"
#define DEBUGCANVAS_ATTRIBUTE_HINTING "hinting"
#define DEBUGCANVAS_ATTRIBUTE_BOUNDS "bounds"
#define DEBUGCANVAS_ATTRIBUTE_PAINT "paint"
#define DEBUGCANVAS_ATTRIBUTE_OUTER "outer"
#define DEBUGCANVAS_ATTRIBUTE_INNER "inner"
#define DEBUGCANVAS_ATTRIBUTE_MODE "mode"
#define DEBUGCANVAS_ATTRIBUTE_POINTS "points"
#define DEBUGCANVAS_ATTRIBUTE_PATH "path"
#define DEBUGCANVAS_ATTRIBUTE_TEXT "text"
#define DEBUGCANVAS_ATTRIBUTE_COLOR "color"
#define DEBUGCANVAS_ATTRIBUTE_ALPHA "alpha"
#define DEBUGCANVAS_ATTRIBUTE_BLENDMODE "blendMode"
#define DEBUGCANVAS_ATTRIBUTE_STYLE "style"
#define DEBUGCANVAS_ATTRIBUTE_STROKEWIDTH "strokeWidth"
#define DEBUGCANVAS_ATTRIBUTE_STROKEMITER "strokeMiter"
#define DEBUGCANVAS_ATTRIBUTE_STROKEJOIN "strokeJoin"
#define DEBUGCANVAS_ATTRIBUTE_CAP "cap"
#define DEBUGCANVAS_ATTRIBUTE_ANTIALIAS "antiAlias"
#define DEBUGCANVAS_ATTRIBUTE_DITHER "dither"
#define DEBUGCANVAS_ATTRIBUTE_FAKEBOLDTEXT "fakeBoldText"
#define DEBUGCANVAS_ATTRIBUTE_LINEARTEXT "linearText"
#define DEBUGCANVAS_ATTRIBUTE_SUBPIXELTEXT "subpixelText"
#define DEBUGCANVAS_ATTRIBUTE_DEVKERNTEXT "devKernText"
#define DEBUGCANVAS_ATTRIBUTE_LCDRENDERTEXT "lcdRenderText"
#define DEBUGCANVAS_ATTRIBUTE_EMBEDDEDBITMAPTEXT "embeddedBitmapText"
#define DEBUGCANVAS_ATTRIBUTE_AUTOHINTING "forceAutoHinting"
#define DEBUGCANVAS_ATTRIBUTE_REGION "region"
#define DEBUGCANVAS_ATTRIBUTE_REGIONOP "op"
#define DEBUGCANVAS_ATTRIBUTE_EDGESTYLE "edgeStyle"
#define DEBUGCANVAS_ATTRIBUTE_DEVICEREGION "deviceRegion"
#define DEBUGCANVAS_ATTRIBUTE_BLUR "blur"
#define DEBUGCANVAS_ATTRIBUTE_SIGMA "sigma"
#define DEBUGCANVAS_ATTRIBUTE_QUALITY "quality"
#define DEBUGCANVAS_ATTRIBUTE_TEXTSIZE "textSize"
#define DEBUGCANVAS_ATTRIBUTE_TEXTSCALEX "textScaleX"
#define DEBUGCANVAS_ATTRIBUTE_TEXTSKEWX "textSkewX"
#define DEBUGCANVAS_ATTRIBUTE_DASHING "dashing"
#define DEBUGCANVAS_ATTRIBUTE_INTERVALS "intervals"
#define DEBUGCANVAS_ATTRIBUTE_PHASE "phase"
#define DEBUGCANVAS_ATTRIBUTE_FILLTYPE "fillType"
#define DEBUGCANVAS_ATTRIBUTE_VERBS "verbs"
#define DEBUGCANVAS_ATTRIBUTE_NAME "name"
#define DEBUGCANVAS_ATTRIBUTE_DATA "data"
#define DEBUGCANVAS_ATTRIBUTE_VALUES "values"
#define DEBUGCANVAS_ATTRIBUTE_SHADER "shader"
#define DEBUGCANVAS_ATTRIBUTE_PATHEFFECT "pathEffect"
#define DEBUGCANVAS_ATTRIBUTE_MASKFILTER "maskFilter"
#define DEBUGCANVAS_ATTRIBUTE_XFERMODE "xfermode"
#define DEBUGCANVAS_ATTRIBUTE_LOOPER "looper"
#define DEBUGCANVAS_ATTRIBUTE_BACKDROP "backdrop"
#define DEBUGCANVAS_ATTRIBUTE_COLORFILTER "colorfilter"
#define DEBUGCANVAS_ATTRIBUTE_IMAGEFILTER "imagefilter"
#define DEBUGCANVAS_ATTRIBUTE_IMAGE "image"
#define DEBUGCANVAS_ATTRIBUTE_BITMAP "bitmap"
#define DEBUGCANVAS_ATTRIBUTE_SRC "src"
#define DEBUGCANVAS_ATTRIBUTE_DST "dst"
#define DEBUGCANVAS_ATTRIBUTE_CENTER "center"
#define DEBUGCANVAS_ATTRIBUTE_STRICT "strict"
#define DEBUGCANVAS_ATTRIBUTE_DESCRIPTION "description"
#define DEBUGCANVAS_ATTRIBUTE_X "x"
#define DEBUGCANVAS_ATTRIBUTE_Y "y"
#define DEBUGCANVAS_ATTRIBUTE_RUNS "runs"
#define DEBUGCANVAS_ATTRIBUTE_POSITIONS "positions"
#define DEBUGCANVAS_ATTRIBUTE_GLYPHS "glyphs"
#define DEBUGCANVAS_ATTRIBUTE_FONT "font"
#define DEBUGCANVAS_ATTRIBUTE_TYPEFACE "typeface"
#define DEBUGCANVAS_ATTRIBUTE_CUBICS "cubics"
#define DEBUGCANVAS_ATTRIBUTE_COLORS "colors"
#define DEBUGCANVAS_ATTRIBUTE_TEXTURECOORDS "textureCoords"
#define DEBUGCANVAS_ATTRIBUTE_FILTERQUALITY "filterQuality"
#define DEBUGCANVAS_ATTRIBUTE_STARTANGLE "startAngle"
#define DEBUGCANVAS_ATTRIBUTE_SWEEPANGLE "sweepAngle"
#define DEBUGCANVAS_ATTRIBUTE_USECENTER "useCenter"
#define DEBUGCANVAS_ATTRIBUTE_SHORTDESC "shortDesc"
#define DEBUGCANVAS_ATTRIBUTE_UNIQUE_ID "uniqueID"
#define DEBUGCANVAS_ATTRIBUTE_WIDTH "width"
#define DEBUGCANVAS_ATTRIBUTE_HEIGHT "height"
#define DEBUGCANVAS_ATTRIBUTE_ALPHA "alpha"
#define DEBUGCANVAS_ATTRIBUTE_LATTICE "lattice"
#define DEBUGCANVAS_ATTRIBUTE_LATTICEXCOUNT "xCount"
#define DEBUGCANVAS_ATTRIBUTE_LATTICEYCOUNT "yCount"
#define DEBUGCANVAS_ATTRIBUTE_LATTICEXDIVS "xDivs"
#define DEBUGCANVAS_ATTRIBUTE_LATTICEYDIVS "yDivs"
#define DEBUGCANVAS_ATTRIBUTE_LATTICEFLAGS "flags"
#define DEBUGCANVAS_ATTRIBUTE_ZPLANE "zPlane"
#define DEBUGCANVAS_ATTRIBUTE_LIGHTPOSITION "lightPositions"
#define DEBUGCANVAS_ATTRIBUTE_AMBIENTCOLOR "ambientColor"
#define DEBUGCANVAS_ATTRIBUTE_SPOTCOLOR "spotColor"
#define DEBUGCANVAS_ATTRIBUTE_LIGHTRADIUS "lightRadius"

#define DEBUGCANVAS_VERB_MOVE "move"
#define DEBUGCANVAS_VERB_LINE "line"
#define DEBUGCANVAS_VERB_QUAD "quad"
#define DEBUGCANVAS_VERB_CUBIC "cubic"
#define DEBUGCANVAS_VERB_CONIC "conic"
#define DEBUGCANVAS_VERB_CLOSE "close"

#define DEBUGCANVAS_STYLE_FILL "fill"
#define DEBUGCANVAS_STYLE_STROKE "stroke"
#define DEBUGCANVAS_STYLE_STROKEANDFILL "strokeAndFill"

#define DEBUGCANVAS_POINTMODE_POINTS "points"
#define DEBUGCANVAS_POINTMODE_LINES "lines"
#define DEBUGCANVAS_POINTMODE_POLYGON "polygon"

#define DEBUGCANVAS_REGIONOP_DIFFERENCE "difference"
#define DEBUGCANVAS_REGIONOP_INTERSECT "intersect"
#define DEBUGCANVAS_REGIONOP_UNION "union"
#define DEBUGCANVAS_REGIONOP_XOR "xor"
#define DEBUGCANVAS_REGIONOP_REVERSE_DIFFERENCE "reverseDifference"
#define DEBUGCANVAS_REGIONOP_REPLACE "replace"

#define DEBUGCANVAS_BLURSTYLE_NORMAL "normal"
#define DEBUGCANVAS_BLURSTYLE_SOLID "solid"
#define DEBUGCANVAS_BLURSTYLE_OUTER "outer"
#define DEBUGCANVAS_BLURSTYLE_INNER "inner"

#define DEBUGCANVAS_BLURQUALITY_LOW "low"
#define DEBUGCANVAS_BLURQUALITY_HIGH "high"

#define DEBUGCANVAS_FILLTYPE_WINDING "winding"
#define DEBUGCANVAS_FILLTYPE_EVENODD "evenOdd"
#define DEBUGCANVAS_FILLTYPE_INVERSEWINDING "inverseWinding"
#define DEBUGCANVAS_FILLTYPE_INVERSEEVENODD "inverseEvenOdd"

#define DEBUGCANVAS_CAP_BUTT "butt"
#define DEBUGCANVAS_CAP_ROUND "round"
#define DEBUGCANVAS_CAP_SQUARE "square"

#define DEBUGCANVAS_MITER_JOIN "miter"
#define DEBUGCANVAS_ROUND_JOIN "round"
#define DEBUGCANVAS_BEVEL_JOIN "bevel"

#define DEBUGCANVAS_COLORTYPE_ARGB4444 "ARGB4444"
#define DEBUGCANVAS_COLORTYPE_RGBA8888 "RGBA8888"
#define DEBUGCANVAS_COLORTYPE_BGRA8888 "BGRA8888"
#define DEBUGCANVAS_COLORTYPE_565 "565"
#define DEBUGCANVAS_COLORTYPE_GRAY8 "Gray8"
#define DEBUGCANVAS_COLORTYPE_INDEX8 "Index8"
#define DEBUGCANVAS_COLORTYPE_ALPHA8 "Alpha8"

#define DEBUGCANVAS_ALPHATYPE_OPAQUE "opaque"
#define DEBUGCANVAS_ALPHATYPE_PREMUL "premul"
#define DEBUGCANVAS_ALPHATYPE_UNPREMUL "unpremul"
#define DEBUGCANVAS_ALPHATYPE_UNKNOWN "unknown"

#define DEBUGCANVAS_FILTERQUALITY_NONE "none"
#define DEBUGCANVAS_FILTERQUALITY_LOW "low"
#define DEBUGCANVAS_FILTERQUALITY_MEDIUM "medium"
#define DEBUGCANVAS_FILTERQUALITY_HIGH "high"

#define DEBUGCANVAS_HINTING_NONE "none"
#define DEBUGCANVAS_HINTING_SLIGHT "slight"
#define DEBUGCANVAS_HINTING_NORMAL "normal"
#define DEBUGCANVAS_HINTING_FULL "full"

#define DEBUGCANVAS_EDGING_ALIAS "alias"
#define DEBUGCANVAS_EDGING_ANTIALIAS "antialias"
#define DEBUGCANVAS_EDGING_SUBPIXELANTIALIAS "subpixelantialias"

#define DEBUGCANVAS_SHADOWFLAG_TRANSPARENT_OCC "transparentOccluder"
#define DEBUGCANVAS_SHADOWFLAG_GEOMETRIC_ONLY "geometricOnly"

static SkString* str_append(SkString* str, const SkRect& r) {
    str->appendf(" [%g %g %g %g]", r.left(), r.top(), r.right(), r.bottom());
    return str;
}

DrawCommand::DrawCommand(OpType type) : fOpType(type), fVisible(true) {}

const char* DrawCommand::GetCommandString(OpType type) {
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

void DrawCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_COMMAND, this->GetCommandString(fOpType));
    writer.appendBool(DEBUGCANVAS_ATTRIBUTE_VISIBLE, this->isVisible());
}

namespace {

void xlate_and_scale_to_bounds(SkCanvas* canvas, const SkRect& bounds) {
    const SkISize& size = canvas->getBaseLayerSize();

    static const SkScalar kInsetFrac = 0.9f;  // Leave a border around object

    canvas->translate(size.fWidth / 2.0f, size.fHeight / 2.0f);
    if (bounds.width() > bounds.height()) {
        canvas->scale(SkDoubleToScalar((kInsetFrac * size.fWidth) / bounds.width()),
                      SkDoubleToScalar((kInsetFrac * size.fHeight) / bounds.width()));
    } else {
        canvas->scale(SkDoubleToScalar((kInsetFrac * size.fWidth) / bounds.height()),
                      SkDoubleToScalar((kInsetFrac * size.fHeight) / bounds.height()));
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

    SkScalar xScale = SkIntToScalar(size.fWidth - 2) / input.width();
    SkScalar yScale = SkIntToScalar(size.fHeight - 2) / input.height();

    if (input.width() > input.height()) {
        yScale *= input.height() / (float)input.width();
    } else {
        xScale *= input.width() / (float)input.height();
    }

    SkRect dst = SkRect::MakeXYWH(
            SK_Scalar1, SK_Scalar1, xScale * input.width(), yScale * input.height());

    static const int kNumBlocks = 8;

    canvas->clear(0xFFFFFFFF);
    SkISize block = {canvas->imageInfo().width() / kNumBlocks,
                     canvas->imageInfo().height() / kNumBlocks};
    for (int y = 0; y < kNumBlocks; ++y) {
        for (int x = 0; x < kNumBlocks; ++x) {
            SkPaint paint;
            paint.setColor((x + y) % 2 ? SK_ColorLTGRAY : SK_ColorDKGRAY);
            SkRect r = SkRect::MakeXYWH(SkIntToScalar(x * block.width()),
                                        SkIntToScalar(y * block.height()),
                                        SkIntToScalar(block.width()),
                                        SkIntToScalar(block.height()));
            canvas->drawRect(r, paint);
        }
    }

    canvas->drawBitmapRect(input, dst, nullptr);

    if (srcRect) {
        SkRect  r = SkRect::MakeLTRB(srcRect->fLeft * xScale + SK_Scalar1,
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
    rec.fSpotColor    = SK_ColorBLACK;
    canvas->private_draw_shadow_rec(path, rec);
}

static const char* const gBlendModeMap[] = {
        "clear",      "src",        "dst",      "srcOver",    "dstOver",   "srcIn",     "dstIn",
        "srcOut",     "dstOut",     "srcATop",  "dstATop",    "xor",       "plus",      "modulate",

        "screen",

        "overlay",    "darken",     "lighten",  "colorDodge", "colorBurn", "hardLight", "softLight",
        "difference", "exclusion",  "multiply",

        "hue",        "saturation", "color",    "luminosity",
};

static_assert(SK_ARRAY_COUNT(gBlendModeMap) == static_cast<size_t>(SkBlendMode::kLastMode) + 1,
              "blendMode mismatch");
static_assert(SK_ARRAY_COUNT(gBlendModeMap) == static_cast<size_t>(SkBlendMode::kLuminosity) + 1,
              "blendMode mismatch");

void apply_paint_blend_mode(const SkPaint& paint, SkJSONWriter& writer) {
    const auto mode = paint.getBlendMode();
    if (mode != SkBlendMode::kSrcOver) {
        SkASSERT(static_cast<size_t>(mode) < SK_ARRAY_COUNT(gBlendModeMap));
        writer.appendString(DEBUGCANVAS_ATTRIBUTE_BLENDMODE,
                            gBlendModeMap[static_cast<size_t>(mode)]);
    }
}

};  // namespace

void DrawCommand::MakeJsonColor(SkJSONWriter& writer, const SkColor color) {
    writer.beginArray(nullptr, false);
    writer.appendS32(SkColorGetA(color));
    writer.appendS32(SkColorGetR(color));
    writer.appendS32(SkColorGetG(color));
    writer.appendS32(SkColorGetB(color));
    writer.endArray();
}

void DrawCommand::MakeJsonColor4f(SkJSONWriter& writer, const SkColor4f& color) {
    writer.beginArray(nullptr, false);
    writer.appendFloat(color.fA);
    writer.appendFloat(color.fR);
    writer.appendFloat(color.fG);
    writer.appendFloat(color.fB);
    writer.endArray();
}

void DrawCommand::MakeJsonPoint(SkJSONWriter& writer, const SkPoint& point) {
    writer.beginArray(nullptr, false);
    writer.appendFloat(point.x());
    writer.appendFloat(point.y());
    writer.endArray();
}

void DrawCommand::MakeJsonPoint(SkJSONWriter& writer, SkScalar x, SkScalar y) {
    writer.beginArray(nullptr, false);
    writer.appendFloat(x);
    writer.appendFloat(y);
    writer.endArray();
}

void DrawCommand::MakeJsonPoint3(SkJSONWriter& writer, const SkPoint3& point) {
    writer.beginArray(nullptr, false);
    writer.appendFloat(point.x());
    writer.appendFloat(point.y());
    writer.appendFloat(point.z());
    writer.endArray();
}

void DrawCommand::MakeJsonRect(SkJSONWriter& writer, const SkRect& rect) {
    writer.beginArray(nullptr, false);
    writer.appendFloat(rect.left());
    writer.appendFloat(rect.top());
    writer.appendFloat(rect.right());
    writer.appendFloat(rect.bottom());
    writer.endArray();
}

void DrawCommand::MakeJsonIRect(SkJSONWriter& writer, const SkIRect& rect) {
    writer.beginArray(nullptr, false);
    writer.appendS32(rect.left());
    writer.appendS32(rect.top());
    writer.appendS32(rect.right());
    writer.appendS32(rect.bottom());
    writer.endArray();
}

static void make_json_rrect(SkJSONWriter& writer, const SkRRect& rrect) {
    writer.beginArray(nullptr, false);
    DrawCommand::MakeJsonRect(writer, rrect.rect());
    DrawCommand::MakeJsonPoint(writer, rrect.radii(SkRRect::kUpperLeft_Corner));
    DrawCommand::MakeJsonPoint(writer, rrect.radii(SkRRect::kUpperRight_Corner));
    DrawCommand::MakeJsonPoint(writer, rrect.radii(SkRRect::kLowerRight_Corner));
    DrawCommand::MakeJsonPoint(writer, rrect.radii(SkRRect::kLowerLeft_Corner));
    writer.endArray();
}

void DrawCommand::MakeJsonMatrix(SkJSONWriter& writer, const SkMatrix& matrix) {
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

void DrawCommand::MakeJsonPath(SkJSONWriter& writer, const SkPath& path) {
    writer.beginObject();
    switch (path.getFillType()) {
        case SkPath::kWinding_FillType:
            writer.appendString(DEBUGCANVAS_ATTRIBUTE_FILLTYPE, DEBUGCANVAS_FILLTYPE_WINDING);
            break;
        case SkPath::kEvenOdd_FillType:
            writer.appendString(DEBUGCANVAS_ATTRIBUTE_FILLTYPE, DEBUGCANVAS_FILLTYPE_EVENODD);
            break;
        case SkPath::kInverseWinding_FillType:
            writer.appendString(DEBUGCANVAS_ATTRIBUTE_FILLTYPE,
                                DEBUGCANVAS_FILLTYPE_INVERSEWINDING);
            break;
        case SkPath::kInverseEvenOdd_FillType:
            writer.appendString(DEBUGCANVAS_ATTRIBUTE_FILLTYPE,
                                DEBUGCANVAS_FILLTYPE_INVERSEEVENODD);
            break;
    }
    writer.beginArray(DEBUGCANVAS_ATTRIBUTE_VERBS);
    SkPath::Iter iter(path, false);
    SkPoint      pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        if (verb == SkPath::kClose_Verb) {
            writer.appendString(DEBUGCANVAS_VERB_CLOSE);
            continue;
        }
        writer.beginObject();  // verb
        switch (verb) {
            case SkPath::kLine_Verb: {
                writer.appendName(DEBUGCANVAS_VERB_LINE);
                MakeJsonPoint(writer, pts[1]);
                break;
            }
            case SkPath::kQuad_Verb: {
                writer.beginArray(DEBUGCANVAS_VERB_QUAD);
                MakeJsonPoint(writer, pts[1]);
                MakeJsonPoint(writer, pts[2]);
                writer.endArray();  // quad coords
                break;
            }
            case SkPath::kCubic_Verb: {
                writer.beginArray(DEBUGCANVAS_VERB_CUBIC);
                MakeJsonPoint(writer, pts[1]);
                MakeJsonPoint(writer, pts[2]);
                MakeJsonPoint(writer, pts[3]);
                writer.endArray();  // cubic coords
                break;
            }
            case SkPath::kConic_Verb: {
                writer.beginArray(DEBUGCANVAS_VERB_CONIC);
                MakeJsonPoint(writer, pts[1]);
                MakeJsonPoint(writer, pts[2]);
                writer.appendFloat(iter.conicWeight());
                writer.endArray();  // conic coords
                break;
            }
            case SkPath::kMove_Verb: {
                writer.appendName(DEBUGCANVAS_VERB_MOVE);
                MakeJsonPoint(writer, pts[0]);
                break;
            }
            case SkPath::kClose_Verb:
            case SkPath::kDone_Verb:
                // Unreachable
                break;
        }
        writer.endObject();  // verb
    }
    writer.endArray();   // verbs
    writer.endObject();  // path
}

void DrawCommand::MakeJsonRegion(SkJSONWriter& writer, const SkRegion& region) {
    // TODO: Actually serialize the rectangles, rather than just devolving to path
    SkPath path;
    region.getBoundaryPath(&path);
    MakeJsonPath(writer, path);
}

static const char* regionop_name(SkClipOp op) {
    switch (op) {
        case kDifference_SkClipOp: return DEBUGCANVAS_REGIONOP_DIFFERENCE;
        case kIntersect_SkClipOp: return DEBUGCANVAS_REGIONOP_INTERSECT;
        case kUnion_SkClipOp: return DEBUGCANVAS_REGIONOP_UNION;
        case kXOR_SkClipOp: return DEBUGCANVAS_REGIONOP_XOR;
        case kReverseDifference_SkClipOp: return DEBUGCANVAS_REGIONOP_REVERSE_DIFFERENCE;
        case kReplace_SkClipOp: return DEBUGCANVAS_REGIONOP_REPLACE;
        default: SkASSERT(false); return "<invalid region op>";
    }
}

static const char* pointmode_name(SkCanvas::PointMode mode) {
    switch (mode) {
        case SkCanvas::kPoints_PointMode: return DEBUGCANVAS_POINTMODE_POINTS;
        case SkCanvas::kLines_PointMode: return DEBUGCANVAS_POINTMODE_LINES;
        case SkCanvas::kPolygon_PointMode: return DEBUGCANVAS_POINTMODE_POLYGON;
        default: SkASSERT(false); return "<invalid point mode>";
    }
}

static void store_scalar(SkJSONWriter& writer,
                         const char*   key,
                         SkScalar      value,
                         SkScalar      defaultValue) {
    if (value != defaultValue) {
        writer.appendFloat(key, value);
    }
}

static void store_bool(SkJSONWriter& writer, const char* key, bool value, bool defaultValue) {
    if (value != defaultValue) {
        writer.appendBool(key, value);
    }
}

static SkString encode_data(const void*     bytes,
                            size_t          count,
                            const char*     contentType,
                            UrlDataManager& urlDataManager) {
    sk_sp<SkData> data(SkData::MakeWithCopy(bytes, count));
    return urlDataManager.addData(data.get(), contentType);
}

void DrawCommand::flatten(const SkFlattenable* flattenable,
                          SkJSONWriter&        writer,
                          UrlDataManager&      urlDataManager) {
    SkBinaryWriteBuffer buffer;
    flattenable->flatten(buffer);
    void* data = sk_malloc_throw(buffer.bytesWritten());
    buffer.writeToMemory(data);
    SkString url =
            encode_data(data, buffer.bytesWritten(), "application/octet-stream", urlDataManager);
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_NAME, flattenable->getTypeName());
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_DATA, url.c_str());

    writer.beginObject(DEBUGCANVAS_ATTRIBUTE_VALUES);
    JsonWriteBuffer jsonBuffer(&writer, &urlDataManager);
    flattenable->flatten(jsonBuffer);
    writer.endObject();  // values

    sk_free(data);
}

void DrawCommand::WritePNG(SkBitmap bitmap, SkWStream& out) {
    SkPixmap pm;
    SkAssertResult(bitmap.peekPixels(&pm));

    SkPngEncoder::Options options;
    options.fZLibLevel   = 1;
    options.fFilterFlags = SkPngEncoder::FilterFlag::kNone;
    SkPngEncoder::Encode(&out, pm, options);
}

bool DrawCommand::flatten(const SkImage&  image,
                          SkJSONWriter&   writer,
                          UrlDataManager& urlDataManager) {
    size_t       rowBytes = 4 * image.width();
    SkAutoMalloc buffer(rowBytes * image.height());
    SkImageInfo  dstInfo =
            SkImageInfo::Make(image.width(), image.height(), kN32_SkColorType, kPremul_SkAlphaType);
    if (!image.readPixels(dstInfo, buffer.get(), rowBytes, 0, 0)) {
        SkDebugf("readPixels failed\n");
        return false;
    }

    SkBitmap bm;
    bm.installPixels(dstInfo, buffer.get(), rowBytes);

    SkDynamicMemoryWStream out;
    DrawCommand::WritePNG(bm, out);
    sk_sp<SkData> encoded = out.detachAsData();
    SkString      url = encode_data(encoded->data(), encoded->size(), "image/png", urlDataManager);
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_DATA, url.c_str());
    return true;
}

static const char* color_type_name(SkColorType colorType) {
    switch (colorType) {
        case kARGB_4444_SkColorType: return DEBUGCANVAS_COLORTYPE_ARGB4444;
        case kRGBA_8888_SkColorType: return DEBUGCANVAS_COLORTYPE_RGBA8888;
        case kBGRA_8888_SkColorType: return DEBUGCANVAS_COLORTYPE_BGRA8888;
        case kRGB_565_SkColorType: return DEBUGCANVAS_COLORTYPE_565;
        case kGray_8_SkColorType: return DEBUGCANVAS_COLORTYPE_GRAY8;
        case kAlpha_8_SkColorType: return DEBUGCANVAS_COLORTYPE_ALPHA8;
        default: SkASSERT(false); return DEBUGCANVAS_COLORTYPE_RGBA8888;
    }
}

static const char* alpha_type_name(SkAlphaType alphaType) {
    switch (alphaType) {
        case kOpaque_SkAlphaType: return DEBUGCANVAS_ALPHATYPE_OPAQUE;
        case kPremul_SkAlphaType: return DEBUGCANVAS_ALPHATYPE_PREMUL;
        case kUnpremul_SkAlphaType: return DEBUGCANVAS_ALPHATYPE_UNPREMUL;
        default: SkASSERT(false); return DEBUGCANVAS_ALPHATYPE_OPAQUE;
    }
}

bool DrawCommand::flatten(const SkBitmap& bitmap,
                          SkJSONWriter&   writer,
                          UrlDataManager& urlDataManager) {
    sk_sp<SkImage> image(SkImage::MakeFromBitmap(bitmap));
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_COLOR, color_type_name(bitmap.colorType()));
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_ALPHA, alpha_type_name(bitmap.alphaType()));
    bool success = flatten(*image, writer, urlDataManager);
    return success;
}

static void apply_font_hinting(const SkFont& font, SkJSONWriter& writer) {
    SkFontHinting hinting = font.getHinting();
    if (hinting != SkPaintDefaults_Hinting) {
        switch (hinting) {
            case SkFontHinting::kNone:
                writer.appendString(DEBUGCANVAS_ATTRIBUTE_HINTING, DEBUGCANVAS_HINTING_NONE);
                break;
            case SkFontHinting::kSlight:
                writer.appendString(DEBUGCANVAS_ATTRIBUTE_HINTING, DEBUGCANVAS_HINTING_SLIGHT);
                break;
            case SkFontHinting::kNormal:
                writer.appendString(DEBUGCANVAS_ATTRIBUTE_HINTING, DEBUGCANVAS_HINTING_NORMAL);
                break;
            case SkFontHinting::kFull:
                writer.appendString(DEBUGCANVAS_ATTRIBUTE_HINTING, DEBUGCANVAS_HINTING_FULL);
                break;
        }
    }
}

static void apply_font_edging(const SkFont& font, SkJSONWriter& writer) {
    switch (font.getEdging()) {
        case SkFont::Edging::kAlias:
            writer.appendString(DEBUGCANVAS_ATTRIBUTE_EDGING, DEBUGCANVAS_EDGING_ALIAS);
            break;
        case SkFont::Edging::kAntiAlias:
            writer.appendString(DEBUGCANVAS_ATTRIBUTE_EDGING, DEBUGCANVAS_EDGING_ANTIALIAS);
            break;
        case SkFont::Edging::kSubpixelAntiAlias:
            writer.appendString(DEBUGCANVAS_ATTRIBUTE_EDGING, DEBUGCANVAS_EDGING_SUBPIXELANTIALIAS);
            break;
    }
}

static void apply_paint_color(const SkPaint& paint, SkJSONWriter& writer) {
    SkColor color = paint.getColor();
    if (color != SK_ColorBLACK) {
        writer.appendName(DEBUGCANVAS_ATTRIBUTE_COLOR);
        DrawCommand::MakeJsonColor(writer, color);
    }
}

static void apply_paint_style(const SkPaint& paint, SkJSONWriter& writer) {
    SkPaint::Style style = paint.getStyle();
    if (style != SkPaint::kFill_Style) {
        switch (style) {
            case SkPaint::kStroke_Style: {
                writer.appendString(DEBUGCANVAS_ATTRIBUTE_STYLE, DEBUGCANVAS_STYLE_STROKE);
                break;
            }
            case SkPaint::kStrokeAndFill_Style: {
                writer.appendString(DEBUGCANVAS_ATTRIBUTE_STYLE, DEBUGCANVAS_STYLE_STROKEANDFILL);
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
                writer.appendString(DEBUGCANVAS_ATTRIBUTE_CAP, DEBUGCANVAS_CAP_BUTT);
                break;
            case SkPaint::kRound_Cap:
                writer.appendString(DEBUGCANVAS_ATTRIBUTE_CAP, DEBUGCANVAS_CAP_ROUND);
                break;
            case SkPaint::kSquare_Cap:
                writer.appendString(DEBUGCANVAS_ATTRIBUTE_CAP, DEBUGCANVAS_CAP_SQUARE);
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
                writer.appendString(DEBUGCANVAS_ATTRIBUTE_STROKEJOIN, DEBUGCANVAS_MITER_JOIN);
                break;
            case SkPaint::kRound_Join:
                writer.appendString(DEBUGCANVAS_ATTRIBUTE_STROKEJOIN, DEBUGCANVAS_ROUND_JOIN);
                break;
            case SkPaint::kBevel_Join:
                writer.appendString(DEBUGCANVAS_ATTRIBUTE_STROKEJOIN, DEBUGCANVAS_BEVEL_JOIN);
                break;
            default: SkASSERT(false);
        }
    }
}

static void apply_paint_filterquality(const SkPaint& paint, SkJSONWriter& writer) {
    SkFilterQuality quality = paint.getFilterQuality();
    switch (quality) {
        case kNone_SkFilterQuality: break;
        case kLow_SkFilterQuality:
            writer.appendString(DEBUGCANVAS_ATTRIBUTE_FILTERQUALITY, DEBUGCANVAS_FILTERQUALITY_LOW);
            break;
        case kMedium_SkFilterQuality:
            writer.appendString(DEBUGCANVAS_ATTRIBUTE_FILTERQUALITY,
                                DEBUGCANVAS_FILTERQUALITY_MEDIUM);
            break;
        case kHigh_SkFilterQuality:
            writer.appendString(DEBUGCANVAS_ATTRIBUTE_FILTERQUALITY,
                                DEBUGCANVAS_FILTERQUALITY_HIGH);
            break;
    }
}

static void apply_paint_maskfilter(const SkPaint&  paint,
                                   SkJSONWriter&   writer,
                                   UrlDataManager& urlDataManager) {
    SkMaskFilter* maskFilter = paint.getMaskFilter();
    if (maskFilter != nullptr) {
        SkMaskFilterBase::BlurRec blurRec;
        if (as_MFB(maskFilter)->asABlur(&blurRec)) {
            writer.beginObject(DEBUGCANVAS_ATTRIBUTE_BLUR);
            writer.appendFloat(DEBUGCANVAS_ATTRIBUTE_SIGMA, blurRec.fSigma);
            switch (blurRec.fStyle) {
                case SkBlurStyle::kNormal_SkBlurStyle:
                    writer.appendString(DEBUGCANVAS_ATTRIBUTE_STYLE, DEBUGCANVAS_BLURSTYLE_NORMAL);
                    break;
                case SkBlurStyle::kSolid_SkBlurStyle:
                    writer.appendString(DEBUGCANVAS_ATTRIBUTE_STYLE, DEBUGCANVAS_BLURSTYLE_SOLID);
                    break;
                case SkBlurStyle::kOuter_SkBlurStyle:
                    writer.appendString(DEBUGCANVAS_ATTRIBUTE_STYLE, DEBUGCANVAS_BLURSTYLE_OUTER);
                    break;
                case SkBlurStyle::kInner_SkBlurStyle:
                    writer.appendString(DEBUGCANVAS_ATTRIBUTE_STYLE, DEBUGCANVAS_BLURSTYLE_INNER);
                    break;
                default: SkASSERT(false);
            }
            writer.endObject();  // blur
        } else {
            writer.beginObject(DEBUGCANVAS_ATTRIBUTE_MASKFILTER);
            DrawCommand::flatten(maskFilter, writer, urlDataManager);
            writer.endObject();  // maskFilter
        }
    }
}

static void apply_paint_patheffect(const SkPaint&  paint,
                                   SkJSONWriter&   writer,
                                   UrlDataManager& urlDataManager) {
    SkPathEffect* pathEffect = paint.getPathEffect();
    if (pathEffect != nullptr) {
        SkPathEffect::DashInfo dashInfo;
        SkPathEffect::DashType dashType = pathEffect->asADash(&dashInfo);
        if (dashType == SkPathEffect::kDash_DashType) {
            dashInfo.fIntervals = (SkScalar*)sk_malloc_throw(dashInfo.fCount * sizeof(SkScalar));
            pathEffect->asADash(&dashInfo);
            writer.beginObject(DEBUGCANVAS_ATTRIBUTE_DASHING);
            writer.beginArray(DEBUGCANVAS_ATTRIBUTE_INTERVALS, false);
            for (int32_t i = 0; i < dashInfo.fCount; i++) {
                writer.appendFloat(dashInfo.fIntervals[i]);
            }
            writer.endArray();  // intervals
            sk_free(dashInfo.fIntervals);
            writer.appendFloat(DEBUGCANVAS_ATTRIBUTE_PHASE, dashInfo.fPhase);
            writer.endObject();  // dashing
        } else {
            writer.beginObject(DEBUGCANVAS_ATTRIBUTE_PATHEFFECT);
            DrawCommand::flatten(pathEffect, writer, urlDataManager);
            writer.endObject();  // pathEffect
        }
    }
}

static void apply_font_typeface(const SkFont&   font,
                                SkJSONWriter&   writer,
                                UrlDataManager& urlDataManager) {
    SkTypeface* typeface = font.getTypefaceOrDefault();
    if (typeface != nullptr) {
        writer.beginObject(DEBUGCANVAS_ATTRIBUTE_TYPEFACE);
        SkDynamicMemoryWStream buffer;
        typeface->serialize(&buffer);
        void* data = sk_malloc_throw(buffer.bytesWritten());
        buffer.copyTo(data);
        SkString url = encode_data(
                data, buffer.bytesWritten(), "application/octet-stream", urlDataManager);
        writer.appendString(DEBUGCANVAS_ATTRIBUTE_DATA, url.c_str());
        sk_free(data);
        writer.endObject();
    }
}

static void apply_flattenable(const char*     key,
                              SkFlattenable*  flattenable,
                              SkJSONWriter&   writer,
                              UrlDataManager& urlDataManager) {
    if (flattenable != nullptr) {
        writer.beginObject(key);
        DrawCommand::flatten(flattenable, writer, urlDataManager);
        writer.endObject();
    }
}

void DrawCommand::MakeJsonPaint(SkJSONWriter&   writer,
                                const SkPaint&  paint,
                                UrlDataManager& urlDataManager) {
    writer.beginObject();
    store_scalar(writer, DEBUGCANVAS_ATTRIBUTE_STROKEWIDTH, paint.getStrokeWidth(), 0.0f);
    store_scalar(writer,
                 DEBUGCANVAS_ATTRIBUTE_STROKEMITER,
                 paint.getStrokeMiter(),
                 SkPaintDefaults_MiterLimit);
    store_bool(writer, DEBUGCANVAS_ATTRIBUTE_ANTIALIAS, paint.isAntiAlias(), false);
    store_bool(writer, DEBUGCANVAS_ATTRIBUTE_DITHER, paint.isDither(), false);

    apply_paint_color(paint, writer);
    apply_paint_style(paint, writer);
    apply_paint_blend_mode(paint, writer);
    apply_paint_cap(paint, writer);
    apply_paint_join(paint, writer);
    apply_paint_filterquality(paint, writer);
    apply_paint_patheffect(paint, writer, urlDataManager);
    apply_paint_maskfilter(paint, writer, urlDataManager);
    apply_flattenable(DEBUGCANVAS_ATTRIBUTE_SHADER, paint.getShader(), writer, urlDataManager);
#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
    apply_flattenable(DEBUGCANVAS_ATTRIBUTE_LOOPER, paint.getLooper(), writer, urlDataManager);
#endif
    apply_flattenable(
            DEBUGCANVAS_ATTRIBUTE_IMAGEFILTER, paint.getImageFilter(), writer, urlDataManager);
    apply_flattenable(
            DEBUGCANVAS_ATTRIBUTE_COLORFILTER, paint.getColorFilter(), writer, urlDataManager);
    writer.endObject();  // paint
}

static void MakeJsonFont(const SkFont& font, SkJSONWriter& writer, UrlDataManager& urlDataManager) {
    writer.beginObject();
    store_bool(writer, DEBUGCANVAS_ATTRIBUTE_FAKEBOLDTEXT, font.isEmbolden(), false);
    store_bool(writer, DEBUGCANVAS_ATTRIBUTE_LINEARTEXT, font.isLinearMetrics(), false);
    store_bool(writer, DEBUGCANVAS_ATTRIBUTE_SUBPIXELTEXT, font.isSubpixel(), false);
    store_bool(writer, DEBUGCANVAS_ATTRIBUTE_EMBEDDEDBITMAPTEXT, font.isEmbeddedBitmaps(), false);
    store_bool(writer, DEBUGCANVAS_ATTRIBUTE_AUTOHINTING, font.isForceAutoHinting(), false);

    store_scalar(writer, DEBUGCANVAS_ATTRIBUTE_TEXTSIZE, font.getSize(), SkPaintDefaults_TextSize);
    store_scalar(writer, DEBUGCANVAS_ATTRIBUTE_TEXTSCALEX, font.getScaleX(), SK_Scalar1);
    store_scalar(writer, DEBUGCANVAS_ATTRIBUTE_TEXTSCALEX, font.getSkewX(), 0.0f);
    apply_font_edging(font, writer);
    apply_font_hinting(font, writer);
    apply_font_typeface(font, writer, urlDataManager);
    writer.endObject();  // font
}

void DrawCommand::MakeJsonLattice(SkJSONWriter& writer, const SkCanvas::Lattice& lattice) {
    writer.beginObject();
    writer.appendS32(DEBUGCANVAS_ATTRIBUTE_LATTICEXCOUNT, lattice.fXCount);
    writer.appendS32(DEBUGCANVAS_ATTRIBUTE_LATTICEYCOUNT, lattice.fYCount);
    if (nullptr != lattice.fBounds) {
        writer.appendName(DEBUGCANVAS_ATTRIBUTE_BOUNDS);
        MakeJsonIRect(writer, *lattice.fBounds);
    }
    writer.beginArray(DEBUGCANVAS_ATTRIBUTE_LATTICEXDIVS);
    for (int i = 0; i < lattice.fXCount; i++) {
        writer.appendS32(lattice.fXDivs[i]);
    }
    writer.endArray();  // xdivs
    writer.beginArray(DEBUGCANVAS_ATTRIBUTE_LATTICEYDIVS);
    for (int i = 0; i < lattice.fYCount; i++) {
        writer.appendS32(lattice.fYDivs[i]);
    }
    writer.endArray();  // ydivs
    if (nullptr != lattice.fRectTypes) {
        writer.beginArray(DEBUGCANVAS_ATTRIBUTE_LATTICEFLAGS);
        int flagCount = 0;
        for (int row = 0; row < lattice.fYCount + 1; row++) {
            writer.beginArray();
            for (int column = 0; column < lattice.fXCount + 1; column++) {
                writer.appendS32(lattice.fRectTypes[flagCount++]);
            }
            writer.endArray();  // row
        }
        writer.endArray();
    }
    writer.endObject();
}

ClearCommand::ClearCommand(SkColor color) : INHERITED(kClear_OpType) { fColor = color; }

void ClearCommand::execute(SkCanvas* canvas) const { canvas->clear(fColor); }

void ClearCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_COLOR);
    MakeJsonColor(writer, fColor);
}

ClipPathCommand::ClipPathCommand(const SkPath& path, SkClipOp op, bool doAA)
        : INHERITED(kClipPath_OpType) {
    fPath = path;
    fOp   = op;
    fDoAA = doAA;
}

void ClipPathCommand::execute(SkCanvas* canvas) const { canvas->clipPath(fPath, fOp, fDoAA); }

bool ClipPathCommand::render(SkCanvas* canvas) const {
    render_path(canvas, fPath);
    return true;
}

void ClipPathCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_PATH);
    MakeJsonPath(writer, fPath);
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_REGIONOP, regionop_name(fOp));
    writer.appendBool(DEBUGCANVAS_ATTRIBUTE_ANTIALIAS, fDoAA);
}

ClipRegionCommand::ClipRegionCommand(const SkRegion& region, SkClipOp op)
        : INHERITED(kClipRegion_OpType) {
    fRegion = region;
    fOp     = op;
}

void ClipRegionCommand::execute(SkCanvas* canvas) const { canvas->clipRegion(fRegion, fOp); }

void ClipRegionCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_REGION);
    MakeJsonRegion(writer, fRegion);
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_REGIONOP, regionop_name(fOp));
}

ClipRectCommand::ClipRectCommand(const SkRect& rect, SkClipOp op, bool doAA)
        : INHERITED(kClipRect_OpType) {
    fRect = rect;
    fOp   = op;
    fDoAA = doAA;
}

void ClipRectCommand::execute(SkCanvas* canvas) const { canvas->clipRect(fRect, fOp, fDoAA); }

void ClipRectCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_COORDS);
    MakeJsonRect(writer, fRect);
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_REGIONOP, regionop_name(fOp));
    writer.appendBool(DEBUGCANVAS_ATTRIBUTE_ANTIALIAS, fDoAA);

    SkString desc;
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_SHORTDESC, str_append(&desc, fRect)->c_str());
}

ClipRRectCommand::ClipRRectCommand(const SkRRect& rrect, SkClipOp op, bool doAA)
        : INHERITED(kClipRRect_OpType) {
    fRRect = rrect;
    fOp    = op;
    fDoAA  = doAA;
}

void ClipRRectCommand::execute(SkCanvas* canvas) const { canvas->clipRRect(fRRect, fOp, fDoAA); }

bool ClipRRectCommand::render(SkCanvas* canvas) const {
    render_rrect(canvas, fRRect);
    return true;
}

void ClipRRectCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_COORDS);
    make_json_rrect(writer, fRRect);
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_REGIONOP, regionop_name(fOp));
    writer.appendBool(DEBUGCANVAS_ATTRIBUTE_ANTIALIAS, fDoAA);
}

ConcatCommand::ConcatCommand(const SkMatrix& matrix) : INHERITED(kConcat_OpType) {
    fMatrix = matrix;
}

void ConcatCommand::execute(SkCanvas* canvas) const { canvas->concat(fMatrix); }

void ConcatCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_MATRIX);
    MakeJsonMatrix(writer, fMatrix);
}

////

DrawAnnotationCommand::DrawAnnotationCommand(const SkRect& rect,
                                             const char    key[],
                                             sk_sp<SkData> value)
        : INHERITED(kDrawAnnotation_OpType), fRect(rect), fKey(key), fValue(std::move(value)) {}

void DrawAnnotationCommand::execute(SkCanvas* canvas) const {
    canvas->drawAnnotation(fRect, fKey.c_str(), fValue);
}

void DrawAnnotationCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);

    writer.appendName(DEBUGCANVAS_ATTRIBUTE_COORDS);
    MakeJsonRect(writer, fRect);
    writer.appendString("key", fKey.c_str());
    if (fValue.get()) {
        // TODO: dump out the "value"
    }

    SkString desc;
    str_append(&desc, fRect)->appendf(" %s", fKey.c_str());
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_SHORTDESC, desc.c_str());
}

////

DrawBitmapCommand::DrawBitmapCommand(const SkBitmap& bitmap,
                                     SkScalar        left,
                                     SkScalar        top,
                                     const SkPaint*  paint)
        : INHERITED(kDrawBitmap_OpType), fBitmap(bitmap), fLeft(left), fTop(top), fPaint(paint) {}

void DrawBitmapCommand::execute(SkCanvas* canvas) const {
    canvas->drawBitmap(fBitmap, fLeft, fTop, fPaint.getMaybeNull());
}

bool DrawBitmapCommand::render(SkCanvas* canvas) const {
    render_bitmap(canvas, fBitmap);
    return true;
}

void DrawBitmapCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginObject(DEBUGCANVAS_ATTRIBUTE_BITMAP);
    flatten(fBitmap, writer, urlDataManager);
    writer.endObject();
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_COORDS);
    MakeJsonPoint(writer, fLeft, fTop);
    if (fPaint.isValid()) {
        writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }
}

DrawBitmapLatticeCommand::DrawBitmapLatticeCommand(const SkBitmap&          bitmap,
                                                   const SkCanvas::Lattice& lattice,
                                                   const SkRect&            dst,
                                                   const SkPaint*           paint)
        : INHERITED(kDrawBitmapLattice_OpType)
        , fBitmap(bitmap)
        , fLattice(lattice)
        , fDst(dst)
        , fPaint(paint) {}

void DrawBitmapLatticeCommand::execute(SkCanvas* canvas) const {
    canvas->drawBitmapLattice(fBitmap, fLattice, fDst, fPaint.getMaybeNull());
}

bool DrawBitmapLatticeCommand::render(SkCanvas* canvas) const {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clear(0xFFFFFFFF);

    xlate_and_scale_to_bounds(canvas, fDst);

    this->execute(canvas);
    return true;
}

void DrawBitmapLatticeCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginObject(DEBUGCANVAS_ATTRIBUTE_BITMAP);
    flatten(fBitmap, writer, urlDataManager);
    writer.endObject();  // bitmap

    writer.appendName(DEBUGCANVAS_ATTRIBUTE_LATTICE);
    MakeJsonLattice(writer, fLattice);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_DST);
    MakeJsonRect(writer, fDst);
    if (fPaint.isValid()) {
        writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }

    SkString desc;
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_SHORTDESC, str_append(&desc, fDst)->c_str());
}

DrawBitmapNineCommand::DrawBitmapNineCommand(const SkBitmap& bitmap,
                                             const SkIRect&  center,
                                             const SkRect&   dst,
                                             const SkPaint*  paint)
        : INHERITED(kDrawBitmapNine_OpType)
        , fBitmap(bitmap)
        , fCenter(center)
        , fDst(dst)
        , fPaint(paint) {}

void DrawBitmapNineCommand::execute(SkCanvas* canvas) const {
    canvas->drawBitmapNine(fBitmap, fCenter, fDst, fPaint.getMaybeNull());
}

bool DrawBitmapNineCommand::render(SkCanvas* canvas) const {
    SkRect tmp = SkRect::Make(fCenter);
    render_bitmap(canvas, fBitmap, &tmp);
    return true;
}

void DrawBitmapNineCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginObject(DEBUGCANVAS_ATTRIBUTE_BITMAP);
    flatten(fBitmap, writer, urlDataManager);
    writer.endObject();  // bitmap

    writer.appendName(DEBUGCANVAS_ATTRIBUTE_CENTER);
    MakeJsonIRect(writer, fCenter);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_DST);
    MakeJsonRect(writer, fDst);
    if (fPaint.isValid()) {
        writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }
}

DrawBitmapRectCommand::DrawBitmapRectCommand(const SkBitmap&             bitmap,
                                             const SkRect*               src,
                                             const SkRect&               dst,
                                             const SkPaint*              paint,
                                             SkCanvas::SrcRectConstraint constraint)
        : INHERITED(kDrawBitmapRect_OpType)
        , fBitmap(bitmap)
        , fSrc(src)
        , fDst(dst)
        , fPaint(paint)
        , fConstraint(constraint) {}

void DrawBitmapRectCommand::execute(SkCanvas* canvas) const {
    canvas->legacy_drawBitmapRect(
            fBitmap, fSrc.getMaybeNull(), fDst, fPaint.getMaybeNull(), fConstraint);
}

bool DrawBitmapRectCommand::render(SkCanvas* canvas) const {
    render_bitmap(canvas, fBitmap, fSrc.getMaybeNull());
    return true;
}

void DrawBitmapRectCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginObject(DEBUGCANVAS_ATTRIBUTE_BITMAP);
    flatten(fBitmap, writer, urlDataManager);
    writer.endObject();  // bitmap

    if (fSrc.isValid()) {
        writer.appendName(DEBUGCANVAS_ATTRIBUTE_SRC);
        MakeJsonRect(writer, *fSrc);
    }
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_DST);
    MakeJsonRect(writer, fDst);
    if (fPaint.isValid()) {
        writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }
    if (fConstraint == SkCanvas::kStrict_SrcRectConstraint) {
        writer.appendBool(DEBUGCANVAS_ATTRIBUTE_STRICT, true);
    }

    SkString desc;
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_SHORTDESC, str_append(&desc, fDst)->c_str());
}

DrawImageCommand::DrawImageCommand(const SkImage* image,
                                   SkScalar       left,
                                   SkScalar       top,
                                   const SkPaint* paint)
        : INHERITED(kDrawImage_OpType)
        , fImage(SkRef(image))
        , fLeft(left)
        , fTop(top)
        , fPaint(paint) {}

void DrawImageCommand::execute(SkCanvas* canvas) const {
    canvas->drawImage(fImage.get(), fLeft, fTop, fPaint.getMaybeNull());
}

bool DrawImageCommand::render(SkCanvas* canvas) const {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clear(0xFFFFFFFF);

    xlate_and_scale_to_bounds(
            canvas,
            SkRect::MakeXYWH(
                    fLeft, fTop, SkIntToScalar(fImage->width()), SkIntToScalar(fImage->height())));
    this->execute(canvas);
    return true;
}

void DrawImageCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginObject(DEBUGCANVAS_ATTRIBUTE_IMAGE);
    flatten(*fImage, writer, urlDataManager);
    writer.endObject();  // image

    writer.appendName(DEBUGCANVAS_ATTRIBUTE_COORDS);
    MakeJsonPoint(writer, fLeft, fTop);
    if (fPaint.isValid()) {
        writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }

    writer.appendU32(DEBUGCANVAS_ATTRIBUTE_UNIQUE_ID, fImage->uniqueID());
    writer.appendS32(DEBUGCANVAS_ATTRIBUTE_WIDTH, fImage->width());
    writer.appendS32(DEBUGCANVAS_ATTRIBUTE_HEIGHT, fImage->height());
    switch (fImage->alphaType()) {
        case kOpaque_SkAlphaType:
            writer.appendString(DEBUGCANVAS_ATTRIBUTE_ALPHA, DEBUGCANVAS_ALPHATYPE_OPAQUE);
            break;
        case kPremul_SkAlphaType:
            writer.appendString(DEBUGCANVAS_ATTRIBUTE_ALPHA, DEBUGCANVAS_ALPHATYPE_PREMUL);
            break;
        case kUnpremul_SkAlphaType:
            writer.appendString(DEBUGCANVAS_ATTRIBUTE_ALPHA, DEBUGCANVAS_ALPHATYPE_UNPREMUL);
            break;
        default:
            writer.appendString(DEBUGCANVAS_ATTRIBUTE_ALPHA, DEBUGCANVAS_ALPHATYPE_UNKNOWN);
            break;
    }
}

DrawImageLatticeCommand::DrawImageLatticeCommand(const SkImage*           image,
                                                 const SkCanvas::Lattice& lattice,
                                                 const SkRect&            dst,
                                                 const SkPaint*           paint)
        : INHERITED(kDrawImageLattice_OpType)
        , fImage(SkRef(image))
        , fLattice(lattice)
        , fDst(dst)
        , fPaint(paint) {}

void DrawImageLatticeCommand::execute(SkCanvas* canvas) const {
    canvas->drawImageLattice(fImage.get(), fLattice, fDst, fPaint.getMaybeNull());
}

bool DrawImageLatticeCommand::render(SkCanvas* canvas) const {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clear(0xFFFFFFFF);

    xlate_and_scale_to_bounds(canvas, fDst);

    this->execute(canvas);
    return true;
}

void DrawImageLatticeCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginObject(DEBUGCANVAS_ATTRIBUTE_IMAGE);
    flatten(*fImage, writer, urlDataManager);
    writer.endObject();  // image

    writer.appendName(DEBUGCANVAS_ATTRIBUTE_LATTICE);
    MakeJsonLattice(writer, fLattice);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_DST);
    MakeJsonRect(writer, fDst);
    if (fPaint.isValid()) {
        writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }

    SkString desc;
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_SHORTDESC, str_append(&desc, fDst)->c_str());
}

DrawImageRectCommand::DrawImageRectCommand(const SkImage*              image,
                                           const SkRect*               src,
                                           const SkRect&               dst,
                                           const SkPaint*              paint,
                                           SkCanvas::SrcRectConstraint constraint)
        : INHERITED(kDrawImageRect_OpType)
        , fImage(SkRef(image))
        , fSrc(src)
        , fDst(dst)
        , fPaint(paint)
        , fConstraint(constraint) {}

void DrawImageRectCommand::execute(SkCanvas* canvas) const {
    canvas->legacy_drawImageRect(
            fImage.get(), fSrc.getMaybeNull(), fDst, fPaint.getMaybeNull(), fConstraint);
}

bool DrawImageRectCommand::render(SkCanvas* canvas) const {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clear(0xFFFFFFFF);

    xlate_and_scale_to_bounds(canvas, fDst);

    this->execute(canvas);
    return true;
}

void DrawImageRectCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginObject(DEBUGCANVAS_ATTRIBUTE_IMAGE);
    flatten(*fImage, writer, urlDataManager);
    writer.endObject();  // image

    if (fSrc.isValid()) {
        writer.appendName(DEBUGCANVAS_ATTRIBUTE_SRC);
        MakeJsonRect(writer, *fSrc);
    }
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_DST);
    MakeJsonRect(writer, fDst);
    if (fPaint.isValid()) {
        writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }
    if (fConstraint == SkCanvas::kStrict_SrcRectConstraint) {
        writer.appendBool(DEBUGCANVAS_ATTRIBUTE_STRICT, true);
    }

    SkString desc;
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_SHORTDESC, str_append(&desc, fDst)->c_str());
}

DrawImageNineCommand::DrawImageNineCommand(const SkImage* image,
                                           const SkIRect& center,
                                           const SkRect&  dst,
                                           const SkPaint* paint)
        : INHERITED(kDrawImageNine_OpType)
        , fImage(SkRef(image))
        , fCenter(center)
        , fDst(dst)
        , fPaint(paint) {}

void DrawImageNineCommand::execute(SkCanvas* canvas) const {
    canvas->drawImageNine(fImage.get(), fCenter, fDst, fPaint.getMaybeNull());
}

bool DrawImageNineCommand::render(SkCanvas* canvas) const {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clear(0xFFFFFFFF);

    xlate_and_scale_to_bounds(canvas, fDst);

    this->execute(canvas);
    return true;
}

void DrawImageNineCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginObject(DEBUGCANVAS_ATTRIBUTE_IMAGE);
    flatten(*fImage, writer, urlDataManager);
    writer.endObject();  // image

    writer.appendName(DEBUGCANVAS_ATTRIBUTE_CENTER);
    MakeJsonIRect(writer, fCenter);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_DST);
    MakeJsonRect(writer, fDst);
    if (fPaint.isValid()) {
        writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }
}

DrawOvalCommand::DrawOvalCommand(const SkRect& oval, const SkPaint& paint)
        : INHERITED(kDrawOval_OpType) {
    fOval  = oval;
    fPaint = paint;
}

void DrawOvalCommand::execute(SkCanvas* canvas) const { canvas->drawOval(fOval, fPaint); }

bool DrawOvalCommand::render(SkCanvas* canvas) const {
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

void DrawOvalCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_COORDS);
    MakeJsonRect(writer, fOval);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
    MakeJsonPaint(writer, fPaint, urlDataManager);
}

DrawArcCommand::DrawArcCommand(const SkRect&  oval,
                               SkScalar       startAngle,
                               SkScalar       sweepAngle,
                               bool           useCenter,
                               const SkPaint& paint)
        : INHERITED(kDrawArc_OpType) {
    fOval       = oval;
    fStartAngle = startAngle;
    fSweepAngle = sweepAngle;
    fUseCenter  = useCenter;
    fPaint      = paint;
}

void DrawArcCommand::execute(SkCanvas* canvas) const {
    canvas->drawArc(fOval, fStartAngle, fSweepAngle, fUseCenter, fPaint);
}

bool DrawArcCommand::render(SkCanvas* canvas) const {
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

void DrawArcCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_COORDS);
    MakeJsonRect(writer, fOval);
    writer.appendFloat(DEBUGCANVAS_ATTRIBUTE_STARTANGLE, fStartAngle);
    writer.appendFloat(DEBUGCANVAS_ATTRIBUTE_SWEEPANGLE, fSweepAngle);
    writer.appendBool(DEBUGCANVAS_ATTRIBUTE_USECENTER, fUseCenter);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
    MakeJsonPaint(writer, fPaint, urlDataManager);
}

DrawPaintCommand::DrawPaintCommand(const SkPaint& paint) : INHERITED(kDrawPaint_OpType) {
    fPaint = paint;
}

void DrawPaintCommand::execute(SkCanvas* canvas) const { canvas->drawPaint(fPaint); }

bool DrawPaintCommand::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->drawPaint(fPaint);
    return true;
}

void DrawPaintCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
    MakeJsonPaint(writer, fPaint, urlDataManager);
}

DrawBehindCommand::DrawBehindCommand(const SkPaint& paint) : INHERITED(kDrawPaint_OpType) {
    fPaint = paint;
}

void DrawBehindCommand::execute(SkCanvas* canvas) const {
    SkCanvasPriv::DrawBehind(canvas, fPaint);
}

bool DrawBehindCommand::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    SkCanvasPriv::DrawBehind(canvas, fPaint);
    return true;
}

void DrawBehindCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
    MakeJsonPaint(writer, fPaint, urlDataManager);
}

DrawPathCommand::DrawPathCommand(const SkPath& path, const SkPaint& paint)
        : INHERITED(kDrawPath_OpType) {
    fPath  = path;
    fPaint = paint;
}

void DrawPathCommand::execute(SkCanvas* canvas) const { canvas->drawPath(fPath, fPaint); }

bool DrawPathCommand::render(SkCanvas* canvas) const {
    render_path(canvas, fPath);
    return true;
}

void DrawPathCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_PATH);
    MakeJsonPath(writer, fPath);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
    MakeJsonPaint(writer, fPaint, urlDataManager);
}

DrawRegionCommand::DrawRegionCommand(const SkRegion& region, const SkPaint& paint)
        : INHERITED(kDrawRegion_OpType) {
    fRegion = region;
    fPaint  = paint;
}

void DrawRegionCommand::execute(SkCanvas* canvas) const { canvas->drawRegion(fRegion, fPaint); }

bool DrawRegionCommand::render(SkCanvas* canvas) const {
    render_region(canvas, fRegion);
    return true;
}

void DrawRegionCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_REGION);
    MakeJsonRegion(writer, fRegion);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
    MakeJsonPaint(writer, fPaint, urlDataManager);
}

BeginDrawPictureCommand::BeginDrawPictureCommand(const SkPicture* picture,
                                                 const SkMatrix*  matrix,
                                                 const SkPaint*   paint)
        : INHERITED(kBeginDrawPicture_OpType)
        , fPicture(SkRef(picture))
        , fMatrix(matrix)
        , fPaint(paint) {}

void BeginDrawPictureCommand::execute(SkCanvas* canvas) const {
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

bool BeginDrawPictureCommand::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    xlate_and_scale_to_bounds(canvas, fPicture->cullRect());

    canvas->drawPicture(fPicture.get());

    canvas->restore();

    return true;
}

EndDrawPictureCommand::EndDrawPictureCommand(bool restore)
        : INHERITED(kEndDrawPicture_OpType), fRestore(restore) {}

void EndDrawPictureCommand::execute(SkCanvas* canvas) const {
    if (fRestore) {
        canvas->restore();
    }
}

DrawPointsCommand::DrawPointsCommand(SkCanvas::PointMode mode,
                                     size_t              count,
                                     const SkPoint       pts[],
                                     const SkPaint&      paint)
        : INHERITED(kDrawPoints_OpType), fMode(mode), fPts(pts, count), fPaint(paint) {}

void DrawPointsCommand::execute(SkCanvas* canvas) const {
    canvas->drawPoints(fMode, fPts.count(), fPts.begin(), fPaint);
}

bool DrawPointsCommand::render(SkCanvas* canvas) const {
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

void DrawPointsCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_MODE, pointmode_name(fMode));
    writer.beginArray(DEBUGCANVAS_ATTRIBUTE_POINTS);
    for (int i = 0; i < fPts.count(); i++) {
        MakeJsonPoint(writer, fPts[i]);
    }
    writer.endArray();  // points
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
    MakeJsonPaint(writer, fPaint, urlDataManager);
}

DrawTextBlobCommand::DrawTextBlobCommand(sk_sp<SkTextBlob> blob,
                                         SkScalar          x,
                                         SkScalar          y,
                                         const SkPaint&    paint)
        : INHERITED(kDrawTextBlob_OpType)
        , fBlob(std::move(blob))
        , fXPos(x)
        , fYPos(y)
        , fPaint(paint) {}

void DrawTextBlobCommand::execute(SkCanvas* canvas) const {
    canvas->drawTextBlob(fBlob, fXPos, fYPos, fPaint);
}

bool DrawTextBlobCommand::render(SkCanvas* canvas) const {
    canvas->clear(SK_ColorWHITE);
    canvas->save();

    SkRect bounds = fBlob->bounds().makeOffset(fXPos, fYPos);
    xlate_and_scale_to_bounds(canvas, bounds);

    canvas->drawTextBlob(fBlob, fXPos, fYPos, fPaint);

    canvas->restore();

    return true;
}

void DrawTextBlobCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginArray(DEBUGCANVAS_ATTRIBUTE_RUNS);
    SkTextBlobRunIterator iter(fBlob.get());
    while (!iter.done()) {
        writer.beginObject();  // run
        writer.beginArray(DEBUGCANVAS_ATTRIBUTE_GLYPHS);
        for (uint32_t i = 0; i < iter.glyphCount(); i++) {
            writer.appendU32(iter.glyphs()[i]);
        }
        writer.endArray();  // glyphs
        if (iter.positioning() != SkTextBlobRunIterator::kDefault_Positioning) {
            writer.beginArray(DEBUGCANVAS_ATTRIBUTE_POSITIONS);
            const SkScalar* iterPositions = iter.pos();
            for (uint32_t i = 0; i < iter.glyphCount(); i++) {
                switch (iter.positioning()) {
                    case SkTextBlobRunIterator::kFull_Positioning:
                        MakeJsonPoint(writer, iterPositions[i * 2], iterPositions[i * 2 + 1]);
                        break;
                    case SkTextBlobRunIterator::kHorizontal_Positioning:
                        writer.appendFloat(iterPositions[i]);
                        break;
                    case SkTextBlobRunIterator::kDefault_Positioning: break;
                    case SkTextBlobRunIterator::kRSXform_Positioning:
                        // TODO_RSXFORM_BLOB
                        break;
                }
            }
            writer.endArray();  // positions
        }
        writer.appendName(DEBUGCANVAS_ATTRIBUTE_FONT);
        MakeJsonFont(iter.font(), writer, urlDataManager);
        writer.appendName(DEBUGCANVAS_ATTRIBUTE_COORDS);
        MakeJsonPoint(writer, iter.offset());

        writer.endObject();  // run
        iter.next();
    }
    writer.endArray();  // runs
    writer.appendFloat(DEBUGCANVAS_ATTRIBUTE_X, fXPos);
    writer.appendFloat(DEBUGCANVAS_ATTRIBUTE_Y, fYPos);
    SkRect bounds = fBlob->bounds();
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_COORDS);
    MakeJsonRect(writer, bounds);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
    MakeJsonPaint(writer, fPaint, urlDataManager);

    SkString desc;
    // make the bounds local by applying the x,y
    bounds.offset(fXPos, fYPos);
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_SHORTDESC, str_append(&desc, bounds)->c_str());
}

DrawPatchCommand::DrawPatchCommand(const SkPoint  cubics[12],
                                   const SkColor  colors[4],
                                   const SkPoint  texCoords[4],
                                   SkBlendMode    bmode,
                                   const SkPaint& paint)
        : INHERITED(kDrawPatch_OpType), fBlendMode(bmode) {
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

void DrawPatchCommand::execute(SkCanvas* canvas) const {
    canvas->drawPatch(fCubics, fColorsPtr, fTexCoordsPtr, fBlendMode, fPaint);
}

void DrawPatchCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.beginArray(DEBUGCANVAS_ATTRIBUTE_CUBICS);
    for (int i = 0; i < 12; i++) {
        MakeJsonPoint(writer, fCubics[i]);
    }
    writer.endArray();  // cubics
    if (fColorsPtr != nullptr) {
        writer.beginArray(DEBUGCANVAS_ATTRIBUTE_COLORS);
        for (int i = 0; i < 4; i++) {
            MakeJsonColor(writer, fColorsPtr[i]);
        }
        writer.endArray();  // colors
    }
    if (fTexCoordsPtr != nullptr) {
        writer.beginArray(DEBUGCANVAS_ATTRIBUTE_TEXTURECOORDS);
        for (int i = 0; i < 4; i++) {
            MakeJsonPoint(writer, fTexCoords[i]);
        }
        writer.endArray();  // texCoords
    }
    // fBlendMode
}

DrawRectCommand::DrawRectCommand(const SkRect& rect, const SkPaint& paint)
        : INHERITED(kDrawRect_OpType) {
    fRect  = rect;
    fPaint = paint;
}

void DrawRectCommand::execute(SkCanvas* canvas) const { canvas->drawRect(fRect, fPaint); }

void DrawRectCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_COORDS);
    MakeJsonRect(writer, fRect);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
    MakeJsonPaint(writer, fPaint, urlDataManager);

    SkString desc;
    writer.appendString(DEBUGCANVAS_ATTRIBUTE_SHORTDESC, str_append(&desc, fRect)->c_str());
}

DrawRRectCommand::DrawRRectCommand(const SkRRect& rrect, const SkPaint& paint)
        : INHERITED(kDrawRRect_OpType) {
    fRRect = rrect;
    fPaint = paint;
}

void DrawRRectCommand::execute(SkCanvas* canvas) const { canvas->drawRRect(fRRect, fPaint); }

bool DrawRRectCommand::render(SkCanvas* canvas) const {
    render_rrect(canvas, fRRect);
    return true;
}

void DrawRRectCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_COORDS);
    make_json_rrect(writer, fRRect);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
    MakeJsonPaint(writer, fPaint, urlDataManager);
}

DrawDRRectCommand::DrawDRRectCommand(const SkRRect& outer,
                                     const SkRRect& inner,
                                     const SkPaint& paint)
        : INHERITED(kDrawDRRect_OpType) {
    fOuter = outer;
    fInner = inner;
    fPaint = paint;
}

void DrawDRRectCommand::execute(SkCanvas* canvas) const {
    canvas->drawDRRect(fOuter, fInner, fPaint);
}

bool DrawDRRectCommand::render(SkCanvas* canvas) const {
    render_drrect(canvas, fOuter, fInner);
    return true;
}

void DrawDRRectCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_OUTER);
    make_json_rrect(writer, fOuter);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_INNER);
    make_json_rrect(writer, fInner);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
    MakeJsonPaint(writer, fPaint, urlDataManager);
}

DrawShadowCommand::DrawShadowCommand(const SkPath& path, const SkDrawShadowRec& rec)
        : INHERITED(kDrawShadow_OpType) {
    fPath      = path;
    fShadowRec = rec;
}

void DrawShadowCommand::execute(SkCanvas* canvas) const {
    canvas->private_draw_shadow_rec(fPath, fShadowRec);
}

bool DrawShadowCommand::render(SkCanvas* canvas) const {
    render_shadow(canvas, fPath, fShadowRec);
    return true;
}

void DrawShadowCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);

    bool geometricOnly = SkToBool(fShadowRec.fFlags & SkShadowFlags::kGeometricOnly_ShadowFlag);
    bool transparentOccluder =
            SkToBool(fShadowRec.fFlags & SkShadowFlags::kTransparentOccluder_ShadowFlag);

    writer.appendName(DEBUGCANVAS_ATTRIBUTE_PATH);
    MakeJsonPath(writer, fPath);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_ZPLANE);
    MakeJsonPoint3(writer, fShadowRec.fZPlaneParams);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_LIGHTPOSITION);
    MakeJsonPoint3(writer, fShadowRec.fLightPos);
    writer.appendFloat(DEBUGCANVAS_ATTRIBUTE_LIGHTRADIUS, fShadowRec.fLightRadius);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_AMBIENTCOLOR);
    MakeJsonColor(writer, fShadowRec.fAmbientColor);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_SPOTCOLOR);
    MakeJsonColor(writer, fShadowRec.fSpotColor);
    store_bool(writer, DEBUGCANVAS_SHADOWFLAG_TRANSPARENT_OCC, transparentOccluder, false);
    store_bool(writer, DEBUGCANVAS_SHADOWFLAG_GEOMETRIC_ONLY, geometricOnly, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

DrawEdgeAAQuadCommand::DrawEdgeAAQuadCommand(const SkRect&         rect,
                                             const SkPoint         clip[],
                                             SkCanvas::QuadAAFlags aa,
                                             SkColor               color,
                                             SkBlendMode           mode)
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

void DrawEdgeAAQuadCommand::execute(SkCanvas* canvas) const {
    canvas->experimental_DrawEdgeAAQuad(fRect, fHasClip ? fClip : nullptr, fAA, fColor, fMode);
}

DrawEdgeAAImageSetCommand::DrawEdgeAAImageSetCommand(const SkCanvas::ImageSetEntry set[],
                                                     int                           count,
                                                     const SkPoint                 dstClips[],
                                                     const SkMatrix              preViewMatrices[],
                                                     const SkPaint*              paint,
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

void DrawEdgeAAImageSetCommand::execute(SkCanvas* canvas) const {
    canvas->experimental_DrawEdgeAAImageSet(fSet.get(),
                                            fCount,
                                            fDstClips.get(),
                                            fPreViewMatrices.get(),
                                            fPaint.getMaybeNull(),
                                            fConstraint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

DrawDrawableCommand::DrawDrawableCommand(SkDrawable* drawable, const SkMatrix* matrix)
        : INHERITED(kDrawDrawable_OpType), fDrawable(SkRef(drawable)), fMatrix(matrix) {}

void DrawDrawableCommand::execute(SkCanvas* canvas) const {
    canvas->drawDrawable(fDrawable.get(), fMatrix.getMaybeNull());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

DrawVerticesCommand::DrawVerticesCommand(sk_sp<SkVertices> vertices,
                                         SkBlendMode       bmode,
                                         const SkPaint&    paint)
        : INHERITED(kDrawVertices_OpType)
        , fVertices(std::move(vertices))
        , fBlendMode(bmode)
        , fPaint(paint) {}

void DrawVerticesCommand::execute(SkCanvas* canvas) const {
    canvas->drawVertices(fVertices, fBlendMode, fPaint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

DrawAtlasCommand::DrawAtlasCommand(const SkImage*  image,
                                   const SkRSXform xform[],
                                   const SkRect    tex[],
                                   const SkColor   colors[],
                                   int             count,
                                   SkBlendMode     bmode,
                                   const SkRect*   cull,
                                   const SkPaint*  paint)
        : INHERITED(kDrawAtlas_OpType)
        , fImage(SkRef(image))
        , fXform(xform, count)
        , fTex(tex, count)
        , fColors(colors, colors ? count : 0)
        , fBlendMode(bmode)
        , fCull(cull)
        , fPaint(paint) {}

void DrawAtlasCommand::execute(SkCanvas* canvas) const {
    canvas->drawAtlas(fImage.get(),
                      fXform.begin(),
                      fTex.begin(),
                      fColors.isEmpty() ? nullptr : fColors.begin(),
                      fXform.count(),
                      fBlendMode,
                      fCull.getMaybeNull(),
                      fPaint.getMaybeNull());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

RestoreCommand::RestoreCommand() : INHERITED(kRestore_OpType) {}

void RestoreCommand::execute(SkCanvas* canvas) const { canvas->restore(); }

SaveCommand::SaveCommand() : INHERITED(kSave_OpType) {}

void SaveCommand::execute(SkCanvas* canvas) const { canvas->save(); }

SaveLayerCommand::SaveLayerCommand(const SkCanvas::SaveLayerRec& rec)
        : INHERITED(kSaveLayer_OpType)
        , fBounds(rec.fBounds)
        , fPaint(rec.fPaint)
        , fBackdrop(SkSafeRef(rec.fBackdrop))
        , fSaveLayerFlags(rec.fSaveLayerFlags) {}

void SaveLayerCommand::execute(SkCanvas* canvas) const {
    canvas->saveLayer(
            SkCanvas::SaveLayerRec(fBounds.getMaybeNull(), fPaint.getMaybeNull(), fSaveLayerFlags));
}

void SaveLayerCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    if (fBounds.isValid()) {
        writer.appendName(DEBUGCANVAS_ATTRIBUTE_BOUNDS);
        MakeJsonRect(writer, *fBounds);
    }
    if (fPaint.isValid()) {
        writer.appendName(DEBUGCANVAS_ATTRIBUTE_PAINT);
        MakeJsonPaint(writer, *fPaint, urlDataManager);
    }
    if (fBackdrop != nullptr) {
        writer.beginObject(DEBUGCANVAS_ATTRIBUTE_BACKDROP);
        flatten(fBackdrop.get(), writer, urlDataManager);
        writer.endObject();  // backdrop
    }
    if (fSaveLayerFlags != 0) {
        SkDebugf("unsupported: saveLayer flags\n");
        SkASSERT(false);
    }
}

SetMatrixCommand::SetMatrixCommand(const SkMatrix& matrix) : INHERITED(kSetMatrix_OpType) {
    fMatrix = matrix;
}

void SetMatrixCommand::execute(SkCanvas* canvas) const { canvas->setMatrix(fMatrix); }

void SetMatrixCommand::toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const {
    INHERITED::toJSON(writer, urlDataManager);
    writer.appendName(DEBUGCANVAS_ATTRIBUTE_MATRIX);
    MakeJsonMatrix(writer, fMatrix);
}
