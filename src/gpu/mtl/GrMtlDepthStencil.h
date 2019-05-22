/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlDepthStencil_DEFINED
#define GrMtlDepthStencil_DEFINED

#import <metal/metal.h>

#include "include/gpu/GrTypes.h"
#include "src/core/SkOpts.h"
#include <atomic>

class GrMtlGpu;
class GrStencilSettings;

// A wrapper for a MTLDepthStencilState object with caching support.
class GrMtlDepthStencil : public SkRefCnt {
public:
    static GrMtlDepthStencil* Create(const GrMtlGpu*, const GrStencilSettings&, GrSurfaceOrigin);

    ~GrMtlDepthStencil() { fMtlDepthStencilState = nil; }

    id<MTLDepthStencilState> mtlDepthStencil() const { return fMtlDepthStencilState; }

    struct Key {
        struct Face {
            uint32_t fReadMask;
            uint32_t fWriteMask;
            uint32_t fOps;
        };
        Face fFront;
        Face fBack;

        bool operator==(const Key& that) const {
            return this->fFront.fReadMask == that.fFront.fReadMask &&
                   this->fFront.fWriteMask == that.fFront.fWriteMask &&
                   this->fFront.fOps == that.fFront.fOps &&
                   this->fBack.fReadMask == that.fBack.fReadMask &&
                   this->fBack.fWriteMask == that.fBack.fWriteMask &&
                   this->fBack.fOps == that.fBack.fOps;
        }
    };

    // Helpers for hashing GrMtlSampler
    static Key GenerateKey(const GrStencilSettings&, GrSurfaceOrigin);

    static const Key& GetKey(const GrMtlDepthStencil& depthStencil) { return depthStencil.fKey; }
    static uint32_t Hash(const Key& key) {
        return SkOpts::hash(reinterpret_cast<const uint32_t*>(&key), sizeof(Key));
    }

private:
    GrMtlDepthStencil(id<MTLDepthStencilState> mtlDepthStencilState, Key key)
        : fMtlDepthStencilState(mtlDepthStencilState)
        , fKey(key) {}

    id<MTLDepthStencilState> fMtlDepthStencilState;
    Key                      fKey;
};

#endif
