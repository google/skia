/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPromiseImageTexture_DEFINED
#define SkPromiseImageTexture_DEFINED

#include "../private/GrResourceKey.h"
#include "GrBackendSurface.h"

#if SK_SUPPORT_GPU
/**
 * This type is used to fulfill textures for PromiseImages. Once an instance is returned from a
 * PromiseImageTextureFulfillProc it must remain valid until the corresponding
 * PromiseImageTextureReleaseProc is called. For performance reasons it is recommended that the
 * the client reuse a single PromiseImageTexture every time a given texture is returned by
 * the PromiseImageTextureFulfillProc rather than recreating PromiseImageTextures representing
 * the same underlying backend API texture.
 */
class SK_API SkPromiseImageTexture {
public:
    SkPromiseImageTexture() = default;
    SkPromiseImageTexture(const SkPromiseImageTexture&) = delete;
    explicit SkPromiseImageTexture(const GrBackendTexture& backendTexture);
    SkPromiseImageTexture(SkPromiseImageTexture&&);
    ~SkPromiseImageTexture();
    SkPromiseImageTexture& operator=(const SkPromiseImageTexture&) = delete;
    SkPromiseImageTexture& operator=(SkPromiseImageTexture&&);
    const GrBackendTexture& backendTexture() const { return fBackendTexture; }
    bool isValid() const { return SkToBool(fUniqueID); }

    void addKeyToInvalidate(uint32_t contextID, const GrUniqueKey& key);
    uint32_t uniqueID() const { return fUniqueID; }

#if GR_TEST_UTILS
    SkTArray<GrUniqueKey> testingOnly_uniqueKeysToInvalidate() const;
#endif

private:
    SkSTArray<1, GrUniqueKeyInvalidatedMessage> fMessages;
    GrBackendTexture fBackendTexture;
    uint32_t fUniqueID = SK_InvalidUniqueID;
    static std::atomic<uint32_t> gUniqueID;
};
#endif

#endif
