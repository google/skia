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

bool RootHandler::canHandle(const char* method, const char* url) {
    return 0 == strcmp(method, MHD_HTTP_METHOD_GET) &&
           0 == strcmp(url, "/");
}

int RootHandler::handle(Request* request, MHD_Connection* connection,
                        const char* url, const char* method,
                        const char* upload_data, size_t* upload_data_size) {
    return SendTemplate(connection);
}
