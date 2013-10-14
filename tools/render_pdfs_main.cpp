/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkPixelRef.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "PdfRenderer.h"
#include "picture_utils.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

#ifdef SK_USE_CDB
#include "win_dbghelp.h"
#endif

/**
 * render_pdfs
 *
 * Given list of directories and files to use as input, expects to find .skp
 * files and it will convert them to .pdf files writing them in the output
 * directory.
 *
 * Returns zero exit code if all .skp files were converted successfully,
 * otherwise returns error code 1.
 */

static const char PDF_FILE_EXTENSION[] = "pdf";
static const char SKP_FILE_EXTENSION[] = "skp";

static void usage(const char* argv0) {
    SkDebugf("SKP to PDF rendering tool\n");
    SkDebugf("\n"
"Usage: \n"
"     %s <input>... [-w <outputDir>] [--jpegQuality N] \n"
, argv0);
    SkDebugf("\n\n");
    SkDebugf(
"     input:     A list of directories and files to use as input. Files are\n"
"                expected to have the .skp extension.\n\n");
    SkDebugf(
"     outputDir: directory to write the rendered pdfs.\n\n");
    SkDebugf("\n");
        SkDebugf(
"     jpegQuality N: encodes images in JPEG at quality level N, which can\n"
"                    be in range 0-100).\n"
"                    N = -1 will disable JPEG compression.\n"
"                    Default is N = 100, maximum quality.\n\n");
    SkDebugf("\n");
}

/** Replaces the extension of a file.
 * @param path File name whose extension will be changed.
 * @param old_extension The old extension.
 * @param new_extension The new extension.
 * @returns false if the file did not has the expected extension.
 *  if false is returned, contents of path are undefined.
 */
static bool replace_filename_extension(SkString* path,
                                       const char old_extension[],
                                       const char new_extension[]) {
    if (path->endsWith(old_extension)) {
        path->remove(path->size() - strlen(old_extension),
                     strlen(old_extension));
        if (!path->endsWith(".")) {
            return false;
        }
        path->append(new_extension);
        return true;
    }
    return false;
}

int gJpegQuality = 100;
static SkData* encode_to_dct_data(size_t* pixelRefOffset, const SkBitmap& bitmap) {
    if (gJpegQuality == -1) {
        return NULL;
    }

    SkBitmap bm = bitmap;
#if defined(SK_BUILD_FOR_MAC)
    // Workaround bug #1043 where bitmaps with referenced pixels cause
    // CGImageDestinationFinalize to crash
    SkBitmap copy;
    bitmap.deepCopyTo(&copy, bitmap.config());
    bm = copy;
#endif

    SkPixelRef* pr = bm.pixelRef();
    if (pr != NULL) {
        SkData* data = pr->refEncodedData();
        if (data != NULL) {
            *pixelRefOffset = bm.pixelRefOffset();
            return data;
        }
    }

    *pixelRefOffset = 0;
    return SkImageEncoder::EncodeData(bm,
                                      SkImageEncoder::kJPEG_Type,
                                      gJpegQuality);
}

/** Builds the output filename. path = dir/name, and it replaces expected
 * .skp extension with .pdf extention.
 * @param path Output filename.
 * @param name The name of the file.
 * @returns false if the file did not has the expected extension.
 *  if false is returned, contents of path are undefined.
 */
static bool make_output_filepath(SkString* path, const SkString& dir,
                                 const SkString& name) {
    sk_tools::make_filepath(path, dir, name);
    return replace_filename_extension(path,
                                      SKP_FILE_EXTENSION,
                                      PDF_FILE_EXTENSION);
}

/** Write the output of pdf renderer to a file.
 * @param outputDir Output dir.
 * @param inputFilename The skp file that was read.
 * @param renderer The object responsible to write the pdf file.
 */
static SkWStream* open_stream(const SkString& outputDir,
                              const SkString& inputFilename) {
    if (outputDir.isEmpty()) {
        return SkNEW(SkDynamicMemoryWStream);
    }

    SkString outputPath;
    if (!make_output_filepath(&outputPath, outputDir, inputFilename)) {
        return NULL;
    }

    SkFILEWStream* stream = SkNEW_ARGS(SkFILEWStream, (outputPath.c_str()));
    if (!stream->isValid()) {
        SkDebugf("Could not write to file %s\n", outputPath.c_str());
        return NULL;
    }

    return stream;
}

/** Reads an skp file, renders it to pdf and writes the output to a pdf file
 * @param inputPath The skp file to be read.
 * @param outputDir Output dir.
 * @param renderer The object responsible to render the skp object into pdf.
 */
static bool render_pdf(const SkString& inputPath, const SkString& outputDir,
                       sk_tools::PdfRenderer& renderer) {
    SkString inputFilename;
    sk_tools::get_basename(&inputFilename, inputPath);

    SkFILEStream inputStream;
    inputStream.setPath(inputPath.c_str());
    if (!inputStream.isValid()) {
        SkDebugf("Could not open file %s\n", inputPath.c_str());
        return false;
    }

    SkAutoTUnref<SkPicture> picture(SkPicture::CreateFromStream(&inputStream));

    if (NULL == picture.get()) {
        SkDebugf("Could not read an SkPicture from %s\n", inputPath.c_str());
        return false;
    }

    SkDebugf("exporting... [%i %i] %s\n", picture->width(), picture->height(),
             inputPath.c_str());

    SkWStream* stream(open_stream(outputDir, inputFilename));

    if (!stream) {
        return false;
    }

    renderer.init(picture, stream);

    bool success = renderer.render();
    SkDELETE(stream);

    renderer.end();

    return success;
}

/** For each file in the directory or for the file passed in input, call
 * render_pdf.
 * @param input A directory or an skp file.
 * @param outputDir Output dir.
 * @param renderer The object responsible to render the skp object into pdf.
 */
static int process_input(const SkString& input, const SkString& outputDir,
                         sk_tools::PdfRenderer& renderer) {
    int failures = 0;
    if (sk_isdir(input.c_str())) {
        SkOSFile::Iter iter(input.c_str(), SKP_FILE_EXTENSION);
        SkString inputFilename;
        while (iter.next(&inputFilename)) {
            SkString inputPath;
            sk_tools::make_filepath(&inputPath, input, inputFilename);
            if (!render_pdf(inputPath, outputDir, renderer)) {
                ++failures;
            }
        }
    } else {
        SkString inputPath(input);
        if (!render_pdf(inputPath, outputDir, renderer)) {
            ++failures;
        }
    }
    return failures;
}

static void parse_commandline(int argc, char* const argv[],
                              SkTArray<SkString>* inputs,
                              SkString* outputDir) {
    const char* argv0 = argv[0];
    char* const* stop = argv + argc;

    for (++argv; argv < stop; ++argv) {
        if ((0 == strcmp(*argv, "-h")) || (0 == strcmp(*argv, "--help"))) {
            usage(argv0);
            exit(-1);
        } else if (0 == strcmp(*argv, "-w")) {
            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing outputDir for -w\n");
                usage(argv0);
                exit(-1);
            }
            *outputDir = SkString(*argv);
        } else if (0 == strcmp(*argv, "--jpegQuality")) {
            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing argument for --jpegQuality\n");
                usage(argv0);
                exit(-1);
            }
            gJpegQuality = atoi(*argv);
            if (gJpegQuality < -1 || gJpegQuality > 100) {
                SkDebugf("Invalid argument for --jpegQuality\n");
                usage(argv0);
                exit(-1);
            }
        } else {
            inputs->push_back(SkString(*argv));
        }
    }

    if (inputs->count() < 1) {
        usage(argv0);
        exit(-1);
    }
}

int tool_main_core(int argc, char** argv);
int tool_main_core(int argc, char** argv) {
    SkAutoGraphics ag;
    SkTArray<SkString> inputs;

    SkAutoTUnref<sk_tools::PdfRenderer>
        renderer(SkNEW_ARGS(sk_tools::SimplePdfRenderer, (encode_to_dct_data)));
    SkASSERT(renderer.get());

    SkString outputDir;
    parse_commandline(argc, argv, &inputs, &outputDir);

    int failures = 0;
    for (int i = 0; i < inputs.count(); i ++) {
        failures += process_input(inputs[i], outputDir, *renderer);
    }

    if (failures != 0) {
        SkDebugf("Failed to render %i PDFs.\n", failures);
        return 1;
    }

    return 0;
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
#ifdef SK_USE_CDB
    setUpDebuggingFromArgs(argv[0]);
    __try {
#endif
      return tool_main_core(argc, argv);
#ifdef SK_USE_CDB
    }
    __except(GenerateDumpAndPrintCallstack(GetExceptionInformation()))
    {
        return -1;
    }
#endif
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
