/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkDocument.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkMD5.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkTSort.h"

#include "SkDmuxWStream.h"

static const char kUsage[] =
    "This program takes a list of Skia Picture (SKP) files and renders\n"
    "each as a multipage PDF, then prints out the MD5 checksum of the\n"
    "PDF file.  This can be used to verify that changes to the PDF\n"
    "backend will not change PDF output.\n";

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_string2(inputPaths,
               r,
               "",
               "A list of directories and files to use as input.\n"
               "Files are expected to have the .skp extension.");

DEFINE_string2(outputDirectoryPath, w, "", "TODO: document this");

static const char SKP_FILE_EXTENSION[] = ".skp";
static const char PDF_FILE_EXTENSION[] = ".pdf";

// Used by SkTQSort<SkString>()
static bool operator<(const SkString& a, const SkString& b) {
    return strcmp(a.c_str(), b.c_str()) < 0;
}

// Process --inputPaths defined on the command line.  Return false if
// no files found.
static bool process_input_files(SkTArray<SkString>* files) {
    for (int i = 0; i < FLAGS_inputPaths.count(); i++) {
        const char* input = FLAGS_inputPaths[i];
        if (sk_isdir(input)) {
            SkOSFile::Iter iter(input, SKP_FILE_EXTENSION);
            SkString inputFilename;
            while (iter.next(&inputFilename)) {
                files->push_back(SkOSPath::Join(input, inputFilename.c_str()));
            }
        } else {
            if (SkStrEndsWith(input, SKP_FILE_EXTENSION)) {
                if (sk_exists(input)) {
                    files->push_back(SkString(input));
                } else {
                    SkDebugf("file_does_not_exist %s\n", input);
                }
            } else {
                SkDebugf("skipping_file %s\n", input);
            }
        }
    }
    if (files->count() > 0) {
        SkTQSort<SkString>(files->begin(), files->end() - 1);
        return true;
    }
    return false;
}

// Print the given SkPicture to a PDF, breaking on 8.5x11 pages.
static void picture_to_pdf(const SkPicture& picture, SkWStream* out) {
    SkAutoTUnref<SkDocument> pdfDocument(SkDocument::CreatePDF(out));

    int width = picture.cullRect().width();
    int height = picture.cullRect().height();

    const int kLetterWidth = 612;   // 8.5 * 72
    const int kLetterHeight = 792;  // 11 * 72
    SkRect letterRect = SkRect::MakeWH(SkIntToScalar(kLetterWidth),
                                       SkIntToScalar(kLetterHeight));

    int xPages = ((width - 1) / kLetterWidth) + 1;
    int yPages = ((height - 1) / kLetterHeight) + 1;

    for (int y = 0; y < yPages; ++y) {
        for (int x = 0; x < xPages; ++x) {
            int w = SkTMin(kLetterWidth, width - (x * kLetterWidth));
            int h = SkTMin(kLetterHeight, height - (y * kLetterHeight));
            SkCanvas* canvas = pdfDocument->beginPage(w, h);
            canvas->clipRect(letterRect);
            canvas->translate(SkIntToScalar(-kLetterWidth * x),
                              SkIntToScalar(-kLetterHeight * y));
            canvas->drawPicture(&picture);
            canvas->flush();
            pdfDocument->endPage();
        }
    }
    pdfDocument->close();
    out->flush();
}

static bool skp_to_pdf_md5(SkStream* input, SkMD5::Digest* digest) {
    SkAutoTUnref<SkPicture> picture(SkPicture::CreateFromStream(input));
    if (NULL == picture.get()) {
        return false;
    }

    SkMD5 checksumWStream;
    picture_to_pdf(*picture, &checksumWStream);
    checksumWStream.finish(*digest);
    return true;
}

static bool skp_to_pdf_and_md5(SkStream* input,
                               const char* path,
                               SkMD5::Digest* digest) {
    SkAutoTUnref<SkPicture> picture(SkPicture::CreateFromStream(input));
    if (NULL == picture.get()) {
        return false;
    }

    SkMD5 checksumWStream;
    SkFILEWStream fileWStream(path);
    SkWStream* wStreamArray[] = {&checksumWStream, &fileWStream};
    SkDmuxWStream dmuxWStream(wStreamArray, SK_ARRAY_COUNT(wStreamArray));

    picture_to_pdf(*picture, &dmuxWStream);
    checksumWStream.finish(*digest);
    return true;
}

SkString digest_to_hex(const SkMD5::Digest& digest) {
    static const char kHex[] = "0123456789ABCDEF";
    SkString string(2 * sizeof(digest.data));
    char* p = string.writable_str();
    for (size_t i = 0; i < sizeof(digest.data); ++i) {
        uint8_t c = digest.data[i];
        *(p++) = kHex[c >> 4];
        *(p++) = kHex[c & 0xF];
    }
    return string;
}

static void str_replace_ending(SkString* str,
                               const char* oldExt,
                               const char* newExt) {
    SkASSERT(str->endsWith(oldExt));
    SkASSERT(str->size() >= strlen(oldExt));
    str->remove(str->size() - strlen(oldExt), strlen(oldExt));
    str->append(newExt);
}

int main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage(kUsage);
    SkCommandLineFlags::Parse(argc, argv);
    const char* outputDir = FLAGS_outputDirectoryPath.count() > 0
                                    ? FLAGS_outputDirectoryPath[0]
                                    : NULL;
    if (outputDir) {
        sk_mkdir(outputDir);
    }

    SkAutoGraphics ag;
    int successCount = 0;
    SkTArray<SkString> files;
    if (!process_input_files(&files)) {
        SkDebugf("You need to specify a --inputPaths option.\n");
        return 1;
    }
    for (int i = 0; i < files.count(); ++i) {
        SkString basename = SkOSPath::Basename(files[i].c_str());
        SkFILEStream inputStream(files[i].c_str());
        if (!inputStream.isValid()) {
            SkDebugf("could_not_open %s\n", basename.c_str());
            continue;
        }
        SkMD5::Digest digest;

        if (outputDir) {
            SkString path = SkOSPath::Join(outputDir, basename.c_str());
            str_replace_ending(&path, SKP_FILE_EXTENSION, PDF_FILE_EXTENSION);
            if (!skp_to_pdf_and_md5(&inputStream, path.c_str(), &digest)) {
                SkDebugf("invalid_skp %s\n", basename.c_str());
                continue;
            }
        } else {
            if (!skp_to_pdf_md5(&inputStream, &digest)) {
                SkDebugf("invalid_skp %s\n", basename.c_str());
                continue;
            }
        }
        SkString hexDigest = digest_to_hex(digest);
        printf("%s %s\n", hexDigest.c_str(), basename.c_str());
        ++successCount;
    }
    return successCount == files.count() ? 0 : 1;
}

