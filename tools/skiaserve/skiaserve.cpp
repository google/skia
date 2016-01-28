/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCaps.h"
#include "GrContextFactory.h"
#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkJSONCanvas.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkSurface.h"

#include <sys/socket.h>
#include <microhttpd.h>

// To get image decoders linked in we have to do the below magic
#include "SkForceLinking.h"
#include "SkImageDecoder.h"
__SK_FORCE_IMAGE_DECODER_LINKING;

// TODO make this configurable
#define PORT 8888

DEFINE_string(dir, "skps", "Directory to read skp.");
DEFINE_string(name, "desk_carsvg", "skp to load.");
DEFINE_bool(useTemplate, true, "whether or not to use the skdebugger template string.");

// TODO probably want to make this configurable
static const int kImageWidth = 1920;
static const int kImageHeight = 1080;

// TODO move to template file
SkString generateTemplate(SkString source) {
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
        "</head>\n"
        "<body class=\"fullbleed layout vertical\">\n"
            "  <debugger-app-sk>This is the app."
            "  </debugger-app-sk>\n"
        "</body>\n"
        "</html>", source.c_str(), source.c_str());
    return debuggerTemplate;

}

struct UploadContext {
    SkDynamicMemoryWStream fStream;
    MHD_PostProcessor* fPostProcessor;
    MHD_Connection* connection;
};

struct Request {
    Request() : fUploadContext(nullptr) {}
    UploadContext* fUploadContext;
    SkAutoTUnref<SkData> fPNG;
    SkAutoTUnref<SkPicture> fPicture;
};

// TODO factor this out into functions, also handle CPU path
bool setupAndDrawToCanvas(Request* request, SkString* error) {
    GrContextOptions grContextOpts;
    SkAutoTDelete<GrContextFactory> factory(new GrContextFactory(grContextOpts));

    GrContext* context = factory->get(GrContextFactory::kNative_GLContextType,
                                      GrContextFactory::kNone_GLContextOptions);
    int maxRTSize = context->caps()->maxRenderTargetSize();
    SkImageInfo info = SkImageInfo::Make(SkTMin(kImageWidth, maxRTSize),
                                         SkTMin(kImageHeight, maxRTSize),
                                         kN32_SkColorType, kPremul_SkAlphaType);
    uint32_t flags = 0;
    SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTarget(context,
                                                               SkSurface::kNo_Budgeted, info,
                                                               0, &props));
    SkASSERT(surface.get());

    SkGLContext* gl = factory->getContextInfo(GrContextFactory::kNative_GLContextType,
                                              GrContextFactory::kNone_GLContextOptions).fGLContext;
    gl->makeCurrent();

    // draw
    request->fPicture.reset(
        SkPicture::CreateFromStream(request->fUploadContext->fStream.detachAsStream()));
    if (!request->fPicture.get()) {
        error->appendf("Could not create picture from stream.\n");
        return false;
    }

    SkCanvas* canvas = surface->getCanvas();
    canvas->drawPicture(request->fPicture);

    // capture pixels
    SkBitmap bmp;
    bmp.setInfo(canvas->imageInfo());
    if (!canvas->readPixels(&bmp, 0, 0)) {
        error->appendf("Can't read canvas pixels.\n");
        return false;
    }

    // write to png
    request->fPNG.reset(SkImageEncoder::EncodeData(bmp, SkImageEncoder::kPNG_Type, 100));
    if (!request->fPNG) {
        error->appendf("Can't encode a PNG.\n");
        return false;
    }
    return true;
}

static const size_t kBufferSize = 1024;

static int process_upload_data(void* cls, enum MHD_ValueKind kind,
                               const char* key, const char* filename,
                               const char* content_type, const char* transfer_encoding,
                               const char* data, uint64_t off, size_t size) {
    struct UploadContext* uc = reinterpret_cast<UploadContext*>(cls);

    if (0 != size) {
        uc->fStream.write(data, size);
    }
    return MHD_YES;
}

static int SendData(MHD_Connection* connection, const SkData* data, const char* type) {
    MHD_Response* response = MHD_create_response_from_buffer(data->size(),
                                                             const_cast<void*>(data->data()),
                                                             MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Content-Type", type);

    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}

static int SendJSON(MHD_Connection* connection, SkPicture* picture) {
    SkDynamicMemoryWStream stream;
    SkAutoTUnref<SkJSONCanvas> jsonCanvas(new SkJSONCanvas(kImageWidth, kImageHeight, stream));
    jsonCanvas->drawPicture(picture);
    jsonCanvas->finish();

    SkAutoTUnref<SkData> data(stream.copyToData());
    return SendData(connection, data, "application/json");
}

static int SendTemplate(MHD_Connection* connection, bool redirect = false,
                        const char* redirectUrl = nullptr) {
    SkString debuggerTemplate = generateTemplate(SkString("https://debugger.skia.org"));

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

typedef int (*UrlHandler)(Request* request, MHD_Connection* connection,
                          const char* upload_data, size_t* upload_data_size);

int rootHandler(Request* request, MHD_Connection* connection,
                const char* upload_data, size_t* upload_data_size) {
    return SendTemplate(connection);
}

int postHandler(Request* request, MHD_Connection* connection,
                const char* upload_data, size_t* upload_data_size) {
    UploadContext* uc =  request->fUploadContext;

    // New connection
    if (!uc) {
        // TODO make this a method on request
        uc = new UploadContext;
        uc->connection = connection;
        uc->fPostProcessor = MHD_create_post_processor(connection, kBufferSize,
                                                       &process_upload_data, uc);
        SkASSERT(uc->fPostProcessor);

        request->fUploadContext = uc;
        return MHD_YES;
    }

    // in process upload
    if (0 != *upload_data_size) {
        SkASSERT(uc->fPostProcessor);
        MHD_post_process(uc->fPostProcessor, upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
    }

    // end of upload
    MHD_destroy_post_processor(uc->fPostProcessor);
    uc->fPostProcessor = nullptr;

    // TODO response
    SkString error;
    if (!setupAndDrawToCanvas(request, &error)) {
        // TODO send error
        return MHD_YES;
    }

    return SendTemplate(connection, true, "/");
}

int imgHandler(Request* request, MHD_Connection* connection,
               const char* upload_data, size_t* upload_data_size) {
    if (request->fPNG.get()) {
        SkData* data = request->fPNG.get();
        return SendData(connection, data, "image/png");
    }
    return MHD_NO;
}

int infoHandler(Request* request, MHD_Connection* connection,
               const char* upload_data, size_t* upload_data_size) {
    if (request->fPicture.get()) {
        return SendJSON(connection, request->fPicture);
    }
    return MHD_NO;
}

class UrlManager {
public:
    UrlManager() {
        // Register handlers
        fHandlers.push_back({MHD_HTTP_METHOD_GET, "/", rootHandler});
        fHandlers.push_back({MHD_HTTP_METHOD_POST, "/new", postHandler});
        fHandlers.push_back({MHD_HTTP_METHOD_GET, "/img", imgHandler});
        fHandlers.push_back({MHD_HTTP_METHOD_GET, "/cmd", infoHandler});
    }

    // This is clearly not efficient for a large number of urls and handlers
    int invoke(Request* request, MHD_Connection* connection, const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) const {
        for (int i = 0; i < fHandlers.count(); i++) {
            const Url& urlHandler = fHandlers[i];
            if (0 == strcmp(method, urlHandler.fMethod) &&
                0 == strcmp(url, urlHandler.fPath)) {
                    return (*urlHandler.fHandler)(request, connection, upload_data,
                                                  upload_data_size);
            }
        }
        return MHD_NO;
    }

private:
    struct Url {
        const char* fMethod;
        const char* fPath;
        UrlHandler fHandler;
    };
    SkTArray<Url> fHandlers;
};

const UrlManager kUrlManager;

int answer_to_connection(void* cls, struct MHD_Connection* connection,
                         const char* url, const char* method, const char* version,
                         const char* upload_data, size_t* upload_data_size,
                         void** con_cls) {
    SkDebugf("New %s request for %s using version %s\n", method, url, version);

    Request* request = reinterpret_cast<Request*>(cls);
    return kUrlManager.invoke(request, connection, url, method, upload_data, upload_data_size);
}

int skiaserve_main() {
    Request request; // This simple server has one request
    struct MHD_Daemon* daemon;
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, nullptr, nullptr,
                              &answer_to_connection, &request,
                              MHD_OPTION_END);
    if (NULL == daemon) {
        return 1;
    }

    getchar();
    MHD_stop_daemon(daemon);
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    return skiaserve_main();
}
#endif
