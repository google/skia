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

bool SRGBModeHandler::canHandle(const char* method, const char* url) {
    static const char* kBasePath = "/srgbMode/";
    return 0 == strcmp(method, MHD_HTTP_METHOD_POST) &&
           0 == strncmp(url, kBasePath, strlen(kBasePath));
}

int SRGBModeHandler::handle(Request* request, MHD_Connection* connection,
                             const char* url, const char* method,
                             const char* upload_data, size_t* upload_data_size) {
    SkTArray<SkString> commands;
    SkStrSplit(url, "/", &commands);

    if (commands.count() != 2) {
        return MHD_NO;
    }

    int enable;
    if (1 != sscanf(commands[1].c_str(), "%d", &enable)) {
        return MHD_NO;
    }

    bool success = request->setSRGBMode(SkToBool(enable));
    if (!success) {
        return SendError(connection, "Unable to set requested mode");
    }
    return SendOK(connection);
}
