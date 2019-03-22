/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CommonFlags.h"
#include "GrContextOptions.h"
#include "SkExecutor.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkOnce.h"

DEFINE_bool(cpu, true, "master switch for running CPU-bound work.");

DEFINE_bool(dryRun,
            false,
            "just print the tests that would be run, without actually running them.");

DEFINE_bool(gpu, true, "master switch for running GPU-bound work.");

DEFINE_string(images,
              "",
              "List of images and/or directories to decode. A directory with no images"
              " is treated as a fatal error.");

DEFINE_bool(simpleCodec,
            false,
            "Runs of a subset of the codec tests.  "
            "For DM, this means no scaling or subsetting, always using the "
            "canvas color type.  "
            "For nanobench, this means always N32, Premul or Opaque.");

DEFINE_string2(match,
               m,
               nullptr,
               "[~][^]substring[$] [...] of name to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching name to always be skipped\n"
               "^ requires the start of the name to match\n"
               "$ requires the end of the name to match\n"
               "^ and $ requires an exact match\n"
               "If a name does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");

DEFINE_bool2(quiet, q, false, "if true, don't print status updates.");

#ifdef SK_BUILD_FOR_ANDROID
DEFINE_string(skps, "/data/local/tmp/skps", "Directory to read skps from.");
DEFINE_string(lotties, "/data/local/tmp/lotties", "Directory to read (Bodymovin) jsons from.");
#else
DEFINE_string(skps, "skps", "Directory to read skps from.");
DEFINE_string(lotties, "lotties", "Directory to read (Bodymovin) jsons from.");
#endif

DEFINE_bool(nativeFonts,
            true,
            "If true, use native font manager and rendering. "
            "If false, fonts will draw as portably as possible.");

DEFINE_string(svgs, "", "Directory to read SVGs from, or a single SVG file.");

DEFINE_int_2(threads,
               j,
               -1,
               "Run threadsafe tests on a threadpool with this many extra threads, "
               "defaulting to one extra thread per core.");

DEFINE_bool2(verbose, v, false, "enable verbose output from the test driver.");

DEFINE_bool2(veryVerbose, V, false, "tell individual tests to be verbose.");

DEFINE_string2(writePath, w, "", "If set, write bitmaps here as .pngs.");

DEFINE_string(key, "", "Space-separated key/value pairs to add to JSON identifying this builder.");
DEFINE_string(properties,
              "",
              "Space-separated key/value pairs to add to JSON identifying this run.");

DEFINE_bool(analyticAA, true, "If false, disable analytic anti-aliasing");

DEFINE_bool(forceAnalyticAA,
            false,
            "Force analytic anti-aliasing even if the path is complicated: "
            "whether it's concave or convex, we consider a path complicated"
            "if its number of points is comparable to its resolution.");

bool CollectImages(CommandLineFlags::StringArray images, SkTArray<SkString>* output) {
    SkASSERT(output);

    static const char* const exts[] = {
        "bmp",
        "gif",
        "jpg",
        "jpeg",
        "png",
        "webp",
        "ktx",
        "astc",
        "wbmp",
        "ico",
#if !defined(SK_BUILD_FOR_WIN)
        "BMP",
        "GIF",
        "JPG",
        "JPEG",
        "PNG",
        "WEBP",
        "KTX",
        "ASTC",
        "WBMP",
        "ICO",
#endif
#ifdef SK_HAS_HEIF_LIBRARY
        "heic",
#if !defined(SK_BUILD_FOR_WIN)
        "HEIC",
#endif
#endif
#ifdef SK_CODEC_DECODES_RAW
        "arw",
        "cr2",
        "dng",
        "nef",
        "nrw",
        "orf",
        "raf",
        "rw2",
        "pef",
        "srw",
#if !defined(SK_BUILD_FOR_WIN)
        "ARW",
        "CR2",
        "DNG",
        "NEF",
        "NRW",
        "ORF",
        "RAF",
        "RW2",
        "PEF",
        "SRW",
#endif
#endif
    };

    for (int i = 0; i < images.count(); ++i) {
        const char* flag = images[i];
        if (!sk_exists(flag)) {
            SkDebugf("%s does not exist!\n", flag);
            return false;
        }

        if (sk_isdir(flag)) {
            // If the value passed in is a directory, add all the images
            bool foundAnImage = false;
            for (const char* ext : exts) {
                SkOSFile::Iter it(flag, ext);
                SkString       file;
                while (it.next(&file)) {
                    foundAnImage        = true;
                    output->push_back() = SkOSPath::Join(flag, file.c_str());
                }
            }
            if (!foundAnImage) {
                SkDebugf("No supported images found in %s!\n", flag);
                return false;
            }
        } else {
            // Also add the value if it is a single image
            output->push_back() = flag;
        }
    }
    return true;
}

DEFINE_int(gpuThreads,
             2,
             "Create this many extra threads to assist with GPU work, "
             "including software path rendering. Defaults to two.");

static DEFINE_bool(cachePathMasks, true,
                   "Allows path mask textures to be cached in GPU configs.");

static DEFINE_bool(noGS, false, "Disables support for geometry shaders.");

static DEFINE_string(pr, "",
              "Set of enabled gpu path renderers. Defined as a list of: "
              "[~]none [~]dashline [~]nvpr [~]ccpr [~]aahairline [~]aaconvex [~]aalinearizing "
              "[~]small [~]tess] [~]all");

static DEFINE_bool(disableDriverCorrectnessWorkarounds, false,
                   "Disables all GPU driver correctness workarounds");

static DEFINE_bool(reduceOpListSplitting, false, "Improve opList sorting");


static GpuPathRenderers get_named_pathrenderers_flags(const char* name) {
    if (!strcmp(name, "none")) {
        return GpuPathRenderers::kNone;
    } else if (!strcmp(name, "dashline")) {
        return GpuPathRenderers::kDashLine;
    } else if (!strcmp(name, "nvpr")) {
        return GpuPathRenderers::kStencilAndCover;
    } else if (!strcmp(name, "ccpr")) {
        return GpuPathRenderers::kCoverageCounting;
    } else if (!strcmp(name, "aahairline")) {
        return GpuPathRenderers::kAAHairline;
    } else if (!strcmp(name, "aaconvex")) {
        return GpuPathRenderers::kAAConvex;
    } else if (!strcmp(name, "aalinearizing")) {
        return GpuPathRenderers::kAALinearizing;
    } else if (!strcmp(name, "small")) {
        return GpuPathRenderers::kSmall;
    } else if (!strcmp(name, "tess")) {
        return GpuPathRenderers::kTessellating;
    } else if (!strcmp(name, "all")) {
        return GpuPathRenderers::kAll;
    }
    SK_ABORT(SkStringPrintf("error: unknown named path renderer \"%s\"\n", name).c_str());
    return GpuPathRenderers::kNone;
}

static GpuPathRenderers collect_gpu_path_renderers_from_flags() {
    if (FLAGS_pr.isEmpty()) {
        return GpuPathRenderers::kDefault;
    }
    GpuPathRenderers gpuPathRenderers = ('~' == FLAGS_pr[0][0])
        ? GpuPathRenderers::kDefault
        : GpuPathRenderers::kNone;

    for (int i = 0; i < FLAGS_pr.count(); ++i) {
        const char* name = FLAGS_pr[i];
        if (name[0] == '~') {
            gpuPathRenderers &= ~get_named_pathrenderers_flags(&name[1]);
        } else {
            gpuPathRenderers |= get_named_pathrenderers_flags(name);
        }
    }
    return gpuPathRenderers;
}

void SetCtxOptionsFromCommonFlags(GrContextOptions* ctxOptions) {
    static std::unique_ptr<SkExecutor> gGpuExecutor = (0 != FLAGS_gpuThreads)
        ? SkExecutor::MakeFIFOThreadPool(FLAGS_gpuThreads)
        : nullptr;

    ctxOptions->fExecutor                            = gGpuExecutor.get();
    ctxOptions->fAllowPathMaskCaching                = FLAGS_cachePathMasks;
    ctxOptions->fSuppressGeometryShaders             = FLAGS_noGS;
    ctxOptions->fGpuPathRenderers                    = collect_gpu_path_renderers_from_flags();
    ctxOptions->fDisableDriverCorrectnessWorkarounds = FLAGS_disableDriverCorrectnessWorkarounds;

    if (FLAGS_reduceOpListSplitting) {
        ctxOptions->fReduceOpListSplitting = GrContextOptions::Enable::kYes;
    }
}
