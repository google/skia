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

bool ColorModeHandler::canHandle(const char* method, const char* url) {
    static const char* kBasePath = "/colorMode/";
    return 0 == strcmp(method, MHD_HTTP_METHOD_POST) &&
           0 == strncmp(url, kBasePath, strlen(kBasePath));
}

int ColorModeHandler::handle(Request* request, MHD_Connection* connection,
                             const char* url, const char* method,
                             const char* upload_data, size_t* upload_data_size) {
    SkTArray<SkString> commands;
    SkStrSplit(url, "/", &commands);

    if (commands.count() != 2) {
        return MHD_NO;
    }

    int mode;
    if (1 != sscanf(commands[1].c_str(), "%d", &mode)) {
        return MHD_NO;
    }

    bool success = request->setColorMode(mode);
    if (!success) {
        return SendError(connection, "Unable to create requested surface");
    }
    return SendOK(connection);
}
