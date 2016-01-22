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
#include "SkOSFile.h"
#include <SkPicture.h>
#include "SkStream.h"
#include "SkSurface.h"

// temporary junk
#include "SkGradientShader.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
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
SkData* setupAndDrawToCanvas(const char* path, SkString* error) {
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
    SkAutoTDelete<SkStream> stream(SkStream::NewFromFile(path));
    if (stream.get() == nullptr) {
        error->appendf("Could not read %s.\n", path);
        return nullptr;
    }

    SkAutoTUnref<SkPicture> pic(SkPicture::CreateFromStream(stream.get()));
    if (pic.get() == nullptr) {
        error->appendf("Could not read %s as an SkPicture.\n", path);
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

int answer_to_connection(void* cls, struct MHD_Connection* connection,
                         const char* url, const char* method, const char* version,
                         const char* upload_data, size_t* upload_data_size,
                         void** con_cls) {
    printf ("New %s request for %s using version %s\n", method, url, version);

    struct MHD_Response* response;

    // serve html to root
    // TODO better url handling
    int ret;
    if (0 == strcmp("/", url)) {
        SkString debuggerTemplate = generateTemplate(SkString("http://debugger.skia.org"));

        response = MHD_create_response_from_buffer(debuggerTemplate.size(),
                                                   (void*)const_cast<char*>(debuggerTemplate.c_str()),
                                                   MHD_RESPMEM_MUST_COPY);
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return MHD_YES;
    }

    // otherwise serve an image
    // TODO take from post
    SkString resourceName;
    resourceName.appendf("%s/%s.skp", FLAGS_dir[0], FLAGS_name[0]);
    SkDebugf("Loading skp: %s\n", resourceName.c_str());

    SkString error;
    SkAutoTUnref<SkData> data(setupAndDrawToCanvas(resourceName.c_str(), &error));
    if (!data) {
        // TODO send error
        return MHD_YES;
    }

    response = MHD_create_response_from_buffer(data->size(), const_cast<void*>(data->data()),
                                               MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Content-Type", "image/png");
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}

int skiaserve_main() {
    struct MHD_Daemon* daemon;
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL, MHD_OPTION_END);
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
