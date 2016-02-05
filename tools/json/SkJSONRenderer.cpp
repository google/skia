/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJSONRenderer.h"

#include "SkBlurMaskFilter.h"
#include "SkDashPathEffect.h"
#include "SkJSONCanvas.h"
#include "SkJSONCPP.h"
#include "SkPath.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"
#include "SkValidatingReadBuffer.h"

namespace SkJSONRenderer {

class Renderer {
public:
    void getPaint(Json::Value& paint, SkPaint* result);

    void getRect(Json::Value& rect, SkRect* result);

    void getRRect(Json::Value& rrect, SkRRect* result);

    void getPath(Json::Value& path, SkPath* result);

    void getMatrix(Json::Value& matrix, SkMatrix* result);

    SkRegion::Op getRegionOp(Json::Value& op);

    void processCommand(Json::Value& command, SkCanvas* target);

    void processTranslate(Json::Value& command, SkCanvas* target);

    void processScale(Json::Value& command, SkCanvas* target);

    void processMatrix(Json::Value& command, SkCanvas* target);

    void processSave(Json::Value& command, SkCanvas* target);

    void processRestore(Json::Value& command, SkCanvas* target);

    void processSaveLayer(Json::Value& command, SkCanvas* target);

    void processPaint(Json::Value& command, SkCanvas* target);

    void processRect(Json::Value& command, SkCanvas* target);

    void processRRect(Json::Value& command, SkCanvas* target);

    void processOval(Json::Value& command, SkCanvas* target);

    void processPath(Json::Value& command, SkCanvas* target);

    void processText(Json::Value& command, SkCanvas* target);

    void processPosText(Json::Value& command, SkCanvas* target);

    void processTextOnPath(Json::Value& command, SkCanvas* target);

    void processTextBlob(Json::Value& command, SkCanvas* target);

    void processPoints(Json::Value& command, SkCanvas* target);

    void processImage(Json::Value& command, SkCanvas* target);

    void processImageRect(Json::Value& command, SkCanvas* target);

    void processBitmap(Json::Value& command, SkCanvas* target);

    void processBitmapRect(Json::Value& command, SkCanvas* target);

    void processClipRect(Json::Value& command, SkCanvas* target);
    
    void processClipRRect(Json::Value& command, SkCanvas* target);

    void processClipPath(Json::Value& command, SkCanvas* target);
};

void Renderer::processCommand(Json::Value& command, SkCanvas* target) {
    const char* name = command[SKJSONCANVAS_COMMAND].asCString();
    // TODO speed this up with a hash
    if (!strcmp(name, SKJSONCANVAS_COMMAND_TRANSLATE)) {
        this->processTranslate(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_SCALE)) {
        this->processScale(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_MATRIX)) {
        this->processMatrix(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_SAVE)) {
        this->processSave(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_RESTORE)) {
        this->processRestore(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_SAVELAYER)) {
        this->processSaveLayer(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_PAINT)) {
        this->processPaint(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_RECT)) {
        this->processRect(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_RRECT)) {
        this->processRRect(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_OVAL)) {
        this->processOval(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_PATH)) {
        this->processPath(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_TEXT)) {
        this->processText(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_POSTEXT)) {
        this->processPosText(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_TEXTONPATH)) {
        this->processTextOnPath(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_TEXTBLOB)) {
        this->processTextBlob(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_POINTS)) {
        this->processPoints(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_IMAGE)) {
        this->processImage(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_IMAGERECT)) {
        this->processImageRect(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_BITMAP)) {
        this->processBitmap(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_BITMAPRECT)) {
        this->processBitmapRect(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_CLIPRECT)) {
        this->processClipRect(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_CLIPRRECT)) {
        this->processClipRRect(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_CLIPPATH)) {
        this->processClipPath(command, target);
    }
    else {
        SkDebugf("unsupported JSON command: %s\n", name);
    }
}

static void apply_paint_color(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_COLOR)) {
        Json::Value color = jsonPaint[SKJSONCANVAS_ATTRIBUTE_COLOR];
        target->setColor(SkColorSetARGB(color[0].asInt(), color[1].asInt(), color[2].asInt(),
                         color[3].asInt()));
    }
}

// note that the caller is responsible for freeing the pointer
static Json::ArrayIndex decode_data(Json::Value bytes, void** target) {
    Json::ArrayIndex size = bytes.size();
    *target = sk_malloc_throw(size);
    for (Json::ArrayIndex i = 0; i < size; i++) {
        ((uint8_t*) *target)[i] = bytes[i].asInt();
    }
    return size;
}

static SkFlattenable* load_flattenable(Json::Value jsonFlattenable) {
    if (!jsonFlattenable.isMember(SKJSONCANVAS_ATTRIBUTE_NAME)) {
        return nullptr;
    }
    const char* name = jsonFlattenable[SKJSONCANVAS_ATTRIBUTE_NAME].asCString();
    SkFlattenable::Factory factory = SkFlattenable::NameToFactory(name);
    if (factory == nullptr) {
        SkDebugf("no factory for loading '%s'\n", name);
        return nullptr;
    }
    void* data;
    int size = decode_data(jsonFlattenable[SKJSONCANVAS_ATTRIBUTE_BYTES], &data);
    SkValidatingReadBuffer buffer(data, size);
    SkFlattenable* result = factory(buffer);
    free(data);
    if (!buffer.isValid()) {
        SkDebugf("invalid buffer loading flattenable\n");
        return nullptr;
    }
    return result;
}

static SkColorType colortype_from_name(const char* name) {
    if (!strcmp(name, SKJSONCANVAS_COLORTYPE_ARGB4444)) {
        return kARGB_4444_SkColorType;
    }
    else if (!strcmp(name, SKJSONCANVAS_COLORTYPE_RGBA8888)) {
        return kRGBA_8888_SkColorType;
    }
    else if (!strcmp(name, SKJSONCANVAS_COLORTYPE_BGRA8888)) {
        return kBGRA_8888_SkColorType;
    }
    else if (!strcmp(name, SKJSONCANVAS_COLORTYPE_565)) {
        return kRGB_565_SkColorType;
    }
    else if (!strcmp(name, SKJSONCANVAS_COLORTYPE_GRAY8)) {
        return kGray_8_SkColorType;
    }
    else if (!strcmp(name, SKJSONCANVAS_COLORTYPE_INDEX8)) {
        return kIndex_8_SkColorType;
    }
    else if (!strcmp(name, SKJSONCANVAS_COLORTYPE_ALPHA8)) {
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
    if (bitmap->copyTo(dst, colorType)) {
        delete bitmap;
        return dst;
    }
    SkASSERT(false);
    delete dst;
    return bitmap;
}

// caller is responsible for freeing return value
static SkBitmap* load_bitmap(const Json::Value& jsonBitmap) {
    if (!jsonBitmap.isMember(SKJSONCANVAS_ATTRIBUTE_BYTES)) {
        SkDebugf("invalid bitmap\n");
        return nullptr;
    }
    void* data;
    int size = decode_data(jsonBitmap[SKJSONCANVAS_ATTRIBUTE_BYTES], &data);
    SkMemoryStream stream(data, size);
    SkImageDecoder* decoder = SkImageDecoder::Factory(&stream);
    SkBitmap* bitmap = new SkBitmap();
    SkImageDecoder::Result result = decoder->decode(&stream, bitmap, 
                                                    SkImageDecoder::kDecodePixels_Mode);
    free(decoder);
    if (result != SkImageDecoder::kFailure) {
        free(data);
        if (jsonBitmap.isMember(SKJSONCANVAS_ATTRIBUTE_COLOR)) {
            const char* ctName = jsonBitmap[SKJSONCANVAS_ATTRIBUTE_COLOR].asCString();
            SkColorType ct = colortype_from_name(ctName);
            if (ct != kIndex_8_SkColorType) {
                bitmap = convert_colortype(bitmap, ct);
            }
        }
        return bitmap;
    }
    SkDebugf("image decode failed\n");
    free(data);
    return nullptr;
}

static SkImage* load_image(const Json::Value& jsonImage) {
    SkBitmap* bitmap = load_bitmap(jsonImage);
    if (bitmap == nullptr) {
        return nullptr;
    }
    SkImage* result = SkImage::NewFromBitmap(*bitmap);
    delete bitmap;
    return result;
}

static void apply_paint_shader(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_SHADER)) {
        Json::Value jsonShader = jsonPaint[SKJSONCANVAS_ATTRIBUTE_SHADER];
        SkShader* shader = (SkShader*) load_flattenable(jsonShader);
        if (shader != nullptr) {
            target->setShader(shader);
            shader->unref();
        }
    }
}

static void apply_paint_patheffect(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_PATHEFFECT)) {
        Json::Value jsonPathEffect = jsonPaint[SKJSONCANVAS_ATTRIBUTE_PATHEFFECT];
        SkPathEffect* pathEffect = (SkPathEffect*) load_flattenable(jsonPathEffect);
        if (pathEffect != nullptr) {
            target->setPathEffect(pathEffect);
            pathEffect->unref();
        }
    }
}

static void apply_paint_maskfilter(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_MASKFILTER)) {
        Json::Value jsonMaskFilter = jsonPaint[SKJSONCANVAS_ATTRIBUTE_MASKFILTER];
        SkMaskFilter* maskFilter = (SkMaskFilter*) load_flattenable(jsonMaskFilter);
        if (maskFilter != nullptr) {
            target->setMaskFilter(maskFilter);
            maskFilter->unref();
        }
    }
}

static void apply_paint_colorfilter(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_COLORFILTER)) {
        Json::Value jsonColorFilter = jsonPaint[SKJSONCANVAS_ATTRIBUTE_COLORFILTER];
        SkColorFilter* colorFilter = (SkColorFilter*) load_flattenable(jsonColorFilter);
        if (colorFilter != nullptr) {
            target->setColorFilter(colorFilter);
            colorFilter->unref();
        }
    }
}

static void apply_paint_xfermode(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_XFERMODE)) {
        Json::Value jsonXfermode = jsonPaint[SKJSONCANVAS_ATTRIBUTE_XFERMODE];
        SkXfermode* xfermode = (SkXfermode*) load_flattenable(jsonXfermode);
        if (xfermode != nullptr) {
            target->setXfermode(xfermode);
            xfermode->unref();
        }
    }
}

static void apply_paint_imagefilter(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_IMAGEFILTER)) {
        Json::Value jsonImageFilter = jsonPaint[SKJSONCANVAS_ATTRIBUTE_IMAGEFILTER];
        SkImageFilter* imageFilter = (SkImageFilter*) load_flattenable(jsonImageFilter);
        if (imageFilter != nullptr) {
            target->setImageFilter(imageFilter);
            imageFilter->unref();
        }
    }
}

static void apply_paint_style(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_STYLE)) {
        const char* style = jsonPaint[SKJSONCANVAS_ATTRIBUTE_STYLE].asCString();
        if (!strcmp(style, SKJSONCANVAS_STYLE_FILL)) {
            target->setStyle(SkPaint::kFill_Style);
        }
        else if (!strcmp(style, SKJSONCANVAS_STYLE_STROKE)) {
            target->setStyle(SkPaint::kStroke_Style);
        }
        else if (!strcmp(style, SKJSONCANVAS_STYLE_STROKEANDFILL)) {
            target->setStyle(SkPaint::kStrokeAndFill_Style);
        }
    }
}

static void apply_paint_strokewidth(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_STROKEWIDTH)) {
        float strokeWidth = jsonPaint[SKJSONCANVAS_ATTRIBUTE_STROKEWIDTH].asFloat();
        target->setStrokeWidth(strokeWidth);
    }    
}

static void apply_paint_strokemiter(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_STROKEMITER)) {
        float strokeMiter = jsonPaint[SKJSONCANVAS_ATTRIBUTE_STROKEMITER].asFloat();
        target->setStrokeMiter(strokeMiter);
    }    
}

static void apply_paint_cap(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_CAP)) {
        const char* cap = jsonPaint[SKJSONCANVAS_ATTRIBUTE_CAP].asCString();
        if (!strcmp(cap, SKJSONCANVAS_CAP_BUTT)) {
            target->setStrokeCap(SkPaint::kButt_Cap);
        }
        else if (!strcmp(cap, SKJSONCANVAS_CAP_ROUND)) {
            target->setStrokeCap(SkPaint::kRound_Cap);
        }
        else if (!strcmp(cap, SKJSONCANVAS_CAP_SQUARE)) {
            target->setStrokeCap(SkPaint::kSquare_Cap);
        }
    }
}

static void apply_paint_antialias(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_ANTIALIAS)) {
        target->setAntiAlias(jsonPaint[SKJSONCANVAS_ATTRIBUTE_ANTIALIAS].asBool());
    }
}

static void apply_paint_blur(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_BLUR)) {
        Json::Value blur = jsonPaint[SKJSONCANVAS_ATTRIBUTE_BLUR];
        SkScalar sigma = blur[SKJSONCANVAS_ATTRIBUTE_SIGMA].asFloat();
        SkBlurStyle style;
        const char* jsonStyle = blur[SKJSONCANVAS_ATTRIBUTE_STYLE].asCString();
        if (!strcmp(jsonStyle, SKJSONCANVAS_BLURSTYLE_NORMAL)) {
            style = SkBlurStyle::kNormal_SkBlurStyle;
        }
        else if (!strcmp(jsonStyle, SKJSONCANVAS_BLURSTYLE_SOLID)) {
            style = SkBlurStyle::kSolid_SkBlurStyle;
        }
        else if (!strcmp(jsonStyle, SKJSONCANVAS_BLURSTYLE_OUTER)) {
            style = SkBlurStyle::kOuter_SkBlurStyle;
        }
        else if (!strcmp(jsonStyle, SKJSONCANVAS_BLURSTYLE_INNER)) {
            style = SkBlurStyle::kInner_SkBlurStyle;
        }
        else {
            SkASSERT(false);
            style = SkBlurStyle::kNormal_SkBlurStyle;
        }
        SkBlurMaskFilter::BlurFlags flags;
        const char* jsonQuality = blur[SKJSONCANVAS_ATTRIBUTE_QUALITY].asCString();
        if (!strcmp(jsonQuality, SKJSONCANVAS_BLURQUALITY_LOW)) {
            flags = SkBlurMaskFilter::BlurFlags::kNone_BlurFlag;
        }
        else if (!strcmp(jsonQuality, SKJSONCANVAS_BLURQUALITY_HIGH)) {
            flags = SkBlurMaskFilter::BlurFlags::kHighQuality_BlurFlag;
        }
        else {
            SkASSERT(false);
            flags = SkBlurMaskFilter::BlurFlags::kNone_BlurFlag;
        }
        target->setMaskFilter(SkBlurMaskFilter::Create(style, sigma, flags));
    }
}

static void apply_paint_dashing(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_DASHING)) {
        Json::Value dash = jsonPaint[SKJSONCANVAS_ATTRIBUTE_DASHING];
        Json::Value jsonIntervals = dash[SKJSONCANVAS_ATTRIBUTE_INTERVALS];
        Json::ArrayIndex count = jsonIntervals.size();
        SkScalar* intervals = (SkScalar*) sk_malloc_throw(count * sizeof(SkScalar));
        for (Json::ArrayIndex i = 0; i < count; i++) {
            intervals[i] = jsonIntervals[i].asFloat();
        }
        SkScalar phase = dash[SKJSONCANVAS_ATTRIBUTE_PHASE].asFloat();
        target->setPathEffect(SkDashPathEffect::Create(intervals, count, phase));
        free(intervals);
    }
}

static void apply_paint_textalign(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_TEXTALIGN)) {
        SkPaint::Align textAlign;
        const char* jsonAlign = jsonPaint[SKJSONCANVAS_ATTRIBUTE_TEXTALIGN].asCString();
        if (!strcmp(jsonAlign, SKJSONCANVAS_ALIGN_LEFT)) {
            textAlign = SkPaint::kLeft_Align;
        }
        else if (!strcmp(jsonAlign, SKJSONCANVAS_ALIGN_CENTER)) {
            textAlign = SkPaint::kCenter_Align;
        }
        else if (!strcmp(jsonAlign, SKJSONCANVAS_ALIGN_RIGHT)) {
            textAlign = SkPaint::kRight_Align;
        }
        else {
            SkASSERT(false);
            textAlign = SkPaint::kLeft_Align;
        }
        target->setTextAlign(textAlign);
    }
}

static void apply_paint_textsize(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_TEXTSIZE)) {
        float textSize = jsonPaint[SKJSONCANVAS_ATTRIBUTE_TEXTSIZE].asFloat();
        target->setTextSize(textSize);
    }
}

static void apply_paint_textscalex(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_TEXTSCALEX)) {
        float textScaleX = jsonPaint[SKJSONCANVAS_ATTRIBUTE_TEXTSCALEX].asFloat();
        target->setTextScaleX(textScaleX);
    }
}

static void apply_paint_textskewx(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_TEXTSKEWX)) {
        float textSkewX = jsonPaint[SKJSONCANVAS_ATTRIBUTE_TEXTSKEWX].asFloat();
        target->setTextSkewX(textSkewX);
    }
}

static void apply_paint_typeface(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_TYPEFACE)) {
        Json::Value jsonTypeface = jsonPaint[SKJSONCANVAS_ATTRIBUTE_TYPEFACE];
        Json::Value bytes = jsonTypeface[SKJSONCANVAS_ATTRIBUTE_BYTES];
        void* data;
        Json::ArrayIndex length = decode_data(bytes, &data);
        SkMemoryStream buffer(data, length);
        SkTypeface* typeface = SkTypeface::Deserialize(&buffer);
        free(data);
        target->setTypeface(typeface);
    }
}

void Renderer::getPaint(Json::Value& paint, SkPaint* result) {
    apply_paint_color(paint, result);
    apply_paint_shader(paint, result);
    apply_paint_patheffect(paint, result);
    apply_paint_maskfilter(paint, result);
    apply_paint_colorfilter(paint, result);
    apply_paint_xfermode(paint, result);
    apply_paint_imagefilter(paint, result);
    apply_paint_style(paint, result);
    apply_paint_strokewidth(paint, result);
    apply_paint_strokemiter(paint, result);
    apply_paint_cap(paint, result);
    apply_paint_antialias(paint, result);
    apply_paint_blur(paint, result);
    apply_paint_dashing(paint, result);
    apply_paint_textalign(paint, result);
    apply_paint_textsize(paint, result);
    apply_paint_textscalex(paint, result);
    apply_paint_textskewx(paint, result);
    apply_paint_typeface(paint, result);
}

void Renderer::getRect(Json::Value& rect, SkRect* result) {
    result->set(rect[0].asFloat(), rect[1].asFloat(), rect[2].asFloat(), rect[3].asFloat());
}

void Renderer::getRRect(Json::Value& rrect, SkRRect* result) {
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

void Renderer::getMatrix(Json::Value& matrix, SkMatrix* result) {
    SkScalar values[] = { 
        matrix[0][0].asFloat(), matrix[0][1].asFloat(), matrix[0][2].asFloat(),
        matrix[1][0].asFloat(), matrix[1][1].asFloat(), matrix[1][2].asFloat(),
        matrix[2][0].asFloat(), matrix[2][1].asFloat(), matrix[2][2].asFloat() 
    };
    result->set9(values);
}

void Renderer::getPath(Json::Value& path, SkPath* result) {
    const char* fillType = path[SKJSONCANVAS_ATTRIBUTE_FILLTYPE].asCString();
    if (!strcmp(fillType, SKJSONCANVAS_FILLTYPE_WINDING)) {
        result->setFillType(SkPath::kWinding_FillType);
    }
    else if (!strcmp(fillType, SKJSONCANVAS_FILLTYPE_EVENODD)) {
        result->setFillType(SkPath::kEvenOdd_FillType);
    }
    else if (!strcmp(fillType, SKJSONCANVAS_FILLTYPE_INVERSEWINDING)) {
        result->setFillType(SkPath::kInverseWinding_FillType);
    }
    else if (!strcmp(fillType, SKJSONCANVAS_FILLTYPE_INVERSEEVENODD)) {
        result->setFillType(SkPath::kInverseEvenOdd_FillType);
    }
    Json::Value verbs = path[SKJSONCANVAS_ATTRIBUTE_VERBS];
    for (Json::ArrayIndex i = 0; i < verbs.size(); i++) {
        Json::Value verb = verbs[i];
        if (verb.isString()) {
            SkASSERT(!strcmp(verb.asCString(), SKJSONCANVAS_VERB_CLOSE));
            result->close();
        }
        else {
            if (verb.isMember(SKJSONCANVAS_VERB_MOVE)) {
                Json::Value move = verb[SKJSONCANVAS_VERB_MOVE];
                result->moveTo(move[0].asFloat(), move[1].asFloat());
            }
            else if (verb.isMember(SKJSONCANVAS_VERB_LINE)) {
                Json::Value line = verb[SKJSONCANVAS_VERB_LINE];
                result->lineTo(line[0].asFloat(), line[1].asFloat());
            }
            else if (verb.isMember(SKJSONCANVAS_VERB_QUAD)) {
                Json::Value quad = verb[SKJSONCANVAS_VERB_QUAD];
                result->quadTo(quad[0][0].asFloat(), quad[0][1].asFloat(),
                               quad[1][0].asFloat(), quad[1][1].asFloat());
            }
            else if (verb.isMember(SKJSONCANVAS_VERB_CUBIC)) {
                Json::Value cubic = verb[SKJSONCANVAS_VERB_CUBIC];
                result->cubicTo(cubic[0][0].asFloat(), cubic[0][1].asFloat(),
                                cubic[1][0].asFloat(), cubic[1][1].asFloat(),
                                cubic[2][0].asFloat(), cubic[2][1].asFloat());
            }
            else if (verb.isMember(SKJSONCANVAS_VERB_CONIC)) {
                Json::Value conic = verb[SKJSONCANVAS_VERB_CONIC];
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

SkRegion::Op Renderer::getRegionOp(Json::Value& jsonOp) {
    const char* op = jsonOp.asCString();
    if (!strcmp(op, SKJSONCANVAS_REGIONOP_DIFFERENCE)) {
        return SkRegion::kDifference_Op;
    }
    else if (!strcmp(op, SKJSONCANVAS_REGIONOP_INTERSECT)) {
        return SkRegion::kIntersect_Op;
    }
    else if (!strcmp(op, SKJSONCANVAS_REGIONOP_UNION)) {
        return SkRegion::kUnion_Op;
    }
    else if (!strcmp(op, SKJSONCANVAS_REGIONOP_XOR)) {
        return SkRegion::kXOR_Op;
    }
    else if (!strcmp(op, SKJSONCANVAS_REGIONOP_REVERSE_DIFFERENCE)) {
        return SkRegion::kReverseDifference_Op;
    }
    else if (!strcmp(op, SKJSONCANVAS_REGIONOP_REPLACE)) {
        return SkRegion::kReplace_Op;
    }
    SkASSERT(false);
    return SkRegion::kIntersect_Op;
}

void Renderer::processTranslate(Json::Value& command, SkCanvas* target) {
    target->translate(command[SKJSONCANVAS_ATTRIBUTE_X].asFloat(),
                      command[SKJSONCANVAS_ATTRIBUTE_Y].asFloat());
}

void Renderer::processScale(Json::Value& command, SkCanvas* target) {
    target->scale(command[SKJSONCANVAS_ATTRIBUTE_X].asFloat(),
                  command[SKJSONCANVAS_ATTRIBUTE_Y].asFloat());
}

void Renderer::processMatrix(Json::Value& command, SkCanvas* target) {
    SkMatrix matrix;
    this->getMatrix(command[SKJSONCANVAS_ATTRIBUTE_MATRIX], &matrix);
    target->setMatrix(matrix);
}

void Renderer::processSave(Json::Value& command, SkCanvas* target) {
    target->save();
}

void Renderer::processRestore(Json::Value& command, SkCanvas* target) {
    target->restore();
}

void Renderer::processSaveLayer(Json::Value& command, SkCanvas* target) {
    SkCanvas::SaveLayerRec rec;
    SkRect bounds;
    if (command.isMember(SKJSONCANVAS_ATTRIBUTE_BOUNDS)) {
        this->getRect(command[SKJSONCANVAS_ATTRIBUTE_BOUNDS], &bounds);
        rec.fBounds = &bounds;
    }
    SkPaint paint;
    if (command.isMember(SKJSONCANVAS_ATTRIBUTE_PAINT)) {
        this->getPaint(command[SKJSONCANVAS_ATTRIBUTE_PAINT], &paint);
        rec.fPaint = &paint;
    }
    if (command.isMember(SKJSONCANVAS_ATTRIBUTE_BACKDROP)) {
        rec.fBackdrop = (SkImageFilter*) load_flattenable(command[SKJSONCANVAS_ATTRIBUTE_BACKDROP]);
    }
    target->saveLayer(rec);
    if (rec.fBackdrop != nullptr) {
        rec.fBackdrop->unref();
    }
}

void Renderer::processPaint(Json::Value& command, SkCanvas* target) {
    SkPaint paint;
    this->getPaint(command[SKJSONCANVAS_ATTRIBUTE_PAINT], &paint);
    target->drawPaint(paint);
}

void Renderer::processRect(Json::Value& command, SkCanvas* target) {
    SkRect rect;
    this->getRect(command[SKJSONCANVAS_ATTRIBUTE_COORDS], &rect);
    SkPaint paint;
    this->getPaint(command[SKJSONCANVAS_ATTRIBUTE_PAINT], &paint);
    target->drawRect(rect, paint);
}

void Renderer::processRRect(Json::Value& command, SkCanvas* target) {
    SkRRect rrect;
    this->getRRect(command[SKJSONCANVAS_ATTRIBUTE_COORDS], &rrect);
    SkPaint paint;
    this->getPaint(command[SKJSONCANVAS_ATTRIBUTE_PAINT], &paint);
    target->drawRRect(rrect, paint);
}

void Renderer::processOval(Json::Value& command, SkCanvas* target) {
    SkRect rect;
    this->getRect(command[SKJSONCANVAS_ATTRIBUTE_COORDS], &rect);
    SkPaint paint;
    this->getPaint(command[SKJSONCANVAS_ATTRIBUTE_PAINT], &paint);
    target->drawOval(rect, paint);
}

void Renderer::processPath(Json::Value& command, SkCanvas* target) {
    SkPath path;
    this->getPath(command[SKJSONCANVAS_ATTRIBUTE_PATH], &path);
    SkPaint paint;
    this->getPaint(command[SKJSONCANVAS_ATTRIBUTE_PAINT], &paint);
    target->drawPath(path, paint);
}

void Renderer::processText(Json::Value& command, SkCanvas* target) {
    const char* text = command[SKJSONCANVAS_ATTRIBUTE_TEXT].asCString();
    SkPaint paint;
    this->getPaint(command[SKJSONCANVAS_ATTRIBUTE_PAINT], &paint);
    Json::Value coords = command[SKJSONCANVAS_ATTRIBUTE_COORDS];
    target->drawText(text, strlen(text), coords[0].asFloat(), coords[1].asFloat(), paint);
}

void Renderer::processPosText(Json::Value& command, SkCanvas* target) {
    const char* text = command[SKJSONCANVAS_ATTRIBUTE_TEXT].asCString();
    SkPaint paint;
    this->getPaint(command[SKJSONCANVAS_ATTRIBUTE_PAINT], &paint);
    Json::Value coords = command[SKJSONCANVAS_ATTRIBUTE_COORDS];
    int count = (int) coords.size();
    SkPoint* points = (SkPoint*) sk_malloc_throw(count * sizeof(SkPoint));
    for (int i = 0; i < count; i++) {
        points[i] = SkPoint::Make(coords[i][0].asFloat(), coords[i][1].asFloat());
    }
    target->drawPosText(text, strlen(text), points, paint);
    free(points);
}

void Renderer::processTextOnPath(Json::Value& command, SkCanvas* target) {
    const char* text = command[SKJSONCANVAS_ATTRIBUTE_TEXT].asCString();
    SkPath path;
    this->getPath(command[SKJSONCANVAS_ATTRIBUTE_PATH], &path);
    SkMatrix* matrixPtr;
    SkMatrix matrix;
    if (command.isMember(SKJSONCANVAS_ATTRIBUTE_MATRIX)) {
        this->getMatrix(command[SKJSONCANVAS_ATTRIBUTE_MATRIX], &matrix);
        matrixPtr = &matrix;
    }
    else {
        matrixPtr = nullptr;
    }
    SkPaint paint;
    this->getPaint(command[SKJSONCANVAS_ATTRIBUTE_PAINT], &paint);
    target->drawTextOnPath(text, strlen(text), path, matrixPtr, paint);
}

void Renderer::processTextBlob(Json::Value& command, SkCanvas* target) {
    SkTextBlobBuilder builder;
    Json::Value runs = command[SKJSONCANVAS_ATTRIBUTE_RUNS];
    for (Json::ArrayIndex i = 0 ; i < runs.size(); i++) {
        Json::Value run = runs[i];
        SkPaint font;
        font.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        this->getPaint(run[SKJSONCANVAS_ATTRIBUTE_FONT], &font);
        Json::Value glyphs = run[SKJSONCANVAS_ATTRIBUTE_GLYPHS];
        int count = glyphs.size();
        Json::Value coords = run[SKJSONCANVAS_ATTRIBUTE_COORDS];
        SkScalar x = coords[0].asFloat();
        SkScalar y = coords[1].asFloat();
        if (run.isMember(SKJSONCANVAS_ATTRIBUTE_POSITIONS)) {
            Json::Value positions = run[SKJSONCANVAS_ATTRIBUTE_POSITIONS];
            if (positions.size() > 0 && positions[0].isNumeric()) {
                SkTextBlobBuilder::RunBuffer buffer = builder.allocRunPosH(font, count, y);
                for (int j = 0; j < count; j++) {
                    buffer.glyphs[j] = glyphs[j].asUInt();
                    buffer.pos[j] = positions[j].asFloat();
                }
            }
            else {
                SkTextBlobBuilder::RunBuffer buffer = builder.allocRunPos(font, count);
                for (int j = 0; j < count; j++) {
                    buffer.glyphs[j] = glyphs[j].asUInt();
                    buffer.pos[j * 2] = positions[j][0].asFloat();
                    buffer.pos[j * 2 + 1] = positions[j][1].asFloat();
                }
            }
        }
        else {
            SkTextBlobBuilder::RunBuffer buffer = builder.allocRun(font, count, x, y);
            for (int j = 0; j < count; j++) {
                buffer.glyphs[j] = glyphs[j].asUInt();
            }
        }
    }
    SkScalar x = command[SKJSONCANVAS_ATTRIBUTE_X].asFloat();
    SkScalar y = command[SKJSONCANVAS_ATTRIBUTE_Y].asFloat();
    SkPaint paint;
    this->getPaint(command[SKJSONCANVAS_ATTRIBUTE_PAINT], &paint);
    target->drawTextBlob(builder.build(), x, y, paint);
}

void Renderer::processPoints(Json::Value& command, SkCanvas* target) {
    SkCanvas::PointMode mode;
    const char* jsonMode = command[SKJSONCANVAS_ATTRIBUTE_MODE].asCString();
    if (!strcmp(jsonMode, SKJSONCANVAS_POINTMODE_POINTS)) {
        mode = SkCanvas::kPoints_PointMode;
    }
    else if (!strcmp(jsonMode, SKJSONCANVAS_POINTMODE_LINES)) {
        mode = SkCanvas::kLines_PointMode;
    }
    else if (!strcmp(jsonMode, SKJSONCANVAS_POINTMODE_POLYGON)) {
        mode = SkCanvas::kPolygon_PointMode;
    }
    else {
        SkASSERT(false);
        return;
    }
    Json::Value jsonPoints = command[SKJSONCANVAS_ATTRIBUTE_POINTS];
    int count = (int) jsonPoints.size();
    SkPoint* points = (SkPoint*) sk_malloc_throw(count * sizeof(SkPoint));
    for (int i = 0; i < count; i++) {
        points[i] = SkPoint::Make(jsonPoints[i][0].asFloat(), jsonPoints[i][1].asFloat());
    }
    SkPaint paint;
    this->getPaint(command[SKJSONCANVAS_ATTRIBUTE_PAINT], &paint);
    target->drawPoints(mode, count, points, paint);
    free(points);
}

void Renderer::processClipRect(Json::Value& command, SkCanvas* target) {
    SkRect rect;
    this->getRect(command[SKJSONCANVAS_ATTRIBUTE_COORDS], &rect);
    target->clipRect(rect, this->getRegionOp(command[SKJSONCANVAS_ATTRIBUTE_REGIONOP]), 
                     command[SKJSONCANVAS_ATTRIBUTE_ANTIALIAS].asBool());
}

void Renderer::processClipRRect(Json::Value& command, SkCanvas* target) {
    SkRRect rrect;
    this->getRRect(command[SKJSONCANVAS_ATTRIBUTE_COORDS], &rrect);
    target->clipRRect(rrect, this->getRegionOp(command[SKJSONCANVAS_ATTRIBUTE_REGIONOP]), 
                      command[SKJSONCANVAS_ATTRIBUTE_ANTIALIAS].asBool());
}

void Renderer::processClipPath(Json::Value& command, SkCanvas* target) {
    SkPath path;
    this->getPath(command[SKJSONCANVAS_ATTRIBUTE_PATH], &path);
    target->clipPath(path, this->getRegionOp(command[SKJSONCANVAS_ATTRIBUTE_REGIONOP]), 
                     command[SKJSONCANVAS_ATTRIBUTE_ANTIALIAS].asBool());
}

void Renderer::processImage(Json::Value& command, SkCanvas* target) {
    SkImage* image = load_image(command[SKJSONCANVAS_ATTRIBUTE_IMAGE]);
    if (image == nullptr) {
        return;
    }
    Json::Value point = command[SKJSONCANVAS_ATTRIBUTE_COORDS];
    SkPaint* paintPtr;
    SkPaint paint;
    if (command.isMember(SKJSONCANVAS_ATTRIBUTE_PAINT)) {
        this->getPaint(command[SKJSONCANVAS_ATTRIBUTE_PAINT], &paint);
        paintPtr = &paint;
    }
    else {
        paintPtr = nullptr;
    }
    target->drawImage(image, point[0].asFloat(), point[1].asFloat(), paintPtr);
    image->unref();
}

void Renderer::processImageRect(Json::Value& command, SkCanvas* target) {
    SkImage* image = load_image(command[SKJSONCANVAS_ATTRIBUTE_IMAGE]);
    if (image == nullptr) {
        return;
    }
    SkRect dst;
    this->getRect(command[SKJSONCANVAS_ATTRIBUTE_DST], &dst);
    SkPaint* paintPtr;
    SkPaint paint;
    if (command.isMember(SKJSONCANVAS_ATTRIBUTE_PAINT)) {
        this->getPaint(command[SKJSONCANVAS_ATTRIBUTE_PAINT], &paint);
        paintPtr = &paint;
    }
    else {
        paintPtr = nullptr;
    }
    SkCanvas::SrcRectConstraint constraint;
    if (command.isMember(SKJSONCANVAS_ATTRIBUTE_STRICT) && 
        command[SKJSONCANVAS_ATTRIBUTE_STRICT].asBool()) {
        constraint = SkCanvas::kStrict_SrcRectConstraint;
    }
    else {
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }
    if (command.isMember(SKJSONCANVAS_ATTRIBUTE_SRC)) {
        SkRect src;
        this->getRect(command[SKJSONCANVAS_ATTRIBUTE_SRC], &src);
        target->drawImageRect(image, src, dst, paintPtr, constraint);
    }
    else {
        target->drawImageRect(image, dst, paintPtr, constraint);
    }
    image->unref();
}

void Renderer::processBitmap(Json::Value& command, SkCanvas* target) {
    SkImage* image = load_image(command[SKJSONCANVAS_ATTRIBUTE_BITMAP]);
    if (image == nullptr) {
        return;
    }
    Json::Value point = command[SKJSONCANVAS_ATTRIBUTE_COORDS];
    SkPaint* paintPtr;
    SkPaint paint;
    if (command.isMember(SKJSONCANVAS_ATTRIBUTE_PAINT)) {
        this->getPaint(command[SKJSONCANVAS_ATTRIBUTE_PAINT], &paint);
        paintPtr = &paint;
    }
    else {
        paintPtr = nullptr;
    }
    target->drawImage(image, point[0].asFloat(), point[1].asFloat(), paintPtr);
    image->unref();
}

void Renderer::processBitmapRect(Json::Value& command, SkCanvas* target) {
    SkBitmap* bitmap = load_bitmap(command[SKJSONCANVAS_ATTRIBUTE_BITMAP]);
    if (bitmap == nullptr) {
        return;
    }
    SkRect dst;
    this->getRect(command[SKJSONCANVAS_ATTRIBUTE_DST], &dst);
    SkPaint* paintPtr;
    SkPaint paint;
    if (command.isMember(SKJSONCANVAS_ATTRIBUTE_PAINT)) {
        this->getPaint(command[SKJSONCANVAS_ATTRIBUTE_PAINT], &paint);
        paintPtr = &paint;
    }
    else {
        paintPtr = nullptr;
    }
    SkCanvas::SrcRectConstraint constraint;
    if (command.isMember(SKJSONCANVAS_ATTRIBUTE_STRICT) && 
        command[SKJSONCANVAS_ATTRIBUTE_STRICT].asBool()) {
        constraint = SkCanvas::kStrict_SrcRectConstraint;
    }
    else {
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }
    if (command.isMember(SKJSONCANVAS_ATTRIBUTE_SRC)) {
        SkRect src;
        this->getRect(command[SKJSONCANVAS_ATTRIBUTE_SRC], &src);
        target->drawBitmapRect(*bitmap, src, dst, paintPtr, constraint);
    }
    else {
        target->drawBitmapRect(*bitmap, dst, paintPtr, constraint);
    }
    free(bitmap);
}

void render(const char* json, SkCanvas* target) {
    Renderer renderer;
    Json::Reader reader;
    Json::Value root;
    if (reader.parse(std::string(json), root)) {
        SkASSERT(root[SKJSONCANVAS_VERSION].asInt() == 1);
        Json::Value commands = root[SKJSONCANVAS_COMMANDS];
        for (Json::ArrayIndex i = 0; i < commands.size(); i++) {
            renderer.processCommand(commands[i], target);
        }
    }
    else {
        SkDebugf(json);
        SkFAIL("json parse failure");
    }
}

} // namespace
