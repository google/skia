
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

 /** Check if the argument is non-null, and if so, call obj->AddRef() and return obj.
  */
template <typename T> static inline T* GrSafeComAddRef(T* obj) {
    if (obj) {
        obj->AddRef();
    }
    return obj;
}

/** Check if the argument is non-null, and if so, call obj->Release()
 */
template <typename T> static inline void GrSafeComRelease(T* obj) {
    if (obj) {
        obj->Release();
    }
}

template <typename T> class gr_cp {
public:
    using element_type = T;

    constexpr gr_cp() : fObject(nullptr) {}

    /**
     *  Shares the underlying object by calling AddRef(), so that both the argument and the newly
     *  created gr_cp both have a reference to it.
     */
    gr_cp(const gr_cp<T>& that) : fObject(GrSafeComAddRef(that.get())) {}

    /**
     *  Move the underlying object from the argument to the newly created gr_cp. Afterwards only
     *  the new gr_cp will have a reference to the object, and the argument will point to null.
     *  No call to AddRef() or Release() will be made.
     */
    gr_cp(gr_cp<T>&& that) : fObject(that.release()) {}

    /**
     *  Adopt the bare object into the newly created gr_cp.
     *  No call to AddRef() or Release() will be made.
     */
    explicit gr_cp(T* obj) {
        fObject = obj;
    }

    /**
     *  Calls Release() on the underlying object pointer.
     */
    ~gr_cp() {
        GrSafeComRelease(fObject);
        SkDEBUGCODE(fObject = nullptr);
    }

    /**
     *  Shares the underlying object referenced by the argument by calling AddRef() on it. If this
     *  gr_cp previously had a reference to an object (i.e. not null) it will call Release()
     *  on that object.
     */
    gr_cp<T>& operator=(const gr_cp<T>& that) {
        if (this != &that) {
            this->reset(GrSafeComAddRef(that.get()));
        }
        return *this;
    }

    /**
     *  Move the underlying object from the argument to the gr_cp. If the gr_cp
     *  previously held a reference to another object, Release() will be called on that object.
     *  No call to AddRef() will be made.
     */
    gr_cp<T>& operator=(gr_cp<T>&& that) {
        this->reset(that.release());
        return *this;
    }

    T* get() const { return fObject; }
    T* operator->() const { return fObject; }
    T** operator&() { return &fObject; }

    /**
     *  Adopt the new object, and call Release() on any previously held object (if not null).
     *  No call to AddRef() will be made.
     */
    void reset(T* object = nullptr) {
        T* oldObject = fObject;
        fObject = object;
        GrSafeComRelease(oldObject);
    }

    /**
     *  Shares the new object by calling AddRef() on it. If this gr_cp previously had a
     *  reference to an object (i.e. not null) it will call Release() on that object.
     */
    void retain(T* object) {
        if (this->fObject != object) {
            this->reset(GrSafeComAddRef(object));
        }
    }

    /**
     *  Return the original object, and set the internal object to nullptr.
     *  The caller must assume ownership of the object, and manage its reference count directly.
     *  No call to Release() will be made.
     */
    T* SK_WARN_UNUSED_RESULT release() {
        T* obj = fObject;
        fObject = nullptr;
        return obj;
    }

private:
    T* fObject;
};

template <typename T> inline bool operator==(const gr_cp<T>& a,
                                             const gr_cp<T>& b) {
    return a.get() == b.get();
}

template <typename T> inline bool operator!=(const gr_cp<T>& a,
                                             const gr_cp<T>& b) {
    return a.get() != b.get();
}

// Note: there is no notion of Borrowed or Adopted resources in the D3D backend,
// so Ganesh will ref fResource once it's asked to wrap it.
// Clients are responsible for releasing their own ref to avoid memory leaks.
struct GrD3DTextureResourceInfo {
    gr_cp<ID3D12Resource>    fResource;
    D3D12_RESOURCE_STATES    fResourceState;
    DXGI_FORMAT              fFormat;
    uint32_t                 fLevelCount;
    GrProtected              fProtected;

    GrD3DTextureResourceInfo()
            : fResource(nullptr)
            , fResourceState(D3D12_RESOURCE_STATE_COMMON)
            , fFormat(DXGI_FORMAT_UNKNOWN)
            , fLevelCount(0)
            , fProtected(GrProtected::kNo) {}

    GrD3DTextureResourceInfo(ID3D12Resource* resource,
                             D3D12_RESOURCE_STATES resourceState,
                             DXGI_FORMAT format,
                             uint32_t levelCount,
                             GrProtected isProtected = GrProtected::kNo)
            : fResource(resource)
            , fResourceState(resourceState)
            , fFormat(format)
            , fLevelCount(levelCount)
            , fProtected(isProtected) {}

    GrD3DTextureResourceInfo(const GrD3DTextureResourceInfo& info,
                             GrD3DResourceStateEnum resourceState)
            : fResource(info.fResource)
            , fResourceState(static_cast<D3D12_RESOURCE_STATES>(resourceState))
            , fFormat(info.fFormat)
            , fLevelCount(info.fLevelCount)
            , fProtected(info.fProtected) {}

#if GR_TEST_UTILS
    bool operator==(const GrD3DTextureResourceInfo& that) const {
        return fResource == that.fResource && fResourceState == that.fResourceState &&
               fFormat == that.fFormat && fLevelCount == that.fLevelCount &&
               fProtected == that.fProtected;
    }
#endif
};

#endif
