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
#include "gm_error.h"
#include "gm_expectations.h"
#include "system_preferences.h"
#include "CrashHandler.h"
#include "ProcStats.h"
#include "Resources.h"
#include "SamplePipeControllers.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkDeferredCanvas.h"
#include "SkDevice.h"
#include "SkDocument.h"
#include "SkDrawFilter.h"
#include "SkForceLinking.h"
#include "SkGPipe.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkJSONCPP.h"
#include "SkMultiPictureDraw.h"
#include "SkOSFile.h"
#include "SkPDFRasterizer.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkSurface.h"
#include "SkTArray.h"
#include "SkTDict.h"

#ifdef SK_DEBUG
static const bool kDebugOnly = true;
#define GR_DUMP_FONT_CACHE 0
#else
static const bool kDebugOnly = false;
#endif

__SK_FORCE_IMAGE_DECODER_LINKING;

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "SkGpuDevice.h"
typedef GrContextFactory::GLContextType GLContextType;
#define DEFAULT_CACHE_VALUE -1
static int gGpuCacheSizeBytes;
static int gGpuCacheSizeCount;
#else
class GrContextFactory;
class GrContext;
class GrSurface;
typedef int GLContextType;
typedef int GrGLStandard;
#endif

#define DEBUGFAIL_SEE_STDERR SkDEBUGFAIL("see stderr for message")

DECLARE_bool(useDocumentInsteadOfDevice);

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
#endif

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

// TODO(epoger): Right now, various places in this code assume that all the
// image files read/written by GM use this file extension.
// Search for references to this constant to find these assumptions.
const static char kPNG_FileExtension[] = "png";

enum Backend {
    kRaster_Backend,
    kGPU_Backend,
    kPDF_Backend,
    kXPS_Backend,
};

enum BbhType {
    kNone_BbhType,
    kRTree_BbhType,
};

enum ConfigFlags {
    kNone_ConfigFlag  = 0x0,
    /* Write GM images if a write path is provided. */
    kWrite_ConfigFlag = 0x1,
    /* Read reference GM images if a read path is provided. */
    kRead_ConfigFlag  = 0x2,
    kRW_ConfigFlag    = (kWrite_ConfigFlag | kRead_ConfigFlag),
    /* Use distance fields for rendering text */
    kDFText_ConfigFlag = 0x4,
    kRWDFT_ConfigFlag = (kRW_ConfigFlag | kDFText_ConfigFlag),
};

struct ConfigData {
    SkColorType                     fColorType;
    Backend                         fBackend;
    GLContextType                   fGLContextType; // GPU backend only
    int                             fSampleCnt;     // GPU backend only
    ConfigFlags                     fFlags;
    const char*                     fName;
    bool                            fRunByDefault;
};

struct PDFRasterizerData {
    bool        (*fRasterizerFunction)(SkStream*, SkBitmap*);
    const char* fName;
    bool        fRunByDefault;
};

class BWTextDrawFilter : public SkDrawFilter {
public:
    bool filter(SkPaint*, Type) SK_OVERRIDE;
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

static SkData* encode_to_dct_data(size_t* pixelRefOffset, const SkBitmap& bitmap);
DECLARE_int32(pdfRasterDpi);

const static ErrorCombination kDefaultIgnorableErrorTypes = ErrorCombination()
    .plus(kMissingExpectations_ErrorType)
    .plus(kIntentionallySkipped_ErrorType);

class GMMain {
public:
    GMMain() : fUseFileHierarchy(false), fWriteChecksumBasedFilenames(false),
               fIgnorableErrorTypes(kDefaultIgnorableErrorTypes),
               fMismatchPath(NULL), fMissingExpectationsPath(NULL), fTestsRun(0),
               fRenderModesEncountered(1) {}

    /**
     * Assemble shortNamePlusConfig from (surprise!) shortName and configName.
     *
     * The method for doing so depends on whether we are using hierarchical naming.
     * For example, shortName "selftest1" and configName "8888" could be assembled into
     * either "selftest1_8888" or "8888/selftest1".
     */
    SkString make_shortname_plus_config(const char *shortName, const char *configName) {
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

    /**
     * Assemble filename, suitable for writing out the results of a particular test.
     */
    SkString make_filename(const char *path,
                           const char *shortName,
                           const char *configName,
                           const char *renderModeDescriptor,
                           const char *suffix) {
        SkString filename = make_shortname_plus_config(shortName, configName);
        filename.append(renderModeDescriptor);
        filename.appendUnichar('.');
        filename.append(suffix);
        return SkOSPath::Join(path, filename.c_str());
    }

    /**
     * Assemble filename suitable for writing out an SkBitmap.
     */
    SkString makeBitmapFilename(const char *path,
                                const char *shortName,
                                const char *configName,
                                const char *renderModeDescriptor,
                                const GmResultDigest &bitmapDigest) {
        if (fWriteChecksumBasedFilenames) {
            SkString filename;
            filename.append(bitmapDigest.getHashType());
            filename.appendUnichar('_');
            filename.append(shortName);
            filename.appendUnichar('_');
            filename.append(bitmapDigest.getDigestValue());
            filename.appendUnichar('.');
            filename.append(kPNG_FileExtension);
            return SkOSPath::Join(path, filename.c_str());
        } else {
            return make_filename(path, shortName, configName, renderModeDescriptor,
                                 kPNG_FileExtension);
        }
    }

    /* since PNG insists on unpremultiplying our alpha, we take no
       precision chances and force all pixels to be 100% opaque,
       otherwise on compare we may not get a perfect match.
    */
    static void force_all_opaque(const SkBitmap& bitmap) {
        SkColorType colorType = bitmap.colorType();
        switch (colorType) {
        case kN32_SkColorType:
            force_all_opaque_8888(bitmap);
            break;
        case kRGB_565_SkColorType:
            // nothing to do here; 565 bitmaps are inherently opaque
            break;
        default:
            SkDebugf("unsupported bitmap colorType %d\n", colorType);
            DEBUGFAIL_SEE_STDERR;
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

    static ErrorCombination WriteBitmap(const SkString& path, const SkBitmap& bitmap) {
        // TODO(epoger): Now that we have removed force_all_opaque()
        // from this method, we should be able to get rid of the
        // transformation to 8888 format also.
        SkBitmap copy;
        bitmap.copyTo(&copy, kN32_SkColorType);
        if (!SkImageEncoder::EncodeFile(path.c_str(), copy,
                                        SkImageEncoder::kPNG_Type,
                                        100)) {
            SkDebugf("FAILED to write bitmap: %s\n", path.c_str());
            return ErrorCombination(kWritingReferenceImage_ErrorType);
        }
        return kEmpty_ErrorCombination;
    }

    /**
     * Add all render modes encountered thus far to the "modes" array.
     */
    void GetRenderModesEncountered(SkTArray<SkString> &modes) {
        SkTDict<int>::Iter iter(this->fRenderModesEncountered);
        const char* mode;
        while ((mode = iter.next(NULL)) != NULL) {
            SkString modeAsString = SkString(mode);
            // TODO(epoger): It seems a bit silly that all of these modes were
            // recorded with a leading "-" which we have to remove here
            // (except for mode "", which means plain old original mode).
            // But that's how renderModeDescriptor has been passed into
            // compare_test_results_to_reference_bitmap() historically,
            // and changing that now may affect other parts of our code.
            if (modeAsString.startsWith("-")) {
                modeAsString.remove(0, 1);
            }
            modes.push_back(modeAsString);
        }
    }

    /**
     * Returns true if failures on this test should be ignored.
     */
    bool shouldIgnoreTest(const char *name) const {
        for (int i = 0; i < fIgnorableTestNames.count(); i++) {
            if (fIgnorableTestNames[i].equals(name)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Calls RecordTestResults to record that we skipped a test.
     *
     * Depending on the backend, this may mean that we skipped a single rendermode, or all
     * rendermodes; see http://skbug.com/1994 and https://codereview.chromium.org/129203002/
     */
    void RecordSkippedTest(const SkString& shortNamePlusConfig,
                           const char renderModeDescriptor [], Backend backend) {
        if (kRaster_Backend == backend) {
            // Skipping a test on kRaster_Backend means that we will skip ALL renderModes
            // (as opposed to other backends, on which we only run the default renderMode).
            //
            // We cannot call RecordTestResults yet, because we won't know the full set of
            // renderModes until we have run all tests.
            fTestsSkippedOnAllRenderModes.push_back(shortNamePlusConfig);
        } else {
            this->RecordTestResults(kIntentionallySkipped_ErrorType, shortNamePlusConfig,
                                    renderModeDescriptor);
        }
    }

    /**
     * Records the results of this test in fTestsRun and fFailedTests.
     *
     * We even record successes, and errors that we regard as
     * "ignorable"; we can filter them out later.
     */
    void RecordTestResults(const ErrorCombination& errorCombination,
                           const SkString& shortNamePlusConfig,
                           const char renderModeDescriptor []) {
        // Things to do regardless of errorCombination.
        fTestsRun++;
        int renderModeCount = 0;
        this->fRenderModesEncountered.find(renderModeDescriptor, &renderModeCount);
        renderModeCount++;
        this->fRenderModesEncountered.set(renderModeDescriptor, renderModeCount);

        if (errorCombination.isEmpty()) {
            return;
        }

        // Things to do only if there is some error condition.
        SkString fullName = shortNamePlusConfig;
        fullName.append(renderModeDescriptor);
        for (int typeInt = 0; typeInt <= kLast_ErrorType; typeInt++) {
            ErrorType type = static_cast<ErrorType>(typeInt);
            if (errorCombination.includes(type)) {
                fFailedTests[type].push_back(fullName);
            }
        }
    }

    /**
     * Return the number of significant (non-ignorable) errors we have
     * encountered so far.
     */
    int NumSignificantErrors() {
        int significantErrors = 0;
        for (int typeInt = 0; typeInt <= kLast_ErrorType; typeInt++) {
            ErrorType type = static_cast<ErrorType>(typeInt);
            if (!fIgnorableErrorTypes.includes(type)) {
                significantErrors += fFailedTests[type].count();
            }
        }
        return significantErrors;
    }

    /**
     * Display the summary of results with this ErrorType.
     *
     * @param type which ErrorType
     * @param verbose whether to be all verbose about it
     */
    void DisplayResultTypeSummary(ErrorType type, bool verbose) {
        bool isIgnorableType = fIgnorableErrorTypes.includes(type);

        SkString line;
        if (isIgnorableType) {
            line.append("[ ] ");
        } else {
            line.append("[*] ");
        }

        SkTArray<SkString> *failedTestsOfThisType = &fFailedTests[type];
        int count = failedTestsOfThisType->count();
        line.appendf("%d %s", count, getErrorTypeName(type));
        if (!isIgnorableType || verbose) {
            line.append(":");
            for (int i = 0; i < count; ++i) {
                line.append(" ");
                line.append((*failedTestsOfThisType)[i]);
            }
        }
        SkDebugf("%s\n", line.c_str());
    }

    /**
     * List contents of fFailedTests to stdout.
     *
     * @param verbose whether to be all verbose about it
     */
    void ListErrors(bool verbose) {
        // First, print a single summary line.
        SkString summary;
        summary.appendf("Ran %d tests:", fTestsRun);
        for (int typeInt = 0; typeInt <= kLast_ErrorType; typeInt++) {
            ErrorType type = static_cast<ErrorType>(typeInt);
            summary.appendf(" %s=%d", getErrorTypeName(type), fFailedTests[type].count());
        }
        SkDebugf("%s\n", summary.c_str());

        // Now, for each failure type, list the tests that failed that way.
        for (int typeInt = 0; typeInt <= kLast_ErrorType; typeInt++) {
            this->DisplayResultTypeSummary(static_cast<ErrorType>(typeInt), verbose);
        }
        SkDebugf("(results marked with [*] will cause nonzero return value)\n");
    }

    static ErrorCombination write_document(const SkString& path, SkStreamAsset* asset) {
        SkFILEWStream stream(path.c_str());
        if (!stream.writeStream(asset, asset->getLength())) {
            SkDebugf("FAILED to write document: %s\n", path.c_str());
            return ErrorCombination(kWritingReferenceImage_ErrorType);
        }
        return kEmpty_ErrorCombination;
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
    static void setup_bitmap(const ConfigData& gRec, const SkISize& size,
                             SkBitmap* bitmap) {
        bitmap->allocPixels(SkImageInfo::Make(size.width(), size.height(),
                                              gRec.fColorType, kPremul_SkAlphaType));
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
     *    CON: The bitmaps (and hash digests) for these non-PNG cases would be
     *         different than those for the PNG-compared cases, and in the
     *         case of a failed renderMode comparison, how would we write the
     *         image to disk for examination?
     *
     * 2. Always compute image hash digests from PNG format (either
     *    directly from the the bytes of a PNG file, or capturing the
     *    bytes we would have written to disk if we were writing the
     *    bitmap out as a PNG).
     *    PRO: I think this would allow us to never force opaque, and to
     *         the extent that alpha channel data can be preserved in a PNG
     *         file, we could observe it.
     *    CON: If we read a bitmap from disk, we need to take its hash digest
     *         from the source PNG (we can't compute it from the bitmap we
     *         read out of the PNG, because we will have already premultiplied
     *         the alpha).
     *    CON: Seems wasteful to convert a bitmap to PNG format just to take
     *         its hash digest. (Although we're wasting lots of effort already
     *         calling force_all_opaque().)
     *
     * 3. Make the alpha premultiply/unpremultiply routines 100% consistent,
     *    so we can transform images back and forth without fear of off-by-one
     *    errors.
     *    CON: Math is hard.
     *
     * 4. Perform a "close enough" comparison of bitmaps (+/- 1 bit in each
     *    channel), rather than demanding absolute equality.
     *    CON: Can't do this with hash digests.
     */
    static void complete_bitmap(SkBitmap* bitmap) {
        force_all_opaque(*bitmap);
    }

    static void InstallFilter(SkCanvas* canvas);

    static void invokeGM(GM* gm, SkCanvas* canvas, bool isPDF, bool isDeferred) {
        SkAutoCanvasRestore acr(canvas, true);

        if (!isPDF) {
            canvas->concat(gm->getInitialTransform());
        }
        InstallFilter(canvas);
        gm->setCanvasIsDeferred(isDeferred);
        gm->draw(canvas);
        canvas->setDrawFilter(NULL);
    }

    static ErrorCombination generate_image(GM* gm, const ConfigData& gRec,
                                           GrSurface* gpuTarget,
                                           SkBitmap* bitmap,
                                           bool deferred) {
        const SkISize size (gm->getISize());

        SkAutoTUnref<SkSurface> surface(CreateSurface(gRec, size, gpuTarget));
        SkAutoTUnref<SkCanvas> canvas;

        if (deferred) {
            canvas.reset(SkDeferredCanvas::Create(surface));
        } else {
            canvas.reset(SkRef(surface->getCanvas()));
        }
        invokeGM(gm, canvas, false, deferred);
        canvas->flush();

        setup_bitmap(gRec, size, bitmap);
        surface->readPixels(bitmap->info(), bitmap->getPixels(), bitmap->rowBytes(), 0, 0);
        complete_bitmap(bitmap);
        return kEmpty_ErrorCombination;
    }

    static void DrawPictureToSurface(SkSurface* surf,
                                     const SkPicture* pict,
                                     SkScalar scale,
                                     bool tile,
                                     bool useMPD) {
        SkASSERT(surf->width() == pict->cullRect().width() &&
                 surf->height() == pict->cullRect().height());

        if (tile) {
            SkMultiPictureDraw mpd;
            SkTDArray<SkSurface*> surfaces;

            const SkISize tileSize = SkISize::Make(16, 16);

            const SkImageInfo ii = surf->getCanvas()->imageInfo().makeWH(tileSize.width(),
                                                                         tileSize.height());

            for (int tileY = 0; tileY < pict->cullRect().height(); tileY += tileSize.height()) {
                for (int tileX = 0; tileX < pict->cullRect().width(); tileX += tileSize.width()) {

                    *surfaces.append() = surf->getCanvas()->newSurface(ii);

                    InstallFilter(surfaces.top()->getCanvas());

                    SkMatrix matrix;
                    matrix.setTranslate(-pict->cullRect().fLeft, -pict->cullRect().fTop);
                    matrix.postTranslate(-SkIntToScalar(tileX), -SkIntToScalar(tileY));
                    matrix.postScale(scale, scale);

                    if (useMPD) {
                        mpd.add(surfaces.top()->getCanvas(), pict, &matrix, NULL);
                    } else {
                        surfaces.top()->getCanvas()->drawPicture(pict, &matrix, NULL);
                    }
                }
            }

            mpd.draw();

            SkPaint gatherPaint;
            gatherPaint.setXfermodeMode(SkXfermode::kSrc_Mode);

            int tileIndex = 0;
            for (int tileY = 0; tileY < pict->cullRect().height(); tileY += tileSize.height()) {
                for (int tileX = 0; tileX < pict->cullRect().width(); tileX += tileSize.width()) {
                    surf->getCanvas()->drawImage(surfaces[tileIndex]->newImageSnapshot(),
                                                 SkIntToScalar(tileX), SkIntToScalar(tileY),
                                                 &gatherPaint);
                    surfaces[tileIndex]->unref();
                    tileIndex++;
                }
            }
        } else {
            InstallFilter(surf->getCanvas());

            SkMatrix matrix;
            matrix.setTranslate(-pict->cullRect().fLeft, -pict->cullRect().fTop);
            matrix.postScale(scale, scale);

            if (useMPD) {
                SkMultiPictureDraw mpd;
                mpd.add(surf->getCanvas(), pict, &matrix, NULL);
                mpd.draw();
            } else {
                surf->getCanvas()->drawPicture(pict, &matrix, NULL);
            }
        }
    }

    static void generate_image_from_picture(GM* gm, const ConfigData& config,
                                            GrSurface* gpuTarget,
                                            SkPicture* pict, SkBitmap* bitmap,
                                            SkScalar scale = SK_Scalar1,
                                            bool tile = false) {
        const SkISize size = gm->getISize();

        SkAutoTUnref<SkSurface> surf(CreateSurface(config, size, gpuTarget));

        DrawPictureToSurface(surf, pict, scale, tile, false);

        setup_bitmap(config, size, bitmap);

        surf->readPixels(bitmap->info(), bitmap->getPixels(), bitmap->rowBytes(), 0, 0);

        complete_bitmap(bitmap);
    }

    static bool generate_pdf(GM* gm, SkDynamicMemoryWStream& pdf) {
#ifdef SK_SUPPORT_PDF
        SkMatrix initialTransform = gm->getInitialTransform();
        if (FLAGS_useDocumentInsteadOfDevice) {
            SkISize pageISize = gm->getISize();
            SkAutoTUnref<SkDocument> pdfDoc(
                    SkDocument::CreatePDF(&pdf, NULL,
                                          encode_to_dct_data,
                                          SkIntToScalar(FLAGS_pdfRasterDpi)));

            if (!pdfDoc.get()) {
                return false;
            }

            SkCanvas* canvas = NULL;
            canvas = pdfDoc->beginPage(SkIntToScalar(pageISize.width()),
                                       SkIntToScalar(pageISize.height()));
            canvas->concat(initialTransform);

            invokeGM(gm, canvas, true, false);

            return pdfDoc->close();
        } else {
            SkISize pageSize = gm->getISize();
            SkPDFDevice* dev = NULL;
            if (initialTransform.isIdentity()) {
                dev = new SkPDFDevice(pageSize, pageSize, initialTransform);
            } else {
                SkRect content = SkRect::MakeWH(SkIntToScalar(pageSize.width()),
                                                SkIntToScalar(pageSize.height()));
                initialTransform.mapRect(&content);
                if (!content.intersect(0, 0, SkIntToScalar(pageSize.width()),
                                       SkIntToScalar(pageSize.height()))) {
                    content.setEmpty();
                }
                SkISize contentSize =
                    SkISize::Make(SkScalarRoundToInt(content.width()),
                                  SkScalarRoundToInt(content.height()));
                dev = new SkPDFDevice(pageSize, contentSize, initialTransform);
            }
            dev->setDCTEncoder(encode_to_dct_data);
            dev->setRasterDpi(SkIntToScalar(FLAGS_pdfRasterDpi));
            SkAutoUnref aur(dev);
            SkCanvas c(dev);
            invokeGM(gm, &c, true, false);
            SkPDFDocument doc;
            doc.appendPage(dev);
            doc.emitPDF(&pdf);
        }
#endif  // SK_SUPPORT_PDF
        return true; // Do not report failure if pdf is not supported.
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

        if ((kN32_SkColorType != expectedBitmap.colorType()) ||
            (kN32_SkColorType != actualBitmap.colorType())) {
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
        SkDebugf("---- %s: %d (of %d) differing pixels, "
                 "max per-channel mismatch R=%d G=%d B=%d A=%d\n",
                 testName, differingPixels, width*height, errR, errG, errB, errA);
    }

    /**
     * Compares actual hash digest to expectations, returning the set of errors
     * (if any) that we saw along the way.
     *
     * If fMismatchPath has been set, and there are pixel diffs, then the
     * actual bitmap will be written out to a file within fMismatchPath.
     * And similarly for fMissingExpectationsPath...
     *
     * @param expectations what expectations to compare actualBitmap against
     * @param actualBitmapAndDigest the SkBitmap we actually generated, and its GmResultDigest
     * @param shortName name of test, e.g. "selftest1"
     * @param configName name of config, e.g. "8888"
     * @param renderModeDescriptor e.g., "-rtree", "-deferred"
     * @param addToJsonSummary whether to add these results (both actual and
     *        expected) to the JSON summary. Regardless of this setting, if
     *        we find an image mismatch in this test, we will write these
     *        results to the JSON summary.  (This is so that we will always
     *        report errors across rendering modes, such as pipe vs tiled.
     *        See https://codereview.chromium.org/13650002/ )
     */
    ErrorCombination compare_to_expectations(Expectations expectations,
                                             const BitmapAndDigest& actualBitmapAndDigest,
                                             const char *shortName, const char *configName,
                                             const char *renderModeDescriptor,
                                             bool addToJsonSummary) {
        ErrorCombination errors;
        SkString shortNamePlusConfig = make_shortname_plus_config(shortName, configName);
        SkString completeNameString(shortNamePlusConfig);
        completeNameString.append(renderModeDescriptor);
        completeNameString.append(".");
        completeNameString.append(kPNG_FileExtension);
        const char* completeName = completeNameString.c_str();

        if (expectations.empty()) {
            errors.add(kMissingExpectations_ErrorType);

            // Write out the "actuals" for any tests without expectations, if we have
            // been directed to do so.
            if (fMissingExpectationsPath) {
                SkString path = this->makeBitmapFilename(fMissingExpectationsPath, shortName,
                                                         configName, renderModeDescriptor,
                                                         actualBitmapAndDigest.fDigest);
                WriteBitmap(path, actualBitmapAndDigest.fBitmap);
            }

        } else if (!expectations.match(actualBitmapAndDigest.fDigest)) {
            addToJsonSummary = true;
            // The error mode we record depends on whether this was running
            // in a non-standard renderMode.
            if ('\0' == *renderModeDescriptor) {
                errors.add(kExpectationsMismatch_ErrorType);
            } else {
                errors.add(kRenderModeMismatch_ErrorType);
            }

            // Write out the "actuals" for any mismatches, if we have
            // been directed to do so.
            if (fMismatchPath) {
                SkString path = this->makeBitmapFilename(fMismatchPath, shortName, configName,
                                                         renderModeDescriptor,
                                                         actualBitmapAndDigest.fDigest);
                WriteBitmap(path, actualBitmapAndDigest.fBitmap);
            }

            // If we have access to a single expected bitmap, log more
            // detail about the mismatch.
            const SkBitmap *expectedBitmapPtr = expectations.asBitmap();
            if (expectedBitmapPtr) {
                report_bitmap_diffs(*expectedBitmapPtr, actualBitmapAndDigest.fBitmap,
                                    completeName);
            }
        }

        if (addToJsonSummary) {
            add_actual_results_to_json_summary(completeName, actualBitmapAndDigest.fDigest, errors,
                                               expectations.ignoreFailure());
            add_expected_results_to_json_summary(completeName, expectations);
        }

        return errors;
    }

    /**
     * Add this result to the appropriate JSON collection of actual results (but just ONE),
     * depending on errors encountered.
     */
    void add_actual_results_to_json_summary(const char testName[],
                                            const GmResultDigest &actualResultDigest,
                                            ErrorCombination errors,
                                            bool ignoreFailure) {
        Json::Value jsonActualResults = actualResultDigest.asJsonTypeValuePair();
        Json::Value *resultCollection = NULL;

        if (errors.isEmpty()) {
            resultCollection = &this->fJsonActualResults_Succeeded;
        } else if (errors.includes(kRenderModeMismatch_ErrorType)) {
            resultCollection = &this->fJsonActualResults_Failed;
        } else if (errors.includes(kExpectationsMismatch_ErrorType)) {
            if (ignoreFailure) {
                resultCollection = &this->fJsonActualResults_FailureIgnored;
            } else {
                resultCollection = &this->fJsonActualResults_Failed;
            }
        } else if (errors.includes(kMissingExpectations_ErrorType)) {
            // TODO: What about the case where there IS an expected
            // image hash digest, but that gm test doesn't actually
            // run?  For now, those cases will always be ignored,
            // because gm only looks at expectations that correspond
            // to gm tests that were actually run.
            //
            // Once we have the ability to express expectations as a
            // JSON file, we should fix this (and add a test case for
            // which an expectation is given but the test is never
            // run).
            resultCollection = &this->fJsonActualResults_NoComparison;
        }

        // If none of the above cases match, we don't add it to ANY tally of actual results.
        if (resultCollection) {
            (*resultCollection)[testName] = jsonActualResults;
        }
    }

    /**
     * Add this test to the JSON collection of expected results.
     */
    void add_expected_results_to_json_summary(const char testName[],
                                              Expectations expectations) {
        this->fJsonExpectedResults[testName] = expectations.asJsonValue();
    }

    /**
     * Compare actualBitmap to expectations stored in this->fExpectationsSource.
     *
     * @param gm which test generated the actualBitmap
     * @param gRec
     * @param configName The config name to look for in the expectation file.
     * @param actualBitmapAndDigest ptr to bitmap generated by this run, or NULL
     *        if we don't have a usable bitmap representation
     */
    ErrorCombination compareTestResultsToStoredExpectations(
        GM* gm, const ConfigData& gRec, const char* configName,
        const BitmapAndDigest* actualBitmapAndDigest) {
        ErrorCombination errors;

        if (NULL == actualBitmapAndDigest) {
            // Note that we intentionally skipped validating the results for
            // this test, because we don't know how to generate an SkBitmap
            // version of the output.
            errors.add(ErrorCombination(kIntentionallySkipped_ErrorType));
        } else if (!(gRec.fFlags & kWrite_ConfigFlag)) {
            // We don't record the results for this test or compare them
            // against any expectations, because the output image isn't
            // meaningful.
            // See https://code.google.com/p/skia/issues/detail?id=1410 ('some
            // GM result images not available for download from Google Storage')
            errors.add(ErrorCombination(kIntentionallySkipped_ErrorType));
        } else {
            ExpectationsSource *expectationsSource = this->fExpectationsSource.get();
            SkString nameWithExtension = make_shortname_plus_config(gm->getName(), configName);
            nameWithExtension.append(".");
            nameWithExtension.append(kPNG_FileExtension);

            if (expectationsSource && (gRec.fFlags & kRead_ConfigFlag)) {
                /*
                 * Get the expected results for this test, as one or more allowed
                 * hash digests. The current implementation of expectationsSource
                 * get this by computing the hash digest of a single PNG file on disk.
                 *
                 * TODO(epoger): This relies on the fact that
                 * force_all_opaque() was called on the bitmap before it
                 * was written to disk as a PNG in the first place. If
                 * not, the hash digest returned here may not match the
                 * hash digest of actualBitmap, which *has* been run through
                 * force_all_opaque().
                 * See comments above complete_bitmap() for more detail.
                 */
                Expectations expectations = expectationsSource->get(nameWithExtension.c_str());
                if (this->shouldIgnoreTest(gm->getName())) {
                    expectations.setIgnoreFailure(true);
                }
                errors.add(compare_to_expectations(expectations, *actualBitmapAndDigest,
                                                   gm->getName(), configName, "", true));
            } else {
                // If we are running without expectations, we still want to
                // record the actual results.
                add_actual_results_to_json_summary(nameWithExtension.c_str(),
                                                   actualBitmapAndDigest->fDigest,
                                                   ErrorCombination(kMissingExpectations_ErrorType),
                                                   false);
                errors.add(ErrorCombination(kMissingExpectations_ErrorType));
            }
        }
        return errors;
    }

    /**
     * Compare actualBitmap to referenceBitmap.
     *
     * @param shortName test name, e.g. "selftest1"
     * @param configName configuration name, e.g. "8888"
     * @param renderModeDescriptor
     * @param actualBitmap actual bitmap generated by this run
     * @param referenceBitmap bitmap we expected to be generated
     */
    ErrorCombination compare_test_results_to_reference_bitmap(
        const char *shortName, const char *configName, const char *renderModeDescriptor,
        SkBitmap& actualBitmap, const SkBitmap* referenceBitmap) {

        SkASSERT(referenceBitmap);
        Expectations expectations(*referenceBitmap);
        BitmapAndDigest actualBitmapAndDigest(actualBitmap);

        // TODO: Eliminate RecordTestResults from here.
        // Results recording code for the test_drawing path has been refactored so that
        // RecordTestResults is only called once, at the topmost level. However, the
        // other paths have not yet been refactored, and RecordTestResults has been added
        // here to maintain proper behavior for calls not coming from the test_drawing path.
        ErrorCombination errors;
        errors.add(compare_to_expectations(expectations, actualBitmapAndDigest, shortName,
                                           configName, renderModeDescriptor, false));
        SkString shortNamePlusConfig = make_shortname_plus_config(shortName, configName);
        RecordTestResults(errors, shortNamePlusConfig, renderModeDescriptor);

        return errors;
    }

    static SkPicture* generate_new_picture(GM* gm, BbhType bbhType, uint32_t recordFlags,
                                           SkScalar scale = SK_Scalar1) {
        SkScalar width = SkScalarMul(SkIntToScalar(gm->getISize().width()), scale);
        SkScalar height = SkScalarMul(SkIntToScalar(gm->getISize().height()), scale);

        SkAutoTDelete<SkBBHFactory> factory;
        if (kRTree_BbhType == bbhType) {
            factory.reset(SkNEW(SkRTreeFactory));
        }
        SkPictureRecorder recorder;
        SkCanvas* cv = recorder.beginRecording(width, height, factory.get(), recordFlags);
        cv->scale(scale, scale);
        invokeGM(gm, cv, false, false);
        return recorder.endRecording();
    }

    static SkPicture* stream_to_new_picture(const SkPicture& src) {
        SkDynamicMemoryWStream storage;
        src.serialize(&storage);
        SkAutoTUnref<SkStreamAsset> pictReadback(storage.detachAsStream());
        SkPicture* retval = SkPicture::CreateFromStream(pictReadback,
                                                        &SkImageDecoder::DecodeMemory);
        return retval;
    }

    // Test: draw into a bitmap or pdf.
    // Depending on flags, possibly compare to an expected image.
    // If writePath is not NULL, also write images (or documents) to the specified path.
    ErrorCombination test_drawing(GM* gm, const ConfigData& gRec,
                                  const SkTDArray<const PDFRasterizerData*> &pdfRasterizers,
                                  const char writePath [],
                                  GrSurface* gpuTarget,
                                  SkBitmap* bitmap) {
        ErrorCombination errors;
        SkDynamicMemoryWStream document;

        if (gRec.fBackend == kRaster_Backend ||
            gRec.fBackend == kGPU_Backend) {
            // Early exit if we can't generate the image.
            errors.add(generate_image(gm, gRec, gpuTarget, bitmap, false));
            if (!errors.isEmpty()) {
                // TODO: Add a test to exercise what the stdout and
                // JSON look like if we get an "early error" while
                // trying to generate the image.
                return errors;
            }

            errors.add(this->writeBitmap(gm, gRec, gRec.fName, writePath, *bitmap));
        } else if (gRec.fBackend == kPDF_Backend) {
            if (!generate_pdf(gm, document)) {
                errors.add(kGeneratePdfFailed_ErrorType);
            } else {
                SkAutoTUnref<SkStreamAsset> documentStream(document.detachAsStream());
                if (writePath && (gRec.fFlags & kWrite_ConfigFlag)) {
                    SkString path = make_filename(writePath, gm->getName(), gRec.fName, "", "pdf");
                    errors.add(write_document(path, documentStream));
                }

                if (!(gm->getFlags() & GM::kSkipPDFRasterization_Flag)) {
                    for (int i = 0; i < pdfRasterizers.count(); i++) {
                        SkBitmap pdfBitmap;
                        documentStream->rewind();
                        bool success = (*pdfRasterizers[i]->fRasterizerFunction)(
                                documentStream.get(), &pdfBitmap);
                        if (!success) {
                            SkDebugf("FAILED to render PDF for %s using renderer %s\n",
                                     gm->getName(),
                                     pdfRasterizers[i]->fName);
                            continue;
                        }

                        SkString configName(gRec.fName);
                        configName.append("-");
                        configName.append(pdfRasterizers[i]->fName);

                        errors.add(this->writeBitmap(gm, gRec, configName.c_str(),
                                                     writePath, pdfBitmap));
                    }
                } else {
                    errors.add(kIntentionallySkipped_ErrorType);
                }
            }
        } else if (gRec.fBackend == kXPS_Backend) {
            generate_xps(gm, document);
            SkAutoTUnref<SkStreamAsset> documentStream(document.detachAsStream());

            errors.add(this->compareTestResultsToStoredExpectations(
                           gm, gRec, gRec.fName, NULL));

            if (writePath && (gRec.fFlags & kWrite_ConfigFlag)) {
                SkString path = make_filename(writePath, gm->getName(), gRec.fName, "", "xps");
                errors.add(write_document(path, documentStream));
            }
        } else {
            SkASSERT(false);
        }
        return errors;
    }

    ErrorCombination test_deferred_drawing(GM* gm,
                                           const ConfigData& gRec,
                                           const SkBitmap& referenceBitmap,
                                           GrSurface* gpuTarget) {
        if (gRec.fBackend == kRaster_Backend ||
            gRec.fBackend == kGPU_Backend) {
            const char renderModeDescriptor[] = "-deferred";
            SkBitmap bitmap;
            // Early exit if we can't generate the image, but this is
            // expected in some cases, so don't report a test failure.
            ErrorCombination errors = generate_image(gm, gRec, gpuTarget, &bitmap, true);
            // TODO(epoger): This logic is the opposite of what is
            // described above... if we succeeded in generating the
            // -deferred image, we exit early!  We should fix this
            // ASAP, because it is hiding -deferred errors... but for
            // now, I'm leaving the logic as it is so that the
            // refactoring change
            // https://codereview.chromium.org/12992003/ is unblocked.
            //
            // Filed as https://code.google.com/p/skia/issues/detail?id=1180
            // ('image-surface gm test is failing in "deferred" mode,
            // and gm is not reporting the failure')
            if (errors.isEmpty()) {
                // TODO(epoger): Report this as a new ErrorType,
                // something like kImageGeneration_ErrorType?
                return kEmpty_ErrorCombination;
            }
            return compare_test_results_to_reference_bitmap(
                gm->getName(), gRec.fName, renderModeDescriptor, bitmap, &referenceBitmap);
        }
        return kEmpty_ErrorCombination;
    }

    static SkSurface* CreateSurface(const ConfigData& config,
                                    const SkISize& size,
                                    GrSurface* gpuTarget) {
        if (config.fBackend == kRaster_Backend) {
            SkImageInfo ii = SkImageInfo::Make(size.width(), size.height(),
                                               config.fColorType, kPremul_SkAlphaType);

            return SkSurface::NewRaster(ii);
        }
#if SK_SUPPORT_GPU
        else {
            uint32_t flags = (config.fFlags & kDFText_ConfigFlag) ?
                                SkSurfaceProps::kUseDistanceFieldFonts_Flag : 0;
            SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
            return SkSurface::NewRenderTargetDirect(gpuTarget->asRenderTarget(), &props);
        }
#endif

        return NULL;
    }

    ErrorCombination writeBitmap(GM* gm,
                                 const ConfigData& config,
                                 const char* configName,
                                 const char* writePath,
                                 const SkBitmap& bitmap) {
        ErrorCombination errors;

        BitmapAndDigest bitmapAndDigest(bitmap);
        errors.add(this->compareTestResultsToStoredExpectations(gm, config,
                                                                configName, &bitmapAndDigest));

        if (writePath && (config.fFlags & kWrite_ConfigFlag)) {
            SkString path;

            path = this->makeBitmapFilename(writePath, gm->getName(), configName,
                                            "", bitmapAndDigest.fDigest);
            errors.add(WriteBitmap(path, bitmapAndDigest.fBitmap));
        }

        return errors;
    }

    ErrorCombination testMPDDrawing(GM* gm,
                                    const ConfigData& config,
                                    const char* writePath,
                                    GrSurface* gpuTarget,
                                    const SkBitmap& referenceBitmap) {
        SkASSERT(kRaster_Backend == config.fBackend || kGPU_Backend == config.fBackend);

        static const uint32_t kMPDFlags = SkPictureRecorder::kComputeSaveLayerInfo_RecordFlag;

        SkAutoTUnref<SkPicture> pict(generate_new_picture(gm, kRTree_BbhType, kMPDFlags));

        SkAutoTUnref<SkSurface> surf(CreateSurface(config, gm->getISize(), gpuTarget));

        DrawPictureToSurface(surf, pict, SK_Scalar1, false, true);

        SkBitmap bitmap;

        setup_bitmap(config, gm->getISize(), &bitmap);

        surf->readPixels(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes(), 0, 0);
        complete_bitmap(&bitmap);

        SkString configName(config.fName);
        configName.append("-mpd");

        return this->writeBitmap(gm, config, configName.c_str(), writePath, bitmap);
    }

    ErrorCombination test_pipe_playback(GM* gm, const ConfigData& gRec,
                                        const SkBitmap& referenceBitmap, bool simulateFailure) {
        const SkString shortNamePlusConfig = make_shortname_plus_config(gm->getName(),
                                                                        gRec.fName);
        ErrorCombination errors;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gPipeWritingFlagCombos); ++i) {
            SkString renderModeDescriptor("-pipe");
            renderModeDescriptor.append(gPipeWritingFlagCombos[i].name);

            if (gm->getFlags() & GM::kSkipPipe_Flag
                || (gPipeWritingFlagCombos[i].flags == SkGPipeWriter::kCrossProcess_Flag
                    && gm->getFlags() & GM::kSkipPipeCrossProcess_Flag)) {
                RecordTestResults(kIntentionallySkipped_ErrorType, shortNamePlusConfig,
                                  renderModeDescriptor.c_str());
                errors.add(kIntentionallySkipped_ErrorType);
            } else {
                SkBitmap bitmap;
                SkISize size = gm->getISize();
                setup_bitmap(gRec, size, &bitmap);
                SkCanvas canvas(bitmap);
                InstallFilter(&canvas);
                // Pass a decoding function so the factory GM (which has an SkBitmap
                // with encoded data) will not fail playback.
                PipeController pipeController(&canvas, &SkImageDecoder::DecodeMemory);
                SkGPipeWriter writer;
                SkCanvas* pipeCanvas = writer.startRecording(&pipeController,
                                                             gPipeWritingFlagCombos[i].flags,
                                                             size.width(), size.height());
                if (!simulateFailure) {
                    invokeGM(gm, pipeCanvas, false, false);
                }
                complete_bitmap(&bitmap);
                writer.endRecording();
                errors.add(compare_test_results_to_reference_bitmap(
                    gm->getName(), gRec.fName, renderModeDescriptor.c_str(), bitmap,
                    &referenceBitmap));
                if (!errors.isEmpty()) {
                    break;
                }
            }
        }
        return errors;
    }

    ErrorCombination test_tiled_pipe_playback(GM* gm, const ConfigData& gRec,
                                              const SkBitmap& referenceBitmap) {
        const SkString shortNamePlusConfig = make_shortname_plus_config(gm->getName(),
                                                                        gRec.fName);
        ErrorCombination errors;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gPipeWritingFlagCombos); ++i) {
            SkString renderModeDescriptor("-tiled pipe");
            renderModeDescriptor.append(gPipeWritingFlagCombos[i].name);

            if ((gm->getFlags() & GM::kSkipPipe_Flag) ||
                (gm->getFlags() & GM::kSkipTiled_Flag)) {
                RecordTestResults(kIntentionallySkipped_ErrorType, shortNamePlusConfig,
                                  renderModeDescriptor.c_str());
                errors.add(kIntentionallySkipped_ErrorType);
            } else {
                SkBitmap bitmap;
                SkISize size = gm->getISize();
                setup_bitmap(gRec, size, &bitmap);
                SkCanvas canvas(bitmap);
                InstallFilter(&canvas);
                TiledPipeController pipeController(bitmap, &SkImageDecoder::DecodeMemory);
                SkGPipeWriter writer;
                SkCanvas* pipeCanvas = writer.startRecording(&pipeController,
                                                             gPipeWritingFlagCombos[i].flags,
                                                             size.width(), size.height());
                invokeGM(gm, pipeCanvas, false, false);
                complete_bitmap(&bitmap);
                writer.endRecording();
                errors.add(compare_test_results_to_reference_bitmap(gm->getName(), gRec.fName,
                                                                    renderModeDescriptor.c_str(),
                                                                    bitmap, &referenceBitmap));
                if (!errors.isEmpty()) {
                    break;
                }
            }
        }
        return errors;
    }

    //
    // member variables.
    // They are public for now, to allow easier setting by tool_main().
    //

    bool fUseFileHierarchy, fWriteChecksumBasedFilenames;
    ErrorCombination fIgnorableErrorTypes;
    SkTArray<SkString> fIgnorableTestNames;

    const char* fMismatchPath;
    const char* fMissingExpectationsPath;

    // collection of tests that have failed with each ErrorType
    SkTArray<SkString> fFailedTests[kLast_ErrorType+1];
    SkTArray<SkString> fTestsSkippedOnAllRenderModes;
    int fTestsRun;
    SkTDict<int> fRenderModesEncountered;

    // Where to read expectations (expected image hash digests, etc.) from.
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

static const ConfigData gRec[] = {
    { kN32_SkColorType,     kRaster_Backend, kDontCare_GLContextType,                  0, kRW_ConfigFlag,    "8888",         true },
    { kRGB_565_SkColorType, kRaster_Backend, kDontCare_GLContextType,                  0, kRW_ConfigFlag,    "565",          true },
#if SK_SUPPORT_GPU
    { kN32_SkColorType, kGPU_Backend,    GrContextFactory::kNative_GLContextType,  0, kRW_ConfigFlag,    "gpu",          true },
    { kN32_SkColorType, kGPU_Backend,    GrContextFactory::kNative_GLContextType, 16, kRW_ConfigFlag,    "msaa16",       false},
    { kN32_SkColorType, kGPU_Backend,    GrContextFactory::kNative_GLContextType,  4, kRW_ConfigFlag,    "msaa4",        false},
    { kN32_SkColorType, kGPU_Backend,    GrContextFactory::kNVPR_GLContextType,    4, kRW_ConfigFlag,    "nvprmsaa4",   true },
    { kN32_SkColorType, kGPU_Backend,    GrContextFactory::kNVPR_GLContextType,   16, kRW_ConfigFlag,    "nvprmsaa16",  false},
    /* Not quite ready to turn on distance field text baselines */
    { kN32_SkColorType, kGPU_Backend,    GrContextFactory::kNative_GLContextType,  0, kRWDFT_ConfigFlag, "gpudft",      false },
    /* The gpudebug context does not generate meaningful images, so don't record
     * the images it generates!  We only run it to look for asserts. */
    { kN32_SkColorType, kGPU_Backend,    GrContextFactory::kDebug_GLContextType,   0, kNone_ConfigFlag,  "gpudebug",     kDebugOnly},
    /* The gpunull context does the least amount of work possible and doesn't
       generate meaninful images, so don't record them!. It can be run to
       isolate the CPU-side processing expense from the GPU-side.
      */
    { kN32_SkColorType, kGPU_Backend,    GrContextFactory::kNull_GLContextType,    0, kNone_ConfigFlag,  "gpunull",      kDebugOnly},
#if SK_ANGLE
    { kN32_SkColorType, kGPU_Backend,    GrContextFactory::kANGLE_GLContextType,   0, kRW_ConfigFlag,    "angle",        true },
    { kN32_SkColorType, kGPU_Backend,    GrContextFactory::kANGLE_GLContextType,  16, kRW_ConfigFlag,    "anglemsaa16",  true },
#endif // SK_ANGLE
#ifdef SK_MESA
    { kN32_SkColorType, kGPU_Backend,    GrContextFactory::kMESA_GLContextType,    0, kRW_ConfigFlag,    "mesa",         true },
#endif // SK_MESA
#endif // SK_SUPPORT_GPU
#ifdef SK_SUPPORT_XPS
    /* At present we have no way of comparing XPS files (either natively or by converting to PNG). */
    { kN32_SkColorType, kXPS_Backend,    kDontCare_GLContextType,                  0, kWrite_ConfigFlag, "xps",          true },
#endif // SK_SUPPORT_XPS
#ifdef SK_SUPPORT_PDF
    { kN32_SkColorType, kPDF_Backend,    kDontCare_GLContextType,                  0, kRW_ConfigFlag,    "pdf",          true },
#endif // SK_SUPPORT_PDF
};

static bool SkNoRasterizePDF(SkStream*, SkBitmap*) { return false; }

static const PDFRasterizerData kPDFRasterizers[] = {
#ifdef SK_BUILD_FOR_MAC
    { &SkPDFDocumentToBitmap, "mac",     true },
#endif
#ifdef SK_BUILD_POPPLER
    { &SkPopplerRasterizePDF, "poppler", true },
#endif
#ifdef SK_BUILD_NATIVE_PDF_RENDERER
    { &SkNativeRasterizePDF,  "native",  true },
#endif  // SK_BUILD_NATIVE_PDF_RENDERER
    // The following exists so that this array is never zero length.
    { &SkNoRasterizePDF,      "none",    false},
};

static const char kDefaultsConfigStr[] = "defaults";
static const char kExcludeConfigChar = '~';
#if SK_SUPPORT_GPU
static const char kGpuAPINameGL[] = "gl";
static const char kGpuAPINameGLES[] = "gles";
#endif

static SkString configUsage() {
    SkString result;
    result.appendf("Space delimited list of which configs to run. Possible options: [");
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        SkASSERT(gRec[i].fName != kDefaultsConfigStr);
        if (i > 0) {
            result.append("|");
        }
        result.appendf("%s", gRec[i].fName);
    }
    result.append("]\n");
    result.appendf("The default value is: \"");
    SkString firstDefault;
    SkString allButFirstDefaults;
    SkString nonDefault;
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        if (gRec[i].fRunByDefault) {
            if (i > 0) {
                result.append(" ");
            }
            result.append(gRec[i].fName);
            if (firstDefault.isEmpty()) {
                firstDefault = gRec[i].fName;
            } else {
                if (!allButFirstDefaults.isEmpty()) {
                    allButFirstDefaults.append(" ");
                }
                allButFirstDefaults.append(gRec[i].fName);
            }
        } else {
            nonDefault = gRec[i].fName;
        }
    }
    result.append("\"\n");
    result.appendf("\"%s\" evaluates to the default set of configs.\n", kDefaultsConfigStr);
    result.appendf("Prepending \"%c\" on a config name excludes it from the set of configs to run.\n"
                   "Exclusions always override inclusions regardless of order.\n",
                   kExcludeConfigChar);
    result.appendf("E.g. \"--config %s %c%s %s\" will run these configs:\n\t%s %s",
                   kDefaultsConfigStr,
                   kExcludeConfigChar,
                   firstDefault.c_str(),
                   nonDefault.c_str(),
                   allButFirstDefaults.c_str(),
                   nonDefault.c_str());
    return result;
}

static SkString pdfRasterizerUsage() {
    SkString result;
    result.appendf("Space delimited list of which PDF rasterizers to run. Possible options: [");
    // For this (and further) loops through kPDFRasterizers, there is a typecast to int to avoid
    // the compiler giving an "comparison of unsigned expression < 0 is always false" warning
    // and turning it into a build-breaking error.
    for (int i = 0; i < (int)SK_ARRAY_COUNT(kPDFRasterizers); ++i) {
        if (i > 0) {
            result.append(" ");
        }
        result.append(kPDFRasterizers[i].fName);
    }
    result.append("]\n");
    result.append("The default value is: \"");
    for (int i = 0; i < (int)SK_ARRAY_COUNT(kPDFRasterizers); ++i) {
        if (kPDFRasterizers[i].fRunByDefault) {
            if (i > 0) {
                result.append(" ");
            }
            result.append(kPDFRasterizers[i].fName);
        }
    }
    result.append("\"");
    return result;
}

// Macro magic to convert a numeric preprocessor token into a string.
// Adapted from http://stackoverflow.com/questions/240353/convert-a-preprocessor-token-to-a-string
// This should probably be moved into one of our common headers...
#define TOSTRING_INTERNAL(x) #x
#define TOSTRING(x) TOSTRING_INTERNAL(x)

// Alphabetized ignoring "no" prefix ("readPath", "noreplay", "resourcePath").
DEFINE_string(config, "", configUsage().c_str());
DEFINE_bool(cpu, true, "Allows non-GPU configs to be run. Applied after --config.");
DEFINE_string(pdfRasterizers, "default", pdfRasterizerUsage().c_str());
DEFINE_bool(deferred, false, "Exercise the deferred rendering test pass.");
DEFINE_bool(mpd, false, "Exercise MultiPictureDraw.");

DEFINE_bool(dryRun, false, "Don't actually run the tests, just print what would have been done.");
DEFINE_string(excludeConfig, "", "Space delimited list of configs to skip.");
DEFINE_bool(forceBWtext, false, "Disable text anti-aliasing.");
#if SK_SUPPORT_GPU
DEFINE_string(gpuAPI, "", "Force use of specific gpu API.  Using \"gl\" "
              "forces OpenGL API. Using \"gles\" forces OpenGL ES API. "
              "Defaults to empty string, which selects the API native to the "
              "system.");
DEFINE_string(gpuCacheSize, "", "<bytes> <count>: Limit the gpu cache to byte size or "
              "object count. " TOSTRING(DEFAULT_CACHE_VALUE) " for either value means "
              "use the default. 0 for either disables the cache.");
DEFINE_bool(gpu, true, "Allows GPU configs to be run. Applied after --config.");
DEFINE_bool(gpuCompressAlphaMasks, false, "Compress masks generated from falling back to "
                                          "software path rendering.");
#endif
DEFINE_bool(hierarchy, false, "Whether to use multilevel directory structure "
            "when reading/writing files.");
DEFINE_string(ignoreErrorTypes, kDefaultIgnorableErrorTypes.asString(" ").c_str(),
              "Space-separated list of ErrorTypes that should be ignored. If any *other* error "
              "types are encountered, the tool will exit with a nonzero return value.");
DEFINE_string(ignoreFailuresFile, "", "Path to file containing a list of tests for which we "
              "should ignore failures.\n"
              "The file should list one test per line, except for comment lines starting with #");
DEFINE_bool2(leaks, l, false, "show leaked ref cnt'd objects.");
DEFINE_string(match, "", "[~][^]substring[$] [...] of test name to run.\n"
              "Multiple matches may be separated by spaces.\n"
              "~ causes a matching test to always be skipped\n"
              "^ requires the start of the test to match\n"
              "$ requires the end of the test to match\n"
              "^ and $ requires an exact match\n"
              "If a test does not match any list entry,\n"
              "it is skipped unless some list entry starts with ~");
DEFINE_string(missingExpectationsPath, "", "Write images for tests without expectations "
              "into this directory.");
DEFINE_string(mismatchPath, "", "Write images for tests that failed due to "
              "pixel mismatches into this directory.");
DEFINE_string(modulo, "", "[--modulo <remainder> <divisor>]: only run tests for which "
              "testIndex %% divisor == remainder.");
DEFINE_bool(pipe, false, "Exercise the SkGPipe replay test pass.");
DEFINE_string2(readPath, r, "", "Read reference images from this dir, and report "
               "any differences between those and the newly generated ones.");
DEFINE_bool(replay, false, "Exercise the SkPicture replay test pass.");

#ifdef SK_BUILD_FOR_ANDROID
DEFINE_bool(resetGpuContext, true, "Reset the GrContext prior to running each GM.");
#else
DEFINE_bool(resetGpuContext, false, "Reset the GrContext prior to running each GM.");
#endif

DEFINE_bool(rtree, false, "Exercise the R-Tree variant of SkPicture test pass.");
DEFINE_bool(serialize, false, "Exercise the SkPicture serialization & deserialization test pass.");
DEFINE_bool(simulatePipePlaybackFailure, false, "Simulate a rendering failure in pipe mode only.");
DEFINE_bool(tiledPipe, false, "Exercise tiled SkGPipe replay.");
DEFINE_bool(tileGrid, false, "Exercise the tile grid variant of SkPicture.");
DEFINE_string(tileGridReplayScales, "", "Space separated list of floating-point scale "
              "factors to be used for tileGrid playback testing. Default value: 1.0");
DEFINE_bool2(verbose, v, false, "Give more detail (e.g. list all GMs run, more info about "
             "each test).");
DEFINE_bool(writeChecksumBasedFilenames, false, "When writing out actual images, use checksum-"
            "based filenames, as rebaseline.py will use when downloading them from Google Storage");
DEFINE_string(writeJsonSummaryPath, "", "Write a JSON-formatted result summary to this file.");
DEFINE_string2(writePath, w, "",  "Write rendered images into this directory.");
DEFINE_string2(writePicturePath, p, "", "Write .skp files into this directory.");
DEFINE_int32(pdfJpegQuality, -1, "Encodes images in JPEG at quality level N, "
             "which can be in range 0-100). N = -1 will disable JPEG compression. "
             "Default is N = 100, maximum quality.");
// TODO(edisonn): pass a matrix instead of forcePerspectiveMatrix
// Either the 9 numbers defining the matrix
// or probably more readable would be to replace it with a set of a few predicates
// Like --prerotate 100 200 10 --posttranslate 10, 10
// Probably define spacial names like centerx, centery, top, bottom, left, right
// then we can write something reabable like --rotate centerx centery 90
DEFINE_bool(forcePerspectiveMatrix, false, "Force a perspective matrix.");
DEFINE_bool(useDocumentInsteadOfDevice, false, "Use SkDocument::CreateFoo instead of SkFooDevice.");
DEFINE_int32(pdfRasterDpi, 72, "Scale at which at which the non suported "
             "features in PDF are rasterized. Must be be in range 0-10000. "
             "Default is 72. N = 0 will disable rasterizing features like "
             "text shadows or perspective bitmaps.");
static SkData* encode_to_dct_data(size_t*, const SkBitmap& bitmap) {
    // Filter output of warnings that JPEG is not available for the image.
    if (bitmap.width() >= 65500 || bitmap.height() >= 65500) return NULL;
    if (FLAGS_pdfJpegQuality == -1) return NULL;

    SkBitmap bm = bitmap;
#if defined(SK_BUILD_FOR_MAC)
    // Workaround bug #1043 where bitmaps with referenced pixels cause
    // CGImageDestinationFinalize to crash
    SkBitmap copy;
    bitmap.deepCopyTo(&copy);
    bm = copy;
#endif

    SkPixelRef* pr = bm.pixelRef();
    if (pr != NULL) {
        SkData* data = pr->refEncodedData();
        if (data != NULL) {
            return data;
        }
    }

    return SkImageEncoder::EncodeData(bm,
                                      SkImageEncoder::kJPEG_Type,
                                      FLAGS_pdfJpegQuality);
}

static int findConfig(const char config[]) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); i++) {
        if (!strcmp(config, gRec[i].fName)) {
            return (int) i;
        }
    }
    return -1;
}

static const PDFRasterizerData* findPDFRasterizer(const char rasterizer[]) {
    for (int i = 0; i < (int)SK_ARRAY_COUNT(kPDFRasterizers); i++) {
        if (!strcmp(rasterizer, kPDFRasterizers[i].fName)) {
            return &kPDFRasterizers[i];
        }
    }
    return NULL;
}

template <typename T> void appendUnique(SkTDArray<T>* array, const T& value) {
    int index = array->find(value);
    if (index < 0) {
        *array->append() = value;
    }
}

/**
 * Run this test in a number of different drawing modes (pipe,
 * deferred, tiled, etc.), confirming that the resulting bitmaps all
 * *exactly* match comparisonBitmap.
 *
 * Returns all errors encountered while doing so.
 */
ErrorCombination run_multiple_modes(GMMain &gmmain, GM *gm,
                                    const ConfigData &compareConfig, GrSurface* gpuTarget,
                                    const SkBitmap &comparisonBitmap,
                                    const SkTDArray<SkScalar> &tileGridReplayScales);
ErrorCombination run_multiple_modes(GMMain &gmmain, GM *gm,
                                    const ConfigData &compareConfig, GrSurface* gpuTarget,
                                    const SkBitmap &comparisonBitmap,
                                    const SkTDArray<SkScalar> &tileGridReplayScales) {
    ErrorCombination errorsForAllModes;
    uint32_t gmFlags = gm->getFlags();
    const SkString shortNamePlusConfig = gmmain.make_shortname_plus_config(gm->getName(),
                                                                           compareConfig.fName);

    SkPicture* pict = gmmain.generate_new_picture(gm, kNone_BbhType, 0);
    SkAutoTUnref<SkPicture> aur(pict);
    if (FLAGS_replay) {
        const char renderModeDescriptor[] = "-replay";
        if (gmFlags & GM::kSkipPicture_Flag) {
            gmmain.RecordTestResults(kIntentionallySkipped_ErrorType, shortNamePlusConfig,
                                     renderModeDescriptor);
            errorsForAllModes.add(kIntentionallySkipped_ErrorType);
        } else {
            SkBitmap bitmap;
            gmmain.generate_image_from_picture(gm, compareConfig, gpuTarget, pict, &bitmap);

            errorsForAllModes.add(gmmain.compare_test_results_to_reference_bitmap(
                gm->getName(), compareConfig.fName, renderModeDescriptor, bitmap,
                &comparisonBitmap));
        }
    }

    if (FLAGS_serialize) {
        const char renderModeDescriptor[] = "-serialize";
        if (gmFlags & GM::kSkipPicture_Flag) {
            gmmain.RecordTestResults(kIntentionallySkipped_ErrorType, shortNamePlusConfig,
                                     renderModeDescriptor);
            errorsForAllModes.add(kIntentionallySkipped_ErrorType);
        } else {
            SkPicture* repict = gmmain.stream_to_new_picture(*pict);
            SkAutoTUnref<SkPicture> aurr(repict);
            SkBitmap bitmap;
            gmmain.generate_image_from_picture(gm, compareConfig, gpuTarget, repict, &bitmap);
            errorsForAllModes.add(gmmain.compare_test_results_to_reference_bitmap(
                gm->getName(), compareConfig.fName, renderModeDescriptor, bitmap,
                &comparisonBitmap));
        }
    }

    if ((1 == FLAGS_writePicturePath.count()) &&
        !(gmFlags & GM::kSkipPicture_Flag)) {
        const char* pictureSuffix = "skp";
        // TODO(epoger): Make sure this still works even though the
        // filename now contains the config name (it used to contain
        // just the shortName).  I think this is actually an
        // *improvement*, because now runs with different configs will
        // write out their SkPictures to separate files rather than
        // overwriting each other.  But we should make sure it doesn't
        // break anybody.
        SkString path = gmmain.make_filename(FLAGS_writePicturePath[0], gm->getName(),
                                             compareConfig.fName, "", pictureSuffix);
        SkFILEWStream stream(path.c_str());
        pict->serialize(&stream);
    }

    if (FLAGS_rtree) {
        const char renderModeDescriptor[] = "-rtree";
        if ((gmFlags & GM::kSkipPicture_Flag) || (gmFlags & GM::kSkipTiled_Flag)) {
            gmmain.RecordTestResults(kIntentionallySkipped_ErrorType, shortNamePlusConfig,
                                     renderModeDescriptor);
            errorsForAllModes.add(kIntentionallySkipped_ErrorType);
        } else {
            SkPicture* pict = gmmain.generate_new_picture(gm, kRTree_BbhType, 0);
            SkAutoTUnref<SkPicture> aur(pict);
            SkBitmap bitmap;
            gmmain.generate_image_from_picture(gm, compareConfig, gpuTarget, pict, &bitmap);
            errorsForAllModes.add(gmmain.compare_test_results_to_reference_bitmap(
                gm->getName(), compareConfig.fName, renderModeDescriptor, bitmap,
                &comparisonBitmap));
        }
    }

    // run the pipe centric GM steps
    if (FLAGS_pipe) {
        errorsForAllModes.add(gmmain.test_pipe_playback(gm, compareConfig, comparisonBitmap,
                                                        FLAGS_simulatePipePlaybackFailure));
        if (FLAGS_tiledPipe) {
            errorsForAllModes.add(gmmain.test_tiled_pipe_playback(gm, compareConfig,
                                                                  comparisonBitmap));
        }
    }
    return errorsForAllModes;
}


/**
 * Run this test in a number of different configs (8888, 565, PDF,
 * etc.), confirming that the resulting bitmaps match expectations
 * (which may be different for each config).
 *
 * Returns all errors encountered while doing so.
 */
ErrorCombination run_multiple_configs(GMMain &gmmain, GM *gm,
                                      const SkTDArray<size_t> &configs,
                                      const SkTDArray<const PDFRasterizerData*> &pdfRasterizers,
                                      const SkTDArray<SkScalar> &tileGridReplayScales,
                                      GrContextFactory *grFactory,
                                      GrGLStandard gpuAPI);
ErrorCombination run_multiple_configs(GMMain &gmmain, GM *gm,
                                      const SkTDArray<size_t> &configs,
                                      const SkTDArray<const PDFRasterizerData*> &pdfRasterizers,
                                      const SkTDArray<SkScalar> &tileGridReplayScales,
                                      GrContextFactory *grFactory,
                                      GrGLStandard gpuAPI) {
    const char renderModeDescriptor[] = "";
    ErrorCombination errorsForAllConfigs;
    uint32_t gmFlags = gm->getFlags();

    for (int i = 0; i < configs.count(); i++) {
        ConfigData config = gRec[configs[i]];
        const SkString shortNamePlusConfig = gmmain.make_shortname_plus_config(gm->getName(),
                                                                               config.fName);

        // Skip any tests that we don't even need to try.
        // If any of these were skipped on a per-GM basis, record them as
        // kIntentionallySkipped.
        if (kPDF_Backend == config.fBackend) {
            if (gmFlags & GM::kSkipPDF_Flag) {
                gmmain.RecordSkippedTest(shortNamePlusConfig,
                                         renderModeDescriptor,
                                         config.fBackend);
                errorsForAllConfigs.add(kIntentionallySkipped_ErrorType);
                continue;
            }
        }
        if ((gmFlags & GM::kSkip565_Flag) &&
            (kRaster_Backend == config.fBackend) &&
            (kRGB_565_SkColorType == config.fColorType)) {
            gmmain.RecordSkippedTest(shortNamePlusConfig,
                                     renderModeDescriptor,
                                     config.fBackend);
            errorsForAllConfigs.add(kIntentionallySkipped_ErrorType);
            continue;
        }
        if (((gmFlags & GM::kSkipGPU_Flag) && kGPU_Backend == config.fBackend) ||
            ((gmFlags & GM::kGPUOnly_Flag) && kGPU_Backend != config.fBackend)) {
            gmmain.RecordSkippedTest(shortNamePlusConfig,
                                     renderModeDescriptor,
                                     config.fBackend);
            errorsForAllConfigs.add(kIntentionallySkipped_ErrorType);
            continue;
        }

        // Now we know that we want to run this test and record its
        // success or failure.
        ErrorCombination errorsForThisConfig;
        GrSurface* gpuTarget = NULL;
#if SK_SUPPORT_GPU
        SkAutoTUnref<GrSurface> auGpuTarget;
        if ((errorsForThisConfig.isEmpty()) && (kGPU_Backend == config.fBackend)) {
            if (FLAGS_resetGpuContext) {
                grFactory->destroyContexts();
            }
            GrContext* gr = grFactory->get(config.fGLContextType, gpuAPI);
            bool grSuccess = false;
            if (gr) {
                // create a render target to back the device
                GrSurfaceDesc desc;
                desc.fConfig = kSkia8888_GrPixelConfig;
                desc.fFlags = kRenderTarget_GrSurfaceFlag;
                desc.fWidth = gm->getISize().width();
                desc.fHeight = gm->getISize().height();
                desc.fSampleCnt = config.fSampleCnt;
                auGpuTarget.reset(gr->createUncachedTexture(desc, NULL, 0));
                if (auGpuTarget) {
                    gpuTarget = auGpuTarget;
                    grSuccess = true;
                    // Set the user specified cache limits if non-default.
                    size_t bytes;
                    int count;
                    gr->getResourceCacheLimits(&count, &bytes);
                    if (DEFAULT_CACHE_VALUE != gGpuCacheSizeBytes) {
                        bytes = static_cast<size_t>(gGpuCacheSizeBytes);
                    }
                    if (DEFAULT_CACHE_VALUE != gGpuCacheSizeCount) {
                        count = gGpuCacheSizeCount;
                    }
                    gr->setResourceCacheLimits(count, bytes);
                }
            }
            if (!grSuccess) {
                errorsForThisConfig.add(kNoGpuContext_ErrorType);
            }
        }
#endif

        SkBitmap comparisonBitmap;

        const char* writePath;
        if (FLAGS_writePath.count() == 1) {
            writePath = FLAGS_writePath[0];
        } else {
            writePath = NULL;
        }

        if (errorsForThisConfig.isEmpty()) {
            errorsForThisConfig.add(gmmain.test_drawing(gm, config, pdfRasterizers,
                                                        writePath, gpuTarget,
                                                        &comparisonBitmap));
            gmmain.RecordTestResults(errorsForThisConfig, shortNamePlusConfig, "");
        }

        // TODO: run only if gmmain.test_drawing succeeded.
        if (kRaster_Backend == config.fBackend) {
            run_multiple_modes(gmmain, gm, config, gpuTarget, comparisonBitmap, tileGridReplayScales);
        }

        if (FLAGS_deferred && errorsForThisConfig.isEmpty() &&
            (kGPU_Backend == config.fBackend || kRaster_Backend == config.fBackend)) {
            errorsForThisConfig.add(gmmain.test_deferred_drawing(gm, config, comparisonBitmap,
                                                                 gpuTarget));
        }

        if (FLAGS_mpd && (kGPU_Backend == config.fBackend || kRaster_Backend == config.fBackend)) {

            if (gmFlags & GM::kSkipPicture_Flag) {
                gmmain.RecordSkippedTest(shortNamePlusConfig,
                                         renderModeDescriptor,
                                         config.fBackend);
                errorsForThisConfig.add(kIntentionallySkipped_ErrorType);
            } else if (!(gmFlags & GM::kGPUOnly_Flag)) {
                errorsForThisConfig.add(gmmain.testMPDDrawing(gm, config,
                                                              writePath, gpuTarget,
                                                              comparisonBitmap));
            }
        }

        errorsForAllConfigs.add(errorsForThisConfig);
    }
    return errorsForAllConfigs;
}


/**
 * Read individual lines from a file, pushing them into the given array.
 *
 * @param filename path to the file to read
 * @param lines array of strings to add the lines to
 * @returns true if able to read lines from the file
 */
static bool read_lines_from_file(const char* filename, SkTArray<SkString> &lines) {
    SkAutoTUnref<SkStream> streamWrapper(SkStream::NewFromFile(filename));
    SkStream *stream = streamWrapper.get();
    if (!stream) {
        SkDebugf("unable to read file '%s'\n", filename);
        return false;
    }

    char c;
    SkString line;
    while (1 == stream->read(&c, 1)) {
        // If we hit either CR or LF, we've completed a line.
        //
        // TODO: If the file uses both CR and LF, this will return an extra blank
        // line for each line of the file.  Which is OK for current purposes...
        //
        // TODO: Does this properly handle unicode?  It doesn't matter for
        // current purposes...
        if ((c == 0x0d) || (c == 0x0a)) {
            lines.push_back(line);
            line.reset();
        } else {
            line.append(&c, 1);
        }
    }
    lines.push_back(line);
    return true;
}

/**
 * Return a list of all entries in an array of strings as a single string
 * of this form:
 * "item1", "item2", "item3"
 */
SkString list_all(const SkTArray<SkString> &stringArray);
SkString list_all(const SkTArray<SkString> &stringArray) {
    SkString total;
    for (int i = 0; i < stringArray.count(); i++) {
        if (i > 0) {
            total.append(", ");
        }
        total.append("\"");
        total.append(stringArray[i]);
        total.append("\"");
    }
    return total;
}

/**
 * Return a list of configuration names, as a single string of this form:
 * "item1", "item2", "item3"
 *
 * @param configs configurations, as a list of indices into gRec
 */
SkString list_all_config_names(const SkTDArray<size_t> &configs);
SkString list_all_config_names(const SkTDArray<size_t> &configs) {
    SkString total;
    for (int i = 0; i < configs.count(); i++) {
        if (i > 0) {
            total.append(", ");
        }
        total.append("\"");
        total.append(gRec[configs[i]].fName);
        total.append("\"");
    }
    return total;
}

static bool prepare_subdirectories(const char *root, bool useFileHierarchy,
                                   const SkTDArray<size_t> &configs,
                                   const SkTDArray<const PDFRasterizerData*>& pdfRasterizers) {
    if (!sk_mkdir(root)) {
        return false;
    }
    if (useFileHierarchy) {
        for (int i = 0; i < configs.count(); i++) {
            ConfigData config = gRec[configs[i]];
            SkString subdir;
            subdir.appendf("%s%c%s", root, SkPATH_SEPARATOR, config.fName);
            if (!sk_mkdir(subdir.c_str())) {
                return false;
            }

            if (config.fBackend == kPDF_Backend) {
                for (int j = 0; j < pdfRasterizers.count(); j++) {
                    SkString pdfSubdir = subdir;
                    pdfSubdir.appendf("-%s", pdfRasterizers[j]->fName);
                    if (!sk_mkdir(pdfSubdir.c_str())) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

static bool parse_flags_configs(SkTDArray<size_t>* outConfigs,
                         GrContextFactory* grFactory, GrGLStandard gpuAPI) {
    SkTDArray<size_t> excludeConfigs;

    for (int i = 0; i < FLAGS_config.count(); i++) {
        const char* config = FLAGS_config[i];
        bool exclude = false;
        if (*config == kExcludeConfigChar) {
            exclude = true;
            config += 1;
        }
        int index = findConfig(config);
        if (index >= 0) {
            if (exclude) {
                *excludeConfigs.append() = index;
            } else {
                appendUnique<size_t>(outConfigs, index);
            }
        } else if (0 == strcmp(kDefaultsConfigStr, config)) {
            if (exclude) {
                SkDebugf("%c%s is not allowed.\n",
                         kExcludeConfigChar, kDefaultsConfigStr);
                return false;
            }
            for (size_t c = 0; c < SK_ARRAY_COUNT(gRec); ++c) {
                if (gRec[c].fRunByDefault) {
                    appendUnique<size_t>(outConfigs, c);
                }
            }
        } else {
            SkDebugf("unrecognized config %s\n", config);
            return false;
        }
    }

    for (int i = 0; i < FLAGS_excludeConfig.count(); i++) {
        int index = findConfig(FLAGS_excludeConfig[i]);
        if (index >= 0) {
            *excludeConfigs.append() = index;
        } else {
            SkDebugf("unrecognized excludeConfig %s\n", FLAGS_excludeConfig[i]);
            return false;
        }
    }

    if (outConfigs->count() == 0) {
        // if no config is specified by user, add the defaults
        for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
            if (gRec[i].fRunByDefault) {
                *outConfigs->append() = i;
            }
        }
    }
    // now remove any explicitly excluded configs
    for (int i = 0; i < excludeConfigs.count(); ++i) {
        int index = outConfigs->find(excludeConfigs[i]);
        if (index >= 0) {
            outConfigs->remove(index);
            // now assert that there was only one copy in configs[]
            SkASSERT(outConfigs->find(excludeConfigs[i]) < 0);
        }
    }

    for (int i = 0; i < outConfigs->count(); ++i) {
        size_t index = (*outConfigs)[i];
        if (kGPU_Backend == gRec[index].fBackend) {
#if SK_SUPPORT_GPU
            if (!FLAGS_gpu) {
                outConfigs->remove(i);
                --i;
                continue;
            }
#endif
        } else if (!FLAGS_cpu) {
            outConfigs->remove(i);
            --i;
            continue;
        }
#if SK_SUPPORT_GPU
        SkASSERT(grFactory != NULL);
        if (kGPU_Backend == gRec[index].fBackend) {
            GrContext* ctx = grFactory->get(gRec[index].fGLContextType, gpuAPI);
            if (NULL == ctx) {
                SkDebugf("GrContext could not be created for config %s. Config will be skipped.\n",
                         gRec[index].fName);
                outConfigs->remove(i);
                --i;
                continue;
            }
            if (gRec[index].fSampleCnt > ctx->getMaxSampleCount()) {
                SkDebugf("Sample count (%d) of config %s is not supported."
                         " Config will be skipped.\n",
                         gRec[index].fSampleCnt, gRec[index].fName);
                outConfigs->remove(i);
                --i;
            }
        }
#endif
    }

    if (outConfigs->isEmpty()) {
        SkDebugf("No configs to run.");
        return false;
    }

    // now show the user the set of configs that will be run.
    SkString configStr("These configs will be run:");
    // show the user the config that will run.
    for (int i = 0; i < outConfigs->count(); ++i) {
        configStr.appendf(" %s", gRec[(*outConfigs)[i]].fName);
    }
    SkDebugf("%s\n", configStr.c_str());

    return true;
}

static bool parse_flags_pdf_rasterizers(const SkTDArray<size_t>& configs,
                                        SkTDArray<const PDFRasterizerData*>* outRasterizers) {
    // No need to run this check (and display the PDF rasterizers message)
    // if no PDF backends are in the configs.
    bool configHasPDF = false;
    for (int i = 0; i < configs.count(); i++) {
        if (gRec[configs[i]].fBackend == kPDF_Backend) {
            configHasPDF = true;
            break;
        }
    }
    if (!configHasPDF) {
        return true;
    }

    if (FLAGS_pdfRasterizers.count() == 1 &&
            !strcmp(FLAGS_pdfRasterizers[0], "default")) {
        for (int i = 0; i < (int)SK_ARRAY_COUNT(kPDFRasterizers); ++i) {
            if (kPDFRasterizers[i].fRunByDefault) {
                *outRasterizers->append() = &kPDFRasterizers[i];
            }
        }
    } else {
        for (int i = 0; i < FLAGS_pdfRasterizers.count(); i++) {
            const char* rasterizer = FLAGS_pdfRasterizers[i];
            const PDFRasterizerData* rasterizerPtr =
                    findPDFRasterizer(rasterizer);
            if (rasterizerPtr == NULL) {
                SkDebugf("unrecognized rasterizer %s\n", rasterizer);
                return false;
            }
            appendUnique<const PDFRasterizerData*>(outRasterizers,
                                                   rasterizerPtr);
        }
    }

    // now show the user the set of configs that will be run.
    SkString configStr("These PDF rasterizers will be run:");
    // show the user the config that will run.
    for (int i = 0; i < outRasterizers->count(); ++i) {
        configStr.appendf(" %s", (*outRasterizers)[i]->fName);
    }
    SkDebugf("%s\n", configStr.c_str());

    return true;
}

static bool parse_flags_ignore_error_types(ErrorCombination* outErrorTypes) {
    if (FLAGS_ignoreErrorTypes.count() > 0) {
        *outErrorTypes = ErrorCombination();
        for (int i = 0; i < FLAGS_ignoreErrorTypes.count(); i++) {
            ErrorType type;
            const char *name = FLAGS_ignoreErrorTypes[i];
            if (!getErrorTypeByName(name, &type)) {
                SkDebugf("cannot find ErrorType with name '%s'\n", name);
                return false;
            } else {
                outErrorTypes->add(type);
            }
        }
    }
    return true;
}

/**
 * Replace contents of ignoreTestNames with a list of test names, indicating
 * which tests' failures should be ignored.
 */
static bool parse_flags_ignore_tests(SkTArray<SkString> &ignoreTestNames) {
    ignoreTestNames.reset();

    // Parse --ignoreFailuresFile
    for (int i = 0; i < FLAGS_ignoreFailuresFile.count(); i++) {
        SkTArray<SkString> linesFromFile;
        if (!read_lines_from_file(FLAGS_ignoreFailuresFile[i], linesFromFile)) {
            return false;
        } else {
            for (int j = 0; j < linesFromFile.count(); j++) {
                SkString thisLine = linesFromFile[j];
                if (thisLine.isEmpty() || thisLine.startsWith('#')) {
                    // skip this line
                } else {
                    ignoreTestNames.push_back(thisLine);
                }
            }
        }
    }

    return true;
}

static bool parse_flags_modulo(int* moduloRemainder, int* moduloDivisor) {
    if (FLAGS_modulo.count() == 2) {
        *moduloRemainder = atoi(FLAGS_modulo[0]);
        *moduloDivisor = atoi(FLAGS_modulo[1]);
        if (*moduloRemainder < 0 || *moduloDivisor <= 0 ||
                *moduloRemainder >= *moduloDivisor) {
            SkDebugf("invalid modulo values.");
            return false;
        }
    }
    return true;
}

#if SK_SUPPORT_GPU
static bool parse_flags_gpu_cache(int* sizeBytes, int* sizeCount) {
    if (FLAGS_gpuCacheSize.count() > 0) {
        if (FLAGS_gpuCacheSize.count() != 2) {
            SkDebugf("--gpuCacheSize requires two arguments\n");
            return false;
        }
        *sizeBytes = atoi(FLAGS_gpuCacheSize[0]);
        *sizeCount = atoi(FLAGS_gpuCacheSize[1]);
    } else {
        *sizeBytes = DEFAULT_CACHE_VALUE;
        *sizeCount = DEFAULT_CACHE_VALUE;
    }
    return true;
}

static bool parse_flags_gl_standard(GrGLStandard* gpuAPI) {
    if (0 == FLAGS_gpuAPI.count()) {
        *gpuAPI = kNone_GrGLStandard;
        return true;
    }
    if (1 == FLAGS_gpuAPI.count()) {
        if (FLAGS_gpuAPI.contains(kGpuAPINameGL)) {
            *gpuAPI = kGL_GrGLStandard;
            return true;
        }
        if (FLAGS_gpuAPI.contains(kGpuAPINameGLES)) {
            *gpuAPI = kGLES_GrGLStandard;
            return true;
        }
    }
    SkDebugf("--gpuAPI invalid api value");
    return false;
}
#endif

static bool parse_flags_tile_grid_replay_scales(SkTDArray<SkScalar>* outScales) {
    *outScales->append() = SK_Scalar1; // By default only test at scale 1.0
    if (FLAGS_tileGridReplayScales.count() > 0) {
        outScales->reset();
        for (int i = 0; i < FLAGS_tileGridReplayScales.count(); i++) {
            double val = atof(FLAGS_tileGridReplayScales[i]);
            if (0 < val) {
                *outScales->append() = SkDoubleToScalar(val);
            }
        }
        if (0 == outScales->count()) {
            // Should have at least one scale
            SkDebugf("--tileGridReplayScales requires at least one scale.\n");
            return false;
        }
    }
    return true;
}

static bool parse_flags_gmmain_paths(GMMain* gmmain) {
    gmmain->fUseFileHierarchy = FLAGS_hierarchy;
    gmmain->fWriteChecksumBasedFilenames = FLAGS_writeChecksumBasedFilenames;

    if (FLAGS_mismatchPath.count() == 1) {
        gmmain->fMismatchPath = FLAGS_mismatchPath[0];
    }

    if (FLAGS_missingExpectationsPath.count() == 1) {
        gmmain->fMissingExpectationsPath = FLAGS_missingExpectationsPath[0];
    }

    if (FLAGS_readPath.count() == 1) {
        const char* readPath = FLAGS_readPath[0];
        if (!sk_exists(readPath)) {
            SkDebugf("readPath %s does not exist!\n", readPath);
            return false;
        }
        if (sk_isdir(readPath)) {
            if (FLAGS_verbose) {
                SkDebugf("reading from %s\n", readPath);
            }
            gmmain->fExpectationsSource.reset(SkNEW_ARGS(
                IndividualImageExpectationsSource, (readPath)));
        } else {
            if (FLAGS_verbose) {
                SkDebugf("reading expectations from JSON summary file %s\n", readPath);
            }
            gmmain->fExpectationsSource.reset(SkNEW_ARGS(JsonExpectationsSource, (readPath)));
        }
    }
    return true;
}

static bool parse_flags_jpeg_quality() {
    if (FLAGS_pdfJpegQuality < -1 || FLAGS_pdfJpegQuality > 100) {
        SkDebugf("%s\n", "pdfJpegQuality must be in [-1 .. 100] range.");
        return false;
    }
    return true;
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SetupCrashHandler();

    SkString usage;
    usage.printf("Run the golden master tests.\n");
    SkCommandLineFlags::SetUsage(usage.c_str());
    SkCommandLineFlags::Parse(argc, argv);

#if SK_ENABLE_INST_COUNT
    if (FLAGS_leaks) {
        gPrintInstCount = true;
    }
#endif

    SkAutoGraphics ag;

    setSystemPreferences();
    GMMain gmmain;

    SkTDArray<size_t> configs;

    int moduloRemainder = -1;
    int moduloDivisor = -1;
    SkTDArray<const PDFRasterizerData*> pdfRasterizers;
    SkTDArray<SkScalar> tileGridReplayScales;
#if SK_SUPPORT_GPU
    GrGLStandard gpuAPI = kNone_GrGLStandard;
    GrContext::Options grContextOpts;
    grContextOpts.fDrawPathToCompressedTexture = FLAGS_gpuCompressAlphaMasks;
    GrContextFactory* grFactory = new GrContextFactory(grContextOpts);
#else
    GrGLStandard gpuAPI = 0;
    GrContextFactory* grFactory = NULL;
#endif

    if (FLAGS_dryRun) {
        SkDebugf( "Doing a dry run; no tests will actually be executed.\n");
    }

    if (!parse_flags_modulo(&moduloRemainder, &moduloDivisor) ||
        !parse_flags_ignore_error_types(&gmmain.fIgnorableErrorTypes) ||
        !parse_flags_ignore_tests(gmmain.fIgnorableTestNames) ||
#if SK_SUPPORT_GPU
        !parse_flags_gpu_cache(&gGpuCacheSizeBytes, &gGpuCacheSizeCount) ||
        !parse_flags_gl_standard(&gpuAPI) ||
#endif
        !parse_flags_tile_grid_replay_scales(&tileGridReplayScales) ||
        !parse_flags_jpeg_quality() ||
        !parse_flags_configs(&configs, grFactory, gpuAPI) ||
        !parse_flags_pdf_rasterizers(configs, &pdfRasterizers) ||
        !parse_flags_gmmain_paths(&gmmain)) {
        return -1;
    }

    if (FLAGS_verbose) {
        if (FLAGS_writePath.count() == 1) {
            SkDebugf("writing to %s\n", FLAGS_writePath[0]);
        }
        if (gmmain.fMismatchPath) {
            SkDebugf("writing mismatches to %s\n", gmmain.fMismatchPath);
        }
        if (gmmain.fMissingExpectationsPath) {
            SkDebugf("writing images without expectations to %s\n",
                     gmmain.fMissingExpectationsPath);
        }
        if (FLAGS_writePicturePath.count() == 1) {
            SkDebugf("writing pictures to %s\n", FLAGS_writePicturePath[0]);
        }
        if (!GetResourcePath().isEmpty()) {
            SkDebugf("reading resources from %s\n", GetResourcePath().c_str());
        }
    }

    int gmsRun = 0;
    int gmIndex = -1;
    SkString moduloStr;

    if (!FLAGS_dryRun) {
        // If we will be writing out files, prepare subdirectories.
        if (FLAGS_writePath.count() == 1) {
            if (!prepare_subdirectories(FLAGS_writePath[0], gmmain.fUseFileHierarchy,
                                        configs, pdfRasterizers)) {
                return -1;
            }
        }
        if (gmmain.fMismatchPath) {
            if (!prepare_subdirectories(gmmain.fMismatchPath, gmmain.fUseFileHierarchy,
                                        configs, pdfRasterizers)) {
                return -1;
            }
        }
        if (gmmain.fMissingExpectationsPath) {
            if (!prepare_subdirectories(gmmain.fMissingExpectationsPath, gmmain.fUseFileHierarchy,
                                        configs, pdfRasterizers)) {
                return -1;
            }
        }
    }
    Iter iter;
    GM* gm;
    while ((gm = iter.next()) != NULL) {
        if (FLAGS_forcePerspectiveMatrix) {
            SkMatrix perspective;
            perspective.setIdentity();
            perspective.setPerspY(SkScalarDiv(SK_Scalar1, SkIntToScalar(1000)));
            perspective.setSkewX(SkScalarDiv(SkIntToScalar(8),
                                 SkIntToScalar(25)));

            gm->setStarterMatrix(perspective);
        }
        SkAutoTDelete<GM> adgm(gm);
        ++gmIndex;
        if (moduloRemainder >= 0) {
            if ((gmIndex % moduloDivisor) != moduloRemainder) {
                continue;
            }
            moduloStr.printf("[%d.%d] ", gmIndex, moduloDivisor);
        }

        const char* shortName = gm->getName();

        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, shortName)) {
            continue;
        }

        gmsRun++;
        SkISize size = gm->getISize();
        SkDebugf("%4dM %sdrawing... %s [%d %d]\n",
                 sk_tools::getMaxResidentSetSizeMB(), moduloStr.c_str(), shortName,
                 size.width(), size.height());
        if (!FLAGS_dryRun)
            run_multiple_configs(gmmain, gm, configs, pdfRasterizers, tileGridReplayScales,
                                 grFactory, gpuAPI);
    }

    if (FLAGS_dryRun)
        return 0;

    SkTArray<SkString> modes;
    gmmain.GetRenderModesEncountered(modes);
    int modeCount = modes.count();

    // Now that we have run all the tests and thus know the full set of renderModes that we
    // tried to run, we can call RecordTestResults() to record the cases in which we skipped
    // ALL renderModes.
    // See http://skbug.com/1994 and https://codereview.chromium.org/129203002/
    int testCount = gmmain.fTestsSkippedOnAllRenderModes.count();
    for (int testNum = 0; testNum < testCount; ++testNum) {
        const SkString &shortNamePlusConfig = gmmain.fTestsSkippedOnAllRenderModes[testNum];
        for (int modeNum = 0; modeNum < modeCount; ++modeNum) {
            gmmain.RecordTestResults(kIntentionallySkipped_ErrorType, shortNamePlusConfig,
                                     modes[modeNum].c_str());
        }
    }

    bool reportError = false;
    if (gmmain.NumSignificantErrors() > 0) {
        reportError = true;
    }

    // We test every GM against every config, and for every raster config also test every mode.
    int rasterConfigs = 0;
    for (int i = 0; i < configs.count(); i++) {
        if (gRec[configs[i]].fBackend == kRaster_Backend) {
            rasterConfigs++;
        }
    }
    // For raster configs, we run all renderModes; for non-raster configs, just default renderMode
    const int expectedNumberOfTests = rasterConfigs * gmsRun * modeCount
                                    + (configs.count() - rasterConfigs) * gmsRun;

    // Output summary to stdout.
    if (FLAGS_verbose) {
        SkDebugf("Ran %d GMs\n", gmsRun);
        SkDebugf("... over %2d configs [%s]\n", configs.count(),
                 list_all_config_names(configs).c_str());
        SkDebugf("...  and %2d modes   [%s]\n", modeCount, list_all(modes).c_str());
        SkDebugf("... so there should be a total of %d tests.\n", expectedNumberOfTests);
    }
    gmmain.ListErrors(FLAGS_verbose);

    // TODO(epoger): Enable this check for Android, too, once we resolve
    // https://code.google.com/p/skia/issues/detail?id=1222
    // ('GM is unexpectedly skipping tests on Android')
#ifndef SK_BUILD_FOR_ANDROID
    if (expectedNumberOfTests != gmmain.fTestsRun) {
        SkDebugf("expected %d tests, but ran or skipped %d tests\n",
                 expectedNumberOfTests, gmmain.fTestsRun);
        reportError = true;
    }
#endif

    if (FLAGS_writeJsonSummaryPath.count() == 1) {
        Json::Value root = CreateJsonTree(
            gmmain.fJsonExpectedResults,
            gmmain.fJsonActualResults_Failed, gmmain.fJsonActualResults_FailureIgnored,
            gmmain.fJsonActualResults_NoComparison, gmmain.fJsonActualResults_Succeeded);
        std::string jsonStdString = root.toStyledString();
        SkFILEWStream stream(FLAGS_writeJsonSummaryPath[0]);
        stream.write(jsonStdString.c_str(), jsonStdString.length());
    }

#if SK_SUPPORT_GPU

#if GR_CACHE_STATS
    for (int i = 0; i < configs.count(); i++) {
        ConfigData config = gRec[configs[i]];

        if (FLAGS_verbose && (kGPU_Backend == config.fBackend)) {
            GrContext* gr = grFactory->get(config.fGLContextType, gpuAPI);

            SkDebugf("config: %s %x\n", config.fName, gr);
            gr->printCacheStats();
        }
    }
#endif

#if GR_DUMP_FONT_CACHE
    for (int i = 0; i < configs.count(); i++) {
        ConfigData config = gRec[configs[i]];

        if (kGPU_Backend == config.fBackend) {
            GrContext* gr = grFactory->get(config.fGLContextType, gpuAPI);

           gr->dumpFontCache();
        }
    }
#endif

    delete grFactory;
#endif

    return (reportError) ? -1 : 0;
}

void GMMain::InstallFilter(SkCanvas* canvas) {
    if (FLAGS_forceBWtext) {
        canvas->setDrawFilter(SkNEW(BWTextDrawFilter))->unref();
    }
}

#if !defined(SK_BUILD_FOR_IOS) && !defined(SK_BUILD_FOR_NACL)
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
