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
#include "SkDebugCanvas.h"
#include "SkJSONCanvas.h"
#include "SkJSONCPP.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkPixelSerializer.h"
#include "SkStream.h"
#include "SkSurface.h"

#include "UrlDataManager.h"

#include <sys/socket.h>
#include <microhttpd.h>
#include "png.h"

// To get image decoders linked in we have to do the below magic
#include "SkForceLinking.h"
#include "SkImageDecoder.h"
__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_string(source, "https://debugger.skia.org", "Where to load the web UI from.");
DEFINE_string(faviconDir, "tools/skiaserve", "The directory of the favicon");
DEFINE_int32(port, 8888, "The port to listen on.");

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
    Request(SkString rootUrl) : fUploadContext(nullptr), fUrlDataManager(rootUrl) {}
    UploadContext* fUploadContext;
    SkAutoTUnref<SkPicture> fPicture;
    SkAutoTUnref<SkDebugCanvas> fDebugCanvas;
    SkAutoTDelete<GrContextFactory> fContextFactory;
    SkAutoTUnref<SkSurface> fSurface;
    UrlDataManager fUrlDataManager;
};

// TODO factor this out into functions, also handle CPU path
SkSurface* setupSurface(GrContextFactory* factory) {
    GrContext* context = factory->get(GrContextFactory::kNative_GLContextType,
                                      GrContextFactory::kNone_GLContextOptions);
    int maxRTSize = context->caps()->maxRenderTargetSize();
    SkImageInfo info = SkImageInfo::Make(SkTMin(kImageWidth, maxRTSize),
                                         SkTMin(kImageHeight, maxRTSize),
                                         kN32_SkColorType, kPremul_SkAlphaType);
    uint32_t flags = 0;
    SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
    SkSurface* surface = SkSurface::NewRenderTarget(context, SkSurface::kNo_Budgeted, info, 0,
                                                    &props);
    SkASSERT(surface);

    return surface;
}

static void write_png_callback(png_structp png_ptr, png_bytep data, png_size_t length) {
    SkWStream* out = (SkWStream*) png_get_io_ptr(png_ptr);
    out->write(data, length);
}

static void write_png(const png_bytep rgba, png_uint_32 width, png_uint_32 height, SkWStream& out) {
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    SkASSERT(png != nullptr);
    png_infop info_ptr = png_create_info_struct(png);
    SkASSERT(info_ptr != nullptr);
    if (setjmp(png_jmpbuf(png))) {
        SkFAIL("png encode error");
    }
    png_set_IHDR(png, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_compression_level(png, 1);
    png_bytepp rows = (png_bytepp) sk_malloc_throw(height * sizeof(png_byte*));
    png_bytep pixels = (png_bytep) sk_malloc_throw(width * height * 3);
    for (png_size_t y = 0; y < height; ++y) {
        const png_bytep src = rgba + y * width * 4;
        rows[y] = pixels + y * width * 3;
        // convert from RGBA to RGB
        for (png_size_t x = 0; x < width; ++x) {
            rows[y][x * 3] = src[x * 4];
            rows[y][x * 3 + 1] = src[x * 4 + 1];
            rows[y][x * 3 + 2] = src[x * 4 + 2];
        }
    }
    png_set_filter(png, 0, PNG_NO_FILTERS);
    png_set_rows(png, info_ptr, &rows[0]);
    png_set_write_fn(png, &out, write_png_callback, NULL);
    png_write_png(png, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_destroy_write_struct(&png, NULL);
    sk_free(rows);
}

SkBitmap* getBitmapFromCanvas(SkCanvas* canvas) {
    SkBitmap* bmp = new SkBitmap();
    SkImageInfo info = SkImageInfo::Make(kImageWidth, kImageHeight, kRGBA_8888_SkColorType,
                kOpaque_SkAlphaType);
    bmp->setInfo(info);
    if (!canvas->readPixels(bmp, 0, 0)) {
        fprintf(stderr, "Can't read pixels\n");
        return nullptr;
    }
    return bmp;
}

SkData* writeCanvasToPng(SkCanvas* canvas) {
    // capture pixels
    SkAutoTDelete<SkBitmap> bmp(getBitmapFromCanvas(canvas));
    SkASSERT(bmp);

    // write to png
    SkDynamicMemoryWStream buffer;
    write_png((const png_bytep) bmp->getPixels(), bmp->width(), bmp->height(), buffer);
    return buffer.copyToData();
}

SkCanvas* getCanvasFromRequest(Request* request) {
    GrContextFactory* factory = request->fContextFactory;
    SkGLContext* gl = factory->getContextInfo(GrContextFactory::kNative_GLContextType,
                                              GrContextFactory::kNone_GLContextOptions).fGLContext;
    gl->makeCurrent();
    SkASSERT(request->fDebugCanvas);
    SkCanvas* target = request->fSurface->getCanvas();
    return target;
}

void drawToCanvas(Request* request, int n) {
    SkCanvas* target = getCanvasFromRequest(request);
    request->fDebugCanvas->drawTo(target, n);
}

SkData* drawToPng(Request* request, int n) {
    drawToCanvas(request, n);
    return writeCanvasToPng(getCanvasFromRequest(request));
}

SkSurface* setupCpuSurface() {
    SkImageInfo info = SkImageInfo::Make(kImageWidth, kImageHeight, kN32_SkColorType,
                                         kPremul_SkAlphaType);
    return SkSurface::NewRaster(info);
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

// SendOK just sends an empty response with a 200 OK status code.
static int SendOK(MHD_Connection* connection) {
    const char* data = "";

    MHD_Response* response = MHD_create_response_from_buffer(strlen(data),
                                                             (void*)data,
                                                             MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, 200, response);
    MHD_destroy_response(response);
    return ret;
}

static int SendData(MHD_Connection* connection, const SkData* data, const char* type,
                    bool setContentDisposition = false, const char* dispositionString = nullptr) {
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

static int SendJSON(MHD_Connection* connection, SkCanvas* canvas, SkDebugCanvas* debugCanvas,
                    UrlDataManager* urlDataManager, int n) {
    Json::Value root = debugCanvas->toJSON(*urlDataManager, n, canvas);
    SkDynamicMemoryWStream stream;
    stream.writeText(Json::FastWriter().write(root).c_str());

    SkAutoTUnref<SkData> data(stream.copyToData());
    return SendData(connection, data, "application/json");
}

static int SendTemplate(MHD_Connection* connection, bool redirect = false,
                        const char* redirectUrl = nullptr) {
    SkString debuggerTemplate = generateTemplate(SkString(FLAGS_source[0]));

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
    bool canHandle(const char* method, const char* url) override {
        const char* kBasePath = "/cmd";
        return 0 == strncmp(url, kBasePath, strlen(kBasePath));
    }

    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override {
        SkTArray<SkString> commands;
        SkStrSplit(url, "/", &commands);

        if (!request->fPicture.get() || commands.count() > 3) {
            return MHD_NO;
        }

        // /cmd or /cmd/N
        if (0 == strcmp(method, MHD_HTTP_METHOD_GET)) {
            int n;
            if (commands.count() == 1) {
                n = request->fDebugCanvas->getSize() - 1;
            } else {
                sscanf(commands[1].c_str(), "%d", &n);
            }
            return SendJSON(connection, getCanvasFromRequest(request), request->fDebugCanvas,
                            &request->fUrlDataManager, n);
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
            request->fDebugCanvas->toggleCommand(n, toggle);
            return SendOK(connection);
        }

        return MHD_NO;
    }
};

class ImgHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override {
        static const char* kBasePath = "/img";
        return 0 == strcmp(method, MHD_HTTP_METHOD_GET) &&
               0 == strncmp(url, kBasePath, strlen(kBasePath));
    }

    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override {
        SkTArray<SkString> commands;
        SkStrSplit(url, "/", &commands);

        if (!request->fPicture.get() || commands.count() > 2) {
            return MHD_NO;
        }

        int n;
        // /img or /img/N
        if (commands.count() == 1) {
            n = request->fDebugCanvas->getSize() - 1;
        } else {
            sscanf(commands[1].c_str(), "%d", &n);
        }

        SkAutoTUnref<SkData> data(drawToPng(request, n));
        return SendData(connection, data, "image/png");
    }
};

class BreakHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override {
        static const char* kBasePath = "/break";
        return 0 == strcmp(method, MHD_HTTP_METHOD_GET) &&
               0 == strncmp(url, kBasePath, strlen(kBasePath));
    }

    static SkColor GetPixel(Request* request, int x, int y) {
        SkCanvas* canvas = getCanvasFromRequest(request);
        canvas->flush();
        SkAutoTDelete<SkBitmap> bitmap(getBitmapFromCanvas(canvas));
        SkASSERT(bitmap);
        bitmap->lockPixels();
        uint8_t* start = ((uint8_t*) bitmap->getPixels()) + (y * kImageWidth + x) * 4;
        SkColor result = SkColorSetARGB(start[3], start[0], start[1], start[2]);
        bitmap->unlockPixels();
        return result;
    }

    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override {
        SkTArray<SkString> commands;
        SkStrSplit(url, "/", &commands);

        if (!request->fPicture.get() || commands.count() != 4) {
            return MHD_NO;
        }

        // /break/<n>/<x>/<y>
        int n;
        sscanf(commands[1].c_str(), "%d", &n);
        int x;
        sscanf(commands[2].c_str(), "%d", &x);
        int y;
        sscanf(commands[3].c_str(), "%d", &y);

        int count = request->fDebugCanvas->getSize();
        SkASSERT(n < count);

        SkCanvas* canvas = getCanvasFromRequest(request);
        canvas->clear(SK_ColorWHITE);
        int saveCount = canvas->save();
        for (int i = 0; i <= n; ++i) {
            request->fDebugCanvas->getDrawCommandAt(i)->execute(canvas);
        }
        SkColor target = GetPixel(request, x, y);
        Json::Value response(Json::objectValue);
        Json::Value startColor(Json::arrayValue);
        startColor.append(Json::Value(SkColorGetR(target)));
        startColor.append(Json::Value(SkColorGetG(target)));
        startColor.append(Json::Value(SkColorGetB(target)));
        startColor.append(Json::Value(SkColorGetA(target)));
        response["startColor"] = startColor;
        response["endColor"] = startColor;
        response["endOp"] = Json::Value(n);
        for (int i = n + 1; i < n + count; ++i) {
            int index = i % count;
            if (index == 0) {
                // reset canvas for wraparound
                canvas->restoreToCount(saveCount);
                canvas->clear(SK_ColorWHITE);
                saveCount = canvas->save();
            }
            request->fDebugCanvas->getDrawCommandAt(index)->execute(canvas);
            SkColor current = GetPixel(request, x, y);
            if (current != target) {
                Json::Value endColor(Json::arrayValue);
                endColor.append(Json::Value(SkColorGetR(current)));
                endColor.append(Json::Value(SkColorGetG(current)));
                endColor.append(Json::Value(SkColorGetB(current)));
                endColor.append(Json::Value(SkColorGetA(current)));
                response["endColor"] = endColor;
                response["endOp"] = Json::Value(index);
                break;
            }
        }
        canvas->restoreToCount(saveCount);
        SkDynamicMemoryWStream stream;
        stream.writeText(Json::FastWriter().write(response).c_str());
        SkAutoTUnref<SkData> data(stream.copyToData());
        return SendData(connection, data, "application/json");
    }
};

/**
   Updates the clip visualization alpha. On all subsequent /img requests, the clip will be drawn in
   black with the specified alpha. 0 = no visible clip, 255 = fully opaque clip.
 */
class ClipAlphaHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override {
        static const char* kBasePath = "/clipAlpha/";
        return 0 == strcmp(method, MHD_HTTP_METHOD_POST) &&
               0 == strncmp(url, kBasePath, strlen(kBasePath));
    }

    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override {
        SkTArray<SkString> commands;
        SkStrSplit(url, "/", &commands);

        if (!request->fPicture.get() || commands.count() != 2) {
            return MHD_NO;
        }

        int alpha;
        sscanf(commands[1].c_str(), "%d", &alpha);

        request->fDebugCanvas->setClipVizColor(SkColorSetARGB(alpha, 0, 0, 0));
        return SendOK(connection);
    }
};

class PostHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override {
        return 0 == strcmp(method, MHD_HTTP_METHOD_POST) &&
               0 == strcmp(url, "/new");
    }

    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override {
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

        // parse picture from stream
        request->fPicture.reset(
            SkPicture::CreateFromStream(request->fUploadContext->fStream.detachAsStream()));
        if (!request->fPicture.get()) {
            fprintf(stderr, "Could not create picture from stream.\n");
            return MHD_NO;
        }

        // create surface
        GrContextOptions grContextOpts;
        request->fContextFactory.reset(new GrContextFactory(grContextOpts));
        request->fSurface.reset(setupSurface(request->fContextFactory.get()));

        // pour picture into debug canvas
        request->fDebugCanvas.reset(new SkDebugCanvas(kImageWidth, kImageHeight));
        request->fDebugCanvas->drawPicture(request->fPicture);

        // clear upload context
        delete request->fUploadContext;
        request->fUploadContext = nullptr;

        return SendTemplate(connection, true, "/");
    }
};

class DownloadHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override {
        return 0 == strcmp(method, MHD_HTTP_METHOD_GET) &&
               0 == strcmp(url, "/download");
    }

    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override {
        if (!request->fPicture.get()) {
            return MHD_NO;
        }

        // TODO move to a function
        // Playback into picture recorder
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(kImageWidth, kImageHeight);

        request->fDebugCanvas->draw(canvas);

        SkAutoTUnref<SkPicture> picture(recorder.endRecording());

        SkDynamicMemoryWStream outStream;

        SkAutoTUnref<SkPixelSerializer> serializer(SkImageEncoder::CreatePixelSerializer());
        picture->serialize(&outStream, serializer);

        SkAutoTUnref<SkData> data(outStream.copyToData());

        // TODO fancier name handling
        return SendData(connection, data, "application/octet-stream", true,
                        "attachment; filename=something.skp;");
    }
};

class InfoHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override {
        const char* kBaseName = "/info";
        return 0 == strcmp(method, MHD_HTTP_METHOD_GET) &&
               0 == strncmp(url, kBaseName, strlen(kBaseName));
    }

    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override {
        SkTArray<SkString> commands;
        SkStrSplit(url, "/", &commands);

        if (!request->fPicture.get() || commands.count() > 2) {
            return MHD_NO;
        }

        // drawTo
        SkAutoTUnref<SkSurface> surface(setupCpuSurface());
        SkCanvas* canvas = surface->getCanvas();

        int n;
        // /info or /info/N
        if (commands.count() == 1) {
            n = request->fDebugCanvas->getSize() - 1;
        } else {
            sscanf(commands[1].c_str(), "%d", &n);
        }

        // TODO this is really slow and we should cache the matrix and clip
        request->fDebugCanvas->drawTo(canvas, n);

        // make some json
        SkMatrix vm = request->fDebugCanvas->getCurrentMatrix();
        SkIRect clip = request->fDebugCanvas->getCurrentClip();
        Json::Value info(Json::objectValue);
        info["ViewMatrix"] = SkJSONCanvas::MakeMatrix(vm);
        info["ClipRect"] = SkJSONCanvas::MakeIRect(clip);

        std::string json = Json::FastWriter().write(info);

        // We don't want the null terminator so strlen is correct
        SkAutoTUnref<SkData> data(SkData::NewWithCopy(json.c_str(), strlen(json.c_str())));
        return SendData(connection, data, "application/json");
    }
};

class DataHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override {
        static const char* kBaseUrl = "/data";
        return 0 == strcmp(method, MHD_HTTP_METHOD_GET) &&
               0 == strncmp(url, kBaseUrl, strlen(kBaseUrl));
    }

    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override {
        SkTArray<SkString> commands;
        SkStrSplit(url, "/", &commands);

        if (!request->fPicture.get() || commands.count() != 2) {
            return MHD_NO;
        }

        SkAutoTUnref<UrlDataManager::UrlData> urlData(
            SkRef(request->fUrlDataManager.getDataFromUrl(SkString(url))));

        if (urlData) {
            return SendData(connection, urlData->fData.get(), urlData->fContentType.c_str());
        }
        return MHD_NO;
    }
};

class FaviconHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override {
        return 0 == strcmp(method, MHD_HTTP_METHOD_GET) &&
               0 == strcmp(url, "/favicon.ico");
    }

    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override {
        SkString dir(FLAGS_faviconDir[0]);
        dir.append("/favicon.ico");
        FILE* ico = fopen(dir.c_str(), "r");

        SkAutoTUnref<SkData> data(SkData::NewFromFILE(ico));
        int ret = SendData(connection, data, "image/vnd.microsoft.icon");
        fclose(ico);
        return ret;
    }
};


class RootHandler : public UrlHandler {
public:
    bool canHandle(const char* method, const char* url) override {
        return 0 == strcmp(method, MHD_HTTP_METHOD_GET) &&
               0 == strcmp(url, "/");
    }

    int handle(Request* request, MHD_Connection* connection,
               const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) override {
        return SendTemplate(connection);
    }
};

class UrlManager {
public:
    UrlManager() {
        // Register handlers
        fHandlers.push_back(new RootHandler);
        fHandlers.push_back(new PostHandler);
        fHandlers.push_back(new ImgHandler);
        fHandlers.push_back(new ClipAlphaHandler);
        fHandlers.push_back(new CmdHandler);
        fHandlers.push_back(new InfoHandler);
        fHandlers.push_back(new DownloadHandler);
        fHandlers.push_back(new DataHandler);
        fHandlers.push_back(new FaviconHandler);
        fHandlers.push_back(new BreakHandler);
    }

    ~UrlManager() {
        for (int i = 0; i < fHandlers.count(); i++) { delete fHandlers[i]; }
    }

    // This is clearly not efficient for a large number of urls and handlers
    int invoke(Request* request, MHD_Connection* connection, const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) const {
        for (int i = 0; i < fHandlers.count(); i++) {
            if (fHandlers[i]->canHandle(method, url)) {
                return fHandlers[i]->handle(request, connection, url, method, upload_data,
                                            upload_data_size);
            }
        }
        return MHD_NO;
    }

private:
    SkTArray<UrlHandler*> fHandlers;
};

const UrlManager kUrlManager;

int answer_to_connection(void* cls, struct MHD_Connection* connection,
                         const char* url, const char* method, const char* version,
                         const char* upload_data, size_t* upload_data_size,
                         void** con_cls) {
    SkDebugf("New %s request for %s using version %s\n", method, url, version);

    Request* request = reinterpret_cast<Request*>(cls);
    int result = kUrlManager.invoke(request, connection, url, method, upload_data,
                                    upload_data_size);
    if (MHD_NO == result) {
        fprintf(stderr, "Invalid method and / or url: %s %s\n", method, url);
    }
    return result;
}

int skiaserve_main() {
    Request request(SkString("/data")); // This simple server has one request
    struct MHD_Daemon* daemon;
    // TODO Add option to bind this strictly to an address, e.g. localhost, for security.
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, FLAGS_port, nullptr, nullptr,
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
