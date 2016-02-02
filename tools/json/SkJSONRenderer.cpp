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
#include "SkValidatingReadBuffer.h"

namespace SkJSONRenderer {

class Renderer {
public:
    void getPaint(Json::Value& command, SkPaint* paint);

    void getRect(Json::Value& command, const char* name, SkRect* rect);

    void getRRect(Json::Value& command, const char* name, SkRRect* rrect);

    void getPath(Json::Value& command, SkPath* path);

    SkRegion::Op getRegionOp(Json::Value& command);

    void processCommand(Json::Value& command, SkCanvas* target);

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
    if (!strcmp(name, SKJSONCANVAS_COMMAND_MATRIX)) {
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
        return nullptr;
    }
    void* data;
    int size = decode_data(jsonFlattenable[SKJSONCANVAS_ATTRIBUTE_BYTES], &data);
    SkValidatingReadBuffer buffer(data, size);
    SkFlattenable* result = factory(buffer);
    free(data);
    if (!buffer.isValid()) {
        return nullptr;
    }
    return result;
}

// caller is responsible for freeing return value
static SkBitmap* load_bitmap(Json::Value jsonBitmap) {
    if (!jsonBitmap.isMember(SKJSONCANVAS_ATTRIBUTE_BYTES)) {
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
        return bitmap;
    }
    SkDebugf("image decode failed");
    free(data);
    return nullptr;
}

static SkImage* load_image(Json::Value jsonImage) {
    SkBitmap* bitmap = load_bitmap(jsonImage);
    if (bitmap == nullptr) {
        return nullptr;
    }
    SkImage* result = SkImage::NewFromBitmap(*bitmap);
    free(bitmap);
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

void Renderer::getPaint(Json::Value& command, SkPaint* result) {
    Json::Value jsonPaint = command[SKJSONCANVAS_ATTRIBUTE_PAINT];
    apply_paint_color(jsonPaint, result);
    apply_paint_shader(jsonPaint, result);
    apply_paint_patheffect(jsonPaint, result);
    apply_paint_maskfilter(jsonPaint, result);
    apply_paint_xfermode(jsonPaint, result);
    apply_paint_style(jsonPaint, result);
    apply_paint_strokewidth(jsonPaint, result);
    apply_paint_strokemiter(jsonPaint, result);
    apply_paint_cap(jsonPaint, result);
    apply_paint_antialias(jsonPaint, result);
    apply_paint_blur(jsonPaint, result);
    apply_paint_dashing(jsonPaint, result);
    apply_paint_textalign(jsonPaint, result);
    apply_paint_textsize(jsonPaint, result);
    apply_paint_textscalex(jsonPaint, result);
    apply_paint_textskewx(jsonPaint, result);
}

void Renderer::getRect(Json::Value& command, const char* name, SkRect* result) {
    Json::Value rect = command[name];
    result->set(rect[0].asFloat(), rect[1].asFloat(), rect[2].asFloat(), rect[3].asFloat());
}

void Renderer::getRRect(Json::Value& command, const char* name, SkRRect* result) {
    Json::Value rrect = command[name];
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

void Renderer::getPath(Json::Value& command, SkPath* result) {
    Json::Value path = command[SKJSONCANVAS_ATTRIBUTE_PATH];
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

SkRegion::Op Renderer::getRegionOp(Json::Value& command) {
    const char* op = command[SKJSONCANVAS_ATTRIBUTE_REGIONOP].asCString();
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

void Renderer::processMatrix(Json::Value& command, SkCanvas* target) {
    Json::Value jsonMatrix = command[SKJSONCANVAS_ATTRIBUTE_MATRIX];
    SkMatrix matrix;
    SkScalar values[] = { 
        jsonMatrix[0][0].asFloat(), jsonMatrix[0][1].asFloat(), jsonMatrix[0][2].asFloat(),
        jsonMatrix[1][0].asFloat(), jsonMatrix[1][1].asFloat(), jsonMatrix[1][2].asFloat(),
        jsonMatrix[2][0].asFloat(), jsonMatrix[2][1].asFloat(), jsonMatrix[2][2].asFloat() 
    };
    matrix.set9(values);
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
        this->getRect(command, SKJSONCANVAS_ATTRIBUTE_BOUNDS, &bounds);
        rec.fBounds = &bounds;
    }
    SkPaint paint;
    if (command.isMember(SKJSONCANVAS_ATTRIBUTE_PAINT)) {
        this->getPaint(command, &paint);
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
    this->getPaint(command, &paint);
    target->drawPaint(paint);
}

void Renderer::processRect(Json::Value& command, SkCanvas* target) {
    SkRect rect;
    this->getRect(command, SKJSONCANVAS_ATTRIBUTE_COORDS, &rect);
    SkPaint paint;
    this->getPaint(command, &paint);
    target->drawRect(rect, paint);
}

void Renderer::processRRect(Json::Value& command, SkCanvas* target) {
    SkRRect rrect;
    this->getRRect(command, SKJSONCANVAS_ATTRIBUTE_COORDS, &rrect);
    SkPaint paint;
    this->getPaint(command, &paint);
    target->drawRRect(rrect, paint);
}

void Renderer::processOval(Json::Value& command, SkCanvas* target) {
    SkRect rect;
    this->getRect(command, SKJSONCANVAS_ATTRIBUTE_COORDS, &rect);
    SkPaint paint;
    this->getPaint(command, &paint);
    target->drawOval(rect, paint);
}

void Renderer::processPath(Json::Value& command, SkCanvas* target) {
    Json::Value jsonPath = command[SKJSONCANVAS_ATTRIBUTE_PATH];
    SkPath path;
    this->getPath(command, &path);
    SkPaint paint;
    this->getPaint(command, &paint);
    target->drawPath(path, paint);
}

void Renderer::processText(Json::Value& command, SkCanvas* target) {
    const char* text = command[SKJSONCANVAS_ATTRIBUTE_TEXT].asCString();
    SkPaint paint;
    this->getPaint(command, &paint);
    Json::Value coords = command[SKJSONCANVAS_ATTRIBUTE_COORDS];
    target->drawText(text, strlen(text), coords[0].asFloat(), coords[1].asFloat(), paint);
}

void Renderer::processPosText(Json::Value& command, SkCanvas* target) {
    const char* text = command[SKJSONCANVAS_ATTRIBUTE_TEXT].asCString();
    SkPaint paint;
    this->getPaint(command, &paint);
    Json::Value coords = command[SKJSONCANVAS_ATTRIBUTE_COORDS];
    int count = (int) coords.size();
    SkPoint* points = (SkPoint*) sk_malloc_throw(count * sizeof(SkPoint));
    for (int i = 0; i < count; i++) {
        points[i] = SkPoint::Make(coords[i][0].asFloat(), coords[i][1].asFloat());
    }
    target->drawPosText(text, strlen(text), points, paint);
    free(points);
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
    this->getPaint(command, &paint);
    target->drawPoints(mode, count, points, paint);
    free(points);
}

void Renderer::processClipRect(Json::Value& command, SkCanvas* target) {
    SkRect rect;
    this->getRect(command, SKJSONCANVAS_ATTRIBUTE_COORDS, &rect);
    target->clipRect(rect, this->getRegionOp(command), 
                     command[SKJSONCANVAS_ATTRIBUTE_ANTIALIAS].asBool());
}

void Renderer::processClipRRect(Json::Value& command, SkCanvas* target) {
    SkRRect rrect;
    this->getRRect(command, SKJSONCANVAS_ATTRIBUTE_COORDS, &rrect);
    target->clipRRect(rrect, this->getRegionOp(command), 
                      command[SKJSONCANVAS_ATTRIBUTE_ANTIALIAS].asBool());
}

void Renderer::processClipPath(Json::Value& command, SkCanvas* target) {
    SkPath path;
    this->getPath(command, &path);
    target->clipPath(path, this->getRegionOp(command), 
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
        this->getPaint(command, &paint);
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
    this->getRect(command, SKJSONCANVAS_ATTRIBUTE_DST, &dst);
    SkPaint* paintPtr;
    SkPaint paint;
    if (command.isMember(SKJSONCANVAS_ATTRIBUTE_PAINT)) {
        this->getPaint(command, &paint);
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
        this->getRect(command, SKJSONCANVAS_ATTRIBUTE_SRC, &src);
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
        this->getPaint(command, &paint);
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
    this->getRect(command, SKJSONCANVAS_ATTRIBUTE_DST, &dst);
    SkPaint* paintPtr;
    SkPaint paint;
    if (command.isMember(SKJSONCANVAS_ATTRIBUTE_PAINT)) {
        this->getPaint(command, &paint);
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
        this->getRect(command, SKJSONCANVAS_ATTRIBUTE_SRC, &src);
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
