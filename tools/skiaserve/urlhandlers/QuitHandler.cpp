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

bool QuitHandler::canHandle(const char* method, const char* url) {
    const char* kBaseName = "/quitquitquit";
    return 0 == strcmp(method, MHD_HTTP_METHOD_GET) &&
           0 == strncmp(url, kBaseName, strlen(kBaseName));
}

int QuitHandler::handle(Request* request, MHD_Connection* connection,
                        const char* url, const char* method,
                        const char* upload_data, size_t* upload_data_size) {
    exit(0);
}
