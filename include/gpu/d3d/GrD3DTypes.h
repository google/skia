
/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DTypes_DEFINED
#define GrD3DTypes_DEFINED

// This file includes d3d12.h, which in turn includes windows.h, which redefines many
// common identifiers such as:
// * interface
// * small
// * near
// * far
// * CreateSemaphore
// * MemoryBarrier
//
// You should only include this header if you need the Direct3D definitions and are
// prepared to rename those identifiers. Otherwise use GrD3DTypesMinimal.h.

#include "include/gpu/d3d/GrD3DTypesMinimal.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

// Note: there is no notion of Borrowed or Adopted resources in the D3D backend,
// so Ganesh will ref fResource once it's asked to wrap it.
// Clients are responsible for releasing their own ref to avoid memory leaks.
struct GrD3DTextureResourceInfo {
    ComPtr<ID3D12Resource>   fResource;
    D3D12_RESOURCE_STATES    fResourceState;
    DXGI_FORMAT              fFormat;
    uint32_t                 fLevelCount;
    unsigned int             fSampleQualityPattern;
    GrProtected              fProtected;

    GrD3DTextureResourceInfo()
            : fResource(nullptr)
            , fResourceState(D3D12_RESOURCE_STATE_COMMON)
            , fFormat(DXGI_FORMAT_UNKNOWN)
            , fLevelCount(0)
            , fSampleQualityPattern(DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN)
            , fProtected(GrProtected::kNo) {}

    GrD3DTextureResourceInfo(const ComPtr<ID3D12Resource>& resource,
                             D3D12_RESOURCE_STATES resourceState,
                             DXGI_FORMAT format,
                             uint32_t levelCount,
                             unsigned int sampleQualityLevel,
                             GrProtected isProtected = GrProtected::kNo)
            : fResource(resource)
            , fResourceState(resourceState)
            , fFormat(format)
            , fLevelCount(levelCount)
            , fSampleQualityPattern(sampleQualityLevel)
            , fProtected(isProtected) {}

    GrD3DTextureResourceInfo(const GrD3DTextureResourceInfo& info,
                             GrD3DResourceStateEnum resourceState)
            : fResource(info.fResource)
            , fResourceState(static_cast<D3D12_RESOURCE_STATES>(resourceState))
            , fFormat(info.fFormat)
            , fLevelCount(info.fLevelCount)
            , fSampleQualityPattern(info.fSampleQualityPattern)
            , fProtected(info.fProtected) {}

#if GR_TEST_UTILS
    bool operator==(const GrD3DTextureResourceInfo& that) const {
        return fResource == that.fResource && fResourceState == that.fResourceState &&
               fFormat == that.fFormat && fLevelCount == that.fLevelCount &&
               fSampleQualityPattern == that.fSampleQualityPattern && fProtected == that.fProtected;
    }
#endif
};

struct GrD3DFenceInfo {
    GrD3DFenceInfo()
        : fFence(nullptr)
        , fValue(0) {
    }

    ComPtr<ID3D12Fence> fFence;
    uint64_t            fValue;  // signal value for the fence
};

#endif
