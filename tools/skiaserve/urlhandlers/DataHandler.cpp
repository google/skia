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

bool DataHandler::canHandle(const char* method, const char* url) {
    static const char* kBaseUrl = "/data";
    return 0 == strcmp(method, MHD_HTTP_METHOD_GET) &&
           0 == strncmp(url, kBaseUrl, strlen(kBaseUrl));
}

int DataHandler::handle(Request* request, MHD_Connection* connection,
                        const char* url, const char* method,
                        const char* upload_data, size_t* upload_data_size) {
    SkTArray<SkString> commands;
    SkStrSplit(url, "/", &commands);

    if (!request->hasPicture() || commands.count() != 2) {
        return MHD_NO;
    }

    sk_sp<UrlDataManager::UrlData> urlData(
        SkRef(request->fUrlDataManager.getDataFromUrl(SkString(url))));

    if (urlData) {
        return SendData(connection, urlData->fData.get(), urlData->fContentType.c_str());
    }
    return MHD_NO;
}
