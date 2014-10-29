#include <sys/time.h>
#include <sys/resource.h>

#include "GrContextFactory.h"

#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkImageInfo.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "SkSurface.h"

#include "seccomp_bpf.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_string(out, "", "Output basename; fiddle will append the config used and the appropriate extension");
DEFINE_string(source, "", "Filename of the source image.");
DEFINE_int32(width, 256, "Width of output image.");
DEFINE_int32(height, 256, "Height of output image.");
DEFINE_bool(gpu, false, "Use GPU (Mesa) rendering.");
DEFINE_bool(raster, true, "Use Raster rendering.");
DEFINE_bool(pdf, false, "Use PDF rendering.");

// Defined in template.cpp.
extern SkBitmap source;

static bool install_syscall_filter() {

#ifndef SK_UNSAFE_BUILD_DESKTOP_ONLY
    struct sock_filter filter[] = {
        /* Grab the system call number. */
        EXAMINE_SYSCALL,
        /* List allowed syscalls. */
        ALLOW_SYSCALL(exit_group),
        ALLOW_SYSCALL(exit),
        ALLOW_SYSCALL(fstat),
        ALLOW_SYSCALL(read),
        ALLOW_SYSCALL(write),
        ALLOW_SYSCALL(close),
        ALLOW_SYSCALL(mmap),
        ALLOW_SYSCALL(munmap),
        ALLOW_SYSCALL(brk),
        ALLOW_SYSCALL(futex),
        KILL_PROCESS,
    };
    struct sock_fprog prog = {
        SK_ARRAY_COUNT(filter),
        filter,
    };

    // Lock down the app so that it can't get new privs, such as setuid.
    // Calling this is a requirement for an unpriviledged process to use mode
    // 2 seccomp filters, ala SECCOMP_MODE_FILTER, otherwise we'd have to be
    // root.
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("prctl(NO_NEW_PRIVS)");
        goto failed;
    }
    // Now call seccomp and restrict the system calls that can be made to only
    // the ones in the provided filter list.
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog)) {
        perror("prctl(SECCOMP)");
        goto failed;
    }
    return true;

failed:
    if (errno == EINVAL) {
        fprintf(stderr, "SECCOMP_FILTER is not available. :(\n");
    }
    return false;
#else
    return true;
#endif /* SK_UNSAFE_BUILD_DESKTOP_ONLY */
}

static void setLimits() {
    struct rlimit n;

    // Limit to 5 seconds of CPU.
    n.rlim_cur = 5;
    n.rlim_max = 5;
    if (setrlimit(RLIMIT_CPU, &n)) {
        perror("setrlimit(RLIMIT_CPU)");
    }

    // Limit to 150M of Address space.
    n.rlim_cur = 150000000;
    n.rlim_max = 150000000;
    if (setrlimit(RLIMIT_AS, &n)) {
        perror("setrlimit(RLIMIT_CPU)");
    }
}

extern void draw(SkCanvas* canvas);

static void drawAndDump(SkSurface* surface, SkWStream* stream) {
    SkCanvas *canvas = surface->getCanvas();
    draw(canvas);

    // Write out the image as a PNG.
    SkAutoTUnref<SkImage> image(surface->newImageSnapshot());
    SkAutoTUnref<SkData> data(image->encode(SkImageEncoder::kPNG_Type, 100));
    if (NULL == data.get()) {
        printf("Failed to encode\n");
        exit(1);
    }
    stream->write(data->data(), data->size());
}

static void drawRaster(SkWStream* stream, SkImageInfo info) {
    SkAutoTUnref<SkSurface> surface;
    surface.reset(SkSurface::NewRaster(info)); 
    drawAndDump(surface, stream);
}

static void drawGPU(SkWStream* stream, SkImageInfo info) {
    SkAutoTUnref<SkSurface> surface;
    GrContextFactory* grFactory = NULL;

    GrContext::Options grContextOpts;
    grFactory = new GrContextFactory(grContextOpts);
    GrContext* gr = grFactory->get(GrContextFactory::kMESA_GLContextType);
    surface.reset(SkSurface::NewRenderTarget(gr,info));

    drawAndDump(surface, stream);

    delete grFactory;
}

static void drawPDF(SkWStream* stream, SkImageInfo info) {
    printf( "Not implemented yet...\n");
}

int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    SkAutoGraphics init;

    if (FLAGS_out.count() == 0) {
      perror("The --out flag must have an argument.");
      return 1;
    }

    if (FLAGS_source.count() == 1) {
        const char *sourceDir = getenv("WEBTRY_INOUT");
        if (NULL == sourceDir) {
            sourceDir = "/skia_build/inout";
        }

        SkString sourcePath = SkOSPath::Join(sourceDir, FLAGS_source[0]);
        if (!SkImageDecoder::DecodeFile(sourcePath.c_str(), &source)) {
            perror("Unable to read the source image.");
        }
    }

    // make sure to open any needed output files before we set up the security
    // jail

    SkWStream* streams[3];

    if (FLAGS_raster) {
        SkString outPath;
        outPath.printf("%s_raster.png", FLAGS_out[0]);
        streams[0] = SkNEW_ARGS(SkFILEWStream,(outPath.c_str()));
    }
    if (FLAGS_gpu) {
        SkString outPath;
        outPath.printf("%s_gpu.png", FLAGS_out[0]);
        streams[1] = SkNEW_ARGS(SkFILEWStream,(outPath.c_str()));
    }
    if (FLAGS_pdf) {
        SkString outPath;
        outPath.printf("%s.pdf", FLAGS_out[0]);
        streams[2] = SkNEW_ARGS(SkFILEWStream,(outPath.c_str()));
    }

    SkImageInfo info = SkImageInfo::MakeN32(FLAGS_width, FLAGS_height, kPremul_SkAlphaType);

    setLimits();

    if (!install_syscall_filter()) {
        return 1;
    }

    if (FLAGS_raster) {
        drawRaster(streams[0], info);
    }
    if (FLAGS_gpu) {
        drawGPU(streams[1], info);
    }
    if (FLAGS_pdf) {
        drawPDF(streams[2], info);
    }
}
