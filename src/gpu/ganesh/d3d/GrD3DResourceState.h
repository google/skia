/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DResourceState_DEFINED
#define GrD3DResourceState_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/d3d/GrD3DTypes.h"

class GrD3DResourceState : public SkRefCnt {
public:
    GrD3DResourceState(D3D12_RESOURCE_STATES state) : fState(state) {}

    void setResourceState(D3D12_RESOURCE_STATES state) {
        // Defaulting to use std::memory_order_seq_cst
        fState.store(state);
    }

    D3D12_RESOURCE_STATES getResourceState() const {
        // Defaulting to use std::memory_order_seq_cst
        return fState.load();
    }

private:
    std::atomic<D3D12_RESOURCE_STATES> fState;
};

#endif
