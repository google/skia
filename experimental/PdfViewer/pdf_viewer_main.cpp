/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkDevice.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPdfConfig.h"
#include "SkPdfRenderer.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkTypeface.h"
#include "SkTArray.h"
#include "SkNulCanvas.h"

DEFINE_string2(readPath, r, "", "pdf files or directories of pdf files to process.");
DEFINE_string2(writePath, w, "", "Directory to write the rendered pages.");
DEFINE_bool2(noExtensionForOnePagePdf, n, false, "No page extension if only one page.");
DEFINE_bool2(showMemoryUsage, m, false, "Show Memory usage.");
DEFINE_string2(pages, p, "all", "What pages to render and how:\n"
                                "\tall - all pages\n"
                                "\treverse - all pages, in reverse order\n"
                                "\tfirst - first page\n"
                                "\tlast - last page\n"
                                "\tnumber - a specific page number\n"
               );
DEFINE_double(DPI, 72, "DPI to be used for rendering (scale).");
DEFINE_int32(benchLoad, 0, "Load the pdf file minimally N times, without any rendering and \n"
             "\tminimal parsing to ensure correctness. Default 0 (disabled).");
DEFINE_int32(benchRender, 0, "Render the pdf content N times. Default 0 (disabled)");
DEFINE_string2(config, c, "8888", "Canvas to render:\n"
                                  "\t8888 - argb\n"
                                  "\tnul - render in null canvas, any draw will just return.\n"
               );
DEFINE_bool2(transparentBackground, t, false, "Make background transparent instead of white.");

/**
 * Given list of directories and files to use as input, expects to find .pdf
 * files and it will convert them to .png files writing them in the same directory
 * one file for each page.
 *
 * Returns zero exit code if all .pdf files were converted successfully,
 * otherwise returns error code 1.
 */

static const char PDF_FILE_EXTENSION[] = "pdf";
static const char PNG_FILE_EXTENSION[] = "png";

/** Replaces the extension of a file.
 * @param path File name whose extension will be changed.
 * @param old_extension The old extension.
 * @param new_extension The new extension.
 * @returns false if the file did not has the expected extension.
 *  if false is returned, contents of path are undefined.
 */
static bool add_page_and_replace_filename_extension(SkString* path, int page,
                                       const char old_extension[],
                                       const char new_extension[]) {
    if (path->endsWith(old_extension)) {
        path->remove(path->size() - strlen(old_extension),
                     strlen(old_extension));
        if (!path->endsWith(".")) {
            return false;
        }
        if (page >= 0) {
            path->appendf("%i.", page);
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
                                 const SkString& name,
                                 int page) {
    *path = SkOSPath::Join(dir.c_str(), name.c_str());
    return add_page_and_replace_filename_extension(path, page,
                                                   PDF_FILE_EXTENSION,
                                                   PNG_FILE_EXTENSION);
}

static void setup_bitmap(SkBitmap* bitmap, int width, int height, SkColor color) {
    bitmap->allocN32Pixels(width, height);
    bitmap->eraseColor(color);
}

/** Write the output of pdf renderer to a file.
 * @param outputDir Output dir.
 * @param inputFilename The skp file that was read.
 * @param renderer The object responsible to write the pdf file.
 * @param page -1 means there is only one page (0), and render in a file without page extension
 */

#ifdef PDF_TRACE_DIFF_IN_PNG
extern "C" SkBitmap* gDumpBitmap;
extern "C" SkCanvas* gDumpCanvas;
#endif

static bool render_page(const SkString& outputDir,
                        const SkString& inputFilename,
                        const SkPdfRenderer& renderer,
                        int page) {
    SkRect rect = renderer.MediaBox(page < 0 ? 0 :page);

    // Exercise all pdf codepaths as in normal rendering, but no actual bits are changed.
    if (!FLAGS_config.isEmpty() && strcmp(FLAGS_config[0], "nul") == 0) {
        SkBitmap bitmap;
        SkAutoTUnref<SkBaseDevice> device(SkNEW_ARGS(SkBitmapDevice, (bitmap)));
        SkNulCanvas canvas(device);
        renderer.renderPage(page < 0 ? 0 : page, &canvas, rect);
    } else {
        // 8888
        SkRect rect = renderer.MediaBox(page < 0 ? 0 :page);

        SkBitmap bitmap;
        SkScalar width = SkScalarMul(rect.width(),  SkDoubleToScalar(FLAGS_DPI / 72.0));
        SkScalar height = SkScalarMul(rect.height(),  SkDoubleToScalar(FLAGS_DPI / 72.0));

        rect = SkRect::MakeWH(width, height);

        SkColor background = FLAGS_transparentBackground ? SK_ColorTRANSPARENT : SK_ColorWHITE;

#ifdef PDF_DEBUG_3X
        setup_bitmap(&bitmap, 3 * (int)SkScalarToDouble(width), 3 * (int)SkScalarToDouble(height),
                     background);
#else
        setup_bitmap(&bitmap, (int)SkScalarToDouble(width), (int)SkScalarToDouble(height),
                     background);
#endif
        SkAutoTUnref<SkBaseDevice> device;
        if (strcmp(FLAGS_config[0], "8888") == 0) {
            device.reset(SkNEW_ARGS(SkBitmapDevice, (bitmap)));
        } else {
            SkDebugf("unknown --config: %s\n", FLAGS_config[0]);
            return false;
        }
        SkCanvas canvas(device);

#ifdef PDF_TRACE_DIFF_IN_PNG
        gDumpBitmap = &bitmap;
        gDumpCanvas = &canvas;
#endif
        renderer.renderPage(page < 0 ? 0 : page, &canvas, rect);

        SkString outputPath;
        if (!make_output_filepath(&outputPath, outputDir, inputFilename, page)) {
            return false;
        }
        SkImageEncoder::EncodeFile(outputPath.c_str(), bitmap, SkImageEncoder::kPNG_Type, 100);

        if (FLAGS_showMemoryUsage) {
            SkDebugf("Memory usage after page %i rendered: %u\n",
                     page < 0 ? 0 : page, (unsigned int)renderer.bytesUsed());
        }
    }
    return true;
}

/** Reads an skp file, renders it to pdf and writes the output to a pdf file
 * @param inputPath The skp file to be read.
 * @param outputDir Output dir.
 */
static bool process_pdf(const SkString& inputPath, const SkString& outputDir) {
    SkDebugf("Loading PDF:  %s\n", inputPath.c_str());

    SkString inputFilename = SkOSPath::Basename(inputPath.c_str());

    SkAutoTDelete<SkPdfRenderer> renderer(SkPdfRenderer::CreateFromFile(inputPath.c_str()));
    if (NULL == renderer.get()) {
        SkDebugf("Failure loading file %s\n", inputPath.c_str());
        return false;
    }

    if (FLAGS_showMemoryUsage) {
        SkDebugf("Memory usage after load: %u\n", (unsigned int) renderer->bytesUsed());
    }

    // TODO(edisonn): bench timers
    if (FLAGS_benchLoad > 0) {
        for (int i = 0 ; i < FLAGS_benchLoad; i++) {
            SkAutoTDelete<SkPdfRenderer> benchRenderer(
                    SkPdfRenderer::CreateFromFile(inputPath.c_str()));
            if (NULL == benchRenderer.get()) {
                SkDebugf("Failed to load on %ith attempt\n", i);
            } else if (FLAGS_showMemoryUsage) {
                SkDebugf("Memory usage after load %i number : %u\n", i,
                         (unsigned int) benchRenderer->bytesUsed());
            }
        }
    }

    if (!renderer->pages()) {
        // This should never happen, since CreateFromFile will return NULL if there are no pages.
        SkASSERT(false);
        SkDebugf("ERROR: Empty PDF Document %s\n", inputPath.c_str());
        return false;
    }

    bool success = true;
    for (int i = 0; i < FLAGS_benchRender + 1; i++) {
        // TODO(edisonn) if (i == 1) start timer
        if (strcmp(FLAGS_pages[0], "all") == 0) {
            for (int pn = 0; pn < renderer->pages(); ++pn) {
                success &= render_page(outputDir, inputFilename, *renderer,
                        FLAGS_noExtensionForOnePagePdf && renderer->pages() == 1 ? -1 : pn);
            }
        } else if (strcmp(FLAGS_pages[0], "reverse") == 0) {
            for (int pn = renderer->pages() - 1; pn >= 0; --pn) {
                success &= render_page(outputDir, inputFilename, *renderer,
                        FLAGS_noExtensionForOnePagePdf && renderer->pages() == 1 ? -1 : pn);
            }
        } else if (strcmp(FLAGS_pages[0], "first") == 0) {
            success &= render_page(outputDir, inputFilename, *renderer,
                    FLAGS_noExtensionForOnePagePdf && renderer->pages() == 1 ? -1 : 0);
        } else if (strcmp(FLAGS_pages[0], "last") == 0) {
            success &= render_page(outputDir, inputFilename, *renderer,
                    FLAGS_noExtensionForOnePagePdf && renderer->pages() == 1 ? -1
                    : renderer->pages() - 1);
        } else {
            int pn = atoi(FLAGS_pages[0]);
            success &= render_page(outputDir, inputFilename, *renderer,
                    FLAGS_noExtensionForOnePagePdf && renderer->pages() == 1 ? -1 : pn);
        }
    }

    if (!success) {
        SkDebugf("Failures for file %s\n", inputPath.c_str());
    }

    return success;
}

/** For each file in the directory or for the file passed in input, call
 * parse_pdf.
 * @param input A directory or an pdf file.
 * @param outputDir Output dir.
 */
static int process_input(const char* input, const SkString& outputDir) {
    int failures = 0;
    if (sk_isdir(input)) {
        SkOSFile::Iter iter(input, PDF_FILE_EXTENSION);
        SkString inputFilename;
        while (iter.next(&inputFilename)) {
            SkString inputPath = SkOSPath::Join(input, inputFilename.c_str());
            if (!process_pdf(inputPath, outputDir)) {
                ++failures;
            }
        }
    } else {
        SkString inputPath(input);
        if (!process_pdf(inputPath, outputDir)) {
            ++failures;
        }
    }
    return failures;
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage("Parse and Render .pdf files (pdf viewer).");
    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_readPath.isEmpty()) {
        SkDebugf(".pdf files or directories are required.\n");
        exit(-1);
    }

    SkString outputDir;
    if (FLAGS_writePath.count() == 1) {
        outputDir.set(FLAGS_writePath[0]);
    }

    int failures = 0;
    for (int i = 0; i < FLAGS_readPath.count(); i ++) {
        failures += process_input(FLAGS_readPath[i], outputDir);
    }

    reportPdfRenderStats();

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
