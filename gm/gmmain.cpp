/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * Code for the "gm" (Golden Master) rendering comparison tool.
 *
 * If you make changes to this, re-run the self-tests at gm/tests/run.sh
 * to make sure they still pass... you may need to change the expected
 * results of the self-test.
 */

#include "gm.h"
#include "gm_expectations.h"
#include "system_preferences.h"
#include "SkBitmap.h"
#include "SkBitmapChecksummer.h"
#include "SkColorPriv.h"
#include "SkData.h"
#include "SkDeferredCanvas.h"
#include "SkDevice.h"
#include "SkDrawFilter.h"
#include "SkGPipe.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkTileGridPicture.h"
#include "SamplePipeControllers.h"

#ifdef SK_BUILD_FOR_WIN
    // json includes xlocale which generates warning 4530 because we're compiling without
    // exceptions; see https://code.google.com/p/skia/issues/detail?id=1067
    #pragma warning(push)
    #pragma warning(disable : 4530)
#endif
#include "json/value.h"
#ifdef SK_BUILD_FOR_WIN
    #pragma warning(pop)
#endif

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "GrRenderTarget.h"
#include "SkGpuDevice.h"
typedef GrContextFactory::GLContextType GLContextType;
#else
class GrContext;
class GrRenderTarget;
typedef int GLContextType;
#endif

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

// TODO(epoger): We created this ErrorBitfield so that we could record
// multiple error types for the same comparison. But in practice, we
// process its final value in switch() statements, which inherently
// assume that only one error type will be set.
// I think we should probably change this to be an enum, and thus
// constrain ourselves to a single error type per comparison.
typedef int ErrorBitfield;
const static ErrorBitfield ERROR_NONE                    = 0x00;
const static ErrorBitfield ERROR_NO_GPU_CONTEXT          = 0x01;
const static ErrorBitfield ERROR_IMAGE_MISMATCH          = 0x02;
// const static ErrorBitfield ERROR_DIMENSION_MISMATCH      = 0x04; DEPRECATED in https://codereview.appspot.com/7064047
const static ErrorBitfield ERROR_READING_REFERENCE_IMAGE = 0x08;
const static ErrorBitfield ERROR_WRITING_REFERENCE_IMAGE = 0x10;

const static char kJsonKey_ActualResults[]   = "actual-results";
const static char kJsonKey_ActualResults_Failed[]        = "failed";
const static char kJsonKey_ActualResults_FailureIgnored[]= "failure-ignored";
const static char kJsonKey_ActualResults_NoComparison[]  = "no-comparison";
const static char kJsonKey_ActualResults_Succeeded[]     = "succeeded";
const static char kJsonKey_ActualResults_AnyStatus_Checksum[]    = "checksum";

const static char kJsonKey_ExpectedResults[] = "expected-results";
const static char kJsonKey_ExpectedResults_Checksums[]     = "checksums";
const static char kJsonKey_ExpectedResults_IgnoreFailure[] = "ignore-failure";

using namespace skiagm;

struct FailRec {
    SkString    fName;
    bool        fIsPixelError;

    FailRec() : fIsPixelError(false) {}
    FailRec(const SkString& name) : fName(name), fIsPixelError(false) {}
};

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

enum Backend {
    kRaster_Backend,
    kGPU_Backend,
    kPDF_Backend,
    kXPS_Backend,
};

enum BbhType {
    kNone_BbhType,
    kRTree_BbhType,
    kTileGrid_BbhType,
};

enum ConfigFlags {
    kNone_ConfigFlag  = 0x0,
    /* Write GM images if a write path is provided. */
    kWrite_ConfigFlag = 0x1,
    /* Read reference GM images if a read path is provided. */
    kRead_ConfigFlag  = 0x2,
    kRW_ConfigFlag    = (kWrite_ConfigFlag | kRead_ConfigFlag),
};

struct ConfigData {
    SkBitmap::Config                fConfig;
    Backend                         fBackend;
    GLContextType                   fGLContextType; // GPU backend only
    int                             fSampleCnt;     // GPU backend only
    ConfigFlags                     fFlags;
    const char*                     fName;
};

class BWTextDrawFilter : public SkDrawFilter {
public:
    virtual bool filter(SkPaint*, Type) SK_OVERRIDE;
};
bool BWTextDrawFilter::filter(SkPaint* p, Type t) {
    if (kText_Type == t) {
        p->setAntiAlias(false);
    }
    return true;
}

struct PipeFlagComboData {
    const char* name;
    uint32_t flags;
};

static PipeFlagComboData gPipeWritingFlagCombos[] = {
    { "", 0 },
    { " cross-process", SkGPipeWriter::kCrossProcess_Flag },
    { " cross-process, shared address", SkGPipeWriter::kCrossProcess_Flag
        | SkGPipeWriter::kSharedAddressSpace_Flag }
};

class GMMain {
public:
    GMMain() {
        // Set default values of member variables, which tool_main()
        // may override.
        fUseFileHierarchy = false;
        fMismatchPath = NULL;
    }

    SkString make_name(const char shortName[], const char configName[]) {
        SkString name;
        if (0 == strlen(configName)) {
            name.append(shortName);
        } else if (fUseFileHierarchy) {
            name.appendf("%s%c%s", configName, SkPATH_SEPARATOR, shortName);
        } else {
            name.appendf("%s_%s", shortName, configName);
        }
        return name;
    }

    /* since PNG insists on unpremultiplying our alpha, we take no
       precision chances and force all pixels to be 100% opaque,
       otherwise on compare we may not get a perfect match.
    */
    static void force_all_opaque(const SkBitmap& bitmap) {
        SkBitmap::Config config = bitmap.config();
        switch (config) {
        case SkBitmap::kARGB_8888_Config:
            force_all_opaque_8888(bitmap);
            break;
        case SkBitmap::kRGB_565_Config:
            // nothing to do here; 565 bitmaps are inherently opaque
            break;
        default:
            fprintf(stderr, "unsupported bitmap config %d\n", config);
            SkDEBUGFAIL("unsupported bitmap config");
        }
    }

    static void force_all_opaque_8888(const SkBitmap& bitmap) {
        SkAutoLockPixels lock(bitmap);
        for (int y = 0; y < bitmap.height(); y++) {
            for (int x = 0; x < bitmap.width(); x++) {
                *bitmap.getAddr32(x, y) |= (SK_A32_MASK << SK_A32_SHIFT);
            }
        }
    }

    static bool write_bitmap(const SkString& path, const SkBitmap& bitmap) {
        // TODO(epoger): Now that we have removed force_all_opaque()
        // from this method, we should be able to get rid of the
        // transformation to 8888 format also.
        SkBitmap copy;
        bitmap.copyTo(&copy, SkBitmap::kARGB_8888_Config);
        return SkImageEncoder::EncodeFile(path.c_str(), copy,
                                          SkImageEncoder::kPNG_Type, 100);
    }

    // Records an error in fFailedTests, if we want to record errors
    // of this type.
    void RecordError(ErrorBitfield errorType, const SkString& name,
                     const char renderModeDescriptor []) {
        bool isPixelError;
        switch (errorType) {
        case ERROR_NONE:
            return;
        case ERROR_READING_REFERENCE_IMAGE:
            return;
        case ERROR_IMAGE_MISMATCH:
            isPixelError = true;
            break;
        default:
            isPixelError = false;
            break;
        }

        FailRec& rec = fFailedTests.push_back(make_name(
            name.c_str(), renderModeDescriptor));
        rec.fIsPixelError = isPixelError;
    }

    // List contents of fFailedTests via SkDebug.
    void ListErrors() {
        for (int i = 0; i < fFailedTests.count(); ++i) {
            if (fFailedTests[i].fIsPixelError) {
                SkDebugf("\t\t%s pixel_error\n", fFailedTests[i].fName.c_str());
            } else {
                SkDebugf("\t\t%s\n", fFailedTests[i].fName.c_str());
            }
        }
    }

    static bool write_document(const SkString& path,
                               const SkDynamicMemoryWStream& document) {
        SkFILEWStream stream(path.c_str());
        SkAutoDataUnref data(document.copyToData());
        return stream.writeData(data.get());
    }

    /**
     * Prepare an SkBitmap to render a GM into.
     *
     * After you've rendered the GM into the SkBitmap, you must call
     * complete_bitmap()!
     *
     * @todo thudson 22 April 2011 - could refactor this to take in
     * a factory to generate the context, always call readPixels()
     * (logically a noop for rasters, if wasted time), and thus collapse the
     * GPU special case and also let this be used for SkPicture testing.
     */
    static void setup_bitmap(const ConfigData& gRec, SkISize& size,
                             SkBitmap* bitmap) {
        bitmap->setConfig(gRec.fConfig, size.width(), size.height());
        bitmap->allocPixels();
        bitmap->eraseColor(SK_ColorTRANSPARENT);
    }

    /**
     * Any finalization steps we need to perform on the SkBitmap after
     * we have rendered the GM into it.
     *
     * It's too bad that we are throwing away alpha channel data
     * we could otherwise be examining, but this had always been happening
     * before... it was buried within the compare() method at
     * https://code.google.com/p/skia/source/browse/trunk/gm/gmmain.cpp?r=7289#305 .
     *
     * Apparently we need this, at least for bitmaps that are either:
     * (a) destined to be written out as PNG files, or
     * (b) compared against bitmaps read in from PNG files
     * for the reasons described just above the force_all_opaque() method.
     *
     * Neglecting to do this led to the difficult-to-diagnose
     * http://code.google.com/p/skia/issues/detail?id=1079 ('gm generating
     * spurious pixel_error messages as of r7258')
     *
     * TODO(epoger): Come up with a better solution that allows us to
     * compare full pixel data, including alpha channel, while still being
     * robust in the face of transformations to/from PNG files.
     * Options include:
     *
     * 1. Continue to call force_all_opaque(), but ONLY for bitmaps that
     *    will be written to, or compared against, PNG files.
     *    PRO: Preserve/compare alpha channel info for the non-PNG cases
     *         (comparing different renderModes in-memory)
     *    CON: The bitmaps (and checksums) for these non-PNG cases would be
     *         different than those for the PNG-compared cases, and in the
     *         case of a failed renderMode comparison, how would we write the
     *         image to disk for examination?
     *
     * 2. Always compute image checksums from PNG format (either
     *    directly from the the bytes of a PNG file, or capturing the
     *    bytes we would have written to disk if we were writing the
     *    bitmap out as a PNG).
     *    PRO: I think this would allow us to never force opaque, and to
     *         the extent that alpha channel data can be preserved in a PNG
     *         file, we could observe it.
     *    CON: If we read a bitmap from disk, we need to take its checksum
     *         from the source PNG (we can't compute it from the bitmap we
     *         read out of the PNG, because we will have already premultiplied
     *         the alpha).
     *    CON: Seems wasteful to convert a bitmap to PNG format just to take
     *         its checksum. (Although we're wasting lots of effort already
     *         calling force_all_opaque().)
     *
     * 3. Make the alpha premultiply/unpremultiply routines 100% consistent,
     *    so we can transform images back and forth without fear of off-by-one
     *    errors.
     *    CON: Math is hard.
     *
     * 4. Perform a "close enough" comparison of bitmaps (+/- 1 bit in each
     *    channel), rather than demanding absolute equality.
     *    CON: Can't do this with checksums.
     */
    static void complete_bitmap(SkBitmap* bitmap) {
        force_all_opaque(*bitmap);
    }

    static void installFilter(SkCanvas* canvas) {
        if (gForceBWtext) {
            canvas->setDrawFilter(new BWTextDrawFilter)->unref();
        }
    }

    static void invokeGM(GM* gm, SkCanvas* canvas, bool isPDF, bool isDeferred) {
        SkAutoCanvasRestore acr(canvas, true);

        if (!isPDF) {
            canvas->concat(gm->getInitialTransform());
        }
        installFilter(canvas);
        gm->setCanvasIsDeferred(isDeferred);
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

        SkAutoTUnref<SkCanvas> canvas;

        if (gRec.fBackend == kRaster_Backend) {
            SkAutoTUnref<SkDevice> device(new SkDevice(*bitmap));
            if (deferred) {
                canvas.reset(new SkDeferredCanvas(device));
            } else {
                canvas.reset(new SkCanvas(device));
            }
            invokeGM(gm, canvas, false, deferred);
            canvas->flush();
        }
#if SK_SUPPORT_GPU
        else {  // GPU
            if (NULL == context) {
                return ERROR_NO_GPU_CONTEXT;
            }
            SkAutoTUnref<SkDevice> device(new SkGpuDevice(context, rt));
            if (deferred) {
                canvas.reset(new SkDeferredCanvas(device));
            } else {
                canvas.reset(new SkCanvas(device));
            }
            invokeGM(gm, canvas, false, deferred);
            // the device is as large as the current rendertarget, so
            // we explicitly only readback the amount we expect (in
            // size) overwrite our previous allocation
            bitmap->setConfig(SkBitmap::kARGB_8888_Config, size.fWidth,
                              size.fHeight);
            canvas->readPixels(bitmap, 0, 0);
        }
#endif
        complete_bitmap(bitmap);
        return ERROR_NONE;
    }

    static void generate_image_from_picture(GM* gm, const ConfigData& gRec,
                                            SkPicture* pict, SkBitmap* bitmap,
                                            SkScalar scale = SK_Scalar1) {
        SkISize size = gm->getISize();
        setup_bitmap(gRec, size, bitmap);
        SkCanvas canvas(*bitmap);
        installFilter(&canvas);
        canvas.scale(scale, scale);
        canvas.drawPicture(*pict);
        complete_bitmap(bitmap);
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
        invokeGM(gm, &c, true, false);

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
        invokeGM(gm, &c, false, false);
        dev->endSheet();
        dev->endPortfolio();

#endif
    }

    ErrorBitfield write_reference_image(
      const ConfigData& gRec, const char writePath [],
      const char renderModeDescriptor [], const SkString& name,
        SkBitmap& bitmap, SkDynamicMemoryWStream* document) {
        SkString path;
        bool success = false;
        if (gRec.fBackend == kRaster_Backend ||
            gRec.fBackend == kGPU_Backend ||
            (gRec.fBackend == kPDF_Backend && CAN_IMAGE_PDF)) {

            path = make_filename(writePath, renderModeDescriptor, name.c_str(),
                                 "png");
            success = write_bitmap(path, bitmap);
        }
        if (kPDF_Backend == gRec.fBackend) {
            path = make_filename(writePath, renderModeDescriptor, name.c_str(),
                                 "pdf");
            success = write_document(path, *document);
        }
        if (kXPS_Backend == gRec.fBackend) {
            path = make_filename(writePath, renderModeDescriptor, name.c_str(),
                                 "xps");
            success = write_document(path, *document);
        }
        if (success) {
            return ERROR_NONE;
        } else {
            fprintf(stderr, "FAILED to write %s\n", path.c_str());
            RecordError(ERROR_WRITING_REFERENCE_IMAGE, name,
                        renderModeDescriptor);
            return ERROR_WRITING_REFERENCE_IMAGE;
        }
    }

    /**
     * Log more detail about the mistmatch between expectedBitmap and
     * actualBitmap.
     */
    void report_bitmap_diffs(const SkBitmap& expectedBitmap, const SkBitmap& actualBitmap,
                             const char *testName) {
        const int expectedWidth = expectedBitmap.width();
        const int expectedHeight = expectedBitmap.height();
        const int width = actualBitmap.width();
        const int height = actualBitmap.height();
        if ((expectedWidth != width) || (expectedHeight != height)) {
            SkDebugf("---- %s: dimension mismatch -- expected [%d %d], actual [%d %d]\n",
                     testName, expectedWidth, expectedHeight, width, height);
            return;
        }

        if ((SkBitmap::kARGB_8888_Config != expectedBitmap.config()) ||
            (SkBitmap::kARGB_8888_Config != actualBitmap.config())) {
            SkDebugf("---- %s: not computing max per-channel pixel mismatch because non-8888\n",
                     testName);
            return;
        }

        SkAutoLockPixels alp0(expectedBitmap);
        SkAutoLockPixels alp1(actualBitmap);
        int errR = 0;
        int errG = 0;
        int errB = 0;
        int errA = 0;
        int differingPixels = 0;

        for (int y = 0; y < height; ++y) {
            const SkPMColor* expectedPixelPtr = expectedBitmap.getAddr32(0, y);
            const SkPMColor* actualPixelPtr = actualBitmap.getAddr32(0, y);
            for (int x = 0; x < width; ++x) {
                SkPMColor expectedPixel = *expectedPixelPtr++;
                SkPMColor actualPixel = *actualPixelPtr++;
                if (expectedPixel != actualPixel) {
                    differingPixels++;
                    errR = SkMax32(errR, SkAbs32((int)SkGetPackedR32(expectedPixel) -
                                                 (int)SkGetPackedR32(actualPixel)));
                    errG = SkMax32(errG, SkAbs32((int)SkGetPackedG32(expectedPixel) -
                                                 (int)SkGetPackedG32(actualPixel)));
                    errB = SkMax32(errB, SkAbs32((int)SkGetPackedB32(expectedPixel) -
                                                 (int)SkGetPackedB32(actualPixel)));
                    errA = SkMax32(errA, SkAbs32((int)SkGetPackedA32(expectedPixel) -
                                                 (int)SkGetPackedA32(actualPixel)));
                }
            }
        }
        SkDebugf("---- %s: %d (of %d) differing pixels, max per-channel mismatch"
                 " R=%d G=%d B=%d A=%d\n",
                 testName, differingPixels, width*height, errR, errG, errB, errA);
    }

    /**
     * Compares actual checksum to expectations.
     * Returns ERROR_NONE if they match, or some particular error code otherwise
     *
     * If fMismatchPath has been set, and there are pixel diffs, then the
     * actual bitmap will be written out to a file within fMismatchPath.
     *
     * @param expectations what expectations to compare actualBitmap against
     * @param actualBitmap the image we actually generated
     * @param baseNameString name of test without renderModeDescriptor added
     * @param renderModeDescriptor e.g., "-rtree", "-deferred"
     * @param addToJsonSummary whether to add these results (both actual and
     *        expected) to the JSON summary
     *
     * TODO: For now, addToJsonSummary is only set to true within
     * compare_test_results_to_stored_expectations(), so results of our
     * in-memory comparisons (Rtree vs regular, etc.) are not written to the
     * JSON summary.  We may wish to change that.
     */
    ErrorBitfield compare_to_expectations(Expectations expectations,
                                          const SkBitmap& actualBitmap,
                                          const SkString& baseNameString,
                                          const char renderModeDescriptor[],
                                          bool addToJsonSummary=false) {
        ErrorBitfield retval;
        Checksum actualChecksum = SkBitmapChecksummer::Compute64(actualBitmap);
        SkString completeNameString = baseNameString;
        completeNameString.append(renderModeDescriptor);
        const char* completeName = completeNameString.c_str();

        if (expectations.empty()) {
            retval = ERROR_READING_REFERENCE_IMAGE;
        } else if (expectations.match(actualChecksum)) {
            retval = ERROR_NONE;
        } else {
            retval = ERROR_IMAGE_MISMATCH;

            // Write out the "actuals" for any mismatches, if we have
            // been directed to do so.
            if (fMismatchPath) {
                SkString path =
                    make_filename(fMismatchPath, renderModeDescriptor,
                                  baseNameString.c_str(), "png");
                write_bitmap(path, actualBitmap);
            }

            // If we have access to a single expected bitmap, log more
            // detail about the mismatch.
            const SkBitmap *expectedBitmapPtr = expectations.asBitmap();
            if (NULL != expectedBitmapPtr) {
                report_bitmap_diffs(*expectedBitmapPtr, actualBitmap, completeName);
            }
        }
        RecordError(retval, baseNameString, renderModeDescriptor);

        if (addToJsonSummary) {
            add_actual_results_to_json_summary(completeName, actualChecksum,
                                               retval,
                                               expectations.ignoreFailure());
            add_expected_results_to_json_summary(completeName, expectations);
        }

        return retval;
    }

    /**
     * Add this result to the appropriate JSON collection of actual results,
     * depending on status.
     */
    void add_actual_results_to_json_summary(const char testName[],
                                            Checksum actualChecksum,
                                            ErrorBitfield result,
                                            bool ignoreFailure) {
        Json::Value actualResults;
        actualResults[kJsonKey_ActualResults_AnyStatus_Checksum] =
            asJsonValue(actualChecksum);
        if (ERROR_NONE == result) {
            this->fJsonActualResults_Succeeded[testName] = actualResults;
        } else {
            if (ignoreFailure) {
                // TODO: Once we have added the ability to compare
                // actual results against expectations in a JSON file
                // (where we can set ignore-failure to either true or
                // false), add test cases that exercise ignored
                // failures (both for ERROR_READING_REFERENCE_IMAGE
                // and ERROR_IMAGE_MISMATCH).
                this->fJsonActualResults_FailureIgnored[testName] =
                    actualResults;
            } else {
                switch(result) {
                case ERROR_READING_REFERENCE_IMAGE:
                    // TODO: What about the case where there IS an
                    // expected image checksum, but that gm test
                    // doesn't actually run?  For now, those cases
                    // will always be ignored, because gm only looks
                    // at expectations that correspond to gm tests
                    // that were actually run.
                    //
                    // Once we have the ability to express
                    // expectations as a JSON file, we should fix this
                    // (and add a test case for which an expectation
                    // is given but the test is never run).
                    this->fJsonActualResults_NoComparison[testName] =
                        actualResults;
                    break;
                case ERROR_IMAGE_MISMATCH:
                    this->fJsonActualResults_Failed[testName] = actualResults;
                    break;
                default:
                    fprintf(stderr, "encountered unexpected result %d\n",
                            result);
                    SkDEBUGFAIL("encountered unexpected result");
                    break;
                }
            }
        }
    }

    /**
     * Add this test to the JSON collection of expected results.
     */
    void add_expected_results_to_json_summary(const char testName[],
                                              Expectations expectations) {
        // For now, we assume that this collection starts out empty and we
        // just fill it in as we go; once gm accepts a JSON file as input,
        // we'll have to change that.
        Json::Value expectedResults;
        expectedResults[kJsonKey_ExpectedResults_Checksums] =
            expectations.allowedChecksumsAsJson();
        expectedResults[kJsonKey_ExpectedResults_IgnoreFailure] =
            expectations.ignoreFailure();
        this->fJsonExpectedResults[testName] = expectedResults;
    }

    /**
     * Compare actualBitmap to expectations stored in this->fExpectationsSource.
     *
     * @param gm which test generated the actualBitmap
     * @param gRec
     * @param writePath unless this is NULL, write out actual images into this
     *        directory
     * @param actualBitmap bitmap generated by this run
     * @param pdf
     */
    ErrorBitfield compare_test_results_to_stored_expectations(
        GM* gm, const ConfigData& gRec, const char writePath[],
        SkBitmap& actualBitmap, SkDynamicMemoryWStream* pdf) {

        SkString name = make_name(gm->shortName(), gRec.fName);
        ErrorBitfield retval = ERROR_NONE;

        ExpectationsSource *expectationsSource =
            this->fExpectationsSource.get();
        if (expectationsSource && (gRec.fFlags & kRead_ConfigFlag)) {
            /*
             * Get the expected results for this test, as one or more allowed
             * checksums. The current implementation of expectationsSource
             * get this by computing the checksum of a single PNG file on disk.
             *
             * TODO(epoger): This relies on the fact that
             * force_all_opaque() was called on the bitmap before it
             * was written to disk as a PNG in the first place. If
             * not, the checksum returned here may not match the
             * checksum of actualBitmap, which *has* been run through
             * force_all_opaque().
             * See comments above complete_bitmap() for more detail.
             */
            Expectations expectations = expectationsSource->get(name.c_str());
            retval |= compare_to_expectations(expectations, actualBitmap,
                                              name, "", true);
        } else {
            // If we are running without expectations, we still want to
            // record the actual results.
            Checksum actualChecksum =
                SkBitmapChecksummer::Compute64(actualBitmap);
            add_actual_results_to_json_summary(name.c_str(), actualChecksum,
                                               ERROR_READING_REFERENCE_IMAGE,
                                               false);
        }

        // TODO: Consider moving this into compare_to_expectations(),
        // similar to fMismatchPath... for now, we don't do that, because
        // we don't want to write out the actual bitmaps for all
        // renderModes of all tests!  That would be a lot of files.
        if (writePath && (gRec.fFlags & kWrite_ConfigFlag)) {
            retval |= write_reference_image(gRec, writePath, "",
                                            name, actualBitmap, pdf);
        }

        return retval;
    }

    /**
     * Compare actualBitmap to referenceBitmap.
     *
     * @param gm which test generated the bitmap
     * @param gRec
     * @param renderModeDescriptor
     * @param actualBitmap actual bitmap generated by this run
     * @param referenceBitmap bitmap we expected to be generated
     */
    ErrorBitfield compare_test_results_to_reference_bitmap(
        GM* gm, const ConfigData& gRec, const char renderModeDescriptor [],
        SkBitmap& actualBitmap, const SkBitmap* referenceBitmap) {

        SkASSERT(referenceBitmap);
        SkString name = make_name(gm->shortName(), gRec.fName);
        Expectations expectations(*referenceBitmap);
        return compare_to_expectations(expectations, actualBitmap,
                                       name, renderModeDescriptor);
    }

    static SkPicture* generate_new_picture(GM* gm, BbhType bbhType, uint32_t recordFlags,
                                           SkScalar scale = SK_Scalar1) {
        // Pictures are refcounted so must be on heap
        SkPicture* pict;
        int width = SkScalarCeilToInt(SkScalarMul(SkIntToScalar(gm->getISize().width()), scale));
        int height = SkScalarCeilToInt(SkScalarMul(SkIntToScalar(gm->getISize().height()), scale));

        if (kTileGrid_BbhType == bbhType) {
            pict = new SkTileGridPicture(16, 16, width, height);
        } else {
            pict = new SkPicture;
        }
        if (kNone_BbhType != bbhType) {
            recordFlags |= SkPicture::kOptimizeForClippedPlayback_RecordingFlag;
        }
        SkCanvas* cv = pict->beginRecording(width, height, recordFlags);
        cv->scale(scale, scale);
        invokeGM(gm, cv, false, false);
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
    // Depending on flags, possibly compare to an expected image.
    ErrorBitfield test_drawing(GM* gm,
                               const ConfigData& gRec,
                               const char writePath [],
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
                // TODO: Add a test to exercise what the stdout and
                // JSON look like if we get an "early error" while
                // trying to generate the image.
                return errors;
            }
        } else if (gRec.fBackend == kPDF_Backend) {
            generate_pdf(gm, document);
#if CAN_IMAGE_PDF
            SkAutoDataUnref data(document.copyToData());
            SkMemoryStream stream(data->data(), data->size());
            SkPDFDocumentToBitmap(&stream, bitmap);
#endif
        } else if (gRec.fBackend == kXPS_Backend) {
            generate_xps(gm, document);
        }
        return compare_test_results_to_stored_expectations(
            gm, gRec, writePath, *bitmap, &document);
    }

    ErrorBitfield test_deferred_drawing(GM* gm,
                                        const ConfigData& gRec,
                                        const SkBitmap& referenceBitmap,
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
            return compare_test_results_to_reference_bitmap(
                gm, gRec, "-deferred", bitmap, &referenceBitmap);
        }
        return ERROR_NONE;
    }

    ErrorBitfield test_pipe_playback(GM* gm,
                                     const ConfigData& gRec,
                                     const SkBitmap& referenceBitmap) {
        ErrorBitfield errors = ERROR_NONE;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gPipeWritingFlagCombos); ++i) {
            SkBitmap bitmap;
            SkISize size = gm->getISize();
            setup_bitmap(gRec, size, &bitmap);
            SkCanvas canvas(bitmap);
            PipeController pipeController(&canvas);
            SkGPipeWriter writer;
            SkCanvas* pipeCanvas = writer.startRecording(
              &pipeController, gPipeWritingFlagCombos[i].flags);
            invokeGM(gm, pipeCanvas, false, false);
            complete_bitmap(&bitmap);
            writer.endRecording();
            SkString string("-pipe");
            string.append(gPipeWritingFlagCombos[i].name);
            errors |= compare_test_results_to_reference_bitmap(
                gm, gRec, string.c_str(), bitmap, &referenceBitmap);
            if (errors != ERROR_NONE) {
                break;
            }
        }
        return errors;
    }

    ErrorBitfield test_tiled_pipe_playback(
      GM* gm, const ConfigData& gRec, const SkBitmap& referenceBitmap) {
        ErrorBitfield errors = ERROR_NONE;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gPipeWritingFlagCombos); ++i) {
            SkBitmap bitmap;
            SkISize size = gm->getISize();
            setup_bitmap(gRec, size, &bitmap);
            SkCanvas canvas(bitmap);
            TiledPipeController pipeController(bitmap);
            SkGPipeWriter writer;
            SkCanvas* pipeCanvas = writer.startRecording(
              &pipeController, gPipeWritingFlagCombos[i].flags);
            invokeGM(gm, pipeCanvas, false, false);
            complete_bitmap(&bitmap);
            writer.endRecording();
            SkString string("-tiled pipe");
            string.append(gPipeWritingFlagCombos[i].name);
            errors |= compare_test_results_to_reference_bitmap(
                gm, gRec, string.c_str(), bitmap, &referenceBitmap);
            if (errors != ERROR_NONE) {
                break;
            }
        }
        return errors;
    }

    //
    // member variables.
    // They are public for now, to allow easier setting by tool_main().
    //

    bool fUseFileHierarchy;

    const char* fMismatchPath;

    // information about all failed tests we have encountered so far
    SkTArray<FailRec> fFailedTests;

    // Where to read expectations (expected image checksums, etc.) from.
    // If unset, we don't do comparisons.
    SkAutoTUnref<ExpectationsSource> fExpectationsSource;

    // JSON summaries that we generate as we go (just for output).
    Json::Value fJsonExpectedResults;
    Json::Value fJsonActualResults_Failed;
    Json::Value fJsonActualResults_FailureIgnored;
    Json::Value fJsonActualResults_NoComparison;
    Json::Value fJsonActualResults_Succeeded;

}; // end of GMMain class definition

#if SK_SUPPORT_GPU
static const GLContextType kDontCare_GLContextType = GrContextFactory::kNative_GLContextType;
#else
static const GLContextType kDontCare_GLContextType = 0;
#endif

// If the platform does not support writing PNGs of PDFs then there will be no
// reference images to read. However, we can always write the .pdf files
static const ConfigFlags kPDFConfigFlags = CAN_IMAGE_PDF ? kRW_ConfigFlag :
                                                           kWrite_ConfigFlag;

static const ConfigData gRec[] = {
    { SkBitmap::kARGB_8888_Config, kRaster_Backend, kDontCare_GLContextType,                  0, kRW_ConfigFlag,    "8888" },
#if 0   // stop testing this (for now at least) since we want to remove support for it (soon please!!!)
    { SkBitmap::kARGB_4444_Config, kRaster_Backend, kDontCare_GLContextType,                  0, kRW_ConfigFlag,    "4444" },
#endif
    { SkBitmap::kRGB_565_Config,   kRaster_Backend, kDontCare_GLContextType,                  0, kRW_ConfigFlag,    "565" },
#if defined(SK_SCALAR_IS_FLOAT) && SK_SUPPORT_GPU
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kNative_GLContextType,  0, kRW_ConfigFlag,    "gpu" },
#ifndef SK_BUILD_FOR_ANDROID
    // currently we don't want to run MSAA tests on Android
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kNative_GLContextType, 16, kRW_ConfigFlag,    "msaa16" },
#endif
    /* The debug context does not generate images */
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kDebug_GLContextType,   0, kNone_ConfigFlag,  "debug" },
#if SK_ANGLE
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kANGLE_GLContextType,   0, kRW_ConfigFlag,    "angle" },
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kANGLE_GLContextType,  16, kRW_ConfigFlag,    "anglemsaa16" },
#endif // SK_ANGLE
#ifdef SK_MESA
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    GrContextFactory::kMESA_GLContextType,    0, kRW_ConfigFlag,    "mesa" },
#endif // SK_MESA
#endif // defined(SK_SCALAR_IS_FLOAT) && SK_SUPPORT_GPU
#ifdef SK_SUPPORT_XPS
    /* At present we have no way of comparing XPS files (either natively or by converting to PNG). */
    { SkBitmap::kARGB_8888_Config, kXPS_Backend,    kDontCare_GLContextType,                  0, kWrite_ConfigFlag, "xps" },
#endif // SK_SUPPORT_XPS
#ifdef SK_SUPPORT_PDF
    { SkBitmap::kARGB_8888_Config, kPDF_Backend,    kDontCare_GLContextType,                  0, kPDFConfigFlags,   "pdf" },
#endif // SK_SUPPORT_PDF
};

static void usage(const char * argv0) {
    SkDebugf("%s\n", argv0);
    SkDebugf("    [--config ");
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        if (i > 0) {
            SkDebugf("|");
        }
        SkDebugf(gRec[i].fName);
    }
    SkDebugf("]:\n        run these configurations\n");
    SkDebugf(
// Alphabetized ignoring "no" prefix ("readPath", "noreplay", "resourcePath").
// It would probably be better if we allowed both yes-and-no settings for each
// one, e.g.:
// [--replay|--noreplay]: whether to exercise SkPicture replay; default is yes
"    [--nodeferred]: skip the deferred rendering test pass\n"
"    [--disable-missing-warning]: don't print a message to stderr if\n"
"        unable to read a reference image for any tests (NOT default behavior)\n"
"    [--enable-missing-warning]: print message to stderr (but don't fail) if\n"
"        unable to read a reference image for any tests (default behavior)\n"
"    [--exclude-config]: disable this config (may be used multiple times)\n"
"    [--forceBWtext]: disable text anti-aliasing\n"
"    [--help|-h]: show this help message\n"
"    [--hierarchy|--nohierarchy]: whether to use multilevel directory structure\n"
"        when reading/writing files; default is no\n"
"    [--match <substring>]: only run tests whose name includes this substring\n"
"    [--mismatchPath <path>]: write images for tests that failed due to\n"
"        pixel mismatched into this directory"
"    [--modulo <remainder> <divisor>]: only run tests for which \n"
"        testIndex %% divisor == remainder\n"
"    [--nopdf]: skip the pdf rendering test pass\n"
"    [--nopipe]: Skip SkGPipe replay\n"
"    [--readPath|-r <path>]: read reference images from this dir, and report\n"
"        any differences between those and the newly generated ones\n"
"    [--noreplay]: do not exercise SkPicture replay\n"
"    [--resourcePath|-i <path>]: directory that stores image resources\n"
"    [--nortree]: Do not exercise the R-Tree variant of SkPicture\n"
"    [--noserialize]: do not exercise SkPicture serialization & deserialization\n"
"    [--notexturecache]: disable the gpu texture cache\n"
"    [--tiledPipe]: Exercise tiled SkGPipe replay\n"
"    [--notileGrid]: Do not exercise the tile grid variant of SkPicture\n"
"    [--tileGridReplayScales <scales>]: Comma separated list of floating-point scale\n"
"        factors to be used for tileGrid playback testing. Default value: 1.0\n"
"    [--writeJsonSummary <path>]: write a JSON-formatted result summary to this file\n"
"    [--verbose] print diagnostics (e.g. list each config to be tested)\n"
"    [--writePath|-w <path>]: write rendered images into this directory\n"
"    [--writePicturePath|-wp <path>]: write .skp files into this directory\n"
             );
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
#if SK_SUPPORT_GPU
SkAutoTUnref<GrContext> gGrContext;
/**
 * Sets the global GrContext, accessible by individual GMs
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
#else
GrContext* GetGr() { return NULL; }
#endif
}

template <typename T> void appendUnique(SkTDArray<T>* array, const T& value) {
    int index = array->find(value);
    if (index < 0) {
        *array->append() = value;
    }
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {

#if SK_ENABLE_INST_COUNT
    gPrintInstCount = true;
#endif

    SkGraphics::Init();
    // we don't need to see this during a run
    gSkSuppressFontCachePurgeSpew = true;

    setSystemPreferences();
    GMMain gmmain;

    const char* writeJsonSummaryPath = NULL;// if non-null, where we write the JSON summary
    const char* writePath = NULL;   // if non-null, where we write the originals
    const char* writePicturePath = NULL;    // if non-null, where we write serialized pictures
    const char* readPath = NULL;    // if non-null, were we read from to compare
    const char* resourcePath = NULL;// if non-null, where we read from for image resources

    // if true, emit a message when we can't find a reference image to compare
    bool notifyMissingReadReference = true;

    SkTDArray<const char*> fMatches;

    bool doPDF = true;
    bool doReplay = true;
    bool doPipe = true;
    bool doTiledPipe = false;
    bool doSerialize = true;
    bool doDeferred = true;
    bool doRTree = true;
    bool doTileGrid = true;
    bool doVerbose = false;
    bool disableTextureCache = false;
    SkTDArray<size_t> configs;
    SkTDArray<size_t> excludeConfigs;
    SkTDArray<SkScalar> tileGridReplayScales;
    *tileGridReplayScales.append() = SK_Scalar1; // By default only test at scale 1.0
    bool userConfig = false;

    int moduloRemainder = -1;
    int moduloDivisor = -1;

    const char* const commandName = argv[0];
    char* const* stop = argv + argc;
    for (++argv; argv < stop; ++argv) {
        if (strcmp(*argv, "--config") == 0) {
            argv++;
            if (argv < stop) {
                int index = findConfig(*argv);
                if (index >= 0) {
                    appendUnique<size_t>(&configs, index);
                    userConfig = true;
                } else {
                    SkString str;
                    str.printf("unrecognized config %s\n", *argv);
                    SkDebugf(str.c_str());
                    usage(commandName);
                    return -1;
                }
            } else {
                SkDebugf("missing arg for --config\n");
                usage(commandName);
                return -1;
            }
        } else if (strcmp(*argv, "--exclude-config") == 0) {
            argv++;
            if (argv < stop) {
                int index = findConfig(*argv);
                if (index >= 0) {
                    *excludeConfigs.append() = index;
                } else {
                    SkString str;
                    str.printf("unrecognized exclude-config %s\n", *argv);
                    SkDebugf(str.c_str());
                    usage(commandName);
                    return -1;
                }
            } else {
                SkDebugf("missing arg for --exclude-config\n");
                usage(commandName);
                return -1;
            }
        } else if (strcmp(*argv, "--nodeferred") == 0) {
            doDeferred = false;
        } else if (strcmp(*argv, "--disable-missing-warning") == 0) {
            notifyMissingReadReference = false;
        } else if (strcmp(*argv, "--mismatchPath") == 0) {
            argv++;
            if (argv < stop && **argv) {
                gmmain.fMismatchPath = *argv;
            }
        } else if (strcmp(*argv, "--nortree") == 0) {
            doRTree = false;
        } else if (strcmp(*argv, "--notileGrid") == 0) {
            doTileGrid = false;
        } else if (strcmp(*argv, "--tileGridReplayScales") == 0) {
            tileGridReplayScales.reset();
            ++argv;
            if (argv < stop) {
                char* token = strtok(*argv, ",");
                while (NULL != token) {
                    double val = atof(token);
                    if (0 < val) {
                        *tileGridReplayScales.append() = SkDoubleToScalar(val);
                    }
                    token = strtok(NULL, ",");
                }
            }
            if (0 == tileGridReplayScales.count()) {
                // Should have at least one scale
                usage(commandName);
                return -1;
            }
        } else if (strcmp(*argv, "--enable-missing-warning") == 0) {
            notifyMissingReadReference = true;
        } else if (strcmp(*argv, "--forceBWtext") == 0) {
            gForceBWtext = true;
        } else if (strcmp(*argv, "--help") == 0 || strcmp(*argv, "-h") == 0) {
            usage(commandName);
            return -1;
        } else if (strcmp(*argv, "--hierarchy") == 0) {
            gmmain.fUseFileHierarchy = true;
        } else if (strcmp(*argv, "--nohierarchy") == 0) {
            gmmain.fUseFileHierarchy = false;
        } else if (strcmp(*argv, "--match") == 0) {
            ++argv;
            if (argv < stop && **argv) {
                // just record the ptr, no need for a deep copy
                *fMatches.append() = *argv;
            }
        } else if (strcmp(*argv, "--modulo") == 0) {
            ++argv;
            if (argv >= stop) {
                continue;
            }
            moduloRemainder = atoi(*argv);

            ++argv;
            if (argv >= stop) {
                continue;
            }
            moduloDivisor = atoi(*argv);
            if (moduloRemainder < 0 || moduloDivisor <= 0 || moduloRemainder >= moduloDivisor) {
                SkDebugf("invalid modulo values.");
                return -1;
            }
        } else if (strcmp(*argv, "--nopdf") == 0) {
            doPDF = false;
        } else if (strcmp(*argv, "--nopipe") == 0) {
            doPipe = false;
        } else if ((0 == strcmp(*argv, "--readPath")) ||
                   (0 == strcmp(*argv, "-r"))) {
            argv++;
            if (argv < stop && **argv) {
                readPath = *argv;
            }
        } else if (strcmp(*argv, "--noreplay") == 0) {
            doReplay = false;
        } else if ((0 == strcmp(*argv, "--resourcePath")) ||
                   (0 == strcmp(*argv, "-i"))) {
            argv++;
            if (argv < stop && **argv) {
                resourcePath = *argv;
            }
        } else if (strcmp(*argv, "--serialize") == 0) {
            doSerialize = true;
        } else if (strcmp(*argv, "--noserialize") == 0) {
            doSerialize = false;
        } else if (strcmp(*argv, "--notexturecache") == 0) {
            disableTextureCache = true;
        } else if (strcmp(*argv, "--tiledPipe") == 0) {
            doTiledPipe = true;
        } else if (!strcmp(*argv, "--verbose") || !strcmp(*argv, "-v")) {
            doVerbose = true;
        } else if ((0 == strcmp(*argv, "--writePath")) ||
            (0 == strcmp(*argv, "-w"))) {
            argv++;
            if (argv < stop && **argv) {
                writePath = *argv;
            }
        } else if (0 == strcmp(*argv, "--writeJsonSummary")) {
            argv++;
            if (argv < stop && **argv) {
                writeJsonSummaryPath = *argv;
            }
        } else if ((0 == strcmp(*argv, "--writePicturePath")) ||
                   (0 == strcmp(*argv, "-wp"))) {
            argv++;
            if (argv < stop && **argv) {
                writePicturePath = *argv;
            }
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
    // now remove any explicitly excluded configs
    for (int i = 0; i < excludeConfigs.count(); ++i) {
        int index = configs.find(excludeConfigs[i]);
        if (index >= 0) {
            configs.remove(index);
            // now assert that there was only one copy in configs[]
            SkASSERT(configs.find(excludeConfigs[i]) < 0);
        }
    }

    if (doVerbose) {
        SkString str;
        str.printf("gm: %d configs:", configs.count());
        for (int i = 0; i < configs.count(); ++i) {
            str.appendf(" %s", gRec[configs[i]].fName);
        }
        SkDebugf("%s\n", str.c_str());
    }

    GM::SetResourcePath(resourcePath);

    if (readPath) {
        if (!sk_exists(readPath)) {
            fprintf(stderr, "readPath %s does not exist!\n", readPath);
            return -1;
        }
        if (sk_isdir(readPath)) {
            fprintf(stderr, "reading from %s\n", readPath);
            gmmain.fExpectationsSource.reset(SkNEW_ARGS(
                IndividualImageExpectationsSource,
                (readPath, notifyMissingReadReference)));
        } else {
            fprintf(stderr, "reading expectations from JSON summary file %s ",
                    readPath);
            fprintf(stderr, "BUT WE DON'T KNOW HOW TO DO THIS YET!\n");
            return -1;
        }
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

    if (moduloDivisor <= 0) {
        moduloRemainder = -1;
    }
    if (moduloRemainder < 0 || moduloRemainder >= moduloDivisor) {
        moduloRemainder = -1;
    }

    // Accumulate success of all tests.
    int testsRun = 0;
    int testsPassed = 0;
    int testsFailed = 0;
    int testsMissingReferenceImages = 0;

#if SK_SUPPORT_GPU
    GrContextFactory* grFactory = new GrContextFactory;
    if (disableTextureCache) {
        skiagm::GetGr()->setTextureCacheLimits(0, 0);
    }
#endif

    int gmIndex = -1;
    SkString moduloStr;

    // If we will be writing out files, prepare subdirectories.
    if (writePath) {
        if (!sk_mkdir(writePath)) {
            return -1;
        }
        if (gmmain.fUseFileHierarchy) {
            for (int i = 0; i < configs.count(); i++) {
                ConfigData config = gRec[configs[i]];
                SkString subdir;
                subdir.appendf("%s%c%s", writePath, SkPATH_SEPARATOR,
                               config.fName);
                if (!sk_mkdir(subdir.c_str())) {
                    return -1;
                }
            }
        }
    }

    Iter iter;
    GM* gm;
    while ((gm = iter.next()) != NULL) {

        ++gmIndex;
        if (moduloRemainder >= 0) {
            if ((gmIndex % moduloDivisor) != moduloRemainder) {
                continue;
            }
            moduloStr.printf("[%d.%d] ", gmIndex, moduloDivisor);
        }

        const char* shortName = gm->shortName();
        if (skip_name(fMatches, shortName)) {
            SkDELETE(gm);
            continue;
        }

        SkISize size = gm->getISize();
        SkDebugf("%sdrawing... %s [%d %d]\n", moduloStr.c_str(), shortName,
                 size.width(), size.height());

        ErrorBitfield testErrors = ERROR_NONE;
        uint32_t gmFlags = gm->getFlags();

        for (int i = 0; i < configs.count(); i++) {
            ConfigData config = gRec[configs[i]];

            // Skip any tests that we don't even need to try.
            if ((kPDF_Backend == config.fBackend) &&
                (!doPDF || (gmFlags & GM::kSkipPDF_Flag)))
                {
                    continue;
                }
            if ((gmFlags & GM::kSkip565_Flag) &&
                (kRaster_Backend == config.fBackend) &&
                (SkBitmap::kRGB_565_Config == config.fConfig)) {
                continue;
            }

            // Now we know that we want to run this test and record its
            // success or failure.
            ErrorBitfield renderErrors = ERROR_NONE;
            GrRenderTarget* renderTarget = NULL;
#if SK_SUPPORT_GPU
            SkAutoTUnref<GrRenderTarget> rt;
            AutoResetGr autogr;
            if ((ERROR_NONE == renderErrors) &&
                kGPU_Backend == config.fBackend) {
                GrContext* gr = grFactory->get(config.fGLContextType);
                bool grSuccess = false;
                if (gr) {
                    // create a render target to back the device
                    GrTextureDesc desc;
                    desc.fConfig = kSkia8888_PM_GrPixelConfig;
                    desc.fFlags = kRenderTarget_GrTextureFlagBit;
                    desc.fWidth = gm->getISize().width();
                    desc.fHeight = gm->getISize().height();
                    desc.fSampleCnt = config.fSampleCnt;
                    GrTexture* tex = gr->createUncachedTexture(desc, NULL, 0);
                    if (tex) {
                        rt.reset(tex->asRenderTarget());
                        rt.get()->ref();
                        tex->unref();
                        autogr.set(gr);
                        renderTarget = rt.get();
                        grSuccess = NULL != renderTarget;
                    }
                }
                if (!grSuccess) {
                    renderErrors |= ERROR_NO_GPU_CONTEXT;
                }
            }
#endif

            SkBitmap comparisonBitmap;

            if (ERROR_NONE == renderErrors) {
                renderErrors |= gmmain.test_drawing(gm, config, writePath,
                                                    GetGr(),
                                                    renderTarget,
                                                    &comparisonBitmap);
            }

            if (doDeferred && !renderErrors &&
                (kGPU_Backend == config.fBackend ||
                 kRaster_Backend == config.fBackend)) {
                renderErrors |= gmmain.test_deferred_drawing(gm, config,
                                                             comparisonBitmap,
                                                             GetGr(),
                                                             renderTarget);
            }

            testErrors |= renderErrors;
        }

        SkBitmap comparisonBitmap;
        const ConfigData compareConfig =
            { SkBitmap::kARGB_8888_Config, kRaster_Backend, kDontCare_GLContextType, 0, kRW_ConfigFlag, "comparison" };
        testErrors |= gmmain.generate_image(gm, compareConfig, NULL, NULL, &comparisonBitmap, false);

        // run the picture centric GM steps
        if (!(gmFlags & GM::kSkipPicture_Flag)) {

            ErrorBitfield pictErrors = ERROR_NONE;

            //SkAutoTUnref<SkPicture> pict(generate_new_picture(gm));
            SkPicture* pict = gmmain.generate_new_picture(gm, kNone_BbhType, 0);
            SkAutoUnref aur(pict);

            if ((ERROR_NONE == testErrors) && doReplay) {
                SkBitmap bitmap;
                gmmain.generate_image_from_picture(gm, compareConfig, pict,
                                                   &bitmap);
                pictErrors |= gmmain.compare_test_results_to_reference_bitmap(
                    gm, compareConfig, "-replay", bitmap, &comparisonBitmap);
            }

            if ((ERROR_NONE == testErrors) &&
                (ERROR_NONE == pictErrors) &&
                doSerialize) {
                SkPicture* repict = gmmain.stream_to_new_picture(*pict);
                SkAutoUnref aurr(repict);

                SkBitmap bitmap;
                gmmain.generate_image_from_picture(gm, compareConfig, repict,
                                                   &bitmap);
                pictErrors |= gmmain.compare_test_results_to_reference_bitmap(
                    gm, compareConfig, "-serialize", bitmap, &comparisonBitmap);
            }

            if (writePicturePath) {
                const char* pictureSuffix = "skp";
                SkString path = make_filename(writePicturePath, "",
                                              gm->shortName(),
                                              pictureSuffix);
                SkFILEWStream stream(path.c_str());
                pict->serialize(&stream);
            }

            testErrors |= pictErrors;
        }

        // TODO: add a test in which the RTree rendering results in a
        // different bitmap than the standard rendering.  It should
        // show up as failed in the JSON summary, and should be listed
        // in the stdout also.
        if (!(gmFlags & GM::kSkipPicture_Flag) && doRTree) {
            SkPicture* pict = gmmain.generate_new_picture(
                gm, kRTree_BbhType, SkPicture::kUsePathBoundsForClip_RecordingFlag);
            SkAutoUnref aur(pict);
            SkBitmap bitmap;
            gmmain.generate_image_from_picture(gm, compareConfig, pict,
                                               &bitmap);
            testErrors |= gmmain.compare_test_results_to_reference_bitmap(
                gm, compareConfig, "-rtree", bitmap, &comparisonBitmap);
        }

        if (!(gmFlags & GM::kSkipPicture_Flag) && doTileGrid) {
            for(int scaleIndex = 0; scaleIndex < tileGridReplayScales.count(); ++scaleIndex) {
                SkScalar replayScale = tileGridReplayScales[scaleIndex];
                if ((gmFlags & GM::kSkipScaledReplay_Flag) && replayScale != 1)
                    continue;
                // We record with the reciprocal scale to obtain a replay
                // result that can be validated against comparisonBitmap.
                SkScalar recordScale = SkScalarInvert(replayScale);
                SkPicture* pict = gmmain.generate_new_picture(
                    gm, kTileGrid_BbhType, SkPicture::kUsePathBoundsForClip_RecordingFlag,
                    recordScale);
                SkAutoUnref aur(pict);
                SkBitmap bitmap;
                gmmain.generate_image_from_picture(gm, compareConfig, pict,
                                                   &bitmap, replayScale);
                SkString suffix("-tilegrid");
                if (SK_Scalar1 != replayScale) {
                    suffix += "-scale-";
                    suffix.appendScalar(replayScale);
                }
                testErrors |= gmmain.compare_test_results_to_reference_bitmap(
                    gm, compareConfig, suffix.c_str(), bitmap,
                    &comparisonBitmap);
            }
        }

        // run the pipe centric GM steps
        if (!(gmFlags & GM::kSkipPipe_Flag)) {

            ErrorBitfield pipeErrors = ERROR_NONE;

            if ((ERROR_NONE == testErrors) && doPipe) {
                pipeErrors |= gmmain.test_pipe_playback(gm, compareConfig,
                                                        comparisonBitmap);
            }

            if ((ERROR_NONE == testErrors) &&
                (ERROR_NONE == pipeErrors) &&
                doTiledPipe && !(gmFlags & GM::kSkipTiled_Flag)) {
                pipeErrors |= gmmain.test_tiled_pipe_playback(gm, compareConfig,
                                                              comparisonBitmap);
            }

            testErrors |= pipeErrors;
        }

        // Update overall results.
        // We only tabulate the particular error types that we currently
        // care about (e.g., missing reference images). Later on, if we
        // want to also tabulate other error types, we can do so.
        testsRun++;
        if (!gmmain.fExpectationsSource.get() ||
            (ERROR_READING_REFERENCE_IMAGE & testErrors)) {
            testsMissingReferenceImages++;
        } else if (ERROR_NONE == testErrors) {
            testsPassed++;
        } else {
            testsFailed++;
        }

        SkDELETE(gm);
    }
    SkDebugf("Ran %d tests: %d passed, %d failed, %d missing reference images\n",
             testsRun, testsPassed, testsFailed, testsMissingReferenceImages);
    gmmain.ListErrors();

    if (NULL != writeJsonSummaryPath) {
        Json::Value actualResults;
        actualResults[kJsonKey_ActualResults_Failed] =
            gmmain.fJsonActualResults_Failed;
        actualResults[kJsonKey_ActualResults_FailureIgnored] =
            gmmain.fJsonActualResults_FailureIgnored;
        actualResults[kJsonKey_ActualResults_NoComparison] =
            gmmain.fJsonActualResults_NoComparison;
        actualResults[kJsonKey_ActualResults_Succeeded] =
            gmmain.fJsonActualResults_Succeeded;
        Json::Value root;
        root[kJsonKey_ActualResults] = actualResults;
        root[kJsonKey_ExpectedResults] = gmmain.fJsonExpectedResults;
        std::string jsonStdString = root.toStyledString();
        SkFILEWStream stream(writeJsonSummaryPath);
        stream.write(jsonStdString.c_str(), jsonStdString.length());
    }

#if SK_SUPPORT_GPU

#if GR_CACHE_STATS
    for (int i = 0; i < configs.count(); i++) {
        ConfigData config = gRec[configs[i]];

        if (kGPU_Backend == config.fBackend) {
            GrContext* gr = grFactory->get(config.fGLContextType);

            SkDebugf("config: %s %x\n", config.fName, gr);
            gr->printCacheStats();
        }
    }
#endif

    delete grFactory;
#endif
    SkGraphics::Term();

    return (0 == testsFailed) ? 0 : -1;
}

#if !defined(SK_BUILD_FOR_IOS) && !defined(SK_BUILD_FOR_NACL)
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
