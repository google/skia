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

// TODO factor this out into functions, also handle CPU path
SkData* setupAndDrawToCanvas(SkStream* stream, SkString* error) {
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
    SkAutoTUnref<SkPicture> pic(SkPicture::CreateFromStream(stream));
    if (pic.get() == nullptr) {
        error->appendf("Could not create picture from stream.\n");
        return nullptr;
    }

    SkCanvas* canvas = surface->getCanvas();
    canvas->drawPicture(pic);

    // capture pixels
    SkBitmap bmp;
    bmp.setInfo(canvas->imageInfo());
    if (!canvas->readPixels(&bmp, 0, 0)) {
        error->appendf("Can't read canvas pixels.\n");
        return nullptr;
    }

    // write to png
    SkData* data = SkImageEncoder::EncodeData(bmp, SkImageEncoder::kPNG_Type, 100);
    if (!data) {
        error->appendf("Can't encode a PNG.\n");
        return nullptr;
    }
    return data;
}
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
    SkDynamicMemoryWStream stream;
    MHD_PostProcessor* pp;
    MHD_Connection* connection;
};

struct Request {
    Request() : fUploadContext(nullptr) {}
    UploadContext* fUploadContext;
    SkAutoTUnref<SkData> fPNG;
};

static const size_t kBufferSize = 1024;

static int process_upload_data(void* cls, enum MHD_ValueKind kind,
                               const char* key, const char* filename,
                               const char* content_type, const char* transfer_encoding,
                               const char* data, uint64_t off, size_t size) {
    struct UploadContext* uc = reinterpret_cast<UploadContext*>(cls);

    if (0 != size) {
        uc->stream.write(data, size);
    }
    return MHD_YES;
}

static int SendImage(MHD_Connection* connection, const SkData* data) {
    MHD_Response* response = MHD_create_response_from_buffer(data->size(),
                                                             const_cast<void*>(data->data()),
                                                             MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Content-Type", "image/png");

    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}

static int SendTemplate(MHD_Connection* connection) {
    SkString debuggerTemplate = generateTemplate(SkString("http://debugger.skia.org"));

    MHD_Response* response = MHD_create_response_from_buffer(
        debuggerTemplate.size(),
        (void*) const_cast<char*>(debuggerTemplate.c_str()),
        MHD_RESPMEM_MUST_COPY);

    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
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
        uc->pp = MHD_create_post_processor(connection, kBufferSize, &process_upload_data, uc);
        SkASSERT(uc->pp);

        request->fUploadContext = uc;
        return MHD_YES;
    }

    // in process upload
    if (0 != *upload_data_size) {
        SkASSERT(uc->pp);
        MHD_post_process(uc->pp, upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
    }

    // end of upload
    MHD_destroy_post_processor(uc->pp);
    uc->pp = nullptr;

    // TODO response
    SkString error;
    SkData* data = setupAndDrawToCanvas(uc->stream.detachAsStream(), &error);
    if (!data) {
        // TODO send error
        return MHD_YES;
    }

    request->fPNG.reset(data);
    return SendTemplate(connection);
}

int imgHandler(Request* request, MHD_Connection* connection,
               const char* upload_data, size_t* upload_data_size) {
    if (request->fPNG.get()) {
        SkData* data = request->fPNG.get();
        return SendImage(connection, data);
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
