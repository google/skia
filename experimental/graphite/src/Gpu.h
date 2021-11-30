/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Gpu_DEFINED
#define skgpu_Gpu_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/private/SkDeque.h"

#include "experimental/graphite/include/GraphiteTypes.h"

namespace SkSL {
    class Compiler;
}

namespace skgpu {

class BackendTexture;
class Caps;
class CommandBuffer;
class GpuWorkSubmission;
class ResourceProvider;
class TextureInfo;

class Gpu : public SkRefCnt {
public:
    ~Gpu() override;

    /**
     * Gets the capabilities of the draw target.
     */
    const Caps* caps() const { return fCaps.get(); }
    sk_sp<const Caps> refCaps() const;

    SkSL::Compiler* shaderCompiler() const { return fCompiler.get(); }

    ResourceProvider* resourceProvider() const { return fResourceProvider.get(); }

    bool submit(sk_sp<CommandBuffer>);
    void checkForFinishedWork(SyncToCpu);

    BackendTexture createBackendTexture(SkISize dimensions, const TextureInfo&);
    void deleteBackendTexture(BackendTexture&);

#if GRAPHITE_TEST_UTILS
    virtual void testingOnly_startCapture() {}
    virtual void testingOnly_endCapture() {}
#endif

protected:
    Gpu(sk_sp<const Caps>);

    // Subclass must call this to initialize compiler in its constructor.
    void initCompiler();

    std::unique_ptr<ResourceProvider> fResourceProvider;

    using OutstandingSubmission = std::unique_ptr<GpuWorkSubmission>;
    SkDeque fOutstandingSubmissions;

private:
    virtual bool onSubmit(sk_sp<CommandBuffer>) = 0;

    virtual BackendTexture onCreateBackendTexture(SkISize dimensions, const TextureInfo&) = 0;
    virtual void onDeleteBackendTexture(BackendTexture&) = 0;

    sk_sp<const Caps> fCaps;
    // Compiler used for compiling SkSL into backend shader code. We only want to create the
    // compiler once, as there is significant overhead to the first compile.
    std::unique_ptr<SkSL::Compiler> fCompiler;
};

} // namespace skgpu

#endif // skgpu_Gpu_DEFINED
