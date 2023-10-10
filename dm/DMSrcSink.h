/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DMSrcSink_DEFINED
#define DMSrcSink_DEFINED

#include "gm/gm.h"
#include "include/core/SkBBHFactory.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkPicture.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "src/utils/SkMultiPictureDocument.h"
#include "tools/flags/CommonFlagsConfig.h"
#include "tools/gpu/MemoryCache.h"

#include <functional>

//#define TEST_VIA_SVG

namespace skiagm::verifiers {
class VerifierList;
}

namespace skgpu::graphite {
struct Options;
}

namespace DM {

// This is just convenience.  It lets you use either return "foo" or return SkStringPrintf(...).
struct ImplicitString : public SkString {
    template <typename T>
    ImplicitString(const T& s) : SkString(s) {}
    ImplicitString() : SkString("") {}
};
typedef ImplicitString Name;
typedef ImplicitString Path;

class Result {
public:
    enum class Status : int { Ok, Fatal, Skip };

    Result(Status status, SkString msg) : fMsg(std::move(msg)), fStatus(status) {}

    Result(const Result&)            = default;
    Result& operator=(const Result&) = default;

    static Result Ok() { return Result{Status::Ok, {}}; }

    static Result Fatal(const char* fmt, ...) SK_PRINTF_LIKE(1, 2) {
        SkString msg;
        va_list args;
        va_start(args, fmt);
        msg.printVAList(fmt, args);
        va_end(args);

        return Result{Status::Fatal, std::move(msg)};
    }

    static Result Skip(const char* fmt, ...) SK_PRINTF_LIKE(1, 2) {
        SkString msg;
        va_list args;
        va_start(args, fmt);
        msg.printVAList(fmt, args);
        va_end(args);

        return Result{Status::Skip, std::move(msg)};
    }

    bool isOk() { return fStatus == Status::Ok; }
    bool isFatal() { return fStatus == Status::Fatal; }
    bool isSkip() { return fStatus == Status::Skip; }

    const char* c_str() const { return fMsg.c_str(); }
    Status status() const { return fStatus; }

private:
    SkString fMsg;
    Status   fStatus;
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
    [[nodiscard]] virtual Result draw(SkCanvas* canvas) const = 0;
    virtual SkISize size() const = 0;
    virtual Name name() const = 0;
    virtual void modifyGrContextOptions(GrContextOptions*) const  {}
    virtual void modifyGraphiteContextOptions(skgpu::graphite::ContextOptions*) const {}
    virtual bool veto(SinkFlags) const { return false; }

    virtual int pageCount() const { return 1; }
    [[nodiscard]] virtual Result draw([[maybe_unused]] int page, SkCanvas* canvas) const {
        return this->draw(canvas);
    }
    virtual SkISize size([[maybe_unused]] int page) const { return this->size(); }
    // Force Tasks using this Src to run on the main thread?
    virtual bool serial() const { return false; }
};

struct Sink {
    virtual ~Sink() {}
    // You may write to either the bitmap or stream.  If you write to log, we'll print that out.
    [[nodiscard]] virtual Result draw(const Src&, SkBitmap*, SkWStream*, SkString* log) const = 0;

    // Override the color space of this Sink, after creation
    virtual void setColorSpace(sk_sp<SkColorSpace>) {}

    // Force Tasks using this Sink to run on the main thread?
    virtual bool serial() const { return false; }

    // File extension for the content draw() outputs, e.g. "png", "pdf".
    virtual const char* fileExtension() const  = 0;

    virtual SinkFlags flags() const = 0;

    /** Returns the color type and space used by the sink. */
    virtual SkColorInfo colorInfo() const { return SkColorInfo(); }
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

class GMSrc : public Src {
public:
    explicit GMSrc(skiagm::GMFactory);

    Result draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    void modifyGrContextOptions(GrContextOptions* options) const override;
#if defined(SK_GRAPHITE)
    void modifyGraphiteContextOptions(skgpu::graphite::ContextOptions*) const override;
#endif

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

    Result draw(SkCanvas*) const override;
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

    Result draw(SkCanvas*) const override;
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

#ifdef SK_ENABLE_ANDROID_UTILS
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

    Result draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    bool veto(SinkFlags) const override;
private:
    Path                                     fPath;
    Mode                                     fMode;
    CodecSrc::DstColorType                   fDstColorType;
    uint32_t                                 fSampleSize;
};
#endif

class ImageGenSrc : public Src {
public:
    enum Mode {
        kCodec_Mode,    // Use CodecImageGenerator
        kPlatform_Mode, // Uses CG or WIC
    };
    ImageGenSrc(Path, Mode, SkAlphaType, bool);

    Result draw(SkCanvas*) const override;
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

    Result draw(SkCanvas*) const override;
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

    Result draw(SkCanvas*) const override;
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

    Result draw(SkCanvas*) const override;

private:
    SkString fTrail;

    using INHERITED = SKPSrc;
};

#if defined(SK_ENABLE_SKOTTIE)
class SkottieSrc final : public Src {
public:
    explicit SkottieSrc(Path path);

    Result draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    bool veto(SinkFlags) const override;

private:
    // Generates a kTileCount x kTileCount filmstrip with evenly distributed frames.
    inline static constexpr int      kTileCount = 5;

    // Fit kTileCount x kTileCount frames to a 1000x1000 film strip.
    inline static constexpr SkScalar kTargetSize = 1000;
    inline static constexpr SkScalar kTileSize = kTargetSize / kTileCount;

    Path                      fPath;
};
#endif

#if defined(SK_ENABLE_SVG)
} // namespace DM

class SkSVGDOM;

namespace DM {

class SVGSrc : public Src {
public:
    explicit SVGSrc(Path path);

    Result draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    bool veto(SinkFlags) const override;

private:
    Name            fName;
    sk_sp<SkSVGDOM> fDom;
    SkScalar        fScale;

    using INHERITED = Src;
};
#endif // SK_ENABLE_SVG
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

class MSKPSrc : public Src {
public:
    explicit MSKPSrc(Path path);

    int pageCount() const override;
    Result draw(SkCanvas* c) const override;
    Result draw(int, SkCanvas*) const override;
    SkISize size() const override;
    SkISize size(int) const override;
    Name name() const override;

private:
    Path fPath;
    mutable skia_private::TArray<SkDocumentPage> fPages;
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

class NullSink : public Sink {
public:
    NullSink() {}

    Result draw(const Src& src, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return ""; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kNull, SinkFlags::kDirect }; }
};

class GPUSink : public Sink {
public:
    GPUSink(const SkCommandLineConfigGpu*, const GrContextOptions&);

    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    Result onDraw(const Src&, SkBitmap*, SkWStream*, SkString*,
                  const GrContextOptions& baseOptions,
                  std::function<void(GrDirectContext*)> initContext = nullptr,
                  std::function<SkCanvas*(SkCanvas*)> wrapCanvas = nullptr) const;

    skgpu::ContextType contextType() const { return fContextType; }
    const sk_gpu_test::GrContextFactory::ContextOverrides& contextOverrides() const {
        return fContextOverrides;
    }
    SkCommandLineConfigGpu::SurfType surfType() const { return fSurfType; }
    bool serial() const override { return true; }
    const char* fileExtension() const override { return "png"; }
    SinkFlags flags() const override {
        SinkFlags::Multisampled ms = fSampleCount > 1 ? SinkFlags::kMultisampled
                                                      : SinkFlags::kNotMultisampled;
        return SinkFlags{ SinkFlags::kGPU, SinkFlags::kDirect, ms };
    }
    const GrContextOptions& baseContextOptions() const { return fBaseContextOptions; }
    void setColorSpace(sk_sp<SkColorSpace> colorSpace) override { fColorSpace = colorSpace; }
    SkColorInfo colorInfo() const override {
        return SkColorInfo(fColorType, fAlphaType, fColorSpace);
    }

protected:
    sk_sp<SkSurface> createDstSurface(GrDirectContext*, SkISize size) const;
    bool readBack(SkSurface*, SkBitmap* dst) const;

private:
    skgpu::ContextType                                fContextType;
    sk_gpu_test::GrContextFactory::ContextOverrides   fContextOverrides;
    SkCommandLineConfigGpu::SurfType                  fSurfType;
    int                                               fSampleCount;
    uint32_t                                          fSurfaceFlags;
    SkColorType                                       fColorType;
    SkAlphaType                                       fAlphaType;
    sk_sp<SkColorSpace>                               fColorSpace;
    GrContextOptions                                  fBaseContextOptions;
    sk_gpu_test::MemoryCache                          fMemoryCache;
};

// Wrap a gpu canvas in one that routes all text draws through Slugs.
// Note that text blobs that have an RSXForm aren't converted.
class GPUSlugSink : public GPUSink {
public:
    GPUSlugSink(const SkCommandLineConfigGpu*, const GrContextOptions&);

    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

class GPUSerializeSlugSink : public GPUSink {
public:
    GPUSerializeSlugSink(const SkCommandLineConfigGpu*, const GrContextOptions&);

    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

class GPURemoteSlugSink : public GPUSink {
public:
    GPURemoteSlugSink(const SkCommandLineConfigGpu*, const GrContextOptions&);

    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

class GPUPersistentCacheTestingSink : public GPUSink {
public:
    GPUPersistentCacheTestingSink(const SkCommandLineConfigGpu*, const GrContextOptions&);

    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;

    const char* fileExtension() const override {
        // Suppress writing out results from this config - we just want to do our matching test
        return nullptr;
    }

private:
    int fCacheType;

    using INHERITED = GPUSink;
};

class GPUPrecompileTestingSink : public GPUSink {
public:
    GPUPrecompileTestingSink(const SkCommandLineConfigGpu*, const GrContextOptions&);

    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;

    const char* fileExtension() const override {
        // Suppress writing out results from this config - we just want to do our matching test
        return nullptr;
    }

private:
    using INHERITED = GPUSink;
};

// This sink attempts to better simulate the Chrome DDL use-case. It:
//    creates the DDLs on separate recording threads
//    performs all the GPU work on a separate GPU thread
// In the future this should be expanded to:
//    upload on a utility thread w/ access to a shared context
//    compile the programs on the utility thread
//    perform fine grained scheduling of gpu tasks based on their image and program prerequisites
//    create a single "compositing" DDL that is replayed last
class GPUDDLSink : public GPUSink {
public:
    GPUDDLSink(const SkCommandLineConfigGpu*, const GrContextOptions&);

    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;

private:
    Result ddlDraw(const Src&,
                   sk_sp<SkSurface> dstSurface,
                   SkTaskGroup* recordingTaskGroup,
                   SkTaskGroup* gpuTaskGroup,
                   sk_gpu_test::TestContext* gpuTestCtx,
                   GrDirectContext* gpuThreadCtx) const;

    std::unique_ptr<SkExecutor> fRecordingExecutor;
    std::unique_ptr<SkExecutor> fGPUExecutor;

    using INHERITED = GPUSink;
};

class PDFSink : public Sink {
public:
    PDFSink(bool pdfa, SkScalar rasterDpi) : fPDFA(pdfa), fRasterDpi(rasterDpi) {}
    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return "pdf"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }

    bool fPDFA;
    SkScalar fRasterDpi;
};

class XPSSink : public Sink {
public:
    XPSSink();

    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return "xps"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }
};

class RasterSink : public Sink {
public:
    explicit RasterSink(SkColorType);

    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return "png"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kRaster, SinkFlags::kDirect }; }
    void setColorSpace(sk_sp<SkColorSpace> colorSpace) override { fColorSpace = colorSpace; }

    SkColorInfo colorInfo() const override {
        // If there's an appropriate alpha type for this color type, use it, otherwise use premul.
        SkAlphaType alphaType = kPremul_SkAlphaType;
        (void)SkColorTypeValidateAlphaType(fColorType, alphaType, &alphaType);

        return SkColorInfo(fColorType, alphaType, fColorSpace);
    }

private:
    SkColorType         fColorType;
    sk_sp<SkColorSpace> fColorSpace;
};

class SKPSink : public Sink {
public:
    SKPSink();

    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return "skp"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }
};

class DebugSink : public Sink {
public:
    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return "json"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }
};

class SVGSink : public Sink {
public:
    SVGSink(int pageIndex = 0);

    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    const char* fileExtension() const override { return "svg"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }

private:
    int fPageIndex;
};

#if defined(SK_GRAPHITE)

class GraphiteSink : public Sink {
public:
    GraphiteSink(const SkCommandLineConfigGraphite*);

    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    bool serial() const override { return true; }
    const char* fileExtension() const override { return "png"; }
    SinkFlags flags() const override { return SinkFlags{SinkFlags::kGPU, SinkFlags::kDirect}; }
    void setColorSpace(sk_sp<SkColorSpace> colorSpace) override { fColorSpace = colorSpace; }
    SkColorInfo colorInfo() const override {
        return SkColorInfo(fColorType, fAlphaType, fColorSpace);
    }

private:
    skgpu::graphite::ContextOptions fBaseContextOptions;
    skgpu::ContextType fContextType;
    SkColorType fColorType;
    SkAlphaType fAlphaType;
    sk_sp<SkColorSpace> fColorSpace;
};

#endif

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
    void setColorSpace(sk_sp<SkColorSpace> colorSpace) override {
        fSink->setColorSpace(colorSpace);
    }
protected:
    std::unique_ptr<Sink> fSink;
};

class ViaMatrix : public Via {
public:
    ViaMatrix(SkMatrix, Sink*);
    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;

private:
    const SkMatrix fMatrix;
};

class ViaUpright : public Via {
public:
    ViaUpright(SkMatrix, Sink*);
    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;

private:
    const SkMatrix fMatrix;
};

class ViaSerialization : public Via {
public:
    explicit ViaSerialization(Sink* sink) : Via(sink) {}
    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

class ViaPicture : public Via {
public:
    explicit ViaPicture(Sink* sink) : Via(sink) {}
    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

class ViaRuntimeBlend : public Via {
public:
    explicit ViaRuntimeBlend(Sink* sink) : Via(sink) {}
    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

class ViaSVG : public Via {
public:
    explicit ViaSVG(Sink* sink) : Via(sink) {}
    Result draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

}  // namespace DM

#endif//DMSrcSink_DEFINED
