/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "LazyDecodeBitmap.h"
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
#include "SkPictureRecorder.h"
#include "SkStream.h"
#include "SkString.h"

#include "image_expectations.h"
#include "PictureRenderer.h"
#include "PictureRenderingFlags.h"
#include "picture_utils.h"

// Flags used by this file, alphabetically:
DEFINE_bool(bench_record, false, "If true, drop into an infinite loop of recording the picture.");
DECLARE_bool(deferImageDecoding);
DEFINE_string(descriptions, "", "one or more key=value pairs to add to the descriptions section "
              "of the JSON summary.");
DEFINE_string(imageBaseGSUrl, "", "The Google Storage image base URL the images are stored in.");
DEFINE_int32(maxComponentDiff, 256, "Maximum diff on a component, 0 - 256. Components that differ "
             "by more than this amount are considered errors, though all diffs are reported. "
             "Requires --validate.");
DEFINE_string(mismatchPath, "", "Write images for tests that failed due to "
              "pixel mismatches into this directory.");
#if GR_GPU_STATS
DEFINE_bool(gpuStats, false, "Only meaningful with gpu configurations. "
            "Report some GPU call statistics.");
#endif
DEFINE_bool(mpd, false, "If true, use MultiPictureDraw for rendering.");
DEFINE_string(readJsonSummaryPath, "", "JSON file to read image expectations from.");
DECLARE_string(readPath);
DEFINE_bool(writeChecksumBasedFilenames, false,
            "When writing out images, use checksum-based filenames.");
DEFINE_bool(writeEncodedImages, false, "Any time the skp contains an encoded image, write it to a "
            "file rather than decoding it. Requires writePath to be set. Skips drawing the full "
            "skp to a file. Not compatible with deferImageDecoding.");
DEFINE_string(writeJsonSummaryPath, "", "File to write a JSON summary of image results to.");
DEFINE_string2(writePath, w, "", "Directory to write the rendered images into.");
DEFINE_bool(writeWholeImage, false, "In tile mode, write the entire rendered image to a "
            "file, instead of an image for each tile.");
DEFINE_bool(validate, false, "Verify that the rendered image contains the same pixels as "
            "the picture rendered in simple mode. When used in conjunction with --bbh, results "
            "are validated against the picture rendered in the same mode, but without the bbh.");

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *  Table for translating from format of data to a suffix.
 */
struct Format {
    SkImageDecoder::Format  fFormat;
    const char*             fSuffix;
};
static const Format gFormats[] = {
    { SkImageDecoder::kBMP_Format, ".bmp" },
    { SkImageDecoder::kGIF_Format, ".gif" },
    { SkImageDecoder::kICO_Format, ".ico" },
    { SkImageDecoder::kJPEG_Format, ".jpg" },
    { SkImageDecoder::kPNG_Format, ".png" },
    { SkImageDecoder::kWBMP_Format, ".wbmp" },
    { SkImageDecoder::kWEBP_Format, ".webp" },
    { SkImageDecoder::kUnknown_Format, "" },
};

/**
 *  Get an appropriate suffix for an image format.
 */
static const char* get_suffix_from_format(SkImageDecoder::Format format) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gFormats); i++) {
        if (gFormats[i].fFormat == format) {
            return gFormats[i].fSuffix;
        }
    }
    return "";
}

/**
 *  Base name for an image file created from the encoded data in an skp.
 */
static SkString gInputFileName;

/**
 *  Number to be appended to the image file name so that it is unique.
 */
static uint32_t gImageNo;

/**
 *  Set up the name for writing encoded data to a file.
 *  Sets gInputFileName to name, minus any extension ".*"
 *  Sets gImageNo to 0, so images from file "X.skp" will
 *  look like "X_<gImageNo>.<suffix>", beginning with 0
 *  for each new skp.
 */
static void reset_image_file_base_name(const SkString& name) {
    gImageNo = 0;
    // Remove ".skp"
    const char* cName = name.c_str();
    const char* dot = strrchr(cName, '.');
    if (dot != NULL) {
        gInputFileName.set(cName, dot - cName);
    } else {
        gInputFileName.set(name);
    }
}

/**
 *  Write the raw encoded bitmap data to a file.
 */
static bool write_image_to_file(const void* buffer, size_t size, SkBitmap* bitmap) {
    SkASSERT(!FLAGS_writePath.isEmpty());
    SkMemoryStream memStream(buffer, size);
    SkString outPath;
    SkImageDecoder::Format format = SkImageDecoder::GetStreamFormat(&memStream);
    SkString name = SkStringPrintf("%s_%d%s", gInputFileName.c_str(), gImageNo++,
                                   get_suffix_from_format(format));
    SkString dir(FLAGS_writePath[0]);
    outPath = SkOSPath::Join(dir.c_str(), name.c_str());
    SkFILEWStream fileStream(outPath.c_str());
    if (!(fileStream.isValid() && fileStream.write(buffer, size))) {
        SkDebugf("Failed to write encoded data to \"%s\"\n", outPath.c_str());
    }
    // Put in a dummy bitmap.
    return SkImageDecoder::DecodeStream(&memStream, bitmap, kUnknown_SkColorType,
                                        SkImageDecoder::kDecodeBounds_Mode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called only by render_picture().
 */
static bool render_picture_internal(const SkString& inputPath, const SkString* writePath,
                                    const SkString* mismatchPath,
                                    sk_tools::PictureRenderer& renderer,
                                    SkBitmap** out) {
    SkString inputFilename = SkOSPath::Basename(inputPath.c_str());
    SkString writePathString;
    if (writePath && writePath->size() > 0 && !FLAGS_writeEncodedImages) {
        writePathString.set(*writePath);
    }
    SkString mismatchPathString;
    if (mismatchPath && mismatchPath->size() > 0) {
        mismatchPathString.set(*mismatchPath);
    }

    SkFILEStream inputStream;
    inputStream.setPath(inputPath.c_str());
    if (!inputStream.isValid()) {
        SkDebugf("Could not open file %s\n", inputPath.c_str());
        return false;
    }

    SkPicture::InstallPixelRefProc proc;
    if (FLAGS_deferImageDecoding) {
        proc = &sk_tools::LazyDecodeBitmap;
    } else if (FLAGS_writeEncodedImages) {
        SkASSERT(!FLAGS_writePath.isEmpty());
        reset_image_file_base_name(inputFilename);
        proc = &write_image_to_file;
    } else {
        proc = &SkImageDecoder::DecodeMemory;
    }

    SkDebugf("deserializing... %s\n", inputPath.c_str());

    SkAutoTUnref<SkPicture> picture(SkPicture::CreateFromStream(&inputStream, proc));

    if (NULL == picture) {
        SkDebugf("Could not read an SkPicture from %s\n", inputPath.c_str());
        return false;
    }

    while (FLAGS_bench_record) {
        SkPictureRecorder recorder;
        picture->playback(recorder.beginRecording(picture->cullRect().width(), 
                                                  picture->cullRect().height(), 
                                                  NULL, 0));
        SkAutoTUnref<SkPicture> other(recorder.endRecording());
    }

    SkDebugf("drawing... [%f %f %f %f] %s\n", 
             picture->cullRect().fLeft, picture->cullRect().fTop,
             picture->cullRect().fRight, picture->cullRect().fBottom,
             inputPath.c_str());

    renderer.init(picture, &writePathString, &mismatchPathString, &inputFilename,
                  FLAGS_writeChecksumBasedFilenames, FLAGS_mpd);

    renderer.setup();
    renderer.enableWrites();

    bool success = renderer.render(out);
    if (!success) {
        SkDebugf("Failed to render %s\n", inputFilename.c_str());
    }

    renderer.end();

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
        if (fRenderer) {
            fRenderer->setBBoxHierarchyType(fSavedBbhType);
        }
    }

private:
    sk_tools::PictureRenderer* fRenderer;
    sk_tools::PictureRenderer::BBoxHierarchyType fSavedBbhType;
};

/**
 * Render the SKP file(s) within inputPath.
 *
 * @param inputPath path to an individual SKP file, or a directory of SKP files
 * @param writePath if not NULL, write all image(s) generated into this directory
 * @param mismatchPath if not NULL, write any image(s) not matching expectations into this directory
 * @param renderer PictureRenderer to use to render the SKPs
 * @param jsonSummaryPtr if not NULL, add the image(s) generated to this summary
 */
static bool render_picture(const SkString& inputPath, const SkString* writePath,
                           const SkString* mismatchPath, sk_tools::PictureRenderer& renderer,
                           sk_tools::ImageResultsAndExpectations *jsonSummaryPtr) {
    int diffs[256] = {0};
    SkBitmap* bitmap = NULL;
    renderer.setJsonSummaryPtr(jsonSummaryPtr);
    bool success = render_picture_internal(inputPath,
        FLAGS_writeWholeImage ? NULL : writePath,
        FLAGS_writeWholeImage ? NULL : mismatchPath,
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
#if SK_SUPPORT_GPU
            referenceRenderer = SkNEW_ARGS(sk_tools::SimplePictureRenderer,
                                           (renderer.getGrContextOptions()));
#else
            referenceRenderer = SkNEW(sk_tools::SimplePictureRenderer);
#endif
        }
        SkAutoTUnref<sk_tools::PictureRenderer> aurReferenceRenderer(referenceRenderer);

        success = render_picture_internal(inputPath, NULL, NULL, *referenceRenderer,
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

        SkString inputFilename = SkOSPath::Basename(inputPath.c_str());
        SkString outputFilename(inputFilename);
        sk_tools::replace_char(&outputFilename, '.', '_');
        outputFilename.append(".png");

        if (jsonSummaryPtr) {
            sk_tools::ImageDigest imageDigest(*bitmap);
            jsonSummaryPtr->add(inputFilename.c_str(), outputFilename.c_str(), imageDigest);
            if ((mismatchPath) && !mismatchPath->isEmpty() &&
                !jsonSummaryPtr->getExpectation(inputFilename.c_str()).matches(imageDigest)) {
                success &= sk_tools::write_bitmap_to_disk(*bitmap, *mismatchPath, NULL,
                                                          outputFilename);
            }
        }

        if ((writePath) && !writePath->isEmpty()) {
            success &= sk_tools::write_bitmap_to_disk(*bitmap, *writePath, NULL, outputFilename);
        }
    }
    SkDELETE(bitmap);

    return success;
}


static int process_input(const char* input, const SkString* writePath,
                         const SkString* mismatchPath, sk_tools::PictureRenderer& renderer,
                         sk_tools::ImageResultsAndExpectations *jsonSummaryPtr) {
    SkOSFile::Iter iter(input, "skp");
    SkString inputFilename;
    int failures = 0;
    SkDebugf("process_input, %s\n", input);
    if (iter.next(&inputFilename)) {
        do {
            SkString inputPath = SkOSPath::Join(input, inputFilename.c_str());
            if (!render_picture(inputPath, writePath, mismatchPath, renderer, jsonSummaryPtr)) {
                ++failures;
            }
        } while(iter.next(&inputFilename));
    } else if (SkStrEndsWith(input, ".skp")) {
        SkString inputPath(input);
        if (!render_picture(inputPath, writePath, mismatchPath, renderer, jsonSummaryPtr)) {
            ++failures;
        }
    } else {
        SkString warning;
        warning.printf("Warning: skipping %s\n", input);
        SkDebugf("%s", warning.c_str());
    }
    return failures;
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage("Render .skp files.");
    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_readPath.isEmpty()) {
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

    if (FLAGS_writeEncodedImages) {
        if (FLAGS_writePath.isEmpty()) {
            SkDebugf("--writeEncodedImages requires --writePath\n");
            exit(-1);
        }
        if (FLAGS_deferImageDecoding) {
            SkDebugf("--writeEncodedImages is not compatible with --deferImageDecoding\n");
            exit(-1);
        }
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

    SkString writePath;
    if (FLAGS_writePath.count() == 1) {
        writePath.set(FLAGS_writePath[0]);
    }
    SkString mismatchPath;
    if (FLAGS_mismatchPath.count() == 1) {
        mismatchPath.set(FLAGS_mismatchPath[0]);
    }
    sk_tools::ImageResultsAndExpectations jsonSummary;
    sk_tools::ImageResultsAndExpectations* jsonSummaryPtr = NULL;
    if (FLAGS_writeJsonSummaryPath.count() == 1) {
        jsonSummaryPtr = &jsonSummary;
        if (FLAGS_readJsonSummaryPath.count() == 1) {
            SkASSERT(jsonSummary.readExpectationsFile(FLAGS_readJsonSummaryPath[0]));
        }
    }

    int failures = 0;
    for (int i = 0; i < FLAGS_readPath.count(); i ++) {
        failures += process_input(FLAGS_readPath[i], &writePath, &mismatchPath, *renderer.get(),
                                  jsonSummaryPtr);
    }
    if (failures != 0) {
        SkDebugf("Failed to render %i pictures.\n", failures);
        return 1;
    }
#if GR_CACHE_STATS && SK_SUPPORT_GPU
    if (renderer->isUsingGpuDevice()) {
        GrContext* ctx = renderer->getGrContext();
        ctx->printCacheStats();
    }
#endif

#if GR_GPU_STATS && SK_SUPPORT_GPU
    if (FLAGS_gpuStats && renderer->isUsingGpuDevice()) {
        renderer->getGrContext()->printGpuStats();
    }
#endif

    if (FLAGS_writeJsonSummaryPath.count() == 1) {
        // If there were any descriptions on the command line, insert them now.
        for (int i=0; i<FLAGS_descriptions.count(); i++) {
            SkTArray<SkString> tokens;
            SkStrSplit(FLAGS_descriptions[i], "=", &tokens);
            SkASSERT(tokens.count() == 2);
            jsonSummary.addDescription(tokens[0].c_str(), tokens[1].c_str());
        }
        if (FLAGS_imageBaseGSUrl.count() == 1) {
          jsonSummary.setImageBaseGSUrl(FLAGS_imageBaseGSUrl[0]);
        }
        jsonSummary.writeToFile(FLAGS_writeJsonSummaryPath[0]);
    }
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
