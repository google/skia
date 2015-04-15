/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DMSrcSink.h"
#include "SamplePipeControllers.h"
#include "SkCommonFlags.h"
#include "SkCodec.h"
#include "SkData.h"
#include "SkDocument.h"
#include "SkError.h"
#include "SkImageGenerator.h"
#include "SkMultiPictureDraw.h"
#include "SkNullCanvas.h"
#include "SkOSFile.h"
#include "SkPictureData.h"
#include "SkPictureRecorder.h"
#include "SkRandom.h"
#include "SkScanlineDecoder.h"
#include "SkSVGCanvas.h"
#include "SkStream.h"
#include "SkXMLWriter.h"

DEFINE_bool(multiPage, false, "For document-type backends, render the source"
            " into multiple pages");

static bool lazy_decode_bitmap(const void* src, size_t size, SkBitmap* dst) {
    SkAutoTUnref<SkData> encoded(SkData::NewWithCopy(src, size));
    return encoded && SkInstallDiscardablePixelRef(encoded, dst);
}

namespace DM {

GMSrc::GMSrc(skiagm::GMRegistry::Factory factory) : fFactory(factory) {}

Error GMSrc::draw(SkCanvas* canvas) const {
    SkAutoTDelete<skiagm::GM> gm(fFactory(NULL));
    canvas->concat(gm->getInitialTransform());
    gm->draw(canvas);
    return "";
}

SkISize GMSrc::size() const {
    SkAutoTDelete<skiagm::GM> gm(fFactory(NULL));
    return gm->getISize();
}

Name GMSrc::name() const {
    SkAutoTDelete<skiagm::GM> gm(fFactory(NULL));
    return gm->getName();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

CodecSrc::CodecSrc(Path path, Mode mode, DstColorType dstColorType)
    : fPath(path)
    , fMode(mode)
    , fDstColorType(dstColorType)
{}

Error CodecSrc::draw(SkCanvas* canvas) const {
    SkImageInfo canvasInfo;
    if (NULL == canvas->peekPixels(&canvasInfo, NULL)) {
        // TODO: Once we implement GPU paths (e.g. JPEG YUV), we should use a deferred decode to
        // let the GPU handle it.
        return Error::Nonfatal("No need to test decoding to non-raster backend.");
    }

    SkAutoTUnref<SkData> encoded(SkData::NewFromFileName(fPath.c_str()));
    if (!encoded) {
        return SkStringPrintf("Couldn't read %s.", fPath.c_str());
    }
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(encoded));
    if (NULL == codec.get()) {
        return SkStringPrintf("Couldn't create codec for %s.", fPath.c_str());
    }

    // Choose the color type to decode to
    SkImageInfo decodeInfo = codec->getInfo();
    SkColorType canvasColorType = canvasInfo.colorType();
    switch (fDstColorType) {
        case kIndex8_Always_DstColorType:
            decodeInfo = codec->getInfo().makeColorType(kIndex_8_SkColorType);
            if (kRGB_565_SkColorType == canvasColorType) {
                return Error::Nonfatal("Testing non-565 to 565 is uninteresting.");
            }
            break;
        case kGrayscale_Always_DstColorType:
            decodeInfo = codec->getInfo().makeColorType(kGray_8_SkColorType);
            if (kRGB_565_SkColorType == canvasColorType) {
                return Error::Nonfatal("Testing non-565 to 565 is uninteresting.");
            }
            break;
        default:
            decodeInfo = decodeInfo.makeColorType(canvasColorType);
            break;
    }

    // Construct a color table for the decode if necessary
    SkAutoTUnref<SkColorTable> colorTable(NULL);
    SkPMColor* colorPtr = NULL;
    int* colorCountPtr = NULL;
    int maxColors = 256;
    if (kIndex_8_SkColorType == decodeInfo.colorType()) {
        SkPMColor colors[256];
        colorTable.reset(SkNEW_ARGS(SkColorTable, (colors, maxColors)));
        colorPtr = const_cast<SkPMColor*>(colorTable->readColors());
        colorCountPtr = &maxColors;
    }

    // FIXME: Currently we cannot draw unpremultiplied sources.
    if (decodeInfo.alphaType() == kUnpremul_SkAlphaType) {
        decodeInfo = decodeInfo.makeAlphaType(kPremul_SkAlphaType);
    }

    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(decodeInfo, NULL, colorTable.get())) {
        return SkStringPrintf("Image(%s) is too large (%d x %d)\n", fPath.c_str(),
                              decodeInfo.width(), decodeInfo.height());
    }

    switch (fMode) {
        case kNormal_Mode:
            switch (codec->getPixels(decodeInfo, bitmap.getPixels(), bitmap.rowBytes(), NULL,
                    colorPtr, colorCountPtr)) {
                case SkImageGenerator::kSuccess:
                    // We consider incomplete to be valid, since we should still decode what is
                    // available.
                case SkImageGenerator::kIncompleteInput:
                    break;
                case SkImageGenerator::kInvalidConversion:
                    return Error::Nonfatal("Incompatible colortype conversion");
                default:
                    // Everything else is considered a failure.
                    return SkStringPrintf("Couldn't getPixels %s.", fPath.c_str());
            }
            break;
        case kScanline_Mode: {
            SkScanlineDecoder* scanlineDecoder = codec->getScanlineDecoder(decodeInfo);
            if (NULL == scanlineDecoder) {
                return Error::Nonfatal("Cannot use scanline decoder for all images");
            }
            for (int y = 0; y < decodeInfo.height(); ++y) {
                const SkImageGenerator::Result result = scanlineDecoder->getScanlines(
                        bitmap.getAddr(0, y), 1, 0);
                switch (result) {
                    case SkImageGenerator::kSuccess:
                    case SkImageGenerator::kIncompleteInput:
                        break;
                    default:
                        return SkStringPrintf("%s failed after %d scanlines with error message %d",
                                              fPath.c_str(), y-1, (int) result);
                }
            }
            break;
        }
    }
    canvas->drawBitmap(bitmap, 0, 0);
    return "";
}

SkISize CodecSrc::size() const {
    SkAutoTUnref<SkData> encoded(SkData::NewFromFileName(fPath.c_str()));
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(encoded));
    if (NULL != codec) {
        return codec->getInfo().dimensions();
    } else {
        return SkISize::Make(0, 0);
    }
}

Name CodecSrc::name() const {
    return SkOSPath::Basename(fPath.c_str());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ImageSrc::ImageSrc(Path path, int divisor) : fPath(path), fDivisor(divisor) {}

Error ImageSrc::draw(SkCanvas* canvas) const {
    SkImageInfo canvasInfo;
    if (NULL == canvas->peekPixels(&canvasInfo, NULL)) {
        // TODO: Instead, use lazy decoding to allow the GPU to handle cases like YUV.
        return Error::Nonfatal("No need to test decoding to non-raster backend.");
    }

    SkAutoTUnref<SkData> encoded(SkData::NewFromFileName(fPath.c_str()));
    if (!encoded) {
        return SkStringPrintf("Couldn't read %s.", fPath.c_str());
    }
    const SkColorType dstColorType = canvasInfo.colorType();
    if (fDivisor == 0) {
        // Decode the full image.
        SkBitmap bitmap;
        if (!SkImageDecoder::DecodeMemory(encoded->data(), encoded->size(), &bitmap,
                                          dstColorType, SkImageDecoder::kDecodePixels_Mode)) {
            return SkStringPrintf("Couldn't decode %s.", fPath.c_str());
        }
        if (kRGB_565_SkColorType == dstColorType && !bitmap.isOpaque()) {
            // Do not draw a bitmap with alpha to a destination without alpha.
            return Error::Nonfatal("Uninteresting to decode image with alpha into 565.");
        }
        encoded.reset((SkData*)NULL);  // Might as well drop this when we're done with it.
        canvas->drawBitmap(bitmap, 0,0);
        return "";
    }
    // Decode subsets.  This is a little involved.
    SkAutoTDelete<SkMemoryStream> stream(new SkMemoryStream(encoded));
    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(stream.get()));
    if (!decoder) {
        return SkStringPrintf("Can't find a good decoder for %s.", fPath.c_str());
    }
    stream->rewind();
    int w,h;
    if (!decoder->buildTileIndex(stream.detach(), &w, &h) || w*h == 1) {
        return Error::Nonfatal("Subset decoding not supported.");
    }

    // Divide the image into subsets that cover the entire image.
    if (fDivisor > w || fDivisor > h) {
        return SkStringPrintf("divisor %d is too big for %s with dimensions (%d x %d)",
                              fDivisor, fPath.c_str(), w, h);
    }
    const int subsetWidth  = w / fDivisor,
              subsetHeight = h / fDivisor;
    for (int y = 0; y < h; y += subsetHeight) {
        for (int x = 0; x < w; x += subsetWidth) {
            SkBitmap subset;
            SkIRect rect = SkIRect::MakeXYWH(x, y, subsetWidth, subsetHeight);
            if (!decoder->decodeSubset(&subset, rect, dstColorType)) {
                return SkStringPrintf("Could not decode subset (%d, %d, %d, %d).",
                                      x, y, x+subsetWidth, y+subsetHeight);
            }
            if (kRGB_565_SkColorType == dstColorType && !subset.isOpaque()) {
                // Do not draw a bitmap with alpha to a destination without alpha.
                // This is not an error, but there is nothing interesting to show.

                // This should only happen on the first iteration through the loop.
                SkASSERT(0 == x && 0 == y);

                return Error::Nonfatal("Uninteresting to decode image with alpha into 565.");
            }
            canvas->drawBitmap(subset, SkIntToScalar(x), SkIntToScalar(y));
        }
    }
    return "";
}

SkISize ImageSrc::size() const {
    SkAutoTUnref<SkData> encoded(SkData::NewFromFileName(fPath.c_str()));
    SkBitmap bitmap;
    if (!encoded || !SkImageDecoder::DecodeMemory(encoded->data(),
                                                  encoded->size(),
                                                  &bitmap,
                                                  kUnknown_SkColorType,
                                                  SkImageDecoder::kDecodeBounds_Mode)) {
        return SkISize::Make(0,0);
    }
    return bitmap.dimensions();
}

Name ImageSrc::name() const {
    return SkOSPath::Basename(fPath.c_str());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static const SkRect kSKPViewport = {0,0, 1000,1000};

SKPSrc::SKPSrc(Path path) : fPath(path) {}

Error SKPSrc::draw(SkCanvas* canvas) const {
    SkAutoTDelete<SkStream> stream(SkStream::NewFromFile(fPath.c_str()));
    if (!stream) {
        return SkStringPrintf("Couldn't read %s.", fPath.c_str());
    }
    SkAutoTUnref<SkPicture> pic(SkPicture::CreateFromStream(stream, &lazy_decode_bitmap));
    if (!pic) {
        return SkStringPrintf("Couldn't decode %s as a picture.", fPath.c_str());
    }
    stream.reset((SkStream*)NULL);  // Might as well drop this when we're done with it.

    canvas->clipRect(kSKPViewport);
    // Testing TextBlob batching requires that we see individual text blobs more than once
    // TODO remove this and add a flag to DM so we can run skps multiple times
//#define DOUBLE_LOOP
#ifdef DOUBLE_LOOP
    {
        SkAutoCanvasRestore acr(canvas, true);
#endif
        canvas->drawPicture(pic);
#ifdef DOUBLE_LOOP
    }
    canvas->clear(0);
    canvas->drawPicture(pic);
#endif
    return "";
}

SkISize SKPSrc::size() const {
    SkAutoTDelete<SkStream> stream(SkStream::NewFromFile(fPath.c_str()));
    if (!stream) {
        return SkISize::Make(0,0);
    }
    SkPictInfo info;
    if (!SkPicture::InternalOnly_StreamIsSKP(stream, &info)) {
        return SkISize::Make(0,0);
    }
    SkRect viewport = kSKPViewport;
    if (!viewport.intersect(info.fCullRect)) {
        return SkISize::Make(0,0);
    }
    return viewport.roundOut().size();
}

Name SKPSrc::name() const { return SkOSPath::Basename(fPath.c_str()); }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

Error NullSink::draw(const Src& src, SkBitmap*, SkWStream*, SkString*) const {
    SkAutoTDelete<SkCanvas> canvas(SkCreateNullCanvas());
    return src.draw(canvas);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

DEFINE_bool(gpuStats, false, "Append GPU stats to the log for each GPU task?");

GPUSink::GPUSink(GrContextFactory::GLContextType ct,
                 GrGLStandard api,
                 int samples,
                 bool dfText,
                 bool threaded)
    : fContextType(ct)
    , fGpuAPI(api)
    , fSampleCount(samples)
    , fUseDFText(dfText)
    , fThreaded(threaded) {}

int GPUSink::enclave() const {
    return fThreaded ? kAnyThread_Enclave : kGPU_Enclave;
}

void PreAbandonGpuContextErrorHandler(SkError, void*) {}

Error GPUSink::draw(const Src& src, SkBitmap* dst, SkWStream*, SkString* log) const {
    GrContextFactory factory;
    const SkISize size = src.size();
    const SkImageInfo info =
        SkImageInfo::Make(size.width(), size.height(), kN32_SkColorType, kPremul_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(
            NewGpuSurface(&factory, fContextType, fGpuAPI, info, fSampleCount, fUseDFText));
    if (!surface) {
        return "Could not create a surface.";
    }
    if (FLAGS_preAbandonGpuContext) {
        SkSetErrorCallback(&PreAbandonGpuContextErrorHandler, NULL);
        factory.abandonContexts();
    }
    SkCanvas* canvas = surface->getCanvas();
    Error err = src.draw(canvas);
    if (!err.isEmpty()) {
        return err;
    }
    canvas->flush();
    if (FLAGS_gpuStats) {
        canvas->getGrContext()->dumpCacheStats(log);
        canvas->getGrContext()->dumpGpuStats(log);
    }
    dst->allocPixels(info);
    canvas->readPixels(dst, 0, 0);
    if (FLAGS_abandonGpuContext) {
        factory.abandonContexts();
    }
    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static Error draw_skdocument(const Src& src, SkDocument* doc, SkWStream* dst) {
    // Print the given DM:Src to a document, breaking on 8.5x11 pages.
    SkASSERT(doc);
    int width  = src.size().width(),
        height = src.size().height();

    if (FLAGS_multiPage) {
        const int kLetterWidth = 612,  // 8.5 * 72
                kLetterHeight = 792;   // 11 * 72
        const SkRect letter = SkRect::MakeWH(SkIntToScalar(kLetterWidth),
                                             SkIntToScalar(kLetterHeight));

        int xPages = ((width - 1) / kLetterWidth) + 1;
        int yPages = ((height - 1) / kLetterHeight) + 1;

        for (int y = 0; y < yPages; ++y) {
            for (int x = 0; x < xPages; ++x) {
                int w = SkTMin(kLetterWidth, width - (x * kLetterWidth));
                int h = SkTMin(kLetterHeight, height - (y * kLetterHeight));
                SkCanvas* canvas =
                        doc->beginPage(SkIntToScalar(w), SkIntToScalar(h));
                if (!canvas) {
                    return "SkDocument::beginPage(w,h) returned NULL";
                }
                canvas->clipRect(letter);
                canvas->translate(-letter.width() * x, -letter.height() * y);
                Error err = src.draw(canvas);
                if (!err.isEmpty()) {
                    return err;
                }
                doc->endPage();
            }
        }
    } else {
        SkCanvas* canvas =
                doc->beginPage(SkIntToScalar(width), SkIntToScalar(height));
        if (!canvas) {
            return "SkDocument::beginPage(w,h) returned NULL";
        }
        Error err = src.draw(canvas);
        if (!err.isEmpty()) {
            return err;
        }
        doc->endPage();
    }
    if (!doc->close()) {
        return "SkDocument::close() returned false";
    }
    dst->flush();
    return "";
}

PDFSink::PDFSink() {}

Error PDFSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
    SkAutoTUnref<SkDocument> doc(SkDocument::CreatePDF(dst));
    if (!doc) {
        return "SkDocument::CreatePDF() returned NULL";
    }
    return draw_skdocument(src, doc.get(), dst);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

XPSSink::XPSSink() {}

Error XPSSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
    SkAutoTUnref<SkDocument> doc(SkDocument::CreateXPS(dst));
    if (!doc) {
        return "SkDocument::CreateXPS() returned NULL";
    }
    return draw_skdocument(src, doc.get(), dst);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

SKPSink::SKPSink() {}

Error SKPSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
    SkSize size;
    size = src.size();
    SkPictureRecorder recorder;
    Error err = src.draw(recorder.beginRecording(size.width(), size.height()));
    if (!err.isEmpty()) {
        return err;
    }
    SkAutoTUnref<SkPicture> pic(recorder.endRecording());
    pic->serialize(dst);
    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

SVGSink::SVGSink() {}

Error SVGSink::draw(const Src& src, SkBitmap*, SkWStream* dst, SkString*) const {
    SkAutoTDelete<SkXMLWriter> xmlWriter(SkNEW_ARGS(SkXMLStreamWriter, (dst)));
    SkAutoTUnref<SkCanvas> canvas(SkSVGCanvas::Create(
        SkRect::MakeWH(SkIntToScalar(src.size().width()), SkIntToScalar(src.size().height())),
        xmlWriter));
    return src.draw(canvas);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

RasterSink::RasterSink(SkColorType colorType) : fColorType(colorType) {}

Error RasterSink::draw(const Src& src, SkBitmap* dst, SkWStream*, SkString*) const {
    const SkISize size = src.size();
    // If there's an appropriate alpha type for this color type, use it, otherwise use premul.
    SkAlphaType alphaType = kPremul_SkAlphaType;
    (void)SkColorTypeValidateAlphaType(fColorType, alphaType, &alphaType);

    dst->allocPixels(SkImageInfo::Make(size.width(), size.height(), fColorType, alphaType));
    dst->eraseColor(SK_ColorTRANSPARENT);
    SkCanvas canvas(*dst);
    return src.draw(&canvas);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static SkISize auto_compute_translate(SkMatrix* matrix, int srcW, int srcH) {
    SkRect bounds = SkRect::MakeIWH(srcW, srcH);
    matrix->mapRect(&bounds);
    matrix->postTranslate(-bounds.x(), -bounds.y());
    return SkISize::Make(SkScalarRoundToInt(bounds.width()), SkScalarRoundToInt(bounds.height()));
}

ViaMatrix::ViaMatrix(SkMatrix matrix, Sink* sink) : fMatrix(matrix), fSink(sink) {}

Error ViaMatrix::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    // We turn our arguments into a Src, then draw that Src into our Sink to fill bitmap or stream.
    struct ProxySrc : public Src {
        const Src&  fSrc;
        SkMatrix    fMatrix;
        SkISize     fSize;

        ProxySrc(const Src& src, SkMatrix matrix) : fSrc(src), fMatrix(matrix) {
            fSize = auto_compute_translate(&fMatrix, src.size().width(), src.size().height());
        }

        Error draw(SkCanvas* canvas) const override {
            canvas->concat(fMatrix);
            return fSrc.draw(canvas);
        }
        SkISize size() const override { return fSize; }
        Name name() const override { sk_throw(); return ""; }  // No one should be calling this.
    } proxy(src, fMatrix);
    return fSink->draw(proxy, bitmap, stream, log);
}

// Undoes any flip or 90 degree rotate without changing the scale of the bitmap.
// This should be pixel-preserving.
ViaUpright::ViaUpright(SkMatrix matrix, Sink* sink) : fMatrix(matrix), fSink(sink) {}

Error ViaUpright::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    Error err = fSink->draw(src, bitmap, stream, log);
    if (!err.isEmpty()) {
        return err;
    }

    SkMatrix inverse;
    if (!fMatrix.rectStaysRect() || !fMatrix.invert(&inverse)) {
        return "Cannot upright --matrix.";
    }
    SkMatrix upright = SkMatrix::I();
    upright.setScaleX(SkScalarSignAsScalar(inverse.getScaleX()));
    upright.setScaleY(SkScalarSignAsScalar(inverse.getScaleY()));
    upright.setSkewX(SkScalarSignAsScalar(inverse.getSkewX()));
    upright.setSkewY(SkScalarSignAsScalar(inverse.getSkewY()));

    SkBitmap uprighted;
    SkISize size = auto_compute_translate(&upright, bitmap->width(), bitmap->height());
    uprighted.allocPixels(bitmap->info().makeWH(size.width(), size.height()));

    SkCanvas canvas(uprighted);
    canvas.concat(upright);
    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    canvas.drawBitmap(*bitmap, 0, 0, &paint);

    *bitmap = uprighted;
    bitmap->lockPixels();
    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ViaPipe::ViaPipe(Sink* sink) : fSink(sink) {}

Error ViaPipe::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    // We turn ourselves into another Src that draws our argument into bitmap/stream via pipe.
    struct ProxySrc : public Src {
        const Src& fSrc;
        ProxySrc(const Src& src) : fSrc(src) {}

        Error draw(SkCanvas* canvas) const override {
            SkISize size = this->size();
            PipeController controller(canvas, &SkImageDecoder::DecodeMemory);
            SkGPipeWriter pipe;
            const uint32_t kFlags = 0; // We mirror SkDeferredCanvas, which doesn't use any flags.
            return fSrc.draw(pipe.startRecording(&controller, kFlags, size.width(), size.height()));
        }
        SkISize size() const override { return fSrc.size(); }
        Name name() const override { sk_throw(); return ""; }  // No one should be calling this.
    } proxy(src);
    return fSink->draw(proxy, bitmap, stream, log);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ViaSerialization::ViaSerialization(Sink* sink) : fSink(sink) {}

Error ViaSerialization::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log)
    const {
    // Record our Src into a picture.
    SkSize size;
    size = src.size();
    SkPictureRecorder recorder;
    Error err = src.draw(recorder.beginRecording(size.width(), size.height()));
    if (!err.isEmpty()) {
        return err;
    }
    SkAutoTUnref<SkPicture> pic(recorder.endRecording());

    // Serialize it and then deserialize it.
    SkDynamicMemoryWStream wStream;
    pic->serialize(&wStream);
    SkAutoTDelete<SkStream> rStream(wStream.detachAsStream());
    SkAutoTUnref<SkPicture> deserialized(SkPicture::CreateFromStream(rStream, &lazy_decode_bitmap));

    // Turn that deserialized picture into a Src, draw it into our Sink to fill bitmap or stream.
    struct ProxySrc : public Src {
        const SkPicture* fPic;
        const SkISize fSize;
        ProxySrc(const SkPicture* pic, SkISize size) : fPic(pic), fSize(size) {}

        Error draw(SkCanvas* canvas) const override {
            canvas->drawPicture(fPic);
            return "";
        }
        SkISize size() const override { return fSize; }
        Name name() const override { sk_throw(); return ""; }  // No one should be calling this.
    } proxy(deserialized, src.size());
    return fSink->draw(proxy, bitmap, stream, log);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ViaTiles::ViaTiles(int w, int h, SkBBHFactory* factory, Sink* sink)
    : fW(w)
    , fH(h)
    , fFactory(factory)
    , fSink(sink) {}

Error ViaTiles::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    // Record our Src into a picture.
    SkSize size;
    size = src.size();
    SkPictureRecorder recorder;
    Error err = src.draw(recorder.beginRecording(size.width(), size.height(), fFactory.get()));
    if (!err.isEmpty()) {
        return err;
    }
    SkAutoTUnref<SkPicture> pic(recorder.endRecordingAsPicture());

    // Turn that picture into a Src that draws into our Sink via tiles + MPD.
    struct ProxySrc : public Src {
        const int fW, fH;
        const SkPicture* fPic;
        const SkISize fSize;
        ProxySrc(int w, int h, const SkPicture* pic, SkISize size)
            : fW(w), fH(h), fPic(pic), fSize(size) {}

        Error draw(SkCanvas* canvas) const override {
            const int xTiles = (fSize.width()  + fW - 1) / fW,
                      yTiles = (fSize.height() + fH - 1) / fH;
            SkMultiPictureDraw mpd(xTiles*yTiles);
            SkTDArray<SkSurface*> surfaces;
            surfaces.setReserve(xTiles*yTiles);

            SkImageInfo info = canvas->imageInfo().makeWH(fW, fH);
            for (int j = 0; j < yTiles; j++) {
                for (int i = 0; i < xTiles; i++) {
                    // This lets our ultimate Sink determine the best kind of surface.
                    // E.g., if it's a GpuSink, the surfaces and images are textures.
                    SkSurface* s = canvas->newSurface(info);
                    if (!s) {
                        s = SkSurface::NewRaster(info);  // Some canvases can't create surfaces.
                    }
                    surfaces.push(s);
                    SkCanvas* c = s->getCanvas();
                    c->translate(SkIntToScalar(-i * fW),
                                 SkIntToScalar(-j * fH));  // Line up the canvas with this tile.
                    mpd.add(c, fPic);
                }
            }
            mpd.draw();
            for (int j = 0; j < yTiles; j++) {
                for (int i = 0; i < xTiles; i++) {
                    SkAutoTUnref<SkImage> image(surfaces[i+xTiles*j]->newImageSnapshot());
                    canvas->drawImage(image, SkIntToScalar(i*fW), SkIntToScalar(j*fH));
                }
            }
            surfaces.unrefAll();
            return "";
        }
        SkISize size() const override { return fSize; }
        Name name() const override { sk_throw(); return ""; }  // No one should be calling this.
    } proxy(fW, fH, pic, src.size());
    return fSink->draw(proxy, bitmap, stream, log);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ViaSecondPicture::ViaSecondPicture(Sink* sink) : fSink(sink) {}

// Draw the Src into two pictures, then draw the second picture into the wrapped Sink.
// This tests that any shortcuts we may take while recording that second picture are legal.
Error ViaSecondPicture::draw(
        const Src& src, SkBitmap* bitmap, SkWStream* stream, SkString* log) const {
    struct ProxySrc : public Src {
        const Src& fSrc;
        ProxySrc(const Src& src) : fSrc(src) {}

        Error draw(SkCanvas* canvas) const override {
            SkSize size;
            size = fSrc.size();
            SkPictureRecorder recorder;
            SkAutoTUnref<SkPicture> pic;
            for (int i = 0; i < 2; i++) {
                Error err = fSrc.draw(recorder.beginRecording(size.width(), size.height()));
                if (!err.isEmpty()) {
                    return err;
                }
                pic.reset(recorder.endRecordingAsPicture());
            }
            canvas->drawPicture(pic);
            return "";
        }
        SkISize size() const override { return fSrc.size(); }
        Name name() const override { sk_throw(); return ""; }  // No one should be calling this.
    } proxy(src);
    return fSink->draw(proxy, bitmap, stream, log);
}

}  // namespace DM
