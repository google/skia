/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "PdfRenderer.h"
#include "picture_utils.h"

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
"     %s <input>... -w <outputDir> \n"
, argv0);
    SkDebugf("\n\n");
    SkDebugf(
"     input:     A list of directories and files to use as input. Files are\n"
"                expected to have the .skp extension.\n\n");
    SkDebugf(
"     outputDir: directory to write the rendered pdfs.\n\n");
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
static bool write_output(const SkString& outputDir,
                         const SkString& inputFilename,
                         const sk_tools::PdfRenderer& renderer) {
    if (outputDir.isEmpty()) {
        SkDynamicMemoryWStream stream;
        renderer.write(&stream);
        return true;
    }

    SkString outputPath;
    if (!make_output_filepath(&outputPath, outputDir, inputFilename)) {
        return false;
    }

    SkFILEWStream stream(outputPath.c_str());
    if (!stream.isValid()) {
        SkDebugf("Could not write to file %s\n", outputPath.c_str());
        return false;
    }
    renderer.write(&stream);

    return true;
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

    bool success = false;
    SkAutoTUnref<SkPicture>
        picture(SkNEW_ARGS(SkPicture, (&inputStream, &success)));

    if (!success) {
        SkDebugf("Could not read an SkPicture from %s\n", inputPath.c_str());
        return false;
    }

    SkDebugf("exporting... [%i %i] %s\n", picture->width(), picture->height(),
             inputPath.c_str());

    renderer.init(picture);

    renderer.render();

    success = write_output(outputDir, inputFilename, renderer);

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
        } else {
            inputs->push_back(SkString(*argv));
        }
    }

    if (inputs->count() < 1) {
        usage(argv0);
        exit(-1);
    }
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {

    SkAutoGraphics ag;
    SkTArray<SkString> inputs;

    SkAutoTUnref<sk_tools::PdfRenderer>
        renderer(SkNEW(sk_tools::SimplePdfRenderer));
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

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
