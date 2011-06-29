#include "gm.h"
#include "SkColorPriv.h"
#include "SkData.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkRefCnt.h"

#include "GrContext.h"
#include "SkGpuCanvas.h"
#include "SkGpuDevice.h"
#include "SkEGLContext.h"
#include "SkDevice.h"

#ifdef SK_SUPPORT_PDF
    #include "SkPDFDevice.h"
    #include "SkPDFDocument.h"
#endif

#ifdef SK_BUILD_FOR_MAC
    #include "SkCGUtils.h"
    #define CAN_IMAGE_PDF   true
#else
    #define CAN_IMAGE_PDF   false
#endif

using namespace skiagm;

// need to explicitly declare this, or we get some weird infinite loop llist
template GMRegistry* SkTRegistry<GM*, void*>::gHead;

class Iter {
public:
    Iter() {
        this->reset();
    }

    void reset() {
        fReg = GMRegistry::Head();
    }

    GM* next() {
        if (fReg) {
            GMRegistry::Factory fact = fReg->factory();
            fReg = fReg->next();
            return fact(0);
        }
        return NULL;
    }

    static int Count() {
        const GMRegistry* reg = GMRegistry::Head();
        int count = 0;
        while (reg) {
            count += 1;
            reg = reg->next();
        }
        return count;
    }

private:
    const GMRegistry* fReg;
};

static SkString make_name(const char shortName[], const char configName[]) {
    SkString name(shortName);
    name.appendf("_%s", configName);
    return name;
}

static SkString make_filename(const char path[],
                              const char pathSuffix[],
                              const SkString& name,
                              const char suffix[]) {
    SkString filename(path);
    if (filename.endsWith("/")) {
        filename.remove(filename.size() - 1, 1);
    }
    filename.append(pathSuffix);
    filename.append("/");
    filename.appendf("%s.%s", name.c_str(), suffix);
    return filename;
}

/* since PNG insists on unpremultiplying our alpha, we take no precision chances
    and force all pixels to be 100% opaque, otherwise on compare we may not get
    a perfect match.
 */
static void force_all_opaque(const SkBitmap& bitmap) {
    SkAutoLockPixels lock(bitmap);
    for (int y = 0; y < bitmap.height(); y++) {
        for (int x = 0; x < bitmap.width(); x++) {
            *bitmap.getAddr32(x, y) |= (SK_A32_MASK << SK_A32_SHIFT);
        }
    }
}

static bool write_bitmap(const SkString& path, const SkBitmap& bitmap) {
    SkBitmap copy;
    bitmap.copyTo(&copy, SkBitmap::kARGB_8888_Config);
    force_all_opaque(copy);
    return SkImageEncoder::EncodeFile(path.c_str(), copy,
                                      SkImageEncoder::kPNG_Type, 100);
}

static inline SkPMColor compute_diff_pmcolor(SkPMColor c0, SkPMColor c1) {
    int dr = SkGetPackedR32(c0) - SkGetPackedR32(c1);
    int dg = SkGetPackedG32(c0) - SkGetPackedG32(c1);
    int db = SkGetPackedB32(c0) - SkGetPackedB32(c1);
    return SkPackARGB32(0xFF, SkAbs32(dr), SkAbs32(dg), SkAbs32(db));
}

static void compute_diff(const SkBitmap& target, const SkBitmap& base,
                         SkBitmap* diff) {
    SkAutoLockPixels alp(*diff);

    const int w = target.width();
    const int h = target.height();
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            SkPMColor c0 = *base.getAddr32(x, y);
            SkPMColor c1 = *target.getAddr32(x, y);
            SkPMColor d = 0;
            if (c0 != c1) {
                d = compute_diff_pmcolor(c0, c1);
            }
            *diff->getAddr32(x, y) = d;
        }
    }
}

static bool compare(const SkBitmap& target, const SkBitmap& base,
                    const SkString& name, const char* renderModeDescriptor,
                    SkBitmap* diff) {
    SkBitmap copy;
    const SkBitmap* bm = &target;
    if (target.config() != SkBitmap::kARGB_8888_Config) {
        target.copyTo(&copy, SkBitmap::kARGB_8888_Config);
        bm = &copy;
    }
    SkBitmap baseCopy;
    const SkBitmap* bp = &base;
    if (base.config() != SkBitmap::kARGB_8888_Config) {
        base.copyTo(&baseCopy, SkBitmap::kARGB_8888_Config);
        bp = &baseCopy;
    }

    force_all_opaque(*bm);
    force_all_opaque(*bp);

    const int w = bm->width();
    const int h = bm->height();
    if (w != bp->width() || h != bp->height()) {
        SkDebugf(
"---- %s dimensions mismatch for %s base [%d %d] current [%d %d]\n",
                 renderModeDescriptor, name.c_str(),
                 bp->width(), bp->height(), w, h);
        return false;
    }

    SkAutoLockPixels bmLock(*bm);
    SkAutoLockPixels baseLock(*bp);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            SkPMColor c0 = *bp->getAddr32(x, y);
            SkPMColor c1 = *bm->getAddr32(x, y);
            if (c0 != c1) {
                SkDebugf(
"----- %s pixel mismatch for %s at [%d %d] base 0x%08X current 0x%08X\n",
                         renderModeDescriptor, name.c_str(), x, y, c0, c1);

                if (diff) {
                    diff->setConfig(SkBitmap::kARGB_8888_Config, w, h);
                    diff->allocPixels();
                    compute_diff(*bm, *bp, diff);
                }
                return false;
            }
        }
    }

    // they're equal
    return true;
}

static bool write_pdf(const SkString& path, const SkDynamicMemoryWStream& pdf) {
    SkFILEWStream stream(path.c_str());
    SkAutoDataUnref data(pdf.copyToData());
    return stream.writeData(data.get());
}

enum Backend {
  kRaster_Backend,
  kGPU_Backend,
  kPDF_Backend,
};

struct ConfigData {
    SkBitmap::Config    fConfig;
    Backend             fBackend;
    const char*         fName;
};

/// Returns true if processing should continue, false to skip the
/// remainder of this config for this GM.
//@todo thudson 22 April 2011 - could refactor this to take in
// a factory to generate the context, always call readPixels()
// (logically a noop for rasters, if wasted time), and thus collapse the
// GPU special case and also let this be used for SkPicture testing.
static void setup_bitmap(const ConfigData& gRec, SkISize& size,
                         SkBitmap* bitmap) {
    bitmap->setConfig(gRec.fConfig, size.width(), size.height());
    bitmap->allocPixels();
    bitmap->eraseColor(0);
}

// Returns true if the test should continue, false if the test should
// halt.
static bool generate_image(GM* gm, const ConfigData& gRec,
                           GrContext* context,
                           SkBitmap* bitmap) {
    SkISize size (gm->getISize());
    setup_bitmap(gRec, size, bitmap);
    SkCanvas canvas(*bitmap);

    if (gRec.fBackend == kRaster_Backend) {
        gm->draw(&canvas);
    } else {  // GPU
        if (NULL == context) {
            return false;
        }
        // not a real object, so don't unref it
        GrRenderTarget* rt = SkGpuDevice::Current3DApiRenderTarget();
        SkGpuCanvas gc(context, rt);
        gc.setDevice(new SkGpuDevice(context, rt))->unref();
        gm->draw(&gc);
        // the device is as large as the current rendertarget, so we explicitly
        // only readback the amount we expect (in size)
        // overwrite our previous allocation
        gc.readPixels(SkIRect::MakeSize(size), bitmap);
    }
    return true;
}

static void generate_image_from_picture(GM* gm, const ConfigData& gRec,
                                        SkPicture* pict, SkBitmap* bitmap) {
    SkISize size = gm->getISize();
    setup_bitmap(gRec, size, bitmap);
    SkCanvas canvas(*bitmap);
    canvas.drawPicture(*pict);
}

static void generate_pdf(GM* gm, SkDynamicMemoryWStream& pdf) {
#ifdef SK_SUPPORT_PDF
    SkISize size = gm->getISize();
    SkMatrix identity;
    identity.reset();
    SkPDFDevice* dev = new SkPDFDevice(size, size, identity);
    SkAutoUnref aur(dev);

    SkCanvas c(dev);
    gm->draw(&c);

    SkPDFDocument doc;
    doc.appendPage(dev);
    doc.emitPDF(&pdf);
#endif
}

static bool write_reference_image(const ConfigData& gRec,
                                  const char writePath [],
                                  const char renderModeDescriptor [],
                                  const SkString& name,
                                  SkBitmap& bitmap,
                                  SkDynamicMemoryWStream* pdf) {
    SkString path;
    bool success = false;
    if (gRec.fBackend != kPDF_Backend || CAN_IMAGE_PDF) {
        path = make_filename(writePath, renderModeDescriptor, name, "png");
        success = write_bitmap(path, bitmap);
    }
    if (kPDF_Backend == gRec.fBackend) {
        path = make_filename(writePath, renderModeDescriptor, name, "pdf");
        success = write_pdf(path, *pdf);
    }
    if (!success) {
        fprintf(stderr, "FAILED to write %s\n", path.c_str());
    }
    return success;
}

static bool compare_to_reference_image(const SkString& name,
                                       SkBitmap &bitmap,
                                       const SkBitmap& comparisonBitmap,
                                       const char diffPath [],
                                       const char renderModeDescriptor []) {
    bool success;
    SkBitmap diffBitmap;
    success = compare(bitmap, comparisonBitmap, name, renderModeDescriptor,
                      diffPath ? &diffBitmap : NULL);
    if (!success && diffPath) {
        SkString diffName = make_filename(diffPath, "", name, ".diff.png");
        write_bitmap(diffName, diffBitmap);
    }
    return success;
}

static bool compare_to_reference_image(const char readPath [],
                                       const SkString& name,
                                       SkBitmap &bitmap,
                                       const char diffPath [],
                                       const char renderModeDescriptor []) {
    SkString path = make_filename(readPath, "", name, "png");
    SkBitmap orig;
    bool success = SkImageDecoder::DecodeFile(path.c_str(), &orig,
                        SkBitmap::kARGB_8888_Config,
                        SkImageDecoder::kDecodePixels_Mode, NULL);
    if (success) {
        success = compare_to_reference_image(name, bitmap,
                                             orig, diffPath,
                                             renderModeDescriptor);
    } else {
        fprintf(stderr, "FAILED to read %s\n", path.c_str());
        // we lie here, and report succes, since we're just missing a master
        // image. This way we can check in new tests, and not report failure.
        // A real failure is to draw *differently* from the master image, but
        // that's not the case here.
        success = true;
    }
    return success;
}

static bool handle_test_results(GM* gm,
                                const ConfigData& gRec,
                                const char writePath [],
                                const char readPath [],
                                const char diffPath [],
                                const char renderModeDescriptor [],
                                SkBitmap& bitmap,
                                SkDynamicMemoryWStream* pdf,
                                const SkBitmap* comparisonBitmap) {
    SkString name = make_name(gm->shortName(), gRec.fName);

    if (writePath) {
        write_reference_image(gRec, writePath, renderModeDescriptor,
                              name, bitmap, pdf);
    } else if (readPath && (gRec.fBackend != kPDF_Backend || CAN_IMAGE_PDF)) {
        return compare_to_reference_image(readPath, name, bitmap,
                                   diffPath, renderModeDescriptor);
    } else if (comparisonBitmap) {
        return compare_to_reference_image(name, bitmap,
                                   *comparisonBitmap, diffPath,
                                   renderModeDescriptor);
    }
    return true;
}

static SkPicture* generate_new_picture(GM* gm) {
    // Pictures are refcounted so must be on heap
    SkPicture* pict = new SkPicture;
    SkCanvas* cv = pict->beginRecording(1000, 1000);
    gm->draw(cv);
    pict->endRecording();

    return pict;
}

static SkPicture* stream_to_new_picture(const SkPicture& src) {

    // To do in-memory commiunications with a stream, we need to:
    // * create a dynamic memory stream
    // * copy it into a buffer
    // * create a read stream from it
    // ?!?!

    SkDynamicMemoryWStream storage;
    src.serialize(&storage);

    int streamSize = storage.getOffset();
    SkAutoMalloc dstStorage(streamSize);
    void* dst = dstStorage.get();
    //char* dst = new char [streamSize];
    //@todo thudson 22 April 2011 when can we safely delete [] dst?
    storage.copyTo(dst);
    SkMemoryStream pictReadback(dst, streamSize);
    SkPicture* retval = new SkPicture (&pictReadback);
    return retval;
}

// Test: draw into a bitmap or pdf.
// Depending on flags, possibly compare to an expected image
// and possibly output a diff image if it fails to match.
static bool test_drawing(GM* gm,
                         const ConfigData& gRec,
                         const char writePath [],
                         const char readPath [],
                         const char diffPath [],
                         GrContext* context,
                         SkBitmap* bitmap) {
    SkDynamicMemoryWStream pdf;

    if (gRec.fBackend == kRaster_Backend ||
            gRec.fBackend == kGPU_Backend) {
        // Early exit if we can't generate the image, but this is
        // expected in some cases, so don't report a test failure.
        if (!generate_image(gm, gRec, context, bitmap)) {
            return true;
        }
    } else if (gRec.fBackend == kPDF_Backend) {
        generate_pdf(gm, pdf);
#if CAN_IMAGE_PDF
        SkAutoDataUnref data(pdf.copyToData());
        SkMemoryStream stream(data.data(), data.size());
        SkPDFDocumentToBitmap(&stream, bitmap);
#endif
    }
    return handle_test_results(gm, gRec, writePath, readPath, diffPath,
                        "", *bitmap, &pdf, NULL);
}

static bool test_picture_playback(GM* gm,
                                  const ConfigData& gRec,
                                  const SkBitmap& comparisonBitmap,
                                  const char readPath [],
                                  const char diffPath []) {
    SkPicture* pict = generate_new_picture(gm);
    SkAutoUnref aur(pict);

    if (kRaster_Backend == gRec.fBackend) {
        SkBitmap bitmap;
        generate_image_from_picture(gm, gRec, pict, &bitmap);
        return handle_test_results(gm, gRec, NULL, NULL, diffPath,
                            "-replay", bitmap, NULL, &comparisonBitmap);
    }
    return true;
}

static bool test_picture_serialization(GM* gm,
                                       const ConfigData& gRec,
                                       const SkBitmap& comparisonBitmap,
                                       const char readPath [],
                                       const char diffPath []) {
    SkPicture* pict = generate_new_picture(gm);
    SkAutoUnref aurp(pict);
    SkPicture* repict = stream_to_new_picture(*pict);
    SkAutoUnref aurr(repict);

    if (kRaster_Backend == gRec.fBackend) {
        SkBitmap bitmap;
        generate_image_from_picture(gm, gRec, repict, &bitmap);
        return handle_test_results(gm, gRec, NULL, NULL, diffPath,
                            "-serialize", bitmap, NULL, &comparisonBitmap);
    }
    return true;
}

static void usage(const char * argv0) {
    SkDebugf("%s [-w writePath] [-r readPath] [-d diffPath]\n", argv0);
    SkDebugf("    [--replay] [--serialize]\n");
    SkDebugf("    writePath: directory to write rendered images in.\n");
    SkDebugf(
"    readPath: directory to read reference images from;\n"
"        reports if any pixels mismatch between reference and new images\n");
    SkDebugf("    diffPath: directory to write difference images in.\n");
    SkDebugf("    --replay: exercise SkPicture replay.\n");
    SkDebugf(
"    --serialize: exercise SkPicture serialization & deserialization.\n");
}

static const ConfigData gRec[] = {
    { SkBitmap::kARGB_8888_Config, kRaster_Backend, "8888" },
    { SkBitmap::kARGB_4444_Config, kRaster_Backend, "4444" },
    { SkBitmap::kRGB_565_Config,   kRaster_Backend, "565" },
#ifdef SK_SCALAR_IS_FLOAT
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    "gpu" },
#endif
#ifdef SK_SUPPORT_PDF
    { SkBitmap::kARGB_8888_Config, kPDF_Backend,    "pdf" },
#endif
};

int main(int argc, char * const argv[]) {
    SkAutoGraphics ag;

    const char* writePath = NULL;   // if non-null, where we write the originals
    const char* readPath = NULL;    // if non-null, were we read from to compare
    const char* diffPath = NULL;    // if non-null, where we write our diffs (from compare)

    bool doReplay = true;
    bool doSerialize = false;
    const char* const commandName = argv[0];
    char* const* stop = argv + argc;
    for (++argv; argv < stop; ++argv) {
        if (strcmp(*argv, "-w") == 0) {
            argv++;
            if (argv < stop && **argv) {
                writePath = *argv;
            }
        } else if (strcmp(*argv, "-r") == 0) {
            argv++;
            if (argv < stop && **argv) {
                readPath = *argv;
            }
        } else if (strcmp(*argv, "-d") == 0) {
            argv++;
            if (argv < stop && **argv) {
                diffPath = *argv;
            }
        } else if (strcmp(*argv, "--noreplay") == 0) {
            doReplay = false;
        } else if (strcmp(*argv, "--serialize") == 0) {
            doSerialize = true;
        } else {
          usage(commandName);
          return -1;
        }
    }
    if (argv != stop) {
      usage(commandName);
      return -1;
    }

    int maxW = -1;
    int maxH = -1;
    Iter iter;
    GM* gm;
    while ((gm = iter.next()) != NULL) {
        SkISize size = gm->getISize();
        maxW = SkMax32(size.width(), maxW);
        maxH = SkMax32(size.height(), maxH);
    }
    // setup a GL context for drawing offscreen
    GrContext* context = NULL;
    SkEGLContext eglContext;
    if (eglContext.init(maxW, maxH)) {
        context = GrContext::CreateGLShaderContext();
    }


    if (readPath) {
        fprintf(stderr, "reading from %s\n", readPath);
    } else if (writePath) {
        fprintf(stderr, "writing to %s\n", writePath);
    }

    // Accumulate success of all tests so we can flag error in any
    // one with the return value.
    iter.reset();
    bool overallSuccess = true;
    while ((gm = iter.next()) != NULL) {
        SkISize size = gm->getISize();
        SkDebugf("drawing... %s [%d %d]\n", gm->shortName(),
                 size.width(), size.height());
        SkBitmap forwardRenderedBitmap;

        for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); i++) {
            bool testSuccess = test_drawing(gm, gRec[i],
                         writePath, readPath, diffPath, context,
                         &forwardRenderedBitmap);
            overallSuccess &= testSuccess;

            if (doReplay && testSuccess) {
                testSuccess = test_picture_playback(gm, gRec[i],
                                      forwardRenderedBitmap,
                                      readPath, diffPath);
                overallSuccess &= testSuccess;
            }

            if (doSerialize && testSuccess) {
                testSuccess &= test_picture_serialization(gm, gRec[i],
                                      forwardRenderedBitmap,
                                      readPath, diffPath);
                overallSuccess &= testSuccess;
            }
        }
        SkDELETE(gm);
    }
    if (false == overallSuccess) {
        return -1;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

using namespace skiagm;

GM::GM() {}
GM::~GM() {}

void GM::draw(SkCanvas* canvas) {
    this->onDraw(canvas);
}
