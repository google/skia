#include "DMSrcSink.h"
#include "SamplePipeControllers.h"
#include "SkCommonFlags.h"
#include "SkDocument.h"
#include "SkMultiPictureDraw.h"
#include "SkOSFile.h"
#include "SkPictureRecorder.h"
#include "SkRandom.h"
#include "SkSVGDevice.h"
#include "SkStream.h"

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

ImageSrc::ImageSrc(Path path, int divisor) : fPath(path), fDivisor(divisor) {}

Error ImageSrc::draw(SkCanvas* canvas) const {
    SkAutoTUnref<SkData> encoded(SkData::NewFromFileName(fPath.c_str()));
    if (!encoded) {
        return SkStringPrintf("Couldn't read %s.", fPath.c_str());
    }
    const SkColorType dstColorType = canvas->imageInfo().colorType();
    if (fDivisor == 0) {
        // Decode the full image.
        SkBitmap bitmap;
        if (!SkImageDecoder::DecodeMemory(encoded->data(), encoded->size(), &bitmap,
                                          dstColorType, SkImageDecoder::kDecodePixels_Mode)) {
            return SkStringPrintf("Couldn't decode %s.", fPath.c_str());
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
        return "";  // Not an error.  Subset decoding is not always supported.
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
    SkAutoTUnref<SkPicture> pic(SkPicture::CreateFromStream(stream));
    if (!pic) {
        return SkStringPrintf("Couldn't decode %s as a picture.", fPath.c_str());
    }
    stream.reset((SkStream*)NULL);  // Might as well drop this when we're done with it.
    canvas->clipRect(kSKPViewport);
    canvas->drawPicture(pic);
    return "";
}

SkISize SKPSrc::size() const {
    // This may be unnecessarily large.
    return kSKPViewport.roundOut().size();
}

Name SKPSrc::name() const { return SkOSPath::Basename(fPath.c_str()); }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

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

Error GPUSink::draw(const Src& src, SkBitmap* dst, SkWStream*) const {
    GrContextFactory factory;
    const SkISize size = src.size();
    const SkImageInfo info =
        SkImageInfo::Make(size.width(), size.height(), kN32_SkColorType, kPremul_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(
            NewGpuSurface(&factory, fContextType, fGpuAPI, info, fSampleCount, fUseDFText));
    if (!surface) {
        return "Could not create a surface.";
    }
    SkCanvas* canvas = surface->getCanvas();
    Error err = src.draw(canvas);
    if (!err.isEmpty()) {
        return err;
    }
    canvas->flush();
    dst->allocPixels(info);
    canvas->readPixels(dst, 0,0);
    if (FLAGS_abandonGpuContext) {
        factory.abandonContexts();
    }
    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

PDFSink::PDFSink() {}

Error PDFSink::draw(const Src& src, SkBitmap*, SkWStream* dst) const {
    // Print the given DM:Src to a PDF, breaking on 8.5x11 pages.
    SkAutoTUnref<SkDocument> doc(SkDocument::CreatePDF(dst));

    int width  = src.size().width(),
        height = src.size().height();

    const int kLetterWidth  = 612,  // 8.5 * 72
              kLetterHeight = 792;  // 11 * 72
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
            canvas->clipRect(letter);
            canvas->translate(-letter.width() * x, -letter.height() * y);
            Error err = src.draw(canvas);
            if (!err.isEmpty()) {
                return err;
            }
            doc->endPage();
        }
    }
    doc->close();
    dst->flush();
    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

SKPSink::SKPSink() {}

Error SKPSink::draw(const Src& src, SkBitmap*, SkWStream* dst) const {
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

Error SVGSink::draw(const Src& src, SkBitmap*, SkWStream* dst) const {
    SkAutoTUnref<SkBaseDevice> device(SkSVGDevice::Create(src.size(), dst));
    SkCanvas canvas(device);
    return src.draw(&canvas);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

RasterSink::RasterSink(SkColorType colorType) : fColorType(colorType) {}

Error RasterSink::draw(const Src& src, SkBitmap* dst, SkWStream*) const {
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

ViaMatrix::ViaMatrix(SkMatrix matrix, Sink* sink) : fMatrix(matrix), fSink(sink) {}

Error ViaMatrix::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream) const {
    // We turn our arguments into a Src, then draw that Src into our Sink to fill bitmap or stream.
    struct ProxySrc : public Src {
        const Src& fSrc;
        SkMatrix fMatrix;
        ProxySrc(const Src& src, SkMatrix matrix) : fSrc(src), fMatrix(matrix) {}

        Error draw(SkCanvas* canvas) const SK_OVERRIDE {
            canvas->concat(fMatrix);
            return fSrc.draw(canvas);
        }
        SkISize size() const SK_OVERRIDE { return fSrc.size(); }
        Name name() const SK_OVERRIDE { sk_throw(); return ""; }  // No one should be calling this.
    } proxy(src, fMatrix);
    return fSink->draw(proxy, bitmap, stream);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ViaPipe::ViaPipe(Sink* sink) : fSink(sink) {}

Error ViaPipe::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream) const {
    // We turn ourselves into another Src that draws our argument into bitmap/stream via pipe.
    struct ProxySrc : public Src {
        const Src& fSrc;
        ProxySrc(const Src& src) : fSrc(src) {}

        Error draw(SkCanvas* canvas) const SK_OVERRIDE {
            SkISize size = this->size();
            PipeController controller(canvas, &SkImageDecoder::DecodeMemory);
            SkGPipeWriter pipe;
            const uint32_t kFlags = 0; // We mirror SkDeferredCanvas, which doesn't use any flags.
            return fSrc.draw(pipe.startRecording(&controller, kFlags, size.width(), size.height()));
        }
        SkISize size() const SK_OVERRIDE { return fSrc.size(); }
        Name name() const SK_OVERRIDE { sk_throw(); return ""; }  // No one should be calling this.
    } proxy(src);
    return fSink->draw(proxy, bitmap, stream);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ViaSerialization::ViaSerialization(Sink* sink) : fSink(sink) {}

Error ViaSerialization::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream) const {
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
    SkAutoTUnref<SkPicture> deserialized(SkPicture::CreateFromStream(rStream));

    // Turn that deserialized picture into a Src, draw it into our Sink to fill bitmap or stream.
    struct ProxySrc : public Src {
        const SkPicture* fPic;
        const SkISize fSize;
        ProxySrc(const SkPicture* pic, SkISize size) : fPic(pic), fSize(size) {}

        Error draw(SkCanvas* canvas) const SK_OVERRIDE {
            canvas->drawPicture(fPic);
            return "";
        }
        SkISize size() const SK_OVERRIDE { return fSize; }
        Name name() const SK_OVERRIDE { sk_throw(); return ""; }  // No one should be calling this.
    } proxy(deserialized, src.size());
    return fSink->draw(proxy, bitmap, stream);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ViaTiles::ViaTiles(int w, int h, SkBBHFactory* factory, Sink* sink)
    : fW(w)
    , fH(h)
    , fFactory(factory)
    , fSink(sink) {}

Error ViaTiles::draw(const Src& src, SkBitmap* bitmap, SkWStream* stream) const {
    // Record our Src into a picture.
    SkSize size;
    size = src.size();
    SkPictureRecorder recorder;
    Error err = src.draw(recorder.beginRecording(size.width(), size.height(), fFactory.get()));
    if (!err.isEmpty()) {
        return err;
    }
    SkAutoTUnref<SkPicture> pic(recorder.endRecording());

    // Turn that picture into a Src that draws into our Sink via tiles + MPD.
    struct ProxySrc : public Src {
        const int fW, fH;
        const SkPicture* fPic;
        const SkISize fSize;
        ProxySrc(int w, int h, const SkPicture* pic, SkISize size)
            : fW(w), fH(h), fPic(pic), fSize(size) {}

        Error draw(SkCanvas* canvas) const SK_OVERRIDE {
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
        SkISize size() const SK_OVERRIDE { return fSize; }
        Name name() const SK_OVERRIDE { sk_throw(); return ""; }  // No one should be calling this.
    } proxy(fW, fH, pic, src.size());
    return fSink->draw(proxy, bitmap, stream);
}

}  // namespace DM
