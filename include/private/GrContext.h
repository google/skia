/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContext_DEFINED
#define GrContext_DEFINED

#include "include/core/SkTypes.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrRecordingContext.h"

class GrAtlasManager;
class GrClientMappedBufferManager;
class GrContextPriv;
class GrGpu;
class GrResourceCache;
class GrResourceProvider;
class GrSmallPathAtlasMgr;
class GrStrikeCache;
class SkTaskGroup;

/**
 * This deprecated class is being merged into GrDirectContext and removed.
 * Do not add new subclasses, new API, or attempt to instantiate one.
 * If new API requires direct GPU access, add it to GrDirectContext.
 * Otherwise, add it to GrRecordingContext.
 */
class SK_API GrContext : public GrRecordingContext {
public:
    ~GrContext() override;

    // Provides access to functions that aren't part of the public API.
    GrContextPriv priv();
    const GrContextPriv priv() const;  // NOLINT(readability-const-return-type)
protected:
    GrContext(sk_sp<GrContextThreadSafeProxy>);

    virtual GrAtlasManager* onGetAtlasManager() = 0;
    virtual GrSmallPathAtlasMgr* onGetSmallPathAtlasMgr() = 0;

private:
    friend class GrDirectContext; // for access to fGpu

    // fTaskGroup must appear before anything that uses it (e.g. fGpu), so that it is destroyed
    // after all of its users. Clients of fTaskGroup will generally want to ensure that they call
    // wait() on it as they are being destroyed, to avoid the possibility of pending tasks being
    // invoked after objects they depend upon have already been destroyed.
    std::unique_ptr<SkTaskGroup>            fTaskGroup;
    std::unique_ptr<GrStrikeCache>          fStrikeCache;
    sk_sp<GrGpu>                            fGpu;
    std::unique_ptr<GrResourceCache>        fResourceCache;
    std::unique_ptr<GrResourceProvider>     fResourceProvider;

    bool                                    fDidTestPMConversions;
    // true if the PM/UPM conversion succeeded; false otherwise
    bool                                    fPMUPMConversionsRoundTrip;

    GrContextOptions::PersistentCache*      fPersistentCache;
    GrContextOptions::ShaderErrorHandler*   fShaderErrorHandler;

    std::unique_ptr<GrClientMappedBufferManager> fMappedBufferManager;

    // TODO: have the GrClipStackClip use renderTargetContexts and rm this friending
    friend class GrContextPriv;

    using INHERITED = GrRecordingContext;
};

#endif
