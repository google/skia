/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ContextPriv_DEFINED
#define skgpu_graphite_ContextPriv_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/Context.h"
#include "src/gpu/graphite/QueueManager.h"
#include "src/gpu/graphite/SharedContext.h"

#include <memory>

class SkPixmap;
struct SkImageInfo;

namespace skcpu { class ContextImpl; }

namespace skgpu::graphite {

class Caps;
class GlobalCache;
class Recorder;
class RendererProvider;
class ResourceProvider;
class ShaderCodeDictionary;
class TextureProxy;
enum class PathRendererStrategy;
struct ContextOptions;

/** Class that adds methods to Context that are only intended for use internal to Skia.
    This class is purely a privileged window into Context. It should never have additional
    data members or virtual methods. */
class ContextPriv {
public:
    const Caps* caps() const { return fContext->fSharedContext->caps(); }

    const ShaderCodeDictionary* shaderCodeDictionary() const {
        return fContext->fSharedContext->shaderCodeDictionary();
    }
    ShaderCodeDictionary* shaderCodeDictionary() {
        return fContext->fSharedContext->shaderCodeDictionary();
    }
#if defined(GPU_TEST_UTILS)
    const GlobalCache* globalCache() const {
        return fContext->fSharedContext->globalCache();
    }
    GlobalCache* globalCache() {
        return fContext->fSharedContext->globalCache();
    }
#endif
    const RendererProvider* rendererProvider() const {
        return fContext->fSharedContext->rendererProvider();
    }
    ResourceProvider* resourceProvider() const {
        return fContext->fResourceProvider.get();
    }
    SharedContext* sharedContext() {
        return fContext->fSharedContext.get();
    }
    const skcpu::ContextImpl* cpuContext() const { return fContext->fCPUContext.get(); }

#if defined(GPU_TEST_UTILS)
    void startCapture() {
        fContext->fQueueManager->startCapture();
    }
    void stopCapture() {
        fContext->fQueueManager->stopCapture();
    }

    void deregisterRecorder(const Recorder* recorder) {
        fContext->deregisterRecorder(recorder);
    }

    bool readPixels(const SkPixmap&,
                    const TextureProxy*,
                    const SkImageInfo& srcImageInfo,
                    int srcX, int srcY);

    bool supportsPathRendererStrategy(PathRendererStrategy);
#endif

private:
    friend class Context; // to construct/copy this type.

    explicit ContextPriv(Context* context) : fContext(context) {}

    ContextPriv& operator=(const ContextPriv&) = delete;

    // No taking addresses of this type.
    const ContextPriv* operator&() const;
    ContextPriv *operator&();

    Context* fContext;
};

inline ContextPriv Context::priv() { return ContextPriv(this); }

// NOLINTNEXTLINE(readability-const-return-type)
inline const ContextPriv Context::priv() const {
    return ContextPriv(const_cast<Context *>(this));
}

// This class is friended by the Context and allows the backend ContextFactory functions to
// trampoline through this to call the private Context ctor. We can't directly friend the factory
// functions in Context because they are in a different namespace and we don't want to declare the
// functions in Context.h
class ContextCtorAccessor {
public:
    static std::unique_ptr<Context> MakeContext(sk_sp<SharedContext>,
                                                std::unique_ptr<QueueManager>,
                                                const ContextOptions&);
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ContextPriv_DEFINED
