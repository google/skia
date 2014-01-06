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
DECLARE_string(readPath);
DEFINE_bool(writeEncodedImages, false, "Any time the skp contains an encoded image, write it to a "
            "file rather than decoding it. Requires writePath to be set. Skips drawing the full "
            "skp to a file. Not compatible with deferImageDecoding.");
DEFINE_string(writeJsonSummaryPath, "", "File to write a JSON summary of image results to. "
              "TODO(epoger): Currently, this only works if --writePath is also specified.");
DEFINE_string2(writePath, w, "", "Directory to write the rendered images.");
DEFINE_bool(writeWholeImage, false, "In tile mode, write the entire rendered image to a "
            "file, instead of an image for each tile.");
DEFINE_bool(validate, false, "Verify that the rendered image contains the same pixels as "
            "the picture rendered in simple mode. When used in conjunction with --bbh, results "
            "are validated against the picture rendered in the same mode, but without the bbh.");

DEFINE_bool(bench_record, false, "If true, drop into an infinite loop of recording the picture.");

static void make_output_filepath(SkString* path, const SkString& dir,
                                 const SkString& name) {
    sk_tools::make_filepath(path, dir, name);
    // Remove ".skp"
    path->remove(path->size() - 4, 4);
}

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
    sk_tools::make_filepath(&outPath, dir, name);
    SkFILEWStream fileStream(outPath.c_str());
    if (!(fileStream.isValid() && fileStream.write(buffer, size))) {
        SkDebugf("Failed to write encoded data to \"%s\"\n", outPath.c_str());
    }
    // Put in a dummy bitmap.
    return SkImageDecoder::DecodeStream(&memStream, bitmap, SkBitmap::kNo_Config,
                                        SkImageDecoder::kDecodeBounds_Mode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called only by render_picture().
 */
static bool render_picture_internal(const SkString& inputPath, const SkString* outputDir,
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

    SkPicture* picture = SkPicture::CreateFromStream(&inputStream, proc);

    if (NULL == picture) {
        SkDebugf("Could not read an SkPicture from %s\n", inputPath.c_str());
        return false;
    }

    while (FLAGS_bench_record) {
        const int kRecordFlags = 0;
        SkPicture other;
        picture->draw(other.beginRecording(picture->width(), picture->height(), kRecordFlags));
        other.endRecording();
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
    if (NULL != outputDir && outputDir->size() > 0 && !FLAGS_writeEncodedImages) {
        outputPath = SkNEW(SkString);
        make_output_filepath(outputPath, *outputDir, inputFilename);
    }

    bool success = renderer.render(outputPath, out);
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

/**
 * Render the SKP file(s) within inputPath, writing their bitmap images into outputDir.
 *
 * @param inputPath path to an individual SKP file, or a directory of SKP files
 * @param outputDir if not NULL, write the image(s) generated into this directory
 * @param renderer PictureRenderer to use to render the SKPs
 * @param jsonSummaryPtr if not NULL, add the image(s) generated to this summary
 */
static bool render_picture(const SkString& inputPath, const SkString* outputDir,
                           sk_tools::PictureRenderer& renderer,
                           sk_tools::ImageResultsSummary *jsonSummaryPtr) {
    int diffs[256] = {0};
    SkBitmap* bitmap = NULL;
    renderer.setJsonSummaryPtr(jsonSummaryPtr);
    bool success = render_picture_internal(inputPath,
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

        success = render_picture_internal(inputPath, NULL, *referenceRenderer,
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

        if (NULL != jsonSummaryPtr) {
            // EPOGER: This is a hacky way of constructing the filename associated with the
            // image checksum; we basically are repeating the logic of make_output_filepath()
            // and code below here, within here.
            // It would be better for the filename (without outputDir) to be passed in here,
            // and used both for the checksum file and writing into outputDir.
            //
            // EPOGER: what about including the config type within hashFilename?  That way,
            // we could combine results of different config types without conflicting filenames.
            SkString hashFilename;
            sk_tools::get_basename(&hashFilename, inputPath);
            hashFilename.remove(hashFilename.size() - 4, 4); // Remove ".skp"
            hashFilename.append(".png");
            jsonSummaryPtr->add(hashFilename.c_str(), *bitmap);
        }

        if (NULL != outputDir) {
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
                         sk_tools::PictureRenderer& renderer,
                         sk_tools::ImageResultsSummary *jsonSummaryPtr) {
    SkOSFile::Iter iter(input, "skp");
    SkString inputFilename;
    int failures = 0;
    SkDebugf("process_input, %s\n", input);
    if (iter.next(&inputFilename)) {
        do {
            SkString inputPath;
            SkString inputAsSkString(input);
            sk_tools::make_filepath(&inputPath, inputAsSkString, inputFilename);
            if (!render_picture(inputPath, outputDir, renderer, jsonSummaryPtr)) {
                ++failures;
            }
        } while(iter.next(&inputFilename));
    } else if (SkStrEndsWith(input, ".skp")) {
        SkString inputPath(input);
        if (!render_picture(inputPath, outputDir, renderer, jsonSummaryPtr)) {
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

    if (FLAGS_clone < 0) {
        SkDebugf("--clone must be >= 0. Was %i\n", FLAGS_clone);
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

    SkString outputDir;
    if (FLAGS_writePath.count() == 1) {
        outputDir.set(FLAGS_writePath[0]);
    }
    sk_tools::ImageResultsSummary jsonSummary;
    sk_tools::ImageResultsSummary* jsonSummaryPtr = NULL;
    if (FLAGS_writeJsonSummaryPath.count() == 1) {
        jsonSummaryPtr = &jsonSummary;
    }

    int failures = 0;
    for (int i = 0; i < FLAGS_readPath.count(); i ++) {
        failures += process_input(FLAGS_readPath[i], &outputDir, *renderer.get(), jsonSummaryPtr);
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
#ifdef SK_DEVELOPER
        ctx->dumpFontCache();
#endif
    }
#endif
#endif
    if (FLAGS_writeJsonSummaryPath.count() == 1) {
        jsonSummary.writeToFile(FLAGS_writeJsonSummaryPath[0]);
    }
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
