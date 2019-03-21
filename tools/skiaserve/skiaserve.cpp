/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Request.h"
#include "Response.h"

#include "CommandLineFlags.h"
#include "SkGraphics.h"

#include "urlhandlers/UrlHandler.h"

#include "microhttpd.h"

#include <errno.h>

#if !defined _WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

using namespace Response;

static DEFINE_int32(port, 8888, "The port to listen on.");
static DEFINE_string(address, "127.0.0.1", "The address to bind to.");
static DEFINE_bool(hosted, false, "Running in hosted mode on debugger.skia.org.");

class UrlManager {
public:
    UrlManager() {
        // Register handlers
        fHandlers.push_back(new RootHandler);
        fHandlers.push_back(new PostHandler);
        fHandlers.push_back(new ImgHandler);
        fHandlers.push_back(new ClipAlphaHandler);
        fHandlers.push_back(new EnableGPUHandler);
        fHandlers.push_back(new CmdHandler);
        fHandlers.push_back(new InfoHandler);
        fHandlers.push_back(new DownloadHandler);
        fHandlers.push_back(new DataHandler);
        fHandlers.push_back(new BreakHandler);
        fHandlers.push_back(new OpsHandler);
        fHandlers.push_back(new OpBoundsHandler);
        fHandlers.push_back(new ColorModeHandler);
        fHandlers.push_back(new QuitHandler);
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
    SkGraphics::Init();
    Request request(SkString("/data")); // This simple server has one request

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(FLAGS_port);
    int result = inet_pton(AF_INET, FLAGS_address[0], &address.sin_addr);
    if (result != 1) {
        printf("inet_pton for %s:%d failed with return %d %s\n",
                FLAGS_address[0], FLAGS_port, result, strerror(errno));
        return 1;
    }

    printf("Visit http://%s:%d in your browser.\n", FLAGS_address[0], FLAGS_port);

    struct MHD_Daemon* daemon;
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY
#ifdef SK_DEBUG
                              | MHD_USE_DEBUG
#endif
                              , FLAGS_port, nullptr, nullptr,
                              &answer_to_connection, &request,
                              MHD_OPTION_SOCK_ADDR, &address,
                              MHD_OPTION_END);
    if (nullptr == daemon) {
        SkDebugf("Could not initialize daemon\n");
        return 1;
    }

    if (FLAGS_hosted) {
        while (1) {
            SkDebugf("loop\n");
            #if defined(SK_BUILD_FOR_WIN)
                Sleep(60 * 1000);
            #else
                sleep(60);
            #endif
        }
    } else {
        getchar();
    }
    MHD_stop_daemon(daemon);
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);
    return skiaserve_main();
}
#endif
