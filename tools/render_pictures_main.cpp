/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CopyTilesRenderer.h"
#include "SkBitmap.h"
#include "SkDevice.h"
#include "SkCommandLineFlags.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkMath.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkString.h"
#include "PictureRenderer.h"
#include "PictureRenderingFlags.h"
#include "picture_utils.h"

// Flags used by this file, alphabetically:
DEFINE_int32(clone, 0, "Clone the picture n times before rendering.");
DECLARE_bool(deferImageDecoding);
DEFINE_int32(maxComponentDiff, 256, "Maximum diff on a component, 0 - 256. Components that differ "
             "by more than this amount are considered errors, though all diffs are reported. "
             "Requires --validate.");
DECLARE_string(r);
DEFINE_string(w, "", "Directory to write the rendered images.");
DEFINE_bool(writeWholeImage, false, "In tile mode, write the entire rendered image to a "
            "file, instead of an image for each tile.");
DEFINE_bool(validate, false, "Verify that the rendered image contains the same pixels as "
            "the picture rendered in simple mode. When used in conjunction with --bbh, results "
            "are validated against the picture rendered in the same mode, but without the bbh.");

static void make_output_filepath(SkString* path, const SkString& dir,
                                 const SkString& name) {
    sk_tools::make_filepath(path, dir, name);
    // Remove ".skp"
    path->remove(path->size() - 4, 4);
}

// Defined in PictureRenderingFlags.cpp
extern bool lazy_decode_bitmap(const void* buffer, size_t size, SkBitmap* bitmap);

static bool render_picture(const SkString& inputPath, const SkString* outputDir,
                           sk_tools::PictureRenderer& renderer,
                           SkBitmap** out) {
    SkString inputFilename;
    sk_tools::get_basename(&inputFilename, inputPath);

    SkFILEStream inputStream;
    inputStream.setPath(inputPath.c_str());
    if (!inputStream.isValid()) {
        SkDebugf("Could not open file %s\n", inputPath.c_str());
        return false;
    }

    bool success = false;
    SkPicture* picture;
    if (FLAGS_deferImageDecoding) {
        picture = SkNEW_ARGS(SkPicture, (&inputStream, &success, &lazy_decode_bitmap));
    } else {
        picture = SkNEW_ARGS(SkPicture, (&inputStream, &success, &SkImageDecoder::DecodeMemory));
    }
    if (!success) {
        SkDebugf("Could not read an SkPicture from %s\n", inputPath.c_str());
        return false;
    }

    for (int i = 0; i < FLAGS_clone; ++i) {
        SkPicture* clone = picture->clone();
        SkDELETE(picture);
        picture = clone;
    }

    SkDebugf("drawing... [%i %i] %s\n", picture->width(), picture->height(),
             inputPath.c_str());

    renderer.init(picture);
    renderer.setup();

    SkString* outputPath = NULL;
    if (NULL != outputDir && outputDir->size() > 0) {
        outputPath = SkNEW(SkString);
        make_output_filepath(outputPath, *outputDir, inputFilename);
    }

    success = renderer.render(outputPath, out);
    if (outputPath) {
        if (!success) {
            SkDebugf("Could not write to file %s\n", outputPath->c_str());
        }
        SkDELETE(outputPath);
    }

    renderer.end();

    SkDELETE(picture);
    return success;
}

static inline int getByte(uint32_t value, int index) {
    SkASSERT(0 <= index && index < 4);
    return (value >> (index * 8)) & 0xFF;
}

static int MaxByteDiff(uint32_t v1, uint32_t v2) {
    return SkMax32(SkMax32(abs(getByte(v1, 0) - getByte(v2, 0)), abs(getByte(v1, 1) - getByte(v2, 1))),
                   SkMax32(abs(getByte(v1, 2) - getByte(v2, 2)), abs(getByte(v1, 3) - getByte(v2, 3))));
}

namespace {
class AutoRestoreBbhType {
public:
    AutoRestoreBbhType() {
        fRenderer = NULL;
        fSavedBbhType = sk_tools::PictureRenderer::kNone_BBoxHierarchyType;
    }

    void set(sk_tools::PictureRenderer* renderer,
             sk_tools::PictureRenderer::BBoxHierarchyType bbhType) {
        fRenderer = renderer;
        fSavedBbhType = renderer->getBBoxHierarchyType();
        renderer->setBBoxHierarchyType(bbhType);
    }

    ~AutoRestoreBbhType() {
        if (NULL != fRenderer) {
            fRenderer->setBBoxHierarchyType(fSavedBbhType);
        }
    }

private:
    sk_tools::PictureRenderer* fRenderer;
    sk_tools::PictureRenderer::BBoxHierarchyType fSavedBbhType;
};
}

static bool render_picture(const SkString& inputPath, const SkString* outputDir,
                           sk_tools::PictureRenderer& renderer) {
    int diffs[256] = {0};
    SkBitmap* bitmap = NULL;
    bool success = render_picture(inputPath,
        FLAGS_writeWholeImage ? NULL : outputDir,
        renderer,
        FLAGS_validate || FLAGS_writeWholeImage ? &bitmap : NULL);

    if (!success || ((FLAGS_validate || FLAGS_writeWholeImage) && bitmap == NULL)) {
        SkDebugf("Failed to draw the picture.\n");
        SkDELETE(bitmap);
        return false;
    }

    if (FLAGS_validate) {
        SkBitmap* referenceBitmap = NULL;
        sk_tools::PictureRenderer* referenceRenderer;
        // If the renderer uses a BBoxHierarchy, then the reference renderer
        // will be the same renderer, without the bbh.
        AutoRestoreBbhType arbbh;
        if (sk_tools::PictureRenderer::kNone_BBoxHierarchyType !=
            renderer.getBBoxHierarchyType()) {
            referenceRenderer = &renderer;
            referenceRenderer->ref();  // to match auto unref below
            arbbh.set(referenceRenderer, sk_tools::PictureRenderer::kNone_BBoxHierarchyType);
        } else {
            referenceRenderer = SkNEW(sk_tools::SimplePictureRenderer);
        }
        SkAutoTUnref<sk_tools::PictureRenderer> aurReferenceRenderer(referenceRenderer);

        success = render_picture(inputPath, NULL, *referenceRenderer,
                                 &referenceBitmap);

        if (!success || NULL == referenceBitmap || NULL == referenceBitmap->getPixels()) {
            SkDebugf("Failed to draw the reference picture.\n");
            SkDELETE(bitmap);
            SkDELETE(referenceBitmap);
            return false;
        }

        if (success && (bitmap->width() != referenceBitmap->width())) {
            SkDebugf("Expected image width: %i, actual image width %i.\n",
                     referenceBitmap->width(), bitmap->width());
            SkDELETE(bitmap);
            SkDELETE(referenceBitmap);
            return false;
        }
        if (success && (bitmap->height() != referenceBitmap->height())) {
            SkDebugf("Expected image height: %i, actual image height %i",
                     referenceBitmap->height(), bitmap->height());
            SkDELETE(bitmap);
            SkDELETE(referenceBitmap);
            return false;
        }

        for (int y = 0; success && y < bitmap->height(); y++) {
            for (int x = 0; success && x < bitmap->width(); x++) {
                int diff = MaxByteDiff(*referenceBitmap->getAddr32(x, y),
                                       *bitmap->getAddr32(x, y));
                SkASSERT(diff >= 0 && diff <= 255);
                diffs[diff]++;

                if (diff > FLAGS_maxComponentDiff) {
                    SkDebugf("Expected pixel at (%i %i) exceedds maximum "
                                 "component diff of %i: 0x%x, actual 0x%x\n",
                             x, y, FLAGS_maxComponentDiff,
                             *referenceBitmap->getAddr32(x, y),
                             *bitmap->getAddr32(x, y));
                    SkDELETE(bitmap);
                    SkDELETE(referenceBitmap);
                    return false;
                }
            }
        }
        SkDELETE(referenceBitmap);

        for (int i = 1; i <= 255; ++i) {
            if(diffs[i] > 0) {
                SkDebugf("Number of pixels with max diff of %i is %i\n", i, diffs[i]);
            }
        }
    }

    if (FLAGS_writeWholeImage) {
        sk_tools::force_all_opaque(*bitmap);
        if (NULL != outputDir && FLAGS_writeWholeImage) {
            SkString inputFilename;
            sk_tools::get_basename(&inputFilename, inputPath);
            SkString outputPath;
            make_output_filepath(&outputPath, *outputDir, inputFilename);
            outputPath.append(".png");
            if (!SkImageEncoder::EncodeFile(outputPath.c_str(), *bitmap,
                                            SkImageEncoder::kPNG_Type, 100)) {
                SkDebugf("Failed to draw the picture.\n");
                success = false;
            }
        }
    }
    SkDELETE(bitmap);

    return success;
}


static int process_input(const char* input, const SkString* outputDir,
                         sk_tools::PictureRenderer& renderer) {
    SkOSFile::Iter iter(input, "skp");
    SkString inputFilename;
    int failures = 0;
    SkDebugf("process_input, %s\n", input);
    if (iter.next(&inputFilename)) {
        do {
            SkString inputPath;
            SkString inputAsSkString(input);
            sk_tools::make_filepath(&inputPath, inputAsSkString, inputFilename);
            if (!render_picture(inputPath, outputDir, renderer)) {
                ++failures;
            }
        } while(iter.next(&inputFilename));
    } else if (SkStrEndsWith(input, ".skp")) {
        SkString inputPath(input);
        if (!render_picture(inputPath, outputDir, renderer)) {
            ++failures;
        }
    } else {
        SkString warning;
        warning.printf("Warning: skipping %s\n", input);
        SkDebugf(warning.c_str());
    }
    return failures;
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage("Render .skp files.");
    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_r.isEmpty()) {
        SkDebugf(".skp files or directories are required.\n");
        exit(-1);
    }

    if (FLAGS_maxComponentDiff < 0 || FLAGS_maxComponentDiff > 256) {
        SkDebugf("--maxComponentDiff must be between 0 and 256\n");
        exit(-1);
    }

    if (FLAGS_maxComponentDiff != 256 && !FLAGS_validate) {
        SkDebugf("--maxComponentDiff requires --validate\n");
        exit(-1);
    }

    if (FLAGS_clone < 0) {
        SkDebugf("--clone must be >= 0. Was %i\n", FLAGS_clone);
        exit(-1);
    }

    SkString errorString;
    SkAutoTUnref<sk_tools::PictureRenderer> renderer(parseRenderer(errorString,
                                                                   kRender_PictureTool));
    if (errorString.size() > 0) {
        SkDebugf("%s\n", errorString.c_str());
    }

    if (renderer.get() == NULL) {
        exit(-1);
    }

    SkAutoGraphics ag;

    SkString outputDir;
    if (FLAGS_w.count() == 1) {
        outputDir.set(FLAGS_w[0]);
    }

    int failures = 0;
    for (int i = 0; i < FLAGS_r.count(); i ++) {
        failures += process_input(FLAGS_r[i], &outputDir, *renderer.get());
    }
    if (failures != 0) {
        SkDebugf("Failed to render %i pictures.\n", failures);
        return 1;
    }
#if SK_SUPPORT_GPU
#if GR_CACHE_STATS
    if (renderer->isUsingGpuDevice()) {
        GrContext* ctx = renderer->getGrContext();

        ctx->printCacheStats();
    }
#endif
#endif
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
