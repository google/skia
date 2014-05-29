#include <sys/time.h>
#include <sys/resource.h>

#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkImageInfo.h"
#include "SkStream.h"
#include "SkSurface.h"

#include "seccomp_bpf.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_string(out, "", "Filename of the PNG to write to.");
DEFINE_string(source, "", "Filename of the source image.");

// Defined in template.cpp.
extern SkBitmap source;

static bool install_syscall_filter() {
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
}

static void setLimits() {
    struct rlimit n;

    // Limit to 5 seconds of CPU.
    n.rlim_cur = 5;
    n.rlim_max = 5;
    if (setrlimit(RLIMIT_CPU, &n)) {
        perror("setrlimit(RLIMIT_CPU)");
    }

    // Limit to 50M of Address space.
    n.rlim_cur = 50000000;
    n.rlim_max = 50000000;
    if (setrlimit(RLIMIT_AS, &n)) {
        perror("setrlimit(RLIMIT_CPU)");
    }
}

extern void draw(SkCanvas* canvas);

int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    SkAutoGraphics init;

    if (FLAGS_out.count() == 0) {
      perror("The --out flag must have an argument.");
      return 1;
    }

    if (FLAGS_source.count() == 1) {
       if (!SkImageDecoder::DecodeFile(FLAGS_source[0], &source)) {
           perror("Unable to read the source image.");
       }
    }

    SkFILEWStream stream(FLAGS_out[0]);

    SkImageInfo info = SkImageInfo::MakeN32(256, 256, kPremul_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));
    SkCanvas* canvas = surface->getCanvas();

    setLimits();

    if (!install_syscall_filter()) {
        return 1;
    }

    draw(canvas);

    // Write out the image as a PNG.
    SkAutoTUnref<SkImage> image(surface->newImageSnapshot());
    SkAutoTUnref<SkData> data(image->encode(SkImageEncoder::kPNG_Type, 100));
    if (NULL == data.get()) {
        printf("Failed to encode\n");
        exit(1);
    }
    stream.write(data->data(), data->size());
}
