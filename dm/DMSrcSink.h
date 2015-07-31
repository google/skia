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
#include "SkCanvas.h"
#include "SkCodec.h"
#include "SkData.h"
#include "SkGPipe.h"
#include "SkPicture.h"
#include "gm.h"

namespace DM {

// This is just convenience.  It lets you use either return "foo" or return SkStringPrintf(...).
struct ImplicitString : public SkString {
    template <typename T>
    ImplicitString(const T& s) : SkString(s) {}
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
    // All Srcs must be thread safe.
    virtual ~Src() {}
    virtual Error SK_WARN_UNUSED_RESULT draw(SkCanvas*) const = 0;
    virtual SkISize size() const = 0;
    virtual Name name() const = 0;
    virtual void modifyGrContextOptions(GrContextOptions* options) const {}
    virtual bool veto(SinkFlags) const { return false; }
};

struct Sink {
    virtual ~Sink() {}
    // You may write to either the bitmap or stream.  If you write to log, we'll print that out.
    virtual Error SK_WARN_UNUSED_RESULT draw(const Src&, SkBitmap*, SkWStream*, SkString* log)
        const = 0;
    // Sinks in the same enclave (except kAnyThread_Enclave) will run serially on the same thread.
    virtual int enclave() const = 0;

    // File extension for the content draw() outputs, e.g. "png", "pdf".
    virtual const char* fileExtension() const  = 0;

    virtual SinkFlags flags() const = 0;
};

enum { kAnyThread_Enclave, kGPU_Enclave };
static const int kNumEnclaves = kGPU_Enclave + 1;

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
        kNormal_Mode,
        kScanline_Mode,
        kScanline_Subset_Mode,
        kStripe_Mode, // Tests the skipping of scanlines
        kSubset_Mode, // For codecs that support subsets directly.
    };
    enum DstColorType {
        kGetFromCanvas_DstColorType,
        kIndex8_Always_DstColorType,
        kGrayscale_Always_DstColorType,
    };
    CodecSrc(Path, Mode, DstColorType, float);

    Error draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    bool veto(SinkFlags) const override;
private:
    Path                   fPath;
    Mode                   fMode;
    DstColorType           fDstColorType;
    float                  fScale;
};


class ImageSrc : public Src {
public:
    // divisor == 0 means decode the whole image
    // divisor > 0 means decode in subsets, dividing into a divisor x divisor grid.
    explicit ImageSrc(Path path, int divisor = 0);

    Error draw(SkCanvas*) const override;
    SkISize size() const override;
    Name name() const override;
    bool veto(SinkFlags) const override;
private:
    Path fPath;
    const int  fDivisor;
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
    int enclave() const override { return kAnyThread_Enclave; }
    const char* fileExtension() const override { return ""; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kNull, SinkFlags::kDirect }; }
};


class GPUSink : public Sink {
public:
    GPUSink(GrContextFactory::GLContextType, GrGLStandard, int samples, bool dfText, bool threaded);

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    int enclave() const override;
    const char* fileExtension() const override { return "png"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kGPU, SinkFlags::kDirect }; }
private:
    GrContextFactory::GLContextType fContextType;
    GrGLStandard                    fGpuAPI;
    int                             fSampleCount;
    bool                            fUseDFText;
    bool                            fThreaded;
};

class PDFSink : public Sink {
public:
    PDFSink();

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    int enclave() const override { return kAnyThread_Enclave; }
    const char* fileExtension() const override { return "pdf"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }
};

class XPSSink : public Sink {
public:
    XPSSink();

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    int enclave() const override { return kAnyThread_Enclave; }
    const char* fileExtension() const override { return "xps"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }
};

class RasterSink : public Sink {
public:
    explicit RasterSink(SkColorType);

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    int enclave() const override { return kAnyThread_Enclave; }
    const char* fileExtension() const override { return "png"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kRaster, SinkFlags::kDirect }; }
private:
    SkColorType    fColorType;
};

class SKPSink : public Sink {
public:
    SKPSink();

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    int enclave() const override { return kAnyThread_Enclave; }
    const char* fileExtension() const override { return "skp"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }
};

class SVGSink : public Sink {
public:
    SVGSink();

    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
    int enclave() const override { return kAnyThread_Enclave; }
    const char* fileExtension() const override { return "svg"; }
    SinkFlags flags() const override { return SinkFlags{ SinkFlags::kVector, SinkFlags::kDirect }; }
};


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

class Via : public Sink {
public:
    explicit Via(Sink* sink) : fSink(sink) {}
    const char* fileExtension() const override { return fSink->fileExtension(); }
    int               enclave() const override { return fSink->enclave(); }
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

class ViaPipe : public Via {
public:
    explicit ViaPipe(Sink* sink) : Via(sink) {}
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

class ViaDeferred : public Via {
public:
    explicit ViaDeferred(Sink* sink) : Via(sink) {}
    Error draw(const Src&, SkBitmap*, SkWStream*, SkString*) const override;
};

class ViaSerialization : public Via {
public:
    explicit ViaSerialization(Sink* sink) : Via(sink) {}
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

}  // namespace DM

#endif//DMSrcSink_DEFINED
