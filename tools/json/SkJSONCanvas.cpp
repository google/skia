/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJSONCanvas.h"
#include "SkColorFilter.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkPaintDefaults.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkRRect.h"
#include "SkTextBlob.h"
#include "SkTextBlobRunIterator.h"
#include "SkTypeface.h"
#include "SkWriteBuffer.h"

SkJSONCanvas::SkJSONCanvas(int width, int height, SkWStream& out, bool sendBinaries) 
    : INHERITED(width, height)
    , fOut(out)
    , fRoot(Json::objectValue)
    , fCommands(Json::arrayValue) 
    , fSendBinaries(sendBinaries) {
    fRoot[SKJSONCANVAS_VERSION] = Json::Value(1);
}

void SkJSONCanvas::finish() {
    fRoot[SKJSONCANVAS_COMMANDS] = fCommands;
    fOut.writeText(Json::FastWriter().write(fRoot).c_str());
}

Json::Value SkJSONCanvas::makePoint(const SkPoint& point) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(point.x()));
    result.append(Json::Value(point.y()));
    return result;
}

Json::Value SkJSONCanvas::makePoint(SkScalar x, SkScalar y) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(x));
    result.append(Json::Value(y));
    return result;
}

Json::Value SkJSONCanvas::makeRect(const SkRect& rect) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(rect.left()));
    result.append(Json::Value(rect.top()));
    result.append(Json::Value(rect.right()));
    result.append(Json::Value(rect.bottom()));
    return result;
}

Json::Value SkJSONCanvas::makeRRect(const SkRRect& rrect) {
    Json::Value result(Json::arrayValue);
    result.append(this->makeRect(rrect.rect()));
    result.append(this->makePoint(rrect.radii(SkRRect::kUpperLeft_Corner)));
    result.append(this->makePoint(rrect.radii(SkRRect::kUpperRight_Corner)));
    result.append(this->makePoint(rrect.radii(SkRRect::kLowerRight_Corner)));
    result.append(this->makePoint(rrect.radii(SkRRect::kLowerLeft_Corner)));
    return result;
}

Json::Value SkJSONCanvas::makePath(const SkPath& path) {
    Json::Value result(Json::objectValue);
    switch (path.getFillType()) {
        case SkPath::kWinding_FillType:
            result[SKJSONCANVAS_ATTRIBUTE_FILLTYPE] = SKJSONCANVAS_FILLTYPE_WINDING;
            break;
        case SkPath::kEvenOdd_FillType:
            result[SKJSONCANVAS_ATTRIBUTE_FILLTYPE] = SKJSONCANVAS_FILLTYPE_EVENODD;
            break;
        case SkPath::kInverseWinding_FillType:
            result[SKJSONCANVAS_ATTRIBUTE_FILLTYPE] = SKJSONCANVAS_FILLTYPE_INVERSEWINDING;
            break;
        case SkPath::kInverseEvenOdd_FillType:
            result[SKJSONCANVAS_ATTRIBUTE_FILLTYPE] = SKJSONCANVAS_FILLTYPE_INVERSEEVENODD;
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
                line[SKJSONCANVAS_VERB_LINE] = this->makePoint(pts[1]);
                verbs.append(line);
                break;
            }
            case SkPath::kQuad_Verb: {
                Json::Value quad(Json::objectValue);
                Json::Value coords(Json::arrayValue);
                coords.append(this->makePoint(pts[1]));
                coords.append(this->makePoint(pts[2]));
                quad[SKJSONCANVAS_VERB_QUAD] = coords;
                verbs.append(quad);
                break;
            }
            case SkPath::kCubic_Verb: {
                Json::Value cubic(Json::objectValue);
                Json::Value coords(Json::arrayValue);
                coords.append(this->makePoint(pts[1]));
                coords.append(this->makePoint(pts[2]));
                coords.append(this->makePoint(pts[3]));
                cubic[SKJSONCANVAS_VERB_CUBIC] = coords;
                verbs.append(cubic);
                break;
            }
            case SkPath::kConic_Verb: {
                Json::Value conic(Json::objectValue);
                Json::Value coords(Json::arrayValue);
                coords.append(this->makePoint(pts[1]));
                coords.append(this->makePoint(pts[2]));
                coords.append(Json::Value(iter.conicWeight()));
                conic[SKJSONCANVAS_VERB_CONIC] = coords;
                verbs.append(conic);
                break;
            }
            case SkPath::kMove_Verb: {
                Json::Value move(Json::objectValue);
                move[SKJSONCANVAS_VERB_MOVE] = this->makePoint(pts[0]);
                verbs.append(move);
                break;
            }
            case SkPath::kClose_Verb:
                verbs.append(Json::Value(SKJSONCANVAS_VERB_CLOSE));
                break;
            case SkPath::kDone_Verb:
                break;
        }
    }
    result[SKJSONCANVAS_ATTRIBUTE_VERBS] = verbs;
    return result;
}

Json::Value SkJSONCanvas::makeRegion(const SkRegion& region) {
    return Json::Value("<unimplemented>");
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

static void encode_data(const void* data, size_t count, Json::Value* target) {
    // just use a brain-dead JSON array for now, switch to base64 or something else smarter down the
    // road
    for (size_t i = 0; i < count; i++) {
        target->append(((const uint8_t*)data)[i]);
    }
}

static void flatten(const SkFlattenable* flattenable, Json::Value* target, bool sendBinaries) {
    if (sendBinaries) {
        SkWriteBuffer buffer;
        flattenable->flatten(buffer);
        void* data = sk_malloc_throw(buffer.bytesWritten());
        buffer.writeToMemory(data);
        Json::Value bytes;
        encode_data(data, buffer.bytesWritten(), &bytes);
        Json::Value jsonFlattenable;
        jsonFlattenable[SKJSONCANVAS_ATTRIBUTE_NAME] = Json::Value(flattenable->getTypeName());
        jsonFlattenable[SKJSONCANVAS_ATTRIBUTE_BYTES] = bytes;
        (*target) = jsonFlattenable;
        free(data);
    }
    else {
        (*target)[SKJSONCANVAS_ATTRIBUTE_DESCRIPTION] = Json::Value(flattenable->getTypeName());
    }
}

static bool SK_WARN_UNUSED_RESULT flatten(const SkImage& image, Json::Value* target, 
                                          bool sendBinaries) {
    if (sendBinaries) {
        SkData* encoded = image.encode(SkImageEncoder::kPNG_Type, 100);
        if (encoded == nullptr) {
            // PNG encode doesn't necessarily support all color formats, convert to a different
            // format
            size_t rowBytes = 4 * image.width();
            void* buffer = sk_malloc_throw(rowBytes * image.height());
            SkImageInfo dstInfo = SkImageInfo::Make(image.width(), image.height(), 
                                                    kN32_SkColorType, kPremul_SkAlphaType);
            if (!image.readPixels(dstInfo, buffer, rowBytes, 0, 0)) {
                SkDebugf("readPixels failed\n");
                return false;
            }
            SkImage* converted = SkImage::NewRasterCopy(dstInfo, buffer, rowBytes);
            encoded = converted->encode(SkImageEncoder::kPNG_Type, 100);
            if (encoded == nullptr) {
                SkDebugf("image encode failed\n");
                return false;
            }
            free(converted);
            free(buffer);
        }
        Json::Value bytes;
        encode_data(encoded->data(), encoded->size(), &bytes);
        (*target)[SKJSONCANVAS_ATTRIBUTE_BYTES] = bytes;
        encoded->unref();
    }
    else {
        SkString description = SkStringPrintf("%dx%d pixel image", image.width(), image.height());
        (*target)[SKJSONCANVAS_ATTRIBUTE_DESCRIPTION] = Json::Value(description.c_str());
    }
    return true;
}

static const char* color_type_name(SkColorType colorType) {
    switch (colorType) {
        case kARGB_4444_SkColorType:
            return SKJSONCANVAS_COLORTYPE_ARGB4444;
        case kRGBA_8888_SkColorType:
            return SKJSONCANVAS_COLORTYPE_RGBA8888;
        case kBGRA_8888_SkColorType:
            return SKJSONCANVAS_COLORTYPE_BGRA8888;
        case kRGB_565_SkColorType:
            return SKJSONCANVAS_COLORTYPE_565;
        case kGray_8_SkColorType:
            return SKJSONCANVAS_COLORTYPE_GRAY8;
        case kIndex_8_SkColorType:
            return SKJSONCANVAS_COLORTYPE_INDEX8;
        case kAlpha_8_SkColorType:
            return SKJSONCANVAS_COLORTYPE_ALPHA8;
        default:
            SkASSERT(false);
            return SKJSONCANVAS_COLORTYPE_RGBA8888;
    }
}

static const char* alpha_type_name(SkAlphaType alphaType) {
    switch (alphaType) {
        case kOpaque_SkAlphaType:
            return SKJSONCANVAS_ALPHATYPE_OPAQUE;
        case kPremul_SkAlphaType:
            return SKJSONCANVAS_ALPHATYPE_PREMUL;
        case kUnpremul_SkAlphaType:
            return SKJSONCANVAS_ALPHATYPE_UNPREMUL;
        default:
            SkASSERT(false);
            return SKJSONCANVAS_ALPHATYPE_OPAQUE;
    }
}

static bool SK_WARN_UNUSED_RESULT flatten(const SkBitmap& bitmap, Json::Value* target, 
                                          bool sendBinaries) {
    bitmap.lockPixels();
    SkAutoTUnref<SkImage> image(SkImage::NewFromBitmap(bitmap));
    bitmap.unlockPixels();
    (*target)[SKJSONCANVAS_ATTRIBUTE_COLOR] = Json::Value(color_type_name(bitmap.colorType()));
    (*target)[SKJSONCANVAS_ATTRIBUTE_ALPHA] = Json::Value(alpha_type_name(bitmap.alphaType()));
    bool success = flatten(*image, target, sendBinaries);
    return success;
}

static void apply_paint_color(const SkPaint& paint, Json::Value* target) {
    SkColor color = paint.getColor();
    if (color != SK_ColorBLACK) {
        Json::Value colorValue(Json::arrayValue);
        colorValue.append(Json::Value(SkColorGetA(color)));
        colorValue.append(Json::Value(SkColorGetR(color)));
        colorValue.append(Json::Value(SkColorGetG(color)));
        colorValue.append(Json::Value(SkColorGetB(color)));
        (*target)[SKJSONCANVAS_ATTRIBUTE_COLOR] = colorValue;;
    }
}

static void apply_paint_style(const SkPaint& paint, Json::Value* target) {
    SkPaint::Style style = paint.getStyle();
    if (style != SkPaint::kFill_Style) {
        switch (style) {
            case SkPaint::kStroke_Style: {
                Json::Value stroke(SKJSONCANVAS_STYLE_STROKE);
                (*target)[SKJSONCANVAS_ATTRIBUTE_STYLE] = stroke;
                break;
            }
            case SkPaint::kStrokeAndFill_Style: {
                Json::Value strokeAndFill(SKJSONCANVAS_STYLE_STROKEANDFILL);
                (*target)[SKJSONCANVAS_ATTRIBUTE_STYLE] = strokeAndFill;
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
            case SkPaint::kButt_Cap: {
                (*target)[SKJSONCANVAS_ATTRIBUTE_CAP] = Json::Value(SKJSONCANVAS_CAP_BUTT);
                break;
            }
            case SkPaint::kRound_Cap: {
                (*target)[SKJSONCANVAS_ATTRIBUTE_CAP] = Json::Value(SKJSONCANVAS_CAP_ROUND);
                break;
            }
            case SkPaint::kSquare_Cap: {
                (*target)[SKJSONCANVAS_ATTRIBUTE_CAP] = Json::Value(SKJSONCANVAS_CAP_SQUARE);
                break;
            }
            default: SkASSERT(false);
        }
    }
}
static void apply_paint_maskfilter(const SkPaint& paint, Json::Value* target, bool sendBinaries) {
    SkMaskFilter* maskFilter = paint.getMaskFilter();
    if (maskFilter != nullptr) {
        SkMaskFilter::BlurRec blurRec;
        if (maskFilter->asABlur(&blurRec)) {
            Json::Value blur(Json::objectValue);
            blur[SKJSONCANVAS_ATTRIBUTE_SIGMA] = Json::Value(blurRec.fSigma);
            switch (blurRec.fStyle) {
                case SkBlurStyle::kNormal_SkBlurStyle:
                    blur[SKJSONCANVAS_ATTRIBUTE_STYLE] = Json::Value(SKJSONCANVAS_BLURSTYLE_NORMAL);
                    break;
                case SkBlurStyle::kSolid_SkBlurStyle:
                    blur[SKJSONCANVAS_ATTRIBUTE_STYLE] = Json::Value(SKJSONCANVAS_BLURSTYLE_SOLID);
                    break;
                case SkBlurStyle::kOuter_SkBlurStyle:
                    blur[SKJSONCANVAS_ATTRIBUTE_STYLE] = Json::Value(SKJSONCANVAS_BLURSTYLE_OUTER);
                    break;
                case SkBlurStyle::kInner_SkBlurStyle:
                    blur[SKJSONCANVAS_ATTRIBUTE_STYLE] = Json::Value(SKJSONCANVAS_BLURSTYLE_INNER);
                    break;
                default:
                    SkASSERT(false);
            }
            switch (blurRec.fQuality) {
                case SkBlurQuality::kLow_SkBlurQuality:
                    blur[SKJSONCANVAS_ATTRIBUTE_QUALITY] = Json::Value(SKJSONCANVAS_BLURQUALITY_LOW);
                    break;
                case SkBlurQuality::kHigh_SkBlurQuality:
                    blur[SKJSONCANVAS_ATTRIBUTE_QUALITY] = Json::Value(SKJSONCANVAS_BLURQUALITY_HIGH);
                    break;
                default:
                    SkASSERT(false);
            }
            (*target)[SKJSONCANVAS_ATTRIBUTE_BLUR] = blur;
        }
        else {
            Json::Value jsonMaskFilter;
            flatten(maskFilter, &jsonMaskFilter, sendBinaries);
            (*target)[SKJSONCANVAS_ATTRIBUTE_MASKFILTER] = jsonMaskFilter;
        }
    }
}

static void apply_paint_patheffect(const SkPaint& paint, Json::Value* target, bool sendBinaries) {
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
            free(dashInfo.fIntervals);
            dashing[SKJSONCANVAS_ATTRIBUTE_INTERVALS] = intervals;
            dashing[SKJSONCANVAS_ATTRIBUTE_PHASE] = dashInfo.fPhase;
            (*target)[SKJSONCANVAS_ATTRIBUTE_DASHING] = dashing;
        }
        else {
            Json::Value jsonPathEffect;
            flatten(pathEffect, &jsonPathEffect, sendBinaries);
            (*target)[SKJSONCANVAS_ATTRIBUTE_PATHEFFECT] = jsonPathEffect;
        }
    }
}
    
static void apply_paint_textalign(const SkPaint& paint, Json::Value* target) {
    SkPaint::Align textAlign = paint.getTextAlign();
    if (textAlign != SkPaint::kLeft_Align) {
        switch (textAlign) {
            case SkPaint::kCenter_Align: {
                (*target)[SKJSONCANVAS_ATTRIBUTE_TEXTALIGN] = SKJSONCANVAS_ALIGN_CENTER;
                break;
            }
            case SkPaint::kRight_Align: {
                (*target)[SKJSONCANVAS_ATTRIBUTE_TEXTALIGN] = SKJSONCANVAS_ALIGN_RIGHT;
                break;
            }
            default: SkASSERT(false);
        }
    }
}

static void apply_paint_typeface(const SkPaint& paint, Json::Value* target, 
                                 bool sendBinaries) {
    SkTypeface* typeface = paint.getTypeface();
    if (typeface != nullptr) {
        if (sendBinaries) {
            Json::Value jsonTypeface;
            SkDynamicMemoryWStream buffer;
            typeface->serialize(&buffer);
            void* data = sk_malloc_throw(buffer.bytesWritten());
            buffer.copyTo(data);
            Json::Value bytes;
            encode_data(data, buffer.bytesWritten(), &bytes);
            jsonTypeface[SKJSONCANVAS_ATTRIBUTE_BYTES] = bytes;
            free(data);
            (*target)[SKJSONCANVAS_ATTRIBUTE_TYPEFACE] = jsonTypeface;
        }
    }
}

static void apply_paint_shader(const SkPaint& paint, Json::Value* target, bool sendBinaries) {
    SkFlattenable* shader = paint.getShader();
    if (shader != nullptr) {
        Json::Value jsonShader;
        flatten(shader, &jsonShader, sendBinaries);
        (*target)[SKJSONCANVAS_ATTRIBUTE_SHADER] = jsonShader;
    }
}

static void apply_paint_xfermode(const SkPaint& paint, Json::Value* target, bool sendBinaries) {
    SkFlattenable* xfermode = paint.getXfermode();
    if (xfermode != nullptr) {
        Json::Value jsonXfermode;
        flatten(xfermode, &jsonXfermode, sendBinaries);
        (*target)[SKJSONCANVAS_ATTRIBUTE_XFERMODE] = jsonXfermode;
    }
}

static void apply_paint_imagefilter(const SkPaint& paint, Json::Value* target, bool sendBinaries) {
    SkFlattenable* imageFilter = paint.getImageFilter();
    if (imageFilter != nullptr) {
        Json::Value jsonImageFilter;
        flatten(imageFilter, &jsonImageFilter, sendBinaries);
        (*target)[SKJSONCANVAS_ATTRIBUTE_IMAGEFILTER] = jsonImageFilter;
    }
}

static void apply_paint_colorfilter(const SkPaint& paint, Json::Value* target, bool sendBinaries) {
    SkFlattenable* colorFilter = paint.getColorFilter();
    if (colorFilter != nullptr) {
        Json::Value jsonColorFilter;
        flatten(colorFilter, &jsonColorFilter, sendBinaries);
        (*target)[SKJSONCANVAS_ATTRIBUTE_COLORFILTER] = jsonColorFilter;
    }
}

Json::Value SkJSONCanvas::makePaint(const SkPaint& paint) {
    Json::Value result(Json::objectValue);
    store_scalar(&result, SKJSONCANVAS_ATTRIBUTE_STROKEWIDTH, paint.getStrokeWidth(), 0.0f);
    store_scalar(&result, SKJSONCANVAS_ATTRIBUTE_STROKEMITER, paint.getStrokeMiter(), 
                 SkPaintDefaults_MiterLimit);
    store_bool(&result, SKJSONCANVAS_ATTRIBUTE_ANTIALIAS, paint.isAntiAlias(), false);
    store_scalar(&result, SKJSONCANVAS_ATTRIBUTE_TEXTSIZE, paint.getTextSize(), 
                 SkPaintDefaults_TextSize);
    store_scalar(&result, SKJSONCANVAS_ATTRIBUTE_TEXTSCALEX, paint.getTextScaleX(), SK_Scalar1);
    store_scalar(&result, SKJSONCANVAS_ATTRIBUTE_TEXTSCALEX, paint.getTextSkewX(), 0.0f);
    apply_paint_color(paint, &result);
    apply_paint_style(paint, &result);
    apply_paint_cap(paint, &result);
    apply_paint_textalign(paint, &result);
    apply_paint_patheffect(paint, &result, fSendBinaries);
    apply_paint_maskfilter(paint, &result, fSendBinaries);
    apply_paint_shader(paint, &result, fSendBinaries);
    apply_paint_xfermode(paint, &result, fSendBinaries);
    apply_paint_imagefilter(paint, &result, fSendBinaries);
    apply_paint_colorfilter(paint, &result, fSendBinaries);
    apply_paint_typeface(paint, &result, fSendBinaries);
    return result;
}

Json::Value SkJSONCanvas::MakeIRect(const SkIRect& rect) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(rect.left()));
    result.append(Json::Value(rect.top()));
    result.append(Json::Value(rect.right()));
    result.append(Json::Value(rect.bottom()));
    return result;
}

Json::Value SkJSONCanvas::MakeMatrix(const SkMatrix& matrix) {
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

Json::Value SkJSONCanvas::makeRegionOp(SkRegion::Op op) {
    switch (op) {
        case SkRegion::kDifference_Op:
            return Json::Value(SKJSONCANVAS_REGIONOP_DIFFERENCE);
        case SkRegion::kIntersect_Op:
            return Json::Value(SKJSONCANVAS_REGIONOP_INTERSECT);
        case SkRegion::kUnion_Op:
            return Json::Value(SKJSONCANVAS_REGIONOP_UNION);
        case SkRegion::kXOR_Op:
            return Json::Value(SKJSONCANVAS_REGIONOP_XOR);
        case SkRegion::kReverseDifference_Op:
            return Json::Value(SKJSONCANVAS_REGIONOP_REVERSE_DIFFERENCE);
        case SkRegion::kReplace_Op:
            return Json::Value(SKJSONCANVAS_REGIONOP_REPLACE);
        default:
            SkASSERT(false);
            return Json::Value("<invalid region op>");
    };
}

Json::Value SkJSONCanvas::makePointMode(SkCanvas::PointMode mode) {
    switch (mode) {
        case SkCanvas::kPoints_PointMode:
            return Json::Value(SKJSONCANVAS_POINTMODE_POINTS);
        case SkCanvas::kLines_PointMode:
            return Json::Value(SKJSONCANVAS_POINTMODE_LINES);
        case SkCanvas::kPolygon_PointMode: 
            return Json::Value(SKJSONCANVAS_POINTMODE_POLYGON);
        default:
            SkASSERT(false);
            return Json::Value("<invalid point mode>");
    };
}

void SkJSONCanvas::didConcat(const SkMatrix& matrix) {
    Json::Value command(Json::objectValue);
    switch (matrix.getType()) {
        case SkMatrix::kTranslate_Mask:
            command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_TRANSLATE);
            command[SKJSONCANVAS_ATTRIBUTE_X] = Json::Value(matrix.get(SkMatrix::kMTransX));
            command[SKJSONCANVAS_ATTRIBUTE_Y] = Json::Value(matrix.get(SkMatrix::kMTransY));
            break;
        case SkMatrix::kScale_Mask:
            command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_SCALE);
            command[SKJSONCANVAS_ATTRIBUTE_X] = Json::Value(matrix.get(SkMatrix::kMScaleX));
            command[SKJSONCANVAS_ATTRIBUTE_Y] = Json::Value(matrix.get(SkMatrix::kMScaleY));
            break;
        default:
            this->didSetMatrix(this->getTotalMatrix());
            return;
    }
    fCommands.append(command);
}

void SkJSONCanvas::didSetMatrix(const SkMatrix& matrix) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_MATRIX);
    command[SKJSONCANVAS_ATTRIBUTE_MATRIX] = this->MakeMatrix(matrix);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawPaint(const SkPaint& paint) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_PAINT);
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_RECT);
    command[SKJSONCANVAS_ATTRIBUTE_COORDS] = this->makeRect(rect);
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawOval(const SkRect& rect, const SkPaint& paint) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_OVAL);
    command[SKJSONCANVAS_ATTRIBUTE_COORDS] = this->makeRect(rect);
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_RRECT);
    command[SKJSONCANVAS_ATTRIBUTE_COORDS] = this->makeRRect(rrect);
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_RRECT);
    command[SKJSONCANVAS_ATTRIBUTE_INNER] = this->makeRRect(inner);
    command[SKJSONCANVAS_ATTRIBUTE_OUTER] = this->makeRRect(outer);
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawPoints(SkCanvas::PointMode mode, size_t count, const SkPoint pts[], 
                                const SkPaint& paint) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_POINTS);
    command[SKJSONCANVAS_ATTRIBUTE_MODE] = this->makePointMode(mode);
    Json::Value points(Json::arrayValue);
    for (size_t i = 0; i < count; i++) {
        points.append(this->makePoint(pts[i]));
    }
    command[SKJSONCANVAS_ATTRIBUTE_POINTS] = points;
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawVertices(SkCanvas::VertexMode, int vertexCount, const SkPoint vertices[],
                                  const SkPoint texs[], const SkColor colors[], SkXfermode*,
                                  const uint16_t indices[], int indexCount, const SkPaint&) {
    SkDebugf("unsupported: drawVertices\n");
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_VERTICES);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[],
                               int count, SkXfermode::Mode, const SkRect* cull, const SkPaint*) {
    SkDebugf("unsupported: drawAtlas\n");
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_ATLAS);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_PATH);
    command[SKJSONCANVAS_ATTRIBUTE_PATH] = this->makePath(path);
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawImage(const SkImage* image, SkScalar dx, SkScalar dy, 
                               const SkPaint* paint) {
    Json::Value encoded;
    if (flatten(*image, &encoded, fSendBinaries)) {
        Json::Value command(Json::objectValue);
        command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_IMAGE);
        command[SKJSONCANVAS_ATTRIBUTE_IMAGE] = encoded;
        command[SKJSONCANVAS_ATTRIBUTE_COORDS] = this->makePoint(dx, dy);
        if (paint != nullptr) {
            command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(*paint);
        }
        fCommands.append(command);
    }
}

void SkJSONCanvas::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst, 
                                   const SkPaint* paint, SkCanvas::SrcRectConstraint constraint) {
    Json::Value encoded;
    if (flatten(*image, &encoded, fSendBinaries)) {
        Json::Value command(Json::objectValue);
        command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_IMAGERECT);
        command[SKJSONCANVAS_ATTRIBUTE_IMAGE] = encoded;
        if (src != nullptr) {
            command[SKJSONCANVAS_ATTRIBUTE_SRC] = this->makeRect(*src);
        }
        command[SKJSONCANVAS_ATTRIBUTE_DST] = this->makeRect(dst);
        if (paint != nullptr) {
            command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(*paint);
        }
        if (constraint == SkCanvas::kStrict_SrcRectConstraint) {
            command[SKJSONCANVAS_ATTRIBUTE_STRICT] = Json::Value(true);
        }
        fCommands.append(command);
    }
}

void SkJSONCanvas::onDrawImageNine(const SkImage*, const SkIRect& center, const SkRect& dst,
                                   const SkPaint*) {
    SkDebugf("unsupported: drawImageNine\n");
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_IMAGENINE);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawBitmap(const SkBitmap& bitmap, SkScalar dx, SkScalar dy, 
                                const SkPaint* paint) {
    Json::Value encoded;
    if (flatten(bitmap, &encoded, fSendBinaries)) {
        Json::Value command(Json::objectValue);
        command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_BITMAP);
        command[SKJSONCANVAS_ATTRIBUTE_BITMAP] = encoded;
        command[SKJSONCANVAS_ATTRIBUTE_COORDS] = this->makePoint(dx, dy);
        if (paint != nullptr) {
            command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(*paint);
        }
        fCommands.append(command);
    }
}

void SkJSONCanvas::onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst, 
                                   const SkPaint* paint, SkCanvas::SrcRectConstraint constraint) {
    Json::Value encoded;
    if (flatten(bitmap, &encoded, fSendBinaries)) {
        Json::Value command(Json::objectValue);
        command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_BITMAPRECT);
        command[SKJSONCANVAS_ATTRIBUTE_BITMAP] = encoded;
        if (src != nullptr) {
            command[SKJSONCANVAS_ATTRIBUTE_SRC] = this->makeRect(*src);
        }
        command[SKJSONCANVAS_ATTRIBUTE_DST] = this->makeRect(dst);
        if (paint != nullptr) {
            command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(*paint);
        }
        if (constraint == SkCanvas::kStrict_SrcRectConstraint) {
            command[SKJSONCANVAS_ATTRIBUTE_STRICT] = Json::Value(true);
        }
        fCommands.append(command);
    }
}

void SkJSONCanvas::onDrawBitmapNine(const SkBitmap&, const SkIRect& center, const SkRect& dst,
                                    const SkPaint*) {
    SkDebugf("unsupported: drawBitmapNine\n");
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_BITMAPNINE);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawText(const void* text, size_t byteLength, SkScalar x,
                              SkScalar y, const SkPaint& paint) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_TEXT);
    command[SKJSONCANVAS_ATTRIBUTE_TEXT] = Json::Value((const char*) text, 
                                                       ((const char*) text) + byteLength);
    command[SKJSONCANVAS_ATTRIBUTE_COORDS] = this->makePoint(x, y);
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawPosText(const void* text, size_t byteLength,
                                 const SkPoint pos[], const SkPaint& paint) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_POSTEXT);
    command[SKJSONCANVAS_ATTRIBUTE_TEXT] = Json::Value((const char*) text, 
                                                       ((const char*) text) + byteLength);
    Json::Value coords(Json::arrayValue);
    for (size_t i = 0; i < byteLength; i++) {
        coords.append(this->makePoint(pos[i]));
    }
    command[SKJSONCANVAS_ATTRIBUTE_COORDS] = coords;
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawPosTextH(const void* text, size_t byteLength,
                                  const SkScalar xpos[], SkScalar constY,
                                  const SkPaint& paint) {
    SkDebugf("unsupported: drawPosTextH\n");
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_POSTEXTH);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawTextOnPath(const void* text, size_t byteLength,
                                    const SkPath& path, const SkMatrix* matrix,
                                    const SkPaint& paint) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_TEXTONPATH);
    command[SKJSONCANVAS_ATTRIBUTE_TEXT] = Json::Value((const char*) text, 
                                                       ((const char*) text) + byteLength);
    command[SKJSONCANVAS_ATTRIBUTE_PATH] = this->makePath(path);
    if (matrix != nullptr) {
        command[SKJSONCANVAS_ATTRIBUTE_MATRIX] = this->MakeMatrix(*matrix);
    }
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                  const SkPaint& paint) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_TEXTBLOB);
    Json::Value runs(Json::arrayValue);
    SkTextBlobRunIterator iter(blob);
    while (!iter.done()) {
        Json::Value run(Json::objectValue);
        Json::Value jsonPositions(Json::arrayValue);
        Json::Value jsonGlyphs(Json::arrayValue);
        const SkScalar* iterPositions = iter.pos();
        const uint16_t* iterGlyphs = iter.glyphs();
        for (uint32_t i = 0; i < iter.glyphCount(); i++) {
            switch (iter.positioning()) {
                case SkTextBlob::kFull_Positioning:
                    jsonPositions.append(this->makePoint(iterPositions[i * 2],
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
            run[SKJSONCANVAS_ATTRIBUTE_POSITIONS] = jsonPositions;
        }
        run[SKJSONCANVAS_ATTRIBUTE_GLYPHS] = jsonGlyphs;
        SkPaint fontPaint;
        iter.applyFontToPaint(&fontPaint);
        run[SKJSONCANVAS_ATTRIBUTE_FONT] = this->makePaint(fontPaint);
        run[SKJSONCANVAS_ATTRIBUTE_COORDS] = this->makePoint(iter.offset());
        runs.append(run);
        iter.next();
    }
    command[SKJSONCANVAS_ATTRIBUTE_RUNS] = runs;
    command[SKJSONCANVAS_ATTRIBUTE_X] = Json::Value(x);
    command[SKJSONCANVAS_ATTRIBUTE_Y] = Json::Value(y);
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                               const SkPoint texCoords[4], SkXfermode* xmode, 
                               const SkPaint& paint) {
    SkDebugf("unsupported: drawPatch\n");
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_PATCH);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawDrawable(SkDrawable*, const SkMatrix*) {
    SkDebugf("unsupported: drawDrawable\n");
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_DRAWABLE);
    fCommands.append(command);
}

void SkJSONCanvas::onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_CLIPRECT);
    command[SKJSONCANVAS_ATTRIBUTE_COORDS] = this->makeRect(rect);
    command[SKJSONCANVAS_ATTRIBUTE_REGIONOP] = this->makeRegionOp(op);
    command[SKJSONCANVAS_ATTRIBUTE_ANTIALIAS] = (edgeStyle == SkCanvas::kSoft_ClipEdgeStyle);
    fCommands.append(command);
}

void SkJSONCanvas::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_CLIPRRECT);
    command[SKJSONCANVAS_ATTRIBUTE_COORDS] = this->makeRRect(rrect);
    command[SKJSONCANVAS_ATTRIBUTE_REGIONOP] = this->makeRegionOp(op);
    command[SKJSONCANVAS_ATTRIBUTE_ANTIALIAS] = (edgeStyle == SkCanvas::kSoft_ClipEdgeStyle);
    fCommands.append(command);
}

void SkJSONCanvas::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_CLIPPATH);
    command[SKJSONCANVAS_ATTRIBUTE_PATH] = this->makePath(path);
    command[SKJSONCANVAS_ATTRIBUTE_REGIONOP] = this->makeRegionOp(op);
    command[SKJSONCANVAS_ATTRIBUTE_ANTIALIAS] = (edgeStyle == SkCanvas::kSoft_ClipEdgeStyle);
    fCommands.append(command);
}

void SkJSONCanvas::onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_CLIPREGION);
    command[SKJSONCANVAS_ATTRIBUTE_REGION] = this->makeRegion(deviceRgn);
    command[SKJSONCANVAS_ATTRIBUTE_REGIONOP] = this->makeRegionOp(op);
    fCommands.append(command);
}

void SkJSONCanvas::willSave() {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_SAVE);
    fCommands.append(command);
}

void SkJSONCanvas::willRestore() {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_RESTORE);
    fCommands.append(command);
}

SkCanvas::SaveLayerStrategy SkJSONCanvas::getSaveLayerStrategy(const SaveLayerRec& rec) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_SAVELAYER);
    if (rec.fBounds != nullptr) {
        command[SKJSONCANVAS_ATTRIBUTE_BOUNDS] = this->makeRect(*rec.fBounds);
    }
    if (rec.fPaint != nullptr) {
        command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(*rec.fPaint);
    }
    if (rec.fBackdrop != nullptr) {
        Json::Value backdrop;
        flatten(rec.fBackdrop, &backdrop, fSendBinaries);
        command[SKJSONCANVAS_ATTRIBUTE_BACKDROP] = backdrop;
    }
    if (rec.fSaveLayerFlags != 0) {
        SkDebugf("unsupported: saveLayer flags\n");
    }
    fCommands.append(command);
    return this->INHERITED::getSaveLayerStrategy(rec);
}
