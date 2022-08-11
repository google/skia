/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_SharedContext_DEFINED
#define skgpu_graphite_SharedContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"

#include "include/gpu/graphite/GraphiteTypes.h"

namespace SkSL {
    class Compiler;
}

namespace skgpu {
class SingleOwner;
}

namespace skgpu::graphite {

class BackendTexture;
class Caps;
class CommandBuffer;
class GlobalCache;
class ResourceProvider;
class TextureInfo;

class SharedContext : public SkRefCnt {
public:
    ~SharedContext() override;

    /**
     * Gets the capabilities of the draw target.
     */
    const Caps* caps() const { return fCaps.get(); }
    sk_sp<const Caps> refCaps() const;

    SkSL::Compiler* shaderCompiler() const { return fCompiler.get(); }

    virtual std::unique_ptr<ResourceProvider> makeResourceProvider(sk_sp<GlobalCache>,
                                                                   SingleOwner*) const = 0;

    BackendTexture createBackendTexture(SkISize dimensions, const TextureInfo&);
    void deleteBackendTexture(BackendTexture&);

#if GRAPHITE_TEST_UTILS
    virtual void testingOnly_startCapture() {}
    virtual void testingOnly_endCapture() {}
#endif

protected:
    SharedContext(sk_sp<const Caps>);

    // Subclass must call this to initialize compiler in its constructor.
    void initCompiler();

private:
    virtual BackendTexture onCreateBackendTexture(SkISize dimensions, const TextureInfo&) = 0;
    virtual void onDeleteBackendTexture(BackendTexture&) = 0;

    sk_sp<const Caps> fCaps;
    // Compiler used for compiling SkSL into backend shader code. We only want to create the
    // compiler once, as there is significant overhead to the first compile.
    std::unique_ptr<SkSL::Compiler> fCompiler;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_SharedContext_DEFINED
