/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkDevice.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkPixelRef.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkTSort.h"
#include "PdfRenderer.h"
#include "ProcStats.h"
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


DEFINE_string2(inputPaths, r, "",
              "A list of directories and files to use as input. "
              "Files are expected to have the .skp extension.");

DEFINE_string2(outputDir, w, "",
               "Directory to write the rendered pdfs.");

DEFINE_string2(match, m, "",
               "[~][^]substring[$] [...] of filenames to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching file to always be skipped\n"
               "^ requires the start of the file to match\n"
               "$ requires the end of the file to match\n"
               "^ and $ requires an exact match\n"
               "If a file does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");

DEFINE_int32(jpegQuality, 100,
             "Encodes images in JPEG at quality level N, which can be in "
             "range 0-100).   N = -1 will disable JPEG compression. "
             "Default is N = 100, maximum quality.");

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

// the size_t* parameter is deprecated, so we ignore it
static SkData* encode_to_dct_data(size_t*, const SkBitmap& bitmap) {
    if (FLAGS_jpegQuality == -1) {
        return NULL;
    }

    SkBitmap bm = bitmap;
#if defined(SK_BUILD_FOR_MAC)
    // Workaround bug #1043 where bitmaps with referenced pixels cause
    // CGImageDestinationFinalize to crash
    SkBitmap copy;
    bitmap.deepCopyTo(&copy);
    bm = copy;
#endif

    return SkImageEncoder::EncodeData(
            bm, SkImageEncoder::kJPEG_Type, FLAGS_jpegQuality);
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
    *path = SkOSPath::Join(dir.c_str(), name.c_str());
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
    SkString inputFilename = SkOSPath::Basename(inputPath.c_str());

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

    SkDebugf("exporting... [%-4i %6i] %s\n",
             picture->width(), picture->height(), inputPath.c_str());

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

static bool operator<(const SkString& a, const SkString& b) {
    return strcmp(a.c_str(), b.c_str()) < 0;
}

/** For each file in the directory or for the file passed in input, call
 * render_pdf.
 * @param input A directory or an skp file.
 * @param outputDir Output dir.
 * @param renderer The object responsible to render the skp object into pdf.
 */
static int process_input(
        const SkCommandLineFlags::StringArray& inputs,
        const SkString& outputDir,
        sk_tools::PdfRenderer& renderer) {
    SkTArray<SkString> files;
    for (int i = 0; i < inputs.count(); i ++) {
        const char* input = inputs[i];
        if (sk_isdir(input)) {
            SkOSFile::Iter iter(input, SKP_FILE_EXTENSION);
            SkString inputFilename;
            while (iter.next(&inputFilename)) {
                if (!SkCommandLineFlags::ShouldSkip(
                            FLAGS_match, inputFilename.c_str())) {
                    files.push_back(
                            SkOSPath::Join(input, inputFilename.c_str()));
                }
            }
        } else {
            if (!SkCommandLineFlags::ShouldSkip(FLAGS_match, input)) {
                files.push_back(SkString(input));
            }
        }
    }
    if (files.count() > 0) {
        SkTQSort<SkString>(files.begin(), files.end() - 1);
    }
    int failures = 0;
    for (int i = 0; i < files.count(); i ++) {
        if (!render_pdf(files[i], outputDir, renderer)) {
            ++failures;
        }
    }
    return failures;
}

int tool_main_core(int argc, char** argv);
int tool_main_core(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);

    SkAutoGraphics ag;

    SkAutoTUnref<sk_tools::PdfRenderer>
        renderer(SkNEW_ARGS(sk_tools::SimplePdfRenderer, (encode_to_dct_data)));
    SkASSERT(renderer.get());

    SkString outputDir;
    if (FLAGS_outputDir.count() > 0) {
        outputDir = FLAGS_outputDir[0];
    }

    int failures = process_input(FLAGS_inputPaths, outputDir, *renderer);

    int max_rss_kb = sk_tools::getMaxResidentSetSizeKB();
    if (max_rss_kb >= 0) {
        SkDebugf("%4dM peak ResidentSetSize\n", max_rss_kb / 1024);
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
