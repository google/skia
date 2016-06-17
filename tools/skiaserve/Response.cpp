/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Response.h"

#include "microhttpd.h"

#include "Request.h"

#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkString.h"

DEFINE_string(source, "https://debugger.skia.org", "Where to load the web UI from.");

static SkString generate_template(SkString source) {
    SkString debuggerTemplate;
    debuggerTemplate.appendf(
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <title>SkDebugger</title>\n"
        "    <meta charset=\"utf-8\" />\n"
        "    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=egde,chrome=1\">\n"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "    <script src=\"%s/res/js/core.js\" type=\"text/javascript\" charset=\"utf-8\"></script>\n"
        "    <link href=\"%s/res/vul/elements.html\" rel=\"import\" />\n"
        "    <link rel='shortcut icon' href='https://debugger.skia.org/res/img/favicon.ico' type='image/x-icon'/ >"
        "</head>\n"
        "<body class=\"fullbleed layout vertical\">\n"
            "  <debugger-app-sk>This is the app."
            "  </debugger-app-sk>\n"
        "</body>\n"
        "</html>", source.c_str(), source.c_str());
    return debuggerTemplate;
}

namespace Response {
// SendOK just sends an empty response with a 200 OK status code.
int SendOK(MHD_Connection* connection) {
    const char* data = "";

    MHD_Response* response = MHD_create_response_from_buffer(strlen(data),
                                                             (void*)data,
                                                             MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, 200, response);
    MHD_destroy_response(response);
    return ret;
}

int SendError(MHD_Connection* connection, const char* msg) {
    MHD_Response* response = MHD_create_response_from_buffer(strlen(msg),
                                                             (void*) msg,
                                                             MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, 500, response);
    MHD_destroy_response(response);
    return ret;
}

int SendData(MHD_Connection* connection, const SkData* data, const char* type,
             bool setContentDisposition, const char* dispositionString) {
    MHD_Response* response = MHD_create_response_from_buffer(data->size(),
                                                             const_cast<void*>(data->data()),
                                                             MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Content-Type", type);

    if (setContentDisposition) {
        MHD_add_response_header(response, "Content-Disposition", dispositionString);
    }

    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}

int SendTemplate(MHD_Connection* connection, bool redirect, const char* redirectUrl) {
    SkString debuggerTemplate = generate_template(SkString(FLAGS_source[0]));

    MHD_Response* response = MHD_create_response_from_buffer(
        debuggerTemplate.size(),
        (void*) const_cast<char*>(debuggerTemplate.c_str()),
        MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header (response, "Access-Control-Allow-Origin", "*");

    int status = MHD_HTTP_OK;

    if (redirect) {
        MHD_add_response_header (response, "Location", redirectUrl);
        status = MHD_HTTP_SEE_OTHER;
    }

    int ret = MHD_queue_response(connection, status, response);
    MHD_destroy_response(response);
    return ret;
}

} // namespace Response
