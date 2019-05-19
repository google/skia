/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DMSrcSink_DEFINED
#define DMSrcSink_DEFINED

#include "gm/gm.h"
#include "include/android/SkBitmapRegionDecoder.h"
#include "include/core/SkBBHFactory.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkPicture.h"
#include "src/core/SkBBoxHierarchy.h"
#include "src/utils/SkMultiPictureDocument.h"
#include "tools/flags/CommonFlagsConfig.h"

//#define TEST_VIA_SVG

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
    enum Type { kNull, kGPU, kVector, kRaster } type;
    enum Approach { kDirect, kIndirect } approach;
    enum Multisampled { kNotMultisampled, kMultisampled } multisampled;
    SinkFlags(Type t, Approach a, Multisampled ms = kNotMultisampled)
            : type(t), approach(a), multisampled(ms) {}
};

struct Src {
    virtual ~Src() {}
    virtual Error SK_WARN_UNUSED_RESULT draw(SkCanvas*) const = 0;
    virtual SkISize size() const = 0;
    virtual Name name() const = 0;
    virtual void modifyGrContextOptions(GrContextOptions* options) const {}
    virtual bool veto(SinkFlags) const { return false; }

    virtual int pageCount() const { return 1; }
    virtual Error SK_WARN_UNUSED_RESULT draw(int, SkCanvas* canvas) const {
        return this->draw(canvas);
    }
    virtual SkISize size(int) const { return this->size(); }
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
    explicit GMSrc(skiagm::GMFactory);

    Error draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    void modifyGrContextOptions(GrContextOptions* options) const override;

private:
    skiagm::GMFactory fFactory;
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
        kAnimated_Mode, // For codecs that support animation.
    };
    enum DstColorType {
        kGetFromCanvas_DstColorType,
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
    AndroidCodecSrc(Path, CodecSrc::DstColorType, SkAlphaType, int sampleSize);

    Error draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    bool veto(SinkFlags) const override;
    bool serial() const override { return fRunSerially; }
private:
    Path                    fPath;
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

    BRDSrc(Path, Mode, CodecSrc::DstColorType, uint32_t);

    Error draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    bool veto(SinkFlags) const override;
private:
    Path                                     fPath;
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

class ColorCodecSrc : public Src {
public:
    ColorCodecSrc(Path, bool decode_to_dst);

    Error draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    bool veto(SinkFlags) const override;
private:
    Path fPath;
    bool fDecodeToDst;
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

// This class extracts all the paths from an SKP and then removes unwanted paths according to the
// provided l/r trail. It then just draws the remaining paths. (Non-path draws are thrown out.) It
// is useful for finding a reduced repo case for path drawing bugs.
class BisectSrc : public SKPSrc {
public:
    explicit BisectSrc(Path path, const char* trail);

    Error draw(SkCanvas*) const override;

private:
    SkString fTrail;

    typedef SKPSrc INHERITED;
};


#if defined(SK_ENABLE_SKOTTIE)
class SkottieSrc final : public Src {
public:
    explicit SkottieSrc(Path path);

    Error draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    bool veto(SinkFlags) const override;

private:
    // Generates a kTileCount x kTileCount filmstrip with evenly distributed frames.
    static constexpr int      kTileCount = 5;

    // Fit kTileCount x kTileCount frames to a 1000x1000 film strip.
    static constexpr SkScalar kTargetSize = 1000;
    static constexpr SkScalar kTileSize = kTargetSize / kTileCount;

    Path                      fPath;
};
#endif

#if defined(SK_XML)
} // namespace DM

class SkSVGDOM;

namespace DM {

class SVGSrc : public Src {
public:
    explicit SVGSrc(Path path);

    Error draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    bool veto(SinkFlags) const override;

private:
    Name            fName;
    sk_sp<SkSVGDOM> fDom;
    SkScalar        fScale;

    typedef Src INHERITED;
};
#endif // SK_XML
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

class MSKPSrc : public Src {
public:
    explicit MSKPSrc(Path path);

    int pageCount() const override;
    Error draw(SkCanvas* c) const override;
    Error draw(int, SkCanvas*) const override;
    SkISize size() const override;
    SkISize size(int) const override;
    Name name() const override;

private:
    Path fPath;
    mutable SkTArray<SkDocumentPage> fPages;
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
            sk_gpu_test::GrContextFactory::ContextOverrides,
            SkCommandLineConfigGpu::SurfType surfType, int samples, bool diText,
            SkColorType colorType, SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace,
            bool threaded, const GrContextOptions& grCtxOptions);

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    Error onDraw(const Src&, SkBitmap*, SkWStream*, SkString*,
                 const GrContextOptions& baseOptions) const;

    sk_gpu_test::GrContextFactory::ContextType contextType() const { return fContextType; }
    const sk_gpu_test::GrContextFactory::ContextOverrides& contextOverrides() {
        return fContextOverrides;
    }
    SkCommandLineConfigGpu::SurfType surfType() const { return fSurfType; }
    bool useDIText() const { return fUseDIText; }
    bool serial() const override { return !fThreaded; }
    const char* fileExtension() const override { return "png"; }
    SinkFlags flags() const override {
        SinkFlags::Multisampled ms = fSampleCount > 1 ? SinkFlags::kMultisampled
                                                      : SinkFlags::kNotMultisampled;
        return SinkFlags{ SinkFlags::kGPU, SinkFlags::kDirect, ms };
    }
    const GrContextOptions& baseContextOptions() const { return fBaseContextOptions; }

private:
    sk_gpu_test::GrContextFactory::ContextType        fContextType;
    sk_gpu_test::GrContextFactory::ContextOverrides   fContextOverrides;
    SkCommandLineConfigGpu::SurfType                  fSurfType;
    int                                               fSampleCount;
    bool                                              fUseDIText;
    SkColorType                                       fColorType;
    SkAlphaType                                       fAlphaType;
    sk_sp<SkColorSpace>                               fColorSpace;
    bool                                              fThreaded;
    GrContextOptions                                  fBaseContextOptions;
};

class GPUThreadTestingSink : public GPUSink {
public:
    GPUThreadTestingSink(sk_gpu_test::GrContextFactory::ContextType,
                         sk_gpu_test::GrContextFactory::ContextOverrides,
                         SkCommandLineConfigGpu::SurfType surfType, int samples, bool diText,
                         SkColorType colorType, SkAlphaType alphaType,
                         sk_sp<SkColorSpace> colorSpace, bool threaded,
                         const GrContextOptions& grCtxOptions);

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;

    const char* fileExtension() const override {
        // Suppress writing out results from this config - we just want to do our matching test
        return nullptr;
    }

private:
    std::unique_ptr<SkExecutor> fExecutor;

    typedef GPUSink INHERITED;
};

class GPUPersistentCacheTestingSink : public GPUSink {
public:
    GPUPersistentCacheTestingSink(sk_gpu_test::GrContextFactory::ContextType,
                                  sk_gpu_test::GrContextFactory::ContextOverrides,
                                  SkCommandLineConfigGpu::SurfType surfType, int samples,
                                  bool diText, SkColorType colorType, SkAlphaType alphaType,
                                  sk_sp<SkColorSpace> colorSpace, bool threaded,
                                  const GrContextOptions& grCtxOptions,
                                  int cacheType);

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;

    const char* fileExtension() const override {
        // Suppress writing out results from this config - we just want to do our matching test
        return nullptr;
    }

private:
    int fCacheType;

    typedef GPUSink INHERITED;
};

class PDFSink : public Sink {
public:
    PDFSink(bool pdfa, SkScalar rasterDpi) : fPDFA(pdfa), fRasterDpi(rasterDpi) {}
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return "pdf"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }
    bool fPDFA;
    SkScalar fRasterDpi;
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
    explicit RasterSink(SkColorType, sk_sp<SkColorSpace> = nullptr);

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return "png"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kRaster, SinkFlags::kDirect }; }

private:
    SkColorType         fColorType;
    sk_sp<SkColorSpace> fColorSpace;
};

class ThreadedSink : public RasterSink {
public:
    explicit ThreadedSink(SkColorType, sk_sp<SkColorSpace> = nullptr);
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

class SKPSink : public Sink {
public:
    SKPSink();

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return "skp"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }
};

class DebugSink : public Sink {
public:
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return "json"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }
};

class SVGSink : public Sink {
public:
    SVGSink(int pageIndex = 0);

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return "svg"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }

private:
    int fPageIndex;
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
    std::unique_ptr<Sink> fSink;
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
    std::unique_ptr<SkBBHFactory> fFactory;
};

class ViaDDL : public Via {
public:
    ViaDDL(int numReplays, int numDivisions, Sink* sink);
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
private:
    const int fNumReplays;
    const int fNumDivisions;
};

class ViaSVG : public Via {
public:
    explicit ViaSVG(Sink* sink) : Via(sink) {}
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

}  // namespace DM

#endif//DMSrcSink_DEFINED
