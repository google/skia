/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlSemaphore_DEFINED
#define GrMtlSemaphore_DEFINED

#include "include/gpu/ganesh/GrBackendSemaphore.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrManagedResource.h"
#include "src/gpu/ganesh/GrSemaphore.h"
#include "src/gpu/ganesh/mtl/GrMtlUtil.h"

#include <Metal/Metal.h>

class GrMtlGpu;

class GrMtlEvent : public GrManagedResource {
public:
    static sk_sp<GrMtlEvent> Make(GrMtlGpu* gpu);

    static sk_sp<GrMtlEvent> MakeWrapped(GrMTLHandle event);

    ~GrMtlEvent() override {}

    id<MTLEvent> mtlEvent() const SK_API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0)) {
        return fMtlEvent;
    }

#ifdef SK_TRACE_MANAGED_RESOURCES
    /** output a human-readable dump of this resource's information
     */
    void dumpInfo() const override {
        if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, *)) {
            SkDebugf("GrMtlEvent: %p (%ld refs)\n", fMtlEvent,
                     CFGetRetainCount((CFTypeRef)fMtlEvent));
        }
    }
#endif

    void freeGPUData() const override {
        if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, *)) {
            fMtlEvent = nil;
        }
    }

private:
    GrMtlEvent(id<MTLEvent> mtlEvent) SK_API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0))
        : fMtlEvent(mtlEvent) {}

    mutable id<MTLEvent> fMtlEvent SK_API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0));
};

class GrMtlSemaphore : public GrSemaphore {
public:
    static std::unique_ptr<GrMtlSemaphore> Make(GrMtlGpu* gpu) {
        sk_sp<GrMtlEvent> event = GrMtlEvent::Make(gpu);
        if (!event) {
            return nullptr;
        }
        return std::unique_ptr<GrMtlSemaphore>(new GrMtlSemaphore(std::move(event), 1));
    }

    static std::unique_ptr<GrMtlSemaphore> MakeWrapped(GrMTLHandle mtlEvent, uint64_t value) {
        sk_sp<GrMtlEvent> event = GrMtlEvent::MakeWrapped(mtlEvent);
        if (!event) {
            return nullptr;
        }
        return std::unique_ptr<GrMtlSemaphore>(new GrMtlSemaphore(std::move(event), value));
    }

    ~GrMtlSemaphore() override {}

    sk_sp<GrMtlEvent> event() { return fEvent; }
    uint64_t value() const { return fValue; }

    GrBackendSemaphore backendSemaphore() const override;

private:
    GrMtlSemaphore(sk_sp<GrMtlEvent> event, uint64_t value)
        : fEvent(std::move(event))
        , fValue(value) {}

    void setIsOwned() override {}

    sk_sp<GrMtlEvent> fEvent;
    uint64_t fValue;

    using INHERITED = GrSemaphore;
};

#endif
