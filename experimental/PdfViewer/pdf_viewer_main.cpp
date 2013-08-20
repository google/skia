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
#include "SkNulCanvas.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "GrContext.h"
#include "SkGpuDevice.h"
#endif

#include "SkPdfRenderer.h"

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

#if SK_SUPPORT_GPU
                                  "\tgpu: use the gpu\n"
#endif
                                  "\tnul - render in null canvas, any draw will just return.\n"
               );


// TODO(edisonn): add config for device target(gpu, raster, pdf), + ability not to render at all

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

static void make_filepath(SkString* path, const SkString& dir, const SkString& name) {
    size_t len = dir.size();
    path->set(dir);
    if (0 < len  && '/' != dir[len - 1]) {
        path->append("/");
    }
    path->append(name);
}

static bool is_path_seperator(const char chr) {
#if defined(SK_BUILD_FOR_WIN)
    return chr == '\\' || chr == '/';
#else
    return chr == '/';
#endif
}

static void get_basename(SkString* basename, const SkString& path) {
    if (path.size() == 0) {
        basename->reset();
        return;
    }

    size_t end = path.size() - 1;

    // Paths pointing to directories often have a trailing slash,
    // we remove it so the name is not empty
    if (is_path_seperator(path[end])) {
        if (end == 0) {
            basename->reset();
            return;
        }

        end -= 1;
    }

    size_t i = end;
    do {
        --i;
        if (is_path_seperator(path[i])) {
              const char* basenameStart = path.c_str() + i + 1;
              size_t basenameLength = end - i;
              basename->set(basenameStart, basenameLength);
              return;
        }
    } while (i > 0);

    basename->set(path.c_str(), end + 1);
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
    make_filepath(path, dir, name);
    return add_page_and_replace_filename_extension(path, page,
                                                   PDF_FILE_EXTENSION,
                                                   PNG_FILE_EXTENSION);
}

static void setup_bitmap(SkBitmap* bitmap, int width, int height, SkColor color = SK_ColorTRANSPARENT) {
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

extern "C" SkBitmap* gDumpBitmap;
extern "C" SkCanvas* gDumpCanvas;

#if SK_SUPPORT_GPU
GrContextFactory gContextFactory;
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
        SkScalar width = SkScalarMul(rect.width(),  SkDoubleToScalar(sqrt(FLAGS_DPI / 72.0)));
        SkScalar height = SkScalarMul(rect.height(),  SkDoubleToScalar(sqrt(FLAGS_DPI / 72.0)));

        rect = SkRect::MakeWH(width, height);

#ifdef PDF_DEBUG_3X
        setup_bitmap(&bitmap, 3 * (int)SkScalarToDouble(width), 3 * (int)SkScalarToDouble(height));
#else
        setup_bitmap(&bitmap, (int)SkScalarToDouble(width), (int)SkScalarToDouble(height));
#endif
        SkAutoTUnref<SkBaseDevice> device;
        if (strcmp(FLAGS_config[0], "8888") == 0) {
            device.reset(SkNEW_ARGS(SkBitmapDevice, (bitmap)));
        }
#if SK_SUPPORT_GPU
        else if (strcmp(FLAGS_config[0], "gpu") == 0) {
            SkAutoTUnref<GrSurface> target;
            GrContext* gr = gContextFactory.get(GrContextFactory::kNative_GLContextType);
            if (gr) {
                // create a render target to back the device
                GrTextureDesc desc;
                desc.fConfig = kSkia8888_GrPixelConfig;
                desc.fFlags = kRenderTarget_GrTextureFlagBit;
                desc.fWidth = width;
                desc.fHeight = height;
                desc.fSampleCnt = 0;
                target.reset(gr->createUncachedTexture(desc, NULL, 0));
            }
            if (NULL == target.get()) {
                SkASSERT(0);
                return false;
            }

            device.reset(SkGpuDevice::Create(target));
        }
#endif
        else {
            SkDebugf("unknown --config: %s\n", FLAGS_config[0]);
            return false;
        }
        SkCanvas canvas(device);

        gDumpBitmap = &bitmap;

        gDumpCanvas = &canvas;
        renderer.renderPage(page < 0 ? 0 : page, &canvas, rect);

        SkString outputPath;
        if (!make_output_filepath(&outputPath, outputDir, inputFilename, page)) {
            return false;
        }
        SkImageEncoder::EncodeFile(outputPath.c_str(), bitmap, SkImageEncoder::kPNG_Type, 100);

        if (FLAGS_showMemoryUsage) {
            SkDebugf("Memory usage after page %i rendered: %u\n", page < 0 ? 0 : page, (unsigned int)renderer.bytesUsed());
        }
    }
    return true;
}

/** Reads an skp file, renders it to pdf and writes the output to a pdf file
 * @param inputPath The skp file to be read.
 * @param outputDir Output dir.
 * @param renderer The object responsible to render the skp object into pdf.
 */
static bool process_pdf(const SkString& inputPath, const SkString& outputDir,
                        SkPdfRenderer& renderer) {
    SkDebugf("Loading PDF:  %s\n", inputPath.c_str());

    SkString inputFilename;
    get_basename(&inputFilename, inputPath);

    bool success = true;

    success = renderer.load(inputPath);
    if (FLAGS_showMemoryUsage) {
        SkDebugf("Memory usage after load: %u\n", (unsigned int)renderer.bytesUsed());
    }

    // TODO(edisonn): bench timers
    if (FLAGS_benchLoad > 0) {
        for (int i = 0 ; i < FLAGS_benchLoad; i++) {
            success = renderer.load(inputPath) && success;
            if (FLAGS_showMemoryUsage) {
                SkDebugf("Memory usage after load %i number : %u\n", i, (unsigned int)renderer.bytesUsed());
            }
        }
    }

    if (success) {
        if (!renderer.pages())
        {
            SkDebugf("ERROR: Empty PDF Document %s\n", inputPath.c_str());
            return false;
        } else {
            for (int i = 0; i < FLAGS_benchRender + 1; i++) {
                // TODO(edisonn) if (i == 1) start timer
                if (strcmp(FLAGS_pages[0], "all") == 0) {
                    for (int pn = 0; pn < renderer.pages(); ++pn) {
                        success = render_page(outputDir, inputFilename, renderer, FLAGS_noExtensionForOnePagePdf && renderer.pages() == 1 ? -1 : pn) && success;
                    }
                } else if (strcmp(FLAGS_pages[0], "reverse") == 0) {
                    for (int pn = renderer.pages() - 1; pn >= 0; --pn) {
                        success = render_page(outputDir, inputFilename, renderer, FLAGS_noExtensionForOnePagePdf && renderer.pages() == 1 ? -1 : pn) && success;
                    }
                } else if (strcmp(FLAGS_pages[0], "first") == 0) {
                    success = render_page(outputDir, inputFilename, renderer, FLAGS_noExtensionForOnePagePdf && renderer.pages() == 1 ? -1 : 0) && success;
                } else if (strcmp(FLAGS_pages[0], "last") == 0) {
                    success = render_page(outputDir, inputFilename, renderer, FLAGS_noExtensionForOnePagePdf && renderer.pages() == 1 ? -1 : renderer.pages() - 1) && success;
                } else {
                    int pn = atoi(FLAGS_pages[0]);
                    success = render_page(outputDir, inputFilename, renderer, FLAGS_noExtensionForOnePagePdf && renderer.pages() == 1 ? -1 : pn) && success;
                }
            }
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
 * @param renderer The object responsible to render the skp object into pdf.
 */
static int process_input(const char* input, const SkString& outputDir,
                         SkPdfRenderer& renderer) {
    int failures = 0;
    if (sk_isdir(input)) {
        SkOSFile::Iter iter(input, PDF_FILE_EXTENSION);
        SkString inputFilename;
        while (iter.next(&inputFilename)) {
            SkString inputPath;
            SkString _input;
            _input.append(input);
            make_filepath(&inputPath, _input, inputFilename);
            if (!process_pdf(inputPath, outputDir, renderer)) {
                ++failures;
            }
        }
    } else {
        SkString inputPath(input);
        if (!process_pdf(inputPath, outputDir, renderer)) {
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
        failures += process_input(FLAGS_readPath[i], outputDir, renderer);
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
