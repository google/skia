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

bool DownloadHandler::canHandle(const char* method, const char* url) {
    return 0 == strcmp(method, MHD_HTTP_METHOD_GET) &&
           0 == strcmp(url, "/download");
}

int DownloadHandler::handle(Request* request, MHD_Connection* connection,
                            const char* url, const char* method,
                            const char* upload_data, size_t* upload_data_size) {
    if (!request->hasPicture()) {
        return MHD_NO;
    }

    SkAutoTUnref<SkData> data(request->writeOutSkp());

    // TODO fancier name handling
    return SendData(connection, data, "application/octet-stream", true,
                    "attachment; filename=something.skp;");
}
