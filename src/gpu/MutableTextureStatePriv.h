/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MutableTextureStatePriv_DEFINED
#define skgpu_MutableTextureStatePriv_DEFINED

#include "include/core/SkTypes.h"
#include "include/gpu/MutableTextureState.h"

namespace skgpu {
enum class BackendApi : unsigned int;

class MutableTextureStateData {
public:
    virtual ~MutableTextureStateData();

#if defined(SK_DEBUG)
    virtual BackendApi type() const = 0;
#endif
protected:
    MutableTextureStateData() = default;
    MutableTextureStateData(const MutableTextureStateData&) = default;

    using AnyStateData = MutableTextureState::AnyStateData;

private:
    friend class MutableTextureState;
    virtual void copyTo(AnyStateData&) const = 0;
};

class MutableTextureStatePriv final {
public:
    template <typename StateData>
    static MutableTextureState MakeMutableTextureState(BackendApi backend,
                                                       const StateData& data) {
        return MutableTextureState(backend, data);
    }

    static const MutableTextureStateData* GetStateData(const MutableTextureState& mts) {
        return mts.fStateData.get();
    }

    static const MutableTextureStateData* GetStateData(const MutableTextureState* mts) {
        SkASSERT(mts);
        return mts->fStateData.get();
    }

    static MutableTextureStateData* GetStateData(MutableTextureState* mts) {
        SkASSERT(mts);
        return mts->fStateData.get();
    }
};
}  // namespace skgpu

#endif
