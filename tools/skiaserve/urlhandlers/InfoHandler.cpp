/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "UrlHandler.h"

#include "microhttpd.h"
#include "SkJSONCanvas.h"
#include "../Request.h"
#include "../Response.h"

using namespace Response;

bool InfoHandler::canHandle(const char* method, const char* url) {
    const char* kBaseName = "/info";
    return 0 == strcmp(method, MHD_HTTP_METHOD_GET) &&
           0 == strncmp(url, kBaseName, strlen(kBaseName));
}

int InfoHandler::handle(Request* request, MHD_Connection* connection,
                        const char* url, const char* method,
                        const char* upload_data, size_t* upload_data_size) {
    SkTArray<SkString> commands;
    SkStrSplit(url, "/", &commands);

    if (!request->fPicture.get() || commands.count() > 2) {
        return MHD_NO;
    }

    // drawTo
    SkAutoTUnref<SkSurface> surface(request->createCPUSurface());
    SkCanvas* canvas = surface->getCanvas();

    int n;
    // /info or /info/N
    if (commands.count() == 1) {
        n = request->fDebugCanvas->getSize() - 1;
    } else {
        sscanf(commands[1].c_str(), "%d", &n);
    }

    // TODO this is really slow and we should cache the matrix and clip
    request->fDebugCanvas->drawTo(canvas, n);

    // make some json
    SkMatrix vm = request->fDebugCanvas->getCurrentMatrix();
    SkIRect clip = request->fDebugCanvas->getCurrentClip();
    Json::Value info(Json::objectValue);
    info["ViewMatrix"] = SkJSONCanvas::MakeMatrix(vm);
    info["ClipRect"] = SkJSONCanvas::MakeIRect(clip);

    std::string json = Json::FastWriter().write(info);

    // We don't want the null terminator so strlen is correct
    SkAutoTUnref<SkData> data(SkData::NewWithCopy(json.c_str(), strlen(json.c_str())));
    return SendData(connection, data, "application/json");
}

