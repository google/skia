/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/skiaserve/urlhandlers/UrlHandler.h"

#include "microhttpd.h"
#include "src/core/SkStringUtils.h"
#include "tools/skiaserve/Request.h"
#include "tools/skiaserve/Response.h"

using namespace skia_private;
using namespace Response;

bool OpBoundsHandler::canHandle(const char* method, const char* url) {
    static const char* kBasePath = "/gpuOpBounds/";
    return 0 == strcmp(method, MHD_HTTP_METHOD_POST) &&
           0 == strncmp(url, kBasePath, strlen(kBasePath));
}

int OpBoundsHandler::handle(Request* request, MHD_Connection* connection, const char* url,
                            const char* method, const char* upload_data, size_t* upload_data_size) {
    TArray<SkString> commands;
    SkStrSplit(url, "/", &commands);

    if (!request->hasPicture() || commands.size() != 2) {
        return MHD_NO;
    }

    int enabled;
    sscanf(commands[1].c_str(), "%d", &enabled);

    request->fDebugCanvas->setDrawGpuOpBounds(SkToBool(enabled));
    return SendOK(connection);
}
