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
#include "SkRefCnt.h"

#if SK_SUPPORT_GPU
/**
 * This type is used to fulfill textures for PromiseImages. Once an instance is returned from a
 * PromiseImageTextureFulfillProc it must remain valid until the corresponding
 * PromiseImageTextureReleaseProc is called. For performance reasons it is recommended that the
 * the client reuse a single PromiseImageTexture every time a given texture is returned by
 * the PromiseImageTextureFulfillProc rather than recreating PromiseImageTextures representing
 * the same underlying backend API texture.
 */
class SK_API SkPromiseImageTexture : public SkNVRefCnt<SkPromiseImageTexture> {
public:
    SkPromiseImageTexture() = delete;
    SkPromiseImageTexture(const SkPromiseImageTexture&) = delete;
    SkPromiseImageTexture(SkPromiseImageTexture&&) = delete;
    ~SkPromiseImageTexture();
    SkPromiseImageTexture& operator=(const SkPromiseImageTexture&) = delete;
    SkPromiseImageTexture& operator=(SkPromiseImageTexture&&) = delete;

    static sk_sp<SkPromiseImageTexture> Make(const GrBackendTexture& backendTexture) {
        if (!backendTexture.isValid()) {
            return nullptr;
        }
        return sk_sp<SkPromiseImageTexture>(new SkPromiseImageTexture(backendTexture));
    }

    const GrBackendTexture& backendTexture() const { return fBackendTexture; }

    void addKeyToInvalidate(uint32_t contextID, const GrUniqueKey& key);
    uint32_t uniqueID() const { return fUniqueID; }

#if GR_TEST_UTILS
    SkTArray<GrUniqueKey> testingOnly_uniqueKeysToInvalidate() const;
#endif

private:
    explicit SkPromiseImageTexture(const GrBackendTexture& backendTexture);

    SkSTArray<1, GrUniqueKeyInvalidatedMessage> fMessages;
    GrBackendTexture fBackendTexture;
    uint32_t fUniqueID = SK_InvalidUniqueID;
    static std::atomic<uint32_t> gUniqueID;
};
#endif

#endif
