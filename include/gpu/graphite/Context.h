/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Context_DEFINED
#define skgpu_graphite_Context_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/gpu/graphite/GraphiteTypes.h"

class SkBlenderID;
class SkCombinationBuilder;
class SkRuntimeEffect;

namespace skgpu::graphite {

class BackendTexture;
class CommandBuffer;
class Context;
class ContextPriv;
class GlobalCache;
class Gpu;
struct MtlBackendContext;
class Recorder;
class Recording;
class TextureInfo;

class Context final {
public:
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;

    ~Context();

#ifdef SK_METAL
    static std::unique_ptr<Context> MakeMetal(const skgpu::graphite::MtlBackendContext&);
#endif

    BackendApi backend() const { return fBackend; }

    std::unique_ptr<Recorder> makeRecorder();

    void insertRecording(const InsertRecordingInfo&);
    void submit(SyncToCpu = SyncToCpu::kNo);

    /**
     * Checks whether any asynchronous work is complete and if so calls related callbacks.
     */
    void checkAsyncWorkCompletion();

    // TODO: add "SkShaderID addUserDefinedShader(sk_sp<SkRuntimeEffect>)" here
    // TODO: add "SkColorFilterID addUserDefinedColorFilter(sk_sp<SkRuntimeEffect>)" here
    SkBlenderID addUserDefinedBlender(sk_sp<SkRuntimeEffect>);

    void preCompile(const SkCombinationBuilder&);

    /**
     * Creates a new backend gpu texture matching the dimensinos and TextureInfo. If an invalid
     * TextureInfo or a TextureInfo Skia can't support is passed in, this will return an invalid
     * BackendTexture. Thus the client should check isValid on the returned BackendTexture to know
     * if it succeeded or not.
     *
     * If this does return a valid BackendTexture, the caller is required to use
     * Context::deleteBackendTexture to delete that texture.
     */
    BackendTexture createBackendTexture(SkISize dimensions, const TextureInfo&);

    /**
     * Called to delete the passed in BackendTexture. This should only be called if the
     * BackendTexture was created by calling Context::createBackendTexture. If the BackendTexture is
     * not valid or does not match the BackendApi of the Context then nothing happens.
     *
     * Otherwise this will delete/release the backend object that is wrapped in the BackendTexture.
     * The BackendTexture will be reset to an invalid state and should not be used again.
     */
    void deleteBackendTexture(BackendTexture&);

    // Provides access to functions that aren't part of the public API.
    ContextPriv priv();
    const ContextPriv priv() const;  // NOLINT(readability-const-return-type)

protected:
    Context(sk_sp<Gpu>, BackendApi);

private:
    friend class ContextPriv;

    sk_sp<CommandBuffer> fCurrentCommandBuffer;

    sk_sp<Gpu> fGpu;
    sk_sp<GlobalCache> fGlobalCache;
    BackendApi fBackend;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Context_DEFINED
