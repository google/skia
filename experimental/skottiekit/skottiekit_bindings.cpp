/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/sksg/include/SkSGInvalidationController.h"

#include <string>
#include <vector>

#include <emscripten.h>
#include <emscripten/bind.h>

#if SK_INCLUDE_MANAGED_SKOTTIE
#include "modules/skottie/utils/SkottieUtils.h"
#include "modules/skresources/include/SkResources.h"
#endif // SK_INCLUDE_MANAGED_SKOTTIE

#ifdef SK_GL
#include "include/core/SkImageInfo.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"

#include <GL/gl.h>
#include <emscripten/html5.h>
#endif

using namespace emscripten;

// Self-documenting types
using JSArray = emscripten::val;
using JSObject = emscripten::val;
using JSString = emscripten::val;
using SkPathOrNull = emscripten::val;
using Uint8Array = emscripten::val;
using Float32Array = emscripten::val;

#if SK_INCLUDE_MANAGED_SKOTTIE
namespace {

class SkottieAssetProvider : public skottie::ResourceProvider {
public:
    ~SkottieAssetProvider() override = default;

    // Tried using a map, but that gave strange errors like
    // https://emscripten.org/docs/porting/guidelines/function_pointer_issues.html
    // Not entirely sure why, but perhaps the iterator in the map was
    // confusing enscripten.
    using AssetVec = std::vector<std::pair<SkString, sk_sp<SkData>>>;

    static sk_sp<SkottieAssetProvider> Make(AssetVec assets) {
        if (assets.empty()) {
            return nullptr;
        }

        return sk_sp<SkottieAssetProvider>(new SkottieAssetProvider(std::move(assets)));
    }

    sk_sp<skottie::ImageAsset> loadImageAsset(const char[] /* path */,
                                              const char name[],
                                              const char[] /* id */) const override {
        // For CK/Skottie we ignore paths & IDs, and identify images based solely on name.
        if (auto data = this->findAsset(name)) {
            return skresources::MultiFrameImageAsset::Make(std::move(data));
        }

        return nullptr;
    }

    sk_sp<SkData> loadFont(const char name[], const char[] /* url */) const override {
        // Same as images paths, we ignore font URLs.
        return this->findAsset(name);
    }

private:
    explicit SkottieAssetProvider(AssetVec assets) : fAssets(std::move(assets)) {}

    sk_sp<SkData> findAsset(const char name[]) const {
        for (const auto& asset : fAssets) {
            if (asset.first.equals(name)) {
                return asset.second;
            }
        }

        SkDebugf("Could not find %s\n", name);
        return nullptr;
    }

    const AssetVec fAssets;
};

class ManagedAnimation final : public SkRefCnt {
public:
    static sk_sp<ManagedAnimation> Make(const std::string& json,
                                        sk_sp<skottie::ResourceProvider> rp) {
        auto mgr = std::make_unique<skottie_utils::CustomPropertyManager>();
        auto animation = skottie::Animation::Builder()
                            .setMarkerObserver(mgr->getMarkerObserver())
                            .setPropertyObserver(mgr->getPropertyObserver())
                            .setResourceProvider(rp)
                            .make(json.c_str(), json.size());

        return animation
            ? sk_sp<ManagedAnimation>(new ManagedAnimation(std::move(animation), std::move(mgr)))
            : nullptr;
    }

    ~ManagedAnimation() override = default;

    // skottie::Animation API
    void render(SkCanvas* canvas) const { fAnimation->render(canvas, nullptr); }
    void render(SkCanvas* canvas, const SkRect& dst) const { fAnimation->render(canvas, &dst); }
    // Returns a damage rect.
    SkRect seek(SkScalar t) {
        sksg::InvalidationController ic;
        fAnimation->seek(t, &ic);
        return ic.bounds();
    }
    // Returns a damage rect.
    SkRect seekFrame(double t) {
        sksg::InvalidationController ic;
        fAnimation->seekFrame(t, &ic);
        return ic.bounds();
    }
    double duration() const { return fAnimation->duration(); }
    double fps() const { return fAnimation->fps(); }
    const SkSize& size() const { return fAnimation->size(); }
    std::string version() const { return std::string(fAnimation->version().c_str()); }

    // CustomPropertyManager API
    JSArray getColorProps() const {
        JSArray props = emscripten::val::array();

        for (const auto& cp : fPropMgr->getColorProps()) {
            JSObject prop = emscripten::val::object();
            prop.set("key", cp);
            prop.set("value", fPropMgr->getColor(cp));
            props.call<void>("push", prop);
        }

        return props;
    }

    JSArray getOpacityProps() const {
        JSArray props = emscripten::val::array();

        for (const auto& op : fPropMgr->getOpacityProps()) {
            JSObject prop = emscripten::val::object();
            prop.set("key", op);
            prop.set("value", fPropMgr->getOpacity(op));
            props.call<void>("push", prop);
        }

        return props;
    }

    bool setColor(const std::string& key, SkColor c) {
        return fPropMgr->setColor(key, c);
    }

    bool setOpacity(const std::string& key, float o) {
        return fPropMgr->setOpacity(key, o);
    }

    JSArray getMarkers() const {
        JSArray markers = emscripten::val::array();
        for (const auto& m : fPropMgr->markers()) {
            JSObject marker = emscripten::val::object();
            marker.set("name", m.name);
            marker.set("t0"  , m.t0);
            marker.set("t1"  , m.t1);
            markers.call<void>("push", marker);
        }
        return markers;
    }

private:
    ManagedAnimation(sk_sp<skottie::Animation> animation,
                     std::unique_ptr<skottie_utils::CustomPropertyManager> propMgr)
        : fAnimation(std::move(animation))
        , fPropMgr(std::move(propMgr)) {}

    sk_sp<skottie::Animation>                             fAnimation;
    std::unique_ptr<skottie_utils::CustomPropertyManager> fPropMgr;
};

} // anonymous ns
#endif // SK_INCLUDE_MANAGED_SKOTTIE

struct SimpleImageInfo {
    int width;
    int height;
    SkColorType colorType;
    SkAlphaType alphaType;
    // TODO color spaces?
};

SkImageInfo toSkImageInfo(const SimpleImageInfo& sii) {
    return SkImageInfo::Make(sii.width, sii.height, sii.colorType, sii.alphaType);
}

#ifdef SK_GL
sk_sp<GrDirectContext> MakeGrContext(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context)
{
    EMSCRIPTEN_RESULT r = emscripten_webgl_make_context_current(context);
    if (r < 0) {
        printf("failed to make webgl context current %d\n", r);
        return nullptr;
    }
    // setup GrDirectContext
    auto interface = GrGLMakeNativeInterface();
    return GrDirectContext::MakeGL(interface);
}

sk_sp<SkSurface> MakeOnScreenGLSurface(sk_sp<GrDirectContext> grContext, int width, int height) {
    glClearColor(0, 0, 0, 0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


    // Wrap the frame buffer object attached to the screen in a Skia render
    // target so Skia can render to it
    GrGLint buffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &buffer);
    GrGLFramebufferInfo info;
    info.fFBOID = (GrGLuint) buffer;
    SkColorType colorType;

    GrGLint stencil;
    glGetIntegerv(GL_STENCIL_BITS, &stencil);

    info.fFormat = GL_RGBA8;
    colorType = kRGBA_8888_SkColorType;

    GrBackendRenderTarget target(width, height, 0, stencil, info);

    sk_sp<SkSurface> surface(SkSurface::MakeFromBackendRenderTarget(grContext.get(), target,
                                                                    kBottomLeft_GrSurfaceOrigin,
                                                                    colorType, nullptr, nullptr));
    return surface;
}

sk_sp<SkSurface> MakeRenderTarget(sk_sp<GrDirectContext> grContext, int width, int height) {
    SkImageInfo info = SkImageInfo::MakeN32(width, height, SkAlphaType::kPremul_SkAlphaType);

    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(grContext.get(),
                             SkBudgeted::kYes,
                             info, 0,
                             kBottomLeft_GrSurfaceOrigin,
                             nullptr, true));
    return surface;
}

sk_sp<SkSurface> MakeRenderTarget(sk_sp<GrDirectContext> grContext, SimpleImageInfo sii) {
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(grContext.get(),
                             SkBudgeted::kYes,
                             toSkImageInfo(sii), 0,
                             kBottomLeft_GrSurfaceOrigin,
                             nullptr, true));
    return surface;
}
#endif // SK_GL

// Some signatures below have uintptr_t instead of a pointer to a primitive
// type (e.g. SkScalar). This is necessary because we can't use "bind" (EMSCRIPTEN_BINDINGS)
// and pointers to primitive types (Only bound types like SkPoint). We could if we used
// cwrap (see https://becominghuman.ai/passing-and-returning-webassembly-array-parameters-a0f572c65d97)
// but that requires us to stick to C code and, AFAIK, doesn't allow us to return nice things like
// SkPath or SkCanvas.
//
// So, basically, if we are using C++ and EMSCRIPTEN_BINDINGS, we can't have primitive pointers
// in our function type signatures. (this gives an error message like "Cannot call foo due to unbound
// types Pi, Pf").  But, we can just pretend they are numbers and cast them to be pointers and
// the compiler is happy.
EMSCRIPTEN_BINDINGS(Skottie) {
#ifdef SK_GL
    function("currentContext", &emscripten_webgl_get_current_context);
    function("setCurrentContext", &emscripten_webgl_make_context_current);
    function("MakeGrContext", &MakeGrContext);
    function("MakeOnScreenGLSurface", &MakeOnScreenGLSurface);
    function("MakeRenderTarget",
        select_overload<sk_sp<SkSurface>(sk_sp<GrDirectContext>, int, int)>(&MakeRenderTarget));
    function("MakeRenderTarget",
        select_overload<sk_sp<SkSurface>(sk_sp<GrDirectContext>, SimpleImageInfo)>(&MakeRenderTarget));

    constant("gpu", true);

    class_<GrDirectContext>("GrDirectContext")
    .smart_ptr<sk_sp<GrDirectContext>>("sk_sp<GrDirectContext>");
    // .function("getResourceCacheLimitBytes", optional_override([](GrDirectContext& self)->size_t {
    //     int maxResources = 0;// ignored
    //     size_t currMax = 0;
    //     self.getResourceCacheLimits(&maxResources, &currMax);
    //     return currMax;
    // }))
    // .function("getResourceCacheUsageBytes", optional_override([](GrDirectContext& self)->size_t {
    //     int usedResources = 0;// ignored
    //     size_t currUsage = 0;
    //     self.getResourceCacheUsage(&usedResources, &currUsage);
    //     return currUsage;
    // }))
    // .function("releaseResourcesAndAbandonContext", &GrDirectContext::releaseResourcesAndAbandonContext)
    // .function("setResourceCacheLimitBytes", optional_override([](GrDirectContext& self, size_t maxResourceBytes) {
    //     int maxResources = 0;
    //     size_t currMax = 0; // ignored
    //     self.getResourceCacheLimits(&maxResources, &currMax);
    //     self.setResourceCacheLimits(maxResources, maxResourceBytes);
    // }));
#endif

    // function("getDecodeCacheLimitBytes", &SkResourceCache::GetTotalByteLimit);
    // function("setDecodeCacheLimitBytes", &SkResourceCache::SetTotalByteLimit);
    // function("getDecodeCacheUsedBytes" , &SkResourceCache::GetTotalBytesUsed);

    function("_getRasterDirectSurface", optional_override([](const SimpleImageInfo ii,
                                                             uintptr_t /* uint8_t*  */ pPtr,
                                                             size_t rowBytes)->sk_sp<SkSurface> {
        uint8_t* pixels = reinterpret_cast<uint8_t*>(pPtr);
        SkImageInfo imageInfo = toSkImageInfo(ii);
        return SkSurface::MakeRasterDirect(imageInfo, pixels, rowBytes, nullptr);
    }), allow_raw_pointers());

    class_<SkCanvas>("SkCanvas")
        .function("_clear", optional_override([](SkCanvas& self, uintptr_t /* float* */ cPtr) {
            // See comment above for uintptr_t explanation
            float* fourFloats = reinterpret_cast<float*>(cPtr);
            SkColor4f color = { fourFloats[0], fourFloats[1], fourFloats[2], fourFloats[3]};
            // TODO(reed): SkCanvas.clear() should take SkColor4f, then we could just use it here.
            SkPaint p;
            p.setBlendMode(SkBlendMode::kSrc);
            p.setColor4f(color);
            self.drawPaint(p);
        }));

    class_<SkSurface>("SkSurface")
        .smart_ptr<sk_sp<SkSurface>>("sk_sp<SkSurface>")
        .function("_flush", select_overload<void()>(&SkSurface::flush))
        .function("getCanvas", &SkSurface::getCanvas, allow_raw_pointers());

    class_<skottie::Animation>("Animation")
        .smart_ptr<sk_sp<skottie::Animation>>("sk_sp<Animation>")
        .function("version", optional_override([](skottie::Animation& self)->std::string {
            return std::string(self.version().c_str());
        }))
        .function("size"    , &skottie::Animation::size)
        .function("duration", &skottie::Animation::duration)
        .function("fps"     , &skottie::Animation::fps)
        .function("seek", optional_override([](skottie::Animation& self, SkScalar t)->void {
            self.seek(t);
        }))
        .function("seekFrame", optional_override([](skottie::Animation& self, double t)->void {
            self.seekFrame(t);
        }))
        .function("render", optional_override([](skottie::Animation& self, SkCanvas* canvas)->void {
            self.render(canvas, nullptr);
        }), allow_raw_pointers())
        .function("render", optional_override([](skottie::Animation& self, SkCanvas* canvas,
                                                 const SkRect r)->void {
            self.render(canvas, &r);
        }), allow_raw_pointers());

    function("MakeAnimation", optional_override([](std::string json)->sk_sp<skottie::Animation> {
        return skottie::Animation::Make(json.c_str(), json.length());
    }));
    constant("skottie", true);

#if SK_INCLUDE_MANAGED_SKOTTIE
    class_<ManagedAnimation>("ManagedAnimation")
        .smart_ptr<sk_sp<ManagedAnimation>>("sk_sp<ManagedAnimation>")
        .function("version"   , &ManagedAnimation::version)
        .function("size"      , &ManagedAnimation::size)
        .function("duration"  , &ManagedAnimation::duration)
        .function("fps"       , &ManagedAnimation::fps)
        .function("seek"      , &ManagedAnimation::seek)
        .function("seekFrame" , &ManagedAnimation::seekFrame)
        .function("render"    , select_overload<void(SkCanvas*) const>(&ManagedAnimation::render), allow_raw_pointers())
        .function("render"    , select_overload<void(SkCanvas*, const SkRect&) const>
                                    (&ManagedAnimation::render), allow_raw_pointers());
        // .function("setColor"  , optional_override([](ManagedAnimation& self, const std::string& key, SimpleColor4f c) {
        //     self.setColor(key, c.toSkColor());
        // }))
        // .function("setOpacity", &ManagedAnimation::setOpacity)
        // .function("getMarkers", &ManagedAnimation::getMarkers)
        // .function("getColorProps"  , &ManagedAnimation::getColorProps)
        // .function("getOpacityProps", &ManagedAnimation::getOpacityProps);

    function("_MakeManagedAnimation", optional_override([](std::string json,
                                                           size_t assetCount,
                                                           uintptr_t /* char**     */ nptr,
                                                           uintptr_t /* uint8_t**  */ dptr,
                                                           uintptr_t /* size_t*    */ sptr)
                                                        ->sk_sp<ManagedAnimation> {
        // See the comment in canvaskit_bindings.cpp about the use of uintptr_t
        const auto assetNames = reinterpret_cast<char**   >(nptr);
        const auto assetDatas = reinterpret_cast<uint8_t**>(dptr);
        const auto assetSizes = reinterpret_cast<size_t*  >(sptr);

        SkottieAssetProvider::AssetVec assets;
        assets.reserve(assetCount);

        for (size_t i = 0; i < assetCount; i++) {
            auto name  = SkString(assetNames[i]);
            auto bytes = SkData::MakeFromMalloc(assetDatas[i], assetSizes[i]);
            assets.push_back(std::make_pair(std::move(name), std::move(bytes)));
        }

        return ManagedAnimation::Make(json,
                 skresources::DataURIResourceProviderProxy::Make(
                    SkottieAssetProvider::Make(std::move(assets))));
    }));
    constant("managed_skottie", true);
#endif // SK_INCLUDE_MANAGED_SKOTTIE

    value_object<SimpleImageInfo>("SkImageInfo")
        .field("width",     &SimpleImageInfo::width)
        .field("height",    &SimpleImageInfo::height)
        .field("colorType", &SimpleImageInfo::colorType)
        .field("alphaType", &SimpleImageInfo::alphaType);

    value_object<SkRect>("SkRect")
        .field("fLeft",   &SkRect::fLeft)
        .field("fTop",    &SkRect::fTop)
        .field("fRight",  &SkRect::fRight)
        .field("fBottom", &SkRect::fBottom);

    // {"w": Number, "h", Number}
    value_object<SkSize>("SkSize")
        .field("w",   &SkSize::fWidth)
        .field("h",   &SkSize::fHeight);

    enum_<SkAlphaType>("AlphaType")
        .value("Opaque",   SkAlphaType::kOpaque_SkAlphaType)
        .value("Premul",   SkAlphaType::kPremul_SkAlphaType)
        .value("Unpremul", SkAlphaType::kUnpremul_SkAlphaType);

    enum_<SkColorType>("ColorType")
        .value("RGBA_8888", SkColorType::kRGBA_8888_SkColorType);

}
