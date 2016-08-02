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

bool CmdHandler::canHandle(const char* method, const char* url) {
    const char* kBasePath = "/cmd";
    return 0 == strncmp(url, kBasePath, strlen(kBasePath));
}

int CmdHandler::handle(Request* request, MHD_Connection* connection,
                       const char* url, const char* method,
                       const char* upload_data, size_t* upload_data_size) {
    SkTArray<SkString> commands;
    SkStrSplit(url, "/", &commands);

    if (!request->hasPicture() || commands.count() > 3) {
        return MHD_NO;
    }

    // /cmd or /cmd/N
    if (0 == strcmp(method, MHD_HTTP_METHOD_GET)) {
        int n;
        if (commands.count() == 1) {
            n = request->getLastOp();
        } else {
            sscanf(commands[1].c_str(), "%d", &n);
        }

        sk_sp<SkData> data(request->getJsonOps(n));
        return SendData(connection, data.get(), "application/json");
    }

    // /cmd/N, for now only delete supported
    if (commands.count() == 2 && 0 == strcmp(method, MHD_HTTP_METHOD_DELETE)) {
        int n;
        sscanf(commands[1].c_str(), "%d", &n);
        request->fDebugCanvas->deleteDrawCommandAt(n);
        return SendOK(connection);
    }

    // /cmd/N/[0|1]
    if (commands.count() == 3 && 0 == strcmp(method, MHD_HTTP_METHOD_POST))  {
        int n, toggle;
        sscanf(commands[1].c_str(), "%d", &n);
        sscanf(commands[2].c_str(), "%d", &toggle);
        request->fDebugCanvas->toggleCommand(n, SkToBool(toggle));
        return SendOK(connection);
    }

    return MHD_NO;
}
