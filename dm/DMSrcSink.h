/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DMSrcSink_DEFINED
#define DMSrcSink_DEFINED

#include "DMGpuSupport.h"
#include "SkBBHFactory.h"
#include "SkBBoxHierarchy.h"
#include "SkBitmap.h"
#include "SkBitmapRegionDecoder.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkPicture.h"
#include "gm.h"

namespace DM {

// This is just convenience.  It lets you use either return "foo" or return SkStringPrintf(...).
struct ImplicitString : public SkString {
    template <typename T>
    ImplicitString(const T& s) : SkString(s) {}
    ImplicitString() : SkString("") {}
};
typedef ImplicitString Name;
typedef ImplicitString Path;

class Error {
public:
    Error(const SkString& s) : fMsg(s), fFatal(!this->isEmpty()) {}
    Error(const char* s)     : fMsg(s), fFatal(!this->isEmpty()) {}

    Error(const Error&)            = default;
    Error& operator=(const Error&) = default;

    static Error Nonfatal(const SkString& s) { return Nonfatal(s.c_str()); }
    static Error Nonfatal(const char* s) {
        Error e(s);
        e.fFatal = false;
        return e;
    }

    const char* c_str() const { return fMsg.c_str(); }
    bool isEmpty() const { return fMsg.isEmpty(); }
    bool isFatal() const { return fFatal; }

private:
    SkString fMsg;
    bool     fFatal;
};

struct SinkFlags {
    enum { kNull, kGPU, kVector, kRaster } type;
    enum { kDirect, kIndirect } approach;
};

struct Src {
    virtual ~Src() {}
    virtual Error SK_WARN_UNUSED_RESULT draw(SkCanvas*) const = 0;
    virtual SkISize size() const = 0;
    virtual Name name() const = 0;
    virtual void modifyGrContextOptions(GrContextOptions* options) const {}
    virtual bool veto(SinkFlags) const { return false; }

    // Force Tasks using this Src to run on the main thread?
    virtual bool serial() const { return false; }
};

struct Sink {
    virtual ~Sink() {}
    // You may write to either the bitmap or stream.  If you write to log, we'll print that out.
    virtual Error SK_WARN_UNUSED_RESULT draw(const Src&, SkBitmap*, SkWStream*, SkString* log)
        const = 0;

    // Force Tasks using this Sink to run on the main thread?
    virtual bool serial() const { return false; }

    // File extension for the content draw() outputs, e.g. "png", "pdf".
    virtual const char* fileExtension() const  = 0;

    virtual SinkFlags flags() const = 0;
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

class GMSrc : public Src {
public:
    explicit GMSrc(skiagm::GMRegistry::Factory);

    Error draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    void modifyGrContextOptions(GrContextOptions* options) const override;

private:
    skiagm::GMRegistry::Factory fFactory;
};

class CodecSrc : public Src {
public:
    enum Mode {
        kCodec_Mode,
        // We choose to test only one mode with zero initialized memory.
        // This will exercise all of the interesting cases in SkSwizzler
        // without doubling the size of our test suite.
        kCodecZeroInit_Mode,
        kScanline_Mode,
        kStripe_Mode, // Tests the skipping of scanlines
        kCroppedScanline_Mode, // Tests (jpeg) cropped scanline optimization
        kSubset_Mode, // For codecs that support subsets directly.
    };
    enum DstColorType {
        kGetFromCanvas_DstColorType,
        kIndex8_Always_DstColorType,
        kGrayscale_Always_DstColorType,
        kNonNative8888_Always_DstColorType,
    };
    CodecSrc(Path, Mode, DstColorType, SkAlphaType, float);

    Error draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    bool veto(SinkFlags) const override;
    bool serial() const override { return fRunSerially; }
private:
    Path                    fPath;
    Mode                    fMode;
    DstColorType            fDstColorType;
    SkAlphaType             fDstAlphaType;
    float                   fScale;
    bool                    fRunSerially;
};

class AndroidCodecSrc : public Src {
public:
    enum Mode {
        kFullImage_Mode,
        // Splits the image into multiple subsets using a divisor and decodes the subsets
        // separately.
        kDivisor_Mode,
    };

    AndroidCodecSrc(Path, Mode, CodecSrc::DstColorType, SkAlphaType, int sampleSize);

    Error draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    bool veto(SinkFlags) const override;
    bool serial() const override { return fRunSerially; }
private:
    Path                    fPath;
    Mode                    fMode;
    CodecSrc::DstColorType  fDstColorType;
    SkAlphaType             fDstAlphaType;
    int                     fSampleSize;
    bool                    fRunSerially;
};

// Allows for testing of various implementations of Android's BitmapRegionDecoder
class BRDSrc : public Src {
public:
    enum Mode {
        // Decode the entire image as one region.
        kFullImage_Mode,
        // Splits the image into multiple regions using a divisor and decodes the regions
        // separately.  Also, this test adds a border of a few pixels to each of the regions
        // that it is decoding.  This tests the behavior when a client asks for a region that
        // does not fully fit in the image.
        kDivisor_Mode,
    };

    BRDSrc(Path, SkBitmapRegionDecoder::Strategy, Mode, CodecSrc::DstColorType, uint32_t);

    Error draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    bool veto(SinkFlags) const override;
private:
    Path                                     fPath;
    SkBitmapRegionDecoder::Strategy          fStrategy;
    Mode                                     fMode;
    CodecSrc::DstColorType                   fDstColorType;
    uint32_t                                 fSampleSize;
};

class ImageGenSrc : public Src {
public:
    enum Mode {
        kCodec_Mode,    // Use CodecImageGenerator
        kPlatform_Mode, // Uses CG or WIC
    };
    ImageGenSrc(Path, Mode, SkAlphaType, bool);

    Error draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    bool veto(SinkFlags) const override;
    bool serial() const override { return fRunSerially; }
private:
    Path        fPath;
    Mode        fMode;
    SkAlphaType fDstAlphaType;
    bool        fIsGpu;
    bool        fRunSerially;
};

class SKPSrc : public Src {
public:
    explicit SKPSrc(Path path);

    Error draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
private:
    Path fPath;
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

class NullSink : public Sink {
public:
    NullSink() {}

    Error draw(const Src& src, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return ""; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kNull, SinkFlags::kDirect }; }
};


class GPUSink : public Sink {
public:
    GPUSink(sk_gpu_test::GrContextFactory::ContextType,
            sk_gpu_test::GrContextFactory::ContextOptions,
            int samples, bool diText, SkColorType colorType, SkColorProfileType profileType,
            bool threaded);

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    bool serial() const override { return !fThreaded; }
    const char* fileExtension() const override { return "png"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kGPU, SinkFlags::kDirect }; }
private:
    sk_gpu_test::GrContextFactory::ContextType      fContextType;
    sk_gpu_test::GrContextFactory::ContextOptions   fContextOptions;
    int                                             fSampleCount;
    bool                                            fUseDIText;
    SkColorType                                     fColorType;
    SkColorProfileType                              fProfileType;
    bool                                            fThreaded;
};

class PDFSink : public Sink {
public:
    PDFSink(bool pdfa = false) : fPDFA(pdfa) {}
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return "pdf"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }
    bool fPDFA;
};

class XPSSink : public Sink {
public:
    XPSSink();

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return "xps"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }
};

class RasterSink : public Sink {
public:
    explicit RasterSink(SkColorType, SkColorProfileType=kLinear_SkColorProfileType);

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return "png"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kRaster, SinkFlags::kDirect }; }
private:
    SkColorType        fColorType;
    SkColorProfileType fProfileType;
};

class SKPSink : public Sink {
public:
    SKPSink();

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return "skp"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }
};

class SVGSink : public Sink {
public:
    SVGSink();

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return "svg"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }
};


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

class Via : public Sink {
public:
    explicit Via(Sink* sink) : fSink(sink) {}
    const char* fileExtension() const override { return fSink->fileExtension(); }
    bool               serial() const override { return fSink->serial(); }
    SinkFlags flags() const override {
        SinkFlags flags = fSink->flags();
        flags.approach = SinkFlags::kIndirect;
        return flags;
    }
protected:
    SkAutoTDelete<Sink> fSink;
};

class ViaMatrix : public Via {
public:
    ViaMatrix(SkMatrix, Sink*);
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
private:
    const SkMatrix fMatrix;
};

class ViaUpright : public Via {
public:
    ViaUpright(SkMatrix, Sink*);
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
private:
    const SkMatrix fMatrix;
};

class ViaSerialization : public Via {
public:
    explicit ViaSerialization(Sink* sink) : Via(sink) {}
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

class ViaPicture : public Via {
public:
    explicit ViaPicture(Sink* sink) : Via(sink) {}
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

class ViaTiles : public Via {
public:
    ViaTiles(int w, int h, SkBBHFactory*, Sink*);
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
private:
    const int                   fW, fH;
    SkAutoTDelete<SkBBHFactory> fFactory;
};

class ViaSecondPicture : public Via {
public:
    explicit ViaSecondPicture(Sink* sink) : Via(sink) {}
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

class ViaSingletonPictures : public Via {
public:
    explicit ViaSingletonPictures(Sink* sink) : Via(sink) {}
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

class ViaTwice : public Via {
public:
    explicit ViaTwice(Sink* sink) : Via(sink) {}
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

class ViaMojo : public Via {
public:
    explicit ViaMojo(Sink* sink) : Via(sink) {}
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

}  // namespace DM

#endif//DMSrcSink_DEFINED
