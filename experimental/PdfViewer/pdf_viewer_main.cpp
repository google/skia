#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkDevice.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkTypeface.h"
#include "SkTArray.h"
#include "picture_utils.h"

#include "SkPdfRenderer.h"

DEFINE_string2(readPath, r, "", "pdf files or directories of pdf files to process.");
DEFINE_string2(writePath, w, "", "Directory to write the rendered pages.");
DEFINE_bool2(noExtensionForOnePagePdf, n, false, "No page extension if only one page.");
DEFINE_bool2(showMemoryUsage, m, false, "Show Memory usage.");

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
    sk_tools::make_filepath(path, dir, name);
    return add_page_and_replace_filename_extension(path, page,
                                                   PDF_FILE_EXTENSION,
                                                   PNG_FILE_EXTENSION);
}

static void setup_bitmap(SkBitmap* bitmap, int width, int height, SkColor color = SK_ColorWHITE) {
    bitmap->setConfig(SkBitmap::kARGB_8888_Config, width, height);

    bitmap->allocPixels();
    bitmap->eraseColor(color);
}

/** Write the output of pdf renderer to a file.
 * @param outputDir Output dir.
 * @param inputFilename The skp file that was read.
 * @param renderer The object responsible to write the pdf file.
 * @param page -1 means there is only one page (0), and render in a file without page extension
 */

static bool render_page(const SkString& outputDir,
                         const SkString& inputFilename,
                         const SkPdfRenderer& renderer,
                         int page) {
    if (outputDir.isEmpty()) {
        SkBitmap bitmap;
        setup_bitmap(&bitmap, 1, 1);
        SkAutoTUnref<SkDevice> device(SkNEW_ARGS(SkDevice, (bitmap)));
        SkCanvas canvas(device);
        return renderer.renderPage(page < 0 ? 0 : page, &canvas);
    }

    SkString outputPath;
    if (!make_output_filepath(&outputPath, outputDir, inputFilename, page)) {
        return false;
    }

    SkRect rect = renderer.MediaBox(page < 0 ? 0 :page);

    SkBitmap bitmap;
#ifdef PDF_DEBUG_3X
    setup_bitmap(&bitmap, 3 * (int)SkScalarToDouble(rect.width()), 3 * (int)SkScalarToDouble(rect.height()));
#else
    setup_bitmap(&bitmap, (int)SkScalarToDouble(rect.width()), (int)SkScalarToDouble(rect.height()));
#endif
    SkAutoTUnref<SkDevice> device(SkNEW_ARGS(SkDevice, (bitmap)));
    SkCanvas canvas(device);

    gDumpBitmap = &bitmap;

    gDumpCanvas = &canvas;
    renderer.renderPage(page < 0 ? 0 : page, &canvas);

    SkImageEncoder::EncodeFile(outputPath.c_str(), bitmap, SkImageEncoder::kPNG_Type, 100);

    return true;
}

/** Reads an skp file, renders it to pdf and writes the output to a pdf file
 * @param inputPath The skp file to be read.
 * @param outputDir Output dir.
 * @param renderer The object responsible to render the skp object into pdf.
 */
static bool process_pdf(const SkString& inputPath, const SkString& outputDir,
                        SkPdfRenderer& renderer, bool noPageExt, bool showMemoryUsage) {
    SkDebugf("Loading PDF:  %s\n", inputPath.c_str());

    SkString inputFilename;
    sk_tools::get_basename(&inputFilename, inputPath);

    SkFILEStream inputStream;
    inputStream.setPath(inputPath.c_str());
    if (!inputStream.isValid()) {
        SkDebugf("Could not open file %s\n", inputPath.c_str());
        return false;
    }

    bool success = false;

    success = renderer.load(inputPath);

    if (success) {
        if (showMemoryUsage) {
            SkDebugf("Memory usage after load: %u\n", (unsigned int)renderer.bytesUsed());
        }
        if (!renderer.pages())
        {
            SkDebugf("ERROR: Empty PDF Document %s\n", inputPath.c_str());
            return false;
        } else {
            for (int pn = 0; pn < renderer.pages(); ++pn) {
                success = render_page(outputDir, inputFilename, renderer, noPageExt && renderer.pages() == 1 ? -1 : pn) && success;
                SkDebugf("Memory usage after page %i rendered: %u\n", pn, (unsigned int)renderer.bytesUsed());
            }
        }
    }

    return success;
}

/** For each file in the directory or for the file passed in input, call
 * parse_pdf.
 * @param input A directory or an pdf file.
 * @param outputDir Output dir.
 * @param renderer The object responsible to render the skp object into pdf.
 */
static int process_input(const char* input, const SkString& outputDir,
                         SkPdfRenderer& renderer, bool noPageExt, bool showMemoryUsage) {
    int failures = 0;
    if (sk_isdir(input)) {
        SkOSFile::Iter iter(input, PDF_FILE_EXTENSION);
        SkString inputFilename;
        while (iter.next(&inputFilename)) {
            SkString inputPath;
            SkString _input;
            _input.append(input);
            sk_tools::make_filepath(&inputPath, _input, inputFilename);
            if (!process_pdf(inputPath, outputDir, renderer, noPageExt, showMemoryUsage)) {
                ++failures;
            }
        }
    } else {
        SkString inputPath(input);
        if (!process_pdf(inputPath, outputDir, renderer, noPageExt, showMemoryUsage)) {
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

    SkPdfRenderer renderer;

    SkString outputDir;
    if (FLAGS_writePath.count() == 1) {
        outputDir.set(FLAGS_writePath[0]);
    }

    int failures = 0;
    for (int i = 0; i < FLAGS_readPath.count(); i ++) {
        failures += process_input(FLAGS_readPath[i], outputDir, renderer,
                                  FLAGS_noExtensionForOnePagePdf,
                                  FLAGS_showMemoryUsage);
        renderer.unload();
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
