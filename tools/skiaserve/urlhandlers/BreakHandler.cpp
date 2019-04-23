/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/skiaserve/urlhandlers/UrlHandler.h"

#include "microhttpd.h"
#include "tools/skiaserve/Request.h"
#include "tools/skiaserve/Response.h"

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

    SkDynamicMemoryWStream stream;
    SkJSONWriter writer(&stream, SkJSONWriter::Mode::kFast);
    writer.beginObject(); // root

    writer.appendName("startColor");
    DrawCommand::MakeJsonColor(writer, target);

    bool changed = false;
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
            writer.appendName("endColor");
            DrawCommand::MakeJsonColor(writer, current);
            writer.appendS32("endOp", index);
            changed = true;
            break;
        }
    }
    if (!changed) {
        writer.appendName("endColor");
        DrawCommand::MakeJsonColor(writer, target);
        writer.appendS32("endOp", n);
    }
    canvas->restoreToCount(saveCount);

    writer.endObject(); // root
    writer.flush();
    return SendData(connection, stream.detachAsData().get(), "application/json");
}
