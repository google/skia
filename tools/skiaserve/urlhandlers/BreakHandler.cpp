/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "UrlHandler.h"

#include "microhttpd.h"
#include "../Request.h"
#include "../Response.h"

using namespace Response;

bool BreakHandler::canHandle(const char* method, const char* url) {
    static const char* kBasePath = "/break";
    return 0 == strcmp(method, MHD_HTTP_METHOD_GET) &&
           0 == strncmp(url, kBasePath, strlen(kBasePath));
}

int BreakHandler::handle(Request* request, MHD_Connection* connection,
                         const char* url, const char* method,
                         const char* upload_data, size_t* upload_data_size) {
    SkTArray<SkString> commands;
    SkStrSplit(url, "/", &commands);

    if (!request->hasPicture() || commands.count() != 4) {
        return MHD_NO;
    }

    // /break/<n>/<x>/<y>
    int n;
    sscanf(commands[1].c_str(), "%d", &n);
    int x;
    sscanf(commands[2].c_str(), "%d", &x);
    int y;
    sscanf(commands[3].c_str(), "%d", &y);

    int count = request->fDebugCanvas->getSize();
    SkASSERT(n < count);

    SkCanvas* canvas = request->getCanvas();
    canvas->clear(SK_ColorWHITE);
    int saveCount = canvas->save();
    for (int i = 0; i <= n; ++i) {
        request->fDebugCanvas->getDrawCommandAt(i)->execute(canvas);
    }
    SkColor target = request->getPixel(x, y);
    Json::Value response(Json::objectValue);
    Json::Value startColor(Json::arrayValue);
    startColor.append(Json::Value(SkColorGetR(target)));
    startColor.append(Json::Value(SkColorGetG(target)));
    startColor.append(Json::Value(SkColorGetB(target)));
    startColor.append(Json::Value(SkColorGetA(target)));
    response["startColor"] = startColor;
    response["endColor"] = startColor;
    response["endOp"] = Json::Value(n);
    for (int i = n + 1; i < n + count; ++i) {
        int index = i % count;
        if (index == 0) {
            // reset canvas for wraparound
            canvas->restoreToCount(saveCount);
            canvas->clear(SK_ColorWHITE);
            saveCount = canvas->save();
        }
        request->fDebugCanvas->getDrawCommandAt(index)->execute(canvas);
        SkColor current = request->getPixel(x, y);
        if (current != target) {
            Json::Value endColor(Json::arrayValue);
            endColor.append(Json::Value(SkColorGetR(current)));
            endColor.append(Json::Value(SkColorGetG(current)));
            endColor.append(Json::Value(SkColorGetB(current)));
            endColor.append(Json::Value(SkColorGetA(current)));
            response["endColor"] = endColor;
            response["endOp"] = Json::Value(index);
            break;
        }
    }
    canvas->restoreToCount(saveCount);
    SkDynamicMemoryWStream stream;
    stream.writeText(Json::FastWriter().write(response).c_str());
    SkAutoTUnref<SkData> data(stream.copyToData());
    return SendData(connection, data, "application/json");
}
