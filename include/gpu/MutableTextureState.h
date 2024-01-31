/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MutableTextureState_DEFINED
#define skgpu_MutableTextureState_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkAnySubclass.h"

#include <cstddef>

namespace skgpu {
enum class BackendApi : unsigned int;
class MutableTextureStateData;

/**
 * Since Skia and clients can both modify gpu textures and their connected state, Skia needs a way
 * for clients to inform us if they have modifiend any of this state. In order to not need setters
 * for every single API and state, we use this class to be a generic wrapper around all the mutable
 * state. This class is used for calls that inform Skia of these texture/image state changes by the
 * client as well as for requesting state changes to be done by Skia. The backend specific state
 * that is wrapped by this class are located in files like:
 *   - include/gpu/vk/VulkanMutableTextureState.h
 */
class SK_API MutableTextureState : public SkRefCnt {
public:
    MutableTextureState();
    ~MutableTextureState() override;

    MutableTextureState(const MutableTextureState& that);

    MutableTextureState& operator=(const MutableTextureState& that);

    void set(const MutableTextureState& that);

    BackendApi backend() const { return fBackend; }

    // Returns true if the backend mutable state has been initialized.
    bool isValid() const { return fIsValid; }

private:
    friend class MutableTextureStateData;
    friend class MutableTextureStatePriv;
    // Size determined by looking at the MutableTextureStateData subclasses, then
    // guessing-and-checking. Compiler will complain if this is too small - in that case,
    // just increase the number.
    inline constexpr static size_t kMaxSubclassSize = 16;
    using AnyStateData = SkAnySubclass<MutableTextureStateData, kMaxSubclassSize>;

    template <typename StateData>
    MutableTextureState(BackendApi api, const StateData& data) : fBackend(api), fIsValid(true) {
        fStateData.emplace<StateData>(data);
    }

    AnyStateData fStateData;

    BackendApi fBackend;
    bool fIsValid;
};

} // namespace skgpu

#endif // skgpu_MutableTextureState_DEFINED
