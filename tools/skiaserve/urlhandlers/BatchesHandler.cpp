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

bool BatchesHandler::canHandle(const char* method, const char* url) {
    const char* kBasePath = "/batches";
    return 0 == strncmp(url, kBasePath, strlen(kBasePath));
}

int BatchesHandler::handle(Request* request, MHD_Connection* connection,
                           const char* url, const char* method,
                           const char* upload_data, size_t* upload_data_size) {
    SkTArray<SkString> commands;
    SkStrSplit(url, "/", &commands);

    if (!request->hasPicture() || commands.count() > 1) {
        return MHD_NO;
    }

    // /batches
    if (0 == strcmp(method, MHD_HTTP_METHOD_GET)) {
        int n = request->getLastOp();

        SkAutoTUnref<SkData> data(request->getJsonBatchList(n));
        return SendData(connection, data, "application/json");
    }

    return MHD_NO;
}
