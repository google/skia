/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJSONRenderer.h"
#include "SkJSONCanvas.h"
#include "SkJSONCPP.h"
#include "SkPath.h"

namespace SkJSONRenderer {

class Renderer {
public:
    void getPaint(Json::Value& command, SkPaint* paint);

    void getRect(Json::Value& command, const char* name, SkRect* rect);

    void getRRect(Json::Value& command, const char* name, SkRRect* rrect);

    void processCommand(Json::Value& command, SkCanvas* target);

    void processMatrix(Json::Value& command, SkCanvas* target);

    void processSave(Json::Value& command, SkCanvas* target);

    void processRestore(Json::Value& command, SkCanvas* target);

    void processPaint(Json::Value& command, SkCanvas* target);

    void processRect(Json::Value& command, SkCanvas* target);

    void processRRect(Json::Value& command, SkCanvas* target);

    void processOval(Json::Value& command, SkCanvas* target);

    void processPath(Json::Value& command, SkCanvas* target);

    void processText(Json::Value& command, SkCanvas* target);

    void processPoints(Json::Value& command, SkCanvas* target);

    void processClipRect(Json::Value& command, SkCanvas* target);
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
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_POINTS)) {
        this->processPoints(command, target);
    }
    else if (!strcmp(name, SKJSONCANVAS_COMMAND_CLIPRECT)) {
        this->processClipRect(command, target);
    }
    else {
        SkDebugf("unsupported JSON command: %s\n", name);
    }
}

void Renderer::getPaint(Json::Value& command, SkPaint* result) {
    Json::Value jsonPaint = command[SKJSONCANVAS_ATTRIBUTE_PAINT];
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_COLOR)) {
        Json::Value color = jsonPaint[SKJSONCANVAS_ATTRIBUTE_COLOR];
        result->setColor(SkColorSetARGB(color[0].asInt(), color[1].asInt(), color[2].asInt(),
                         color[3].asInt()));
    }
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_STYLE)) {
        const char* style = jsonPaint[SKJSONCANVAS_ATTRIBUTE_STYLE].asCString();
        if (!strcmp(style, SKJSONCANVAS_STYLE_FILL)) {
            result->setStyle(SkPaint::kFill_Style);
        }
        else if (!strcmp(style, SKJSONCANVAS_STYLE_STROKE)) {
            result->setStyle(SkPaint::kStroke_Style);
        }
        else if (!strcmp(style, SKJSONCANVAS_STYLE_STROKEANDFILL)) {
            result->setStyle(SkPaint::kStrokeAndFill_Style);
        }
    }
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_STROKEWIDTH)) {
        float strokeWidth = jsonPaint[SKJSONCANVAS_ATTRIBUTE_STROKEWIDTH].asFloat();
        result->setStrokeWidth(strokeWidth);
    }
    if (jsonPaint.isMember(SKJSONCANVAS_ATTRIBUTE_ANTIALIAS)) {
        result->setAntiAlias(jsonPaint[SKJSONCANVAS_ATTRIBUTE_ANTIALIAS].asBool());
    }
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
    for (Json::ArrayIndex i = 0; i < jsonPath.size(); i++) {
        Json::Value verb = jsonPath[i];
        if (verb.isString()) {
            SkASSERT(!strcmp(verb.asCString(), SKJSONCANVAS_VERB_CLOSE));
            path.close();
        }
        else {
            if (verb.isMember(SKJSONCANVAS_VERB_MOVE)) {
                Json::Value move = verb[SKJSONCANVAS_VERB_MOVE];
                path.moveTo(move[0].asFloat(), move[1].asFloat());
            }
            else if (verb.isMember(SKJSONCANVAS_VERB_LINE)) {
                Json::Value line = verb[SKJSONCANVAS_VERB_LINE];
                path.lineTo(line[0].asFloat(), line[1].asFloat());
            }
            else if (verb.isMember(SKJSONCANVAS_VERB_QUAD)) {
                Json::Value quad = verb[SKJSONCANVAS_VERB_QUAD];
                path.quadTo(quad[0][0].asFloat(), quad[0][1].asFloat(),
                            quad[1][0].asFloat(), quad[1][1].asFloat());
            }
            else if (verb.isMember(SKJSONCANVAS_VERB_CUBIC)) {
                Json::Value cubic = verb[SKJSONCANVAS_VERB_CUBIC];
                path.cubicTo(cubic[0][0].asFloat(), cubic[0][1].asFloat(),
                             cubic[1][0].asFloat(), cubic[1][1].asFloat(),
                             cubic[2][0].asFloat(), cubic[2][1].asFloat());
            }
            else if (verb.isMember(SKJSONCANVAS_VERB_CONIC)) {
                Json::Value conic = verb[SKJSONCANVAS_VERB_CONIC];
                path.conicTo(conic[0][0].asFloat(), conic[0][1].asFloat(),
                             conic[1][0].asFloat(), conic[1][1].asFloat(),
                             conic[2].asFloat());
            }
            else {
                SkASSERT(false);
            }
        }
    }
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
    target->clipRect(rect);
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
