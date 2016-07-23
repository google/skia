/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColor.h"

struct MHD_Connection;
struct Request;

class UrlHandler {
public:
    virtual ~UrlHandler() {}
    virtual bool canHandle(const char* method, const char* url) = 0;
    virtual int handle(Request* request, MHD_Connection* connection,
                       const char* url, const char* method,
                       const char* upload_data, size_t* upload_data_size) = 0;
};

class CmdHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override;
    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override;
};

class ImgHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override;
    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override;
};

class BreakHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override;
    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override;
};

/**
   Updates the clip visualization alpha. On all subsequent /img requests, the clip will be drawn in
   black with the specified alpha. 0 = no visible clip, 255 = fully opaque clip.
 */
class ClipAlphaHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override;
    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override;
};

/**
   Controls whether GPU rendering is enabled. Posting to /enableGPU/1 turns GPU on, /enableGPU/0
   disables it.
 */
class EnableGPUHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override;
    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override;
};

class PostHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override;
    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override;
};

class DownloadHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override;
    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override;
};

class InfoHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override;
    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override;
};

class DataHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override;
    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override;
};

/*
 * Returns a json descripton of all the batches in the image
 */
class BatchesHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override;
    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override;
};

/*
 * Enables drawing of batch bounds
 */
class BatchBoundsHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override;
    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override;
};

class RootHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override;
    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override;
};

/**
 * Controls how rendering is performed (L32, S32, F16).
 * Posting to /colorMode/0 turns on L32, /colorMode/1 turns on sRGB,
 * /colorMode/2 turns on FP16.
 */
class ColorModeHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override;
    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override;
};

class QuitHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override;
    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override;
};

