/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkDocument.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkTSort.h"
#include "ProcStats.h"

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
    *path = SkOSPath::Join(dir.c_str(), name.c_str());
    return replace_filename_extension(path,
                                      SKP_FILE_EXTENSION,
                                      PDF_FILE_EXTENSION);
}

namespace {
// This is a write-only stream.
class NullWStream : public SkWStream {
public:
    NullWStream() : fBytesWritten(0) { }
    bool write(const void*, size_t size) override {
        fBytesWritten += size;
        return true;
    }
    size_t bytesWritten() const override { return fBytesWritten; }
    size_t fBytesWritten;
};
}  // namespace

/** Write the output of pdf renderer to a file.
 * @param outputDir Output dir.
 * @param inputFilename The skp file that was read.
 * @param renderer The object responsible to write the pdf file.
 */
static SkWStream* open_stream(const SkString& outputDir,
                              const SkString& inputFilename) {
    if (outputDir.isEmpty()) {
        return SkNEW(NullWStream);
    }

    SkString outputPath;
    if (!make_output_filepath(&outputPath, outputDir, inputFilename)) {
        return NULL;
    }

    SkAutoTDelete<SkFILEWStream> stream(
            SkNEW_ARGS(SkFILEWStream, (outputPath.c_str())));
    if (!stream.get() ||  !stream->isValid()) {
        SkDebugf("Could not write to file %s\n", outputPath.c_str());
        return NULL;
    }

    return stream.detach();
}

/**
 *  Given a SkPicture, write a one-page PDF document to the given
 *  output, using the provided encoder.
 */
static bool pdf_to_stream(SkPicture* picture,
                          SkWStream* output) {
    SkAutoTUnref<SkDocument> pdfDocument(
            SkDocument::CreatePDF(output));
    SkCanvas* canvas = pdfDocument->beginPage(picture->cullRect().width(), 
                                              picture->cullRect().height());
    canvas->drawPicture(picture);
    canvas->flush();
    return pdfDocument->close();
}

static bool operator<(const SkString& a, const SkString& b) {
    return strcmp(a.c_str(), b.c_str()) < 0;
}

/**
 * @param A list of directories or a skp files.
 * @returns an alphabetical list of skp files.
 */
static void process_input_files(
        const SkCommandLineFlags::StringArray& inputs,
        SkTArray<SkString>* files) {
    for (int i = 0; i < inputs.count(); i ++) {
        const char* input = inputs[i];
        if (sk_isdir(input)) {
            SkOSFile::Iter iter(input, SKP_FILE_EXTENSION);
            SkString inputFilename;
            while (iter.next(&inputFilename)) {
                if (!SkCommandLineFlags::ShouldSkip(
                            FLAGS_match, inputFilename.c_str())) {
                    files->push_back(
                            SkOSPath::Join(input, inputFilename.c_str()));
                }
            }
        } else {
            if (!SkCommandLineFlags::ShouldSkip(FLAGS_match, input)) {
                files->push_back(SkString(input));
            }
        }
    }
    if (files->count() > 0) {
        SkTQSort<SkString>(files->begin(), files->end() - 1);
    }
}

/** For each input skp file, read it, render it to pdf and write. the
 *  output to a pdf file
 */
int tool_main_core(int argc, char** argv);
int tool_main_core(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);

    SkAutoGraphics ag;

    SkString outputDir;
    if (FLAGS_outputDir.count() > 0) {
        outputDir = FLAGS_outputDir[0];
        if (!sk_mkdir(outputDir.c_str())) {
            SkDebugf("Unable to mkdir '%s'\n", outputDir.c_str());
            return 1;
        }
    }

    SkTArray<SkString> files;
    process_input_files(FLAGS_inputPaths, &files);

    size_t maximumPathLength = 0;
    for (int i = 0; i < files.count(); i ++) {
        SkString basename = SkOSPath::Basename(files[i].c_str());
        maximumPathLength = SkTMax(maximumPathLength, basename.size());
    }

    int failures = 0;
    for (int i = 0; i < files.count(); i ++) {
        SkString basename = SkOSPath::Basename(files[i].c_str());

        SkFILEStream inputStream;
        inputStream.setPath(files[i].c_str());
        if (!inputStream.isValid()) {
            SkDebugf("Could not open file %s\n", files[i].c_str());
            ++failures;
            continue;
        }

        SkAutoTUnref<SkPicture> picture(
                SkPicture::CreateFromStream(&inputStream));
        if (NULL == picture.get()) {
            SkDebugf("Could not read an SkPicture from %s\n",
                     files[i].c_str());
            ++failures;
            continue;
        }
        SkDebugf("[%6g %6g %6g %6g] %-*s",
            picture->cullRect().fLeft, picture->cullRect().fTop,
            picture->cullRect().fRight, picture->cullRect().fBottom,
            maximumPathLength, basename.c_str());

        SkAutoTDelete<SkWStream> stream(open_stream(outputDir, files[i]));
        if (!stream.get()) {
            ++failures;
            continue;
        }
        if (!pdf_to_stream(picture, stream.get())) {
            SkDebugf("Error in PDF Serialization.");
            ++failures;
        }

        int max_rss_mb = sk_tools::getMaxResidentSetSizeMB();
        if (max_rss_mb >= 0) {
            SkDebugf(" %4dM peak rss", max_rss_mb);
        }

        SkDebugf("\n");
    }
    if (failures != 0) {
        SkDebugf("Failed to render %i of %i PDFs.\n", failures, files.count());
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
