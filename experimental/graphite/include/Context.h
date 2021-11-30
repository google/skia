/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Context_DEFINED
#define skgpu_Context_DEFINED

#include <vector>
#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"

#include "experimental/graphite/include/GraphiteTypes.h"

namespace skgpu {

class BackendTexture;
class ContextPriv;
class Gpu;
class Recorder;
class Recording;
class TextureInfo;
namespace mtl { struct BackendContext; }

struct ShaderCombo {
    enum class ShaderType {
        kNone, // does not modify color buffer, e.g. depth and/or stencil only
        kSolidColor,
        kLinearGradient,
        kRadialGradient,
        kSweepGradient,
        kConicalGradient
    };

    ShaderCombo() {}
    ShaderCombo(std::vector<ShaderType> types,
                std::vector<SkTileMode> tileModes)
            : fTypes(std::move(types))
            , fTileModes(std::move(tileModes)) {
    }
    std::vector<ShaderType> fTypes;
    std::vector<SkTileMode> fTileModes;
};

struct PaintCombo {
    std::vector<ShaderCombo> fShaders;
    std::vector<SkBlendMode> fBlendModes;
};

class Context final : public SkRefCnt {
public:
    ~Context() override;

#ifdef SK_METAL
    static sk_sp<Context> MakeMetal(const skgpu::mtl::BackendContext&);
#endif

    BackendApi backend() const { return fBackend; }

    sk_sp<Recorder> createRecorder();

    void insertRecording(std::unique_ptr<Recording>);
    void submit(SyncToCpu = SyncToCpu::kNo);

    void preCompile(const PaintCombo&);

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

    std::vector<std::unique_ptr<Recording>> fRecordings;
    sk_sp<Gpu> fGpu;
    BackendApi fBackend;
};

} // namespace skgpu

#endif // skgpu_Context_DEFINED
