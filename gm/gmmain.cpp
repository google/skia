/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "system_preferences.h"
#include "GrContextFactory.h"
#include "GrRenderTarget.h"
#include "SkColorPriv.h"
#include "SkData.h"
#include "SkDeferredCanvas.h"
#include "SkDevice.h"
#include "SkGPipe.h"
#include "SkGpuCanvas.h"
#include "SkGpuDevice.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkPicture.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SamplePipeControllers.h"

static bool gForceBWtext;

extern bool gSkSuppressFontCachePurgeSpew;

#ifdef SK_SUPPORT_PDF
    #include "SkPDFDevice.h"
    #include "SkPDFDocument.h"
#endif

// Until we resolve http://code.google.com/p/skia/issues/detail?id=455 ,
// stop writing out XPS-format image baselines in gm.
#undef SK_SUPPORT_XPS
#ifdef SK_SUPPORT_XPS
    #include "SkXPSDevice.h"
#endif

#ifdef SK_BUILD_FOR_MAC
    #include "SkCGUtils.h"
    #define CAN_IMAGE_PDF   1
#else
    #define CAN_IMAGE_PDF   0
#endif

typedef int ErrorBitfield;
const static ErrorBitfield ERROR_NONE                    = 0x00;
const static ErrorBitfield ERROR_NO_GPU_CONTEXT          = 0x01;
const static ErrorBitfield ERROR_PIXEL_MISMATCH          = 0x02;
const static ErrorBitfield ERROR_DIMENSION_MISMATCH      = 0x04;
const static ErrorBitfield ERROR_READING_REFERENCE_IMAGE = 0x08;
const static ErrorBitfield ERROR_WRITING_REFERENCE_IMAGE = 0x10;

// If true, emit a messange when we can't find a reference image to compare
static bool gNotifyMissingReadReference;

using namespace skiagm;

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

static ErrorBitfield compare(const SkBitmap& target, const SkBitmap& base,
                             const SkString& name,
                             const char* renderModeDescriptor,
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
        return ERROR_DIMENSION_MISMATCH;
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
                return ERROR_PIXEL_MISMATCH;
            }
        }
    }

    // they're equal
    return ERROR_NONE;
}

static bool write_document(const SkString& path,
                           const SkDynamicMemoryWStream& document) {
    SkFILEWStream stream(path.c_str());
    SkAutoDataUnref data(document.copyToData());
    return stream.writeData(data.get());
}

enum Backend {
  kRaster_Backend,
  kGPU_Backend,
  kPDF_Backend,
  kXPS_Backend,
};

enum ConfigFlags {
    kNone_ConfigFlag  = 0x0,
    /* Write GM images if a write path is provided. */
    kWrite_ConfigFlag = 0x1,
    /* Read comparison GM images if a read path is provided. */
    kRead_ConfigFlag  = 0x2,
    kRW_ConfigFlag    = (kWrite_ConfigFlag | kRead_ConfigFlag),
};

struct ConfigData {
    SkBitmap::Config                fConfig;
    Backend                         fBackend;
    GrContextFactory::GLContextType fGLContextType; // GPU backend only
    int                             fSampleCnt;     // GPU backend only
    ConfigFlags                     fFlags;
    const char*                     fName;
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

#include "SkDrawFilter.h"
class BWTextDrawFilter : public SkDrawFilter {
public:
    virtual void filter(SkPaint*, Type) SK_OVERRIDE;
};
void BWTextDrawFilter::filter(SkPaint* p, Type t) {
    if (kText_Type == t) {
        p->setAntiAlias(false);
    }
}

static void installFilter(SkCanvas* canvas) {
    if (gForceBWtext) {
        canvas->setDrawFilter(new BWTextDrawFilter)->unref();
    }
}

static void invokeGM(GM* gm, SkCanvas* canvas, bool isPDF = false) {
    SkAutoCanvasRestore acr(canvas, true);

    if (!isPDF) {
        canvas->concat(gm->getInitialTransform());
    }
    installFilter(canvas);
    gm->draw(canvas);
    canvas->setDrawFilter(NULL);
}

static ErrorBitfield generate_image(GM* gm, const ConfigData& gRec,
                                    GrContext* context,
                                    GrRenderTarget* rt,
                                    SkBitmap* bitmap,
                                    bool deferred) {
    SkISize size (gm->getISize());
    setup_bitmap(gRec, size, bitmap);

    if (gRec.fBackend == kRaster_Backend) {
        SkCanvas* canvas;
        if (deferred) {
            canvas = new SkDeferredCanvas;
            canvas->setDevice(new SkDevice(*bitmap))->unref();
        } else {
            canvas = new SkCanvas(*bitmap);
        }
        SkAutoUnref canvasUnref(canvas);
        invokeGM(gm, canvas);
        canvas->flush();
    } else {  // GPU
        if (NULL == context) {
            return ERROR_NO_GPU_CONTEXT;
        }
        SkCanvas* gc;
        if (deferred) {
            gc = new SkDeferredCanvas;
        } else {
            gc = new SkGpuCanvas(context, rt);
        }
        SkAutoUnref gcUnref(gc);
        gc->setDevice(new SkGpuDevice(context, rt))->unref();
        invokeGM(gm, gc);
        // the device is as large as the current rendertarget, so we explicitly
        // only readback the amount we expect (in size)
        // overwrite our previous allocation
        bitmap->setConfig(SkBitmap::kARGB_8888_Config, size.fWidth,
                                                       size.fHeight);
        gc->readPixels(bitmap, 0, 0);
    }
    return ERROR_NONE;
}

static void generate_image_from_picture(GM* gm, const ConfigData& gRec,
                                        SkPicture* pict, SkBitmap* bitmap) {
    SkISize size = gm->getISize();
    setup_bitmap(gRec, size, bitmap);
    SkCanvas canvas(*bitmap);
    installFilter(&canvas);
    canvas.drawPicture(*pict);
}

static void generate_pdf(GM* gm, SkDynamicMemoryWStream& pdf) {
#ifdef SK_SUPPORT_PDF
    SkMatrix initialTransform = gm->getInitialTransform();
    SkISize pageSize = gm->getISize();
    SkPDFDevice* dev = NULL;
    if (initialTransform.isIdentity()) {
        dev = new SkPDFDevice(pageSize, pageSize, initialTransform);
    } else {
        SkRect content = SkRect::MakeWH(SkIntToScalar(pageSize.width()),
                                        SkIntToScalar(pageSize.height()));
        initialTransform.mapRect(&content);
        content.intersect(0, 0, SkIntToScalar(pageSize.width()),
                                SkIntToScalar(pageSize.height()));
        SkISize contentSize =
            SkISize::Make(SkScalarRoundToInt(content.width()),
                          SkScalarRoundToInt(content.height()));
        dev = new SkPDFDevice(pageSize, contentSize, initialTransform);
    }
    SkAutoUnref aur(dev);

    SkCanvas c(dev);
    invokeGM(gm, &c, true);

    SkPDFDocument doc;
    doc.appendPage(dev);
    doc.emitPDF(&pdf);
#endif
}

static void generate_xps(GM* gm, SkDynamicMemoryWStream& xps) {
#ifdef SK_SUPPORT_XPS
    SkISize size = gm->getISize();
    
    SkSize trimSize = SkSize::Make(SkIntToScalar(size.width()),
                                   SkIntToScalar(size.height()));
    static const SkScalar inchesPerMeter = SkScalarDiv(10000, 254);
    static const SkScalar upm = 72 * inchesPerMeter;
    SkVector unitsPerMeter = SkPoint::Make(upm, upm);
    static const SkScalar ppm = 200 * inchesPerMeter;
    SkVector pixelsPerMeter = SkPoint::Make(ppm, ppm);

    SkXPSDevice* dev = new SkXPSDevice();
    SkAutoUnref aur(dev);

    SkCanvas c(dev);
    dev->beginPortfolio(&xps);
    dev->beginSheet(unitsPerMeter, pixelsPerMeter, trimSize);
    invokeGM(gm, &c);
    dev->endSheet();
    dev->endPortfolio();

#endif
}

static ErrorBitfield write_reference_image(const ConfigData& gRec,
                                           const char writePath [],
                                           const char renderModeDescriptor [],
                                           const SkString& name,
                                           SkBitmap& bitmap,
                                           SkDynamicMemoryWStream* document) {
    SkString path;
    bool success = false;
    if (gRec.fBackend == kRaster_Backend ||
        gRec.fBackend == kGPU_Backend ||
        (gRec.fBackend == kPDF_Backend && CAN_IMAGE_PDF)) {
    
        path = make_filename(writePath, renderModeDescriptor, name, "png");
        success = write_bitmap(path, bitmap);
    }
    if (kPDF_Backend == gRec.fBackend) {
        path = make_filename(writePath, renderModeDescriptor, name, "pdf");
        success = write_document(path, *document);
    }
    if (kXPS_Backend == gRec.fBackend) {
        path = make_filename(writePath, renderModeDescriptor, name, "xps");
        success = write_document(path, *document);
    }
    if (success) {
        return ERROR_NONE;
    } else {
        fprintf(stderr, "FAILED to write %s\n", path.c_str());
        return ERROR_WRITING_REFERENCE_IMAGE;
    }
}

static ErrorBitfield compare_to_reference_image(const SkString& name,
                                                SkBitmap &bitmap,
                                                const SkBitmap& comparisonBitmap,
                                                const char diffPath [],
                                                const char renderModeDescriptor []) {
    ErrorBitfield errors;
    SkBitmap diffBitmap;
    errors = compare(bitmap, comparisonBitmap, name, renderModeDescriptor,
                     diffPath ? &diffBitmap : NULL);
    if ((ERROR_NONE != errors) && diffPath) {
        // write out the generated image
        SkString genName = make_filename(diffPath, "", name, "png");
        if (!write_bitmap(genName, bitmap)) {
            errors |= ERROR_WRITING_REFERENCE_IMAGE;
        }
    }
    return errors;
}

static ErrorBitfield compare_to_reference_image(const char readPath [],
                                                const SkString& name,
                                                SkBitmap &bitmap,
                                                const char diffPath [],
                                                const char renderModeDescriptor []) {
    SkString path = make_filename(readPath, "", name, "png");
    SkBitmap orig;
    if (SkImageDecoder::DecodeFile(path.c_str(), &orig,
                                   SkBitmap::kARGB_8888_Config,
                                   SkImageDecoder::kDecodePixels_Mode, NULL)) {
        return compare_to_reference_image(name, bitmap,
                                          orig, diffPath,
                                          renderModeDescriptor);
    } else {
        if (gNotifyMissingReadReference) {
            fprintf(stderr, "FAILED to read %s\n", path.c_str());
        }
        return ERROR_READING_REFERENCE_IMAGE;
    }
}

static ErrorBitfield handle_test_results(GM* gm,
                                         const ConfigData& gRec,
                                         const char writePath [],
                                         const char readPath [],
                                         const char diffPath [],
                                         const char renderModeDescriptor [],
                                         SkBitmap& bitmap,
                                         SkDynamicMemoryWStream* pdf,
                                         const SkBitmap* comparisonBitmap) {
    SkString name = make_name(gm->shortName(), gRec.fName);
    ErrorBitfield retval = ERROR_NONE;

    if (readPath && (gRec.fFlags & kRead_ConfigFlag)) {
        retval |= compare_to_reference_image(readPath, name, bitmap,
                                             diffPath, renderModeDescriptor);
    }
    if (writePath && (gRec.fFlags & kWrite_ConfigFlag)) {
        retval |= write_reference_image(gRec, writePath, renderModeDescriptor,
                                        name, bitmap, pdf);
    }
    if (comparisonBitmap) {
        retval |= compare_to_reference_image(name, bitmap,
                                             *comparisonBitmap, diffPath,
                                             renderModeDescriptor);
    }
    return retval;
}

static SkPicture* generate_new_picture(GM* gm) {
    // Pictures are refcounted so must be on heap
    SkPicture* pict = new SkPicture;
    SkCanvas* cv = pict->beginRecording(1000, 1000);
    invokeGM(gm, cv);
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
static ErrorBitfield test_drawing(GM* gm,
                                  const ConfigData& gRec,
                                  const char writePath [],
                                  const char readPath [],
                                  const char diffPath [],
                                  GrContext* context,
                                  GrRenderTarget* rt,
                                  SkBitmap* bitmap) {
    SkDynamicMemoryWStream document;

    if (gRec.fBackend == kRaster_Backend ||
        gRec.fBackend == kGPU_Backend) {
        // Early exit if we can't generate the image.
        ErrorBitfield errors = generate_image(gm, gRec, context, rt, bitmap,
            false);
        if (ERROR_NONE != errors) {
            return errors;
        }
    } else if (gRec.fBackend == kPDF_Backend) {
        generate_pdf(gm, document);
#if CAN_IMAGE_PDF
        SkAutoDataUnref data(document.copyToData());
        SkMemoryStream stream(data.data(), data.size());
        SkPDFDocumentToBitmap(&stream, bitmap);
#endif
    } else if (gRec.fBackend == kXPS_Backend) {
        generate_xps(gm, document);
    }
    return handle_test_results(gm, gRec, writePath, readPath, diffPath,
                               "", *bitmap, &document, NULL);
}

static ErrorBitfield test_deferred_drawing(GM* gm,
                         const ConfigData& gRec,
                         const SkBitmap& comparisonBitmap,
                         const char diffPath [],
                         GrContext* context,
                         GrRenderTarget* rt) {
    SkDynamicMemoryWStream document;

    if (gRec.fBackend == kRaster_Backend ||
        gRec.fBackend == kGPU_Backend) {
        SkBitmap bitmap;
        // Early exit if we can't generate the image, but this is
        // expected in some cases, so don't report a test failure.
        if (!generate_image(gm, gRec, context, rt, &bitmap, true)) {
            return ERROR_NONE;
        }
        return handle_test_results(gm, gRec, NULL, NULL, diffPath,
                                   "-deferred", bitmap, NULL, &comparisonBitmap);
    }
    return ERROR_NONE;
}

static ErrorBitfield test_picture_playback(GM* gm,
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
    } else {
        return ERROR_NONE;
    }
}

static ErrorBitfield test_picture_serialization(GM* gm,
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
    } else {
        return ERROR_NONE;
    }
}

static ErrorBitfield test_pipe_playback(GM* gm,
                                        const ConfigData& gRec,
                                        const SkBitmap& comparisonBitmap,
                                        const char readPath [],
                                        const char diffPath []) {
    if (kRaster_Backend != gRec.fBackend) {
        return ERROR_NONE;
    }
    SkBitmap bitmap;
    SkISize size = gm->getISize();
    setup_bitmap(gRec, size, &bitmap);
    SkCanvas canvas(bitmap);
    PipeController pipeController(&canvas);
    SkGPipeWriter writer;
    SkCanvas* pipeCanvas = writer.startRecording(&pipeController,
            SkGPipeWriter::kCrossProcess_Flag);
    invokeGM(gm, pipeCanvas);
    writer.endRecording();
    return handle_test_results(gm, gRec, NULL, NULL, diffPath,
                               "-pipe", bitmap, NULL, &comparisonBitmap);
}

static ErrorBitfield test_tiled_pipe_playback(GM* gm,
                                        const ConfigData& gRec,
                                        const SkBitmap& comparisonBitmap,
                                        const char readPath [],
                                        const char diffPath []) {
    if (kRaster_Backend != gRec.fBackend) {
        return ERROR_NONE;
    }
    SkBitmap bitmap;
    SkISize size = gm->getISize();
    setup_bitmap(gRec, size, &bitmap);
    TiledPipeController pipeController(bitmap);
    SkGPipeWriter writer;
    SkCanvas* pipeCanvas = writer.startRecording(&pipeController,
                                                 SkGPipeWriter::kCrossProcess_Flag);
    invokeGM(gm, pipeCanvas);
    writer.endRecording();
    return handle_test_results(gm, gRec, NULL, NULL, diffPath,
                               "-tiled pipe", bitmap, NULL, &comparisonBitmap);
}

static void write_picture_serialization(GM* gm, const ConfigData& rec,
                                        const char writePicturePath[]) {
    // only do this once, so we pick raster
    if (kRaster_Backend == rec.fBackend &&
        SkBitmap::kARGB_8888_Config == rec.fConfig) {

        SkAutoTUnref<SkPicture> pict(generate_new_picture(gm));
        
        const char* pictureSuffix = "skp";
        SkString path = make_filename(writePicturePath, "",
                                      SkString(gm->shortName()), pictureSuffix);
        
        SkFILEWStream stream(path.c_str());
        pict->serialize(&stream);
    }
}

static const GrContextFactory::GLContextType kDontCare_GLContextType =
    GrContextFactory::kNative_GLContextType;

// If the platform does not support writing PNGs of PDFs then there will be no
// comparison images to read. However, we can always write the .pdf files
static const ConfigFlags kPDFConfigFlags = CAN_IMAGE_PDF ? kRW_ConfigFlag :
                                                           kWrite_ConfigFlag;

static const ConfigData gRec[] = {
    { SkBitmap::kARGB_8888_Config, kRaster_Backend, kDontCare_GLContextType,                  0, kRW_ConfigFlag,    "8888" },
    { SkBitmap::kARGB_4444_Config, kRaster_Backend, kDontCare_GLContextType,                  0, kRW_ConfigFlag,    "4444" },
    { SkBitmap::kRGB_565_Config,   kRaster_Backend, kDontCare_GLContextType,                  0, kRW_ConfigFlag,    "565" },
#ifdef SK_SCALAR_IS_FLOAT
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kNative_GLContextType,  0, kRW_ConfigFlag,    "gpu" },
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kNative_GLContextType, 16, kRW_ConfigFlag,    "msaa16" },
    /* The debug context does not generate images */
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kDebug_GLContextType,   0, kNone_ConfigFlag,  "debug" },
  #ifdef SK_ANGLE
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kANGLE_GLContextType,   0, kRW_ConfigFlag,    "angle" },
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kANGLE_GLContextType,  16, kRW_ConfigFlag,    "anglemsaa16" },
  #endif
  #ifdef SK_MESA
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kMESA_GLContextType,    0, kRW_ConfigFlag,    "mesa" },
  #endif
#endif
#ifdef SK_SUPPORT_XPS
    /* At present we have no way of comparing XPS files (either natively or by converting to PNG). */
    { SkBitmap::kARGB_8888_Config, kXPS_Backend,    kDontCare_GLContextType,                  0, kWrite_ConfigFlag, "xps" },
#endif
#ifdef SK_SUPPORT_PDF
    { SkBitmap::kARGB_8888_Config, kPDF_Backend,    kDontCare_GLContextType,                  0, kPDFConfigFlags,   "pdf" },
#endif
};

static void usage(const char * argv0) {
    SkDebugf("%s\n", argv0);
    SkDebugf("    [-w writePath] [-r readPath] [-d diffPath] [-i resourcePath]\n");
    SkDebugf("    [-config ");
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        if (i > 0) {
            SkDebugf("|");
        }
        SkDebugf(gRec[i].fName);
    }
    SkDebugf(" ]\n");
    SkDebugf("    [--noreplay] [--pipe] [--serialize] [--forceBWtext] [--nopdf] \n"
             "    [--tiledPipe] \n"
             "    [--nodeferred] [--match substring] [--notexturecache]\n"
             "    [-h|--help]\n"
             );
    SkDebugf("    writePath: directory to write rendered images in.\n");
    SkDebugf(
             "    readPath: directory to read reference images from;\n"
             "        reports if any pixels mismatch between reference and new images\n");
    SkDebugf("    diffPath: directory to write difference images in.\n");
    SkDebugf("    resourcePath: directory that stores image resources.\n");
    SkDebugf("    --noreplay: do not exercise SkPicture replay.\n");
    SkDebugf("    --pipe: Exercise SkGPipe replay.\n");
    SkDebugf("    --tiledPipe: Exercise tiled SkGPipe replay.\n");
    SkDebugf(
             "    --serialize: exercise SkPicture serialization & deserialization.\n");
    SkDebugf("    --forceBWtext: disable text anti-aliasing.\n");
    SkDebugf("    --nopdf: skip the pdf rendering test pass.\n");
    SkDebugf("    --nodeferred: skip the deferred rendering test pass.\n");
    SkDebugf("    --match foo: will only run tests that substring match foo.\n");
    SkDebugf("    --notexturecache: disable the gpu texture cache.\n");
    SkDebugf("    -h|--help : Show this help message. \n");
}

static int findConfig(const char config[]) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); i++) {
        if (!strcmp(config, gRec[i].fName)) {
            return i;
        }
    }
    return -1;
}

static bool skip_name(const SkTDArray<const char*> array, const char name[]) {
    if (0 == array.count()) {
        // no names, so don't skip anything
        return false;
    }
    for (int i = 0; i < array.count(); ++i) {
        if (strstr(name, array[i])) {
            // found the name, so don't skip
            return false;
        }
    }
    return true;
}

namespace skiagm {
SkAutoTUnref<GrContext> gGrContext;
/**
 * Sets the global GrContext, accessible by indivual GMs
 */
static void SetGr(GrContext* grContext) {
    SkSafeRef(grContext);
    gGrContext.reset(grContext);
}

/**
 * Gets the global GrContext, can be called by GM tests.
 */
GrContext* GetGr();
GrContext* GetGr() {
    return gGrContext.get();
}

/**
 * Sets the global GrContext and then resets it to its previous value at
 * destruction.
 */
class AutoResetGr : SkNoncopyable {
public:
    AutoResetGr() : fOld(NULL) {}
    void set(GrContext* context) {
        SkASSERT(NULL == fOld);
        fOld = GetGr();
        SkSafeRef(fOld);
        SetGr(context);
    }
    ~AutoResetGr() { SetGr(fOld); SkSafeUnref(fOld); }
private:
    GrContext* fOld;
};
}

int main(int argc, char * const argv[]) {
    SkGraphics::Init();
    // we don't need to see this during a run
    gSkSuppressFontCachePurgeSpew = true;

    setSystemPreferences();

    const char* writePath = NULL;   // if non-null, where we write the originals
    const char* writePicturePath = NULL;    // if non-null, where we write serialized pictures
    const char* readPath = NULL;    // if non-null, were we read from to compare
    const char* diffPath = NULL;    // if non-null, where we write our diffs (from compare)
    const char* resourcePath = NULL;// if non-null, where we read from for image resources

    SkTDArray<const char*> fMatches;

    bool doPDF = true;
    bool doReplay = true;
    bool doPipe = false;
    bool doTiledPipe = false;
    bool doSerialize = false;
    bool doDeferred = true;
    bool disableTextureCache = false;
    SkTDArray<size_t> configs;
    bool userConfig = false;

    gNotifyMissingReadReference = true;

    const char* const commandName = argv[0];
    char* const* stop = argv + argc;
    for (++argv; argv < stop; ++argv) {
        if (strcmp(*argv, "-w") == 0) {
            argv++;
            if (argv < stop && **argv) {
                writePath = *argv;
            }
        } else if (strcmp(*argv, "-wp") == 0) {
            argv++;
            if (argv < stop && **argv) {
                writePicturePath = *argv;
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
        } else if (strcmp(*argv, "-i") == 0) {
            argv++;
            if (argv < stop && **argv) {
                resourcePath = *argv;
            }
        } else if (strcmp(*argv, "--forceBWtext") == 0) {
            gForceBWtext = true;
        } else if (strcmp(*argv, "--pipe") == 0) {
            doPipe = true;
        } else if (strcmp(*argv, "--tiledPipe") == 0) {
            doTiledPipe = true;
        } else if (strcmp(*argv, "--noreplay") == 0) {
            doReplay = false;
        } else if (strcmp(*argv, "--nopdf") == 0) {
            doPDF = false;
        } else if (strcmp(*argv, "--nodeferred") == 0) {
            doDeferred = false;
        } else if (strcmp(*argv, "--disable-missing-warning") == 0) {
            gNotifyMissingReadReference = false;
        } else if (strcmp(*argv, "--enable-missing-warning") == 0) {
            gNotifyMissingReadReference = true;
        } else if (strcmp(*argv, "--serialize") == 0) {
            doSerialize = true;
        } else if (strcmp(*argv, "--match") == 0) {
            ++argv;
            if (argv < stop && **argv) {
                // just record the ptr, no need for a deep copy
                *fMatches.append() = *argv;
            }
        } else if (strcmp(*argv, "--notexturecache") == 0) {
            disableTextureCache = true;
        } else if (strcmp(*argv, "-config") == 0) {
            argv++;
            if (argv < stop) {
                int index = findConfig(*argv);
                if (index >= 0) {
                    *configs.append() = index;
                    userConfig = true;
                } else {
                    SkString str;
                    str.printf("unrecognized config %s\n", *argv);
                    SkDebugf(str.c_str());
                    usage(commandName);
                    return -1;
                }
            } else {
                SkDebugf("missing arg for -config\n");
                usage(commandName);
                return -1;
            }
        } else if (strcmp(*argv, "--help") == 0 || strcmp(*argv, "-h") == 0) {
            usage(commandName);
            return -1;
        } else {
            usage(commandName);
            return -1;
        }
    }
    if (argv != stop) {
        usage(commandName);
        return -1;
    }

    if (!userConfig) {
        // if no config is specified by user, we add them all.
        for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
            *configs.append() = i;
        }
    }

    GM::SetResourcePath(resourcePath);

    GrContextFactory* grFactory = new GrContextFactory;

    if (readPath) {
        fprintf(stderr, "reading from %s\n", readPath);
    } 
    if (writePath) {
        fprintf(stderr, "writing to %s\n", writePath);
    }
    if (writePicturePath) {
        fprintf(stderr, "writing pictures to %s\n", writePicturePath);
    }
    if (resourcePath) {
        fprintf(stderr, "reading resources from %s\n", resourcePath);
    }

    // Accumulate success of all tests.
    int testsRun = 0;
    int testsPassed = 0;
    int testsFailed = 0;
    int testsMissingReferenceImages = 0;

    if (disableTextureCache) {
        skiagm::GetGr()->setTextureCacheLimits(0, 0);
    }

    Iter iter;
    GM* gm;
    while ((gm = iter.next()) != NULL) {
        const char* shortName = gm->shortName();
        if (skip_name(fMatches, shortName)) {
            SkDELETE(gm);
            continue;
        }

        SkISize size = gm->getISize();
        SkDebugf("drawing... %s [%d %d]\n", shortName,
                 size.width(), size.height());
        SkBitmap forwardRenderedBitmap;

        for (int i = 0; i < configs.count(); i++) {
            ConfigData config = gRec[configs[i]];
            SkAutoTUnref<GrRenderTarget> rt;
            AutoResetGr autogr;
            if (kGPU_Backend == config.fBackend) {
                GrContext* gr = grFactory->get(config.fGLContextType);
                if (!gr) {
                    continue;
                }

                // create a render target to back the device
                GrTextureDesc desc;
                desc.fConfig = kSkia8888_PM_GrPixelConfig;
                desc.fFlags = kRenderTarget_GrTextureFlagBit;
                desc.fWidth = gm->getISize().width();
                desc.fHeight = gm->getISize().height();
                desc.fSampleCnt = config.fSampleCnt;
                GrTexture* tex = gr->createUncachedTexture(desc, NULL, 0);
                if (!tex) {
                    continue;
                }
                rt.reset(tex->asRenderTarget());
                rt.get()->ref();
                tex->unref();

                autogr.set(gr);
            }

            // Skip any tests that we don't even need to try.
            uint32_t gmFlags = gm->getFlags();
            if ((kPDF_Backend == config.fBackend) &&
                (!doPDF || (gmFlags & GM::kSkipPDF_Flag)))
            {
                continue;
            }

            // Now we know that we want to run this test and record its
            // success or failure.
            ErrorBitfield testErrors = ERROR_NONE;

            if ((ERROR_NONE == testErrors) &&
                (kGPU_Backend == config.fBackend) &&
                (NULL == rt.get())) {
                fprintf(stderr, "Could not create render target for gpu.\n");
                testErrors |= ERROR_NO_GPU_CONTEXT;
            }

            if (ERROR_NONE == testErrors) {
                testErrors |= test_drawing(gm, config,
                                           writePath, readPath, diffPath,
                                           GetGr(),
                                           rt.get(), &forwardRenderedBitmap);
            }

            if (doDeferred && !testErrors &&
                (kGPU_Backend == config.fBackend ||
                 kRaster_Backend == config.fBackend)) {
                testErrors |= test_deferred_drawing(gm, config,
                                                    forwardRenderedBitmap,
                                                    diffPath, GetGr(), rt.get());
            }

            if ((ERROR_NONE == testErrors) && doReplay &&
                !(gmFlags & GM::kSkipPicture_Flag)) {
                testErrors |= test_picture_playback(gm, config,
                                                    forwardRenderedBitmap,
                                                    readPath, diffPath);
            }

            if ((ERROR_NONE == testErrors) && doPipe &&
                !(gmFlags & GM::kSkipPipe_Flag)) {
                testErrors |= test_pipe_playback(gm, config,
                                                 forwardRenderedBitmap,
                                                 readPath, diffPath);
            }

            if ((ERROR_NONE == testErrors) && doTiledPipe &&
                !(gmFlags & GM::kSkipPipe_Flag)) {
                testErrors |= test_tiled_pipe_playback(gm, config,
                                                 forwardRenderedBitmap,
                                                 readPath, diffPath);
            }

            if ((ERROR_NONE == testErrors) && doSerialize  &&
                !(gmFlags & GM::kSkipPicture_Flag)) {
                testErrors |= test_picture_serialization(gm, config,
                                                         forwardRenderedBitmap,
                                                         readPath, diffPath);
            }
            
            if (!(gmFlags & GM::kSkipPicture_Flag) && writePicturePath) {
                write_picture_serialization(gm, config, writePicturePath);
            }

            // Update overall results.
            // We only tabulate the particular error types that we currently
            // care about (e.g., missing reference images). Later on, if we
            // want to also tabulate pixel mismatches vs dimension mistmatches
            // (or whatever else), we can do so.
            testsRun++;
            if (ERROR_NONE == testErrors) {
                testsPassed++;
            } else if (ERROR_READING_REFERENCE_IMAGE & testErrors) {
                testsMissingReferenceImages++;
            } else {
                testsFailed++;
            }
        }
        SkDELETE(gm);
    }
    printf("Ran %d tests: %d passed, %d failed, %d missing reference images\n",
           testsRun, testsPassed, testsFailed, testsMissingReferenceImages);

    delete grFactory;
    SkGraphics::Term();

    PRINT_INST_COUNT(SkRefCnt);
    PRINT_INST_COUNT(GrResource);

    return (0 == testsFailed) ? 0 : -1;
}
