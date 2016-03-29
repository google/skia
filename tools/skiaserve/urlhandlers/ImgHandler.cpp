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

bool ImgHandler::canHandle(const char* method, const char* url) {
    static const char* kBasePath = "/img";
    return 0 == strcmp(method, MHD_HTTP_METHOD_GET) &&
           0 == strncmp(url, kBasePath, strlen(kBasePath));
}

int ImgHandler::handle(Request* request, MHD_Connection* connection,
                       const char* url, const char* method,
                       const char* upload_data, size_t* upload_data_size) {
    SkTArray<SkString> commands;
    SkStrSplit(url, "/", &commands);

    if (!request->hasPicture() || commands.count() > 3) {
        return MHD_NO;
    }

    int n, m = -1;
    // /img or /img/N
    if (commands.count() == 1) {
        n = request->fDebugCanvas->getSize() - 1;
    } else if (commands.count() == 2) {
        sscanf(commands[1].c_str(), "%d", &n);
    } else {
        sscanf(commands[1].c_str(), "%d", &n);
        sscanf(commands[2].c_str(), "%d", &m);
    }

    SkAutoTUnref<SkData> data(request->drawToPng(n, m));
    return SendData(connection, data, "image/png");
}
