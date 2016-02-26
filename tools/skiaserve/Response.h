/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Reponse_DEFINED
#define Reponse_DEFINED

struct MHD_Connection;
struct Request;
class SkData;

namespace Response {
    // SendOK just sends an empty response with a 200 OK status code.
    int SendOK(MHD_Connection* connection);

    int SendError(MHD_Connection* connection, const char* msg);

    int SendData(MHD_Connection* connection, const SkData* data, const char* type,
                 bool setContentDisposition = false, const char* dispositionString = nullptr);

    int SendTemplate(MHD_Connection* connection, bool redirect = false,
                     const char* redirectUrl = nullptr);
}

#endif
