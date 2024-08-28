/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/ganesh/vk/GrVkBackendSemaphore.h"

#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "src/gpu/ganesh/GrBackendSemaphorePriv.h"

class GrVkBackendSemaphoreData final : public GrBackendSemaphoreData {
public:
    GrVkBackendSemaphoreData(VkSemaphore semaphore) : fSemaphore(semaphore) {}

    VkSemaphore semaphore() const { return fSemaphore; }

private:
    void copyTo(AnySemaphoreData& data) const override {
        data.emplace<GrVkBackendSemaphoreData>(fSemaphore);
    }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kVulkan; }
#endif

    VkSemaphore fSemaphore;
};

static const GrVkBackendSemaphoreData* get_and_cast_data(const GrBackendSemaphore& sem) {
    auto data = GrBackendSemaphorePriv::GetBackendData(sem);
    SkASSERT(!data || data->type() == GrBackendApi::kVulkan);
    return static_cast<const GrVkBackendSemaphoreData*>(data);
}

namespace GrBackendSemaphores {
GrBackendSemaphore MakeVk(VkSemaphore semaphore) {
    GrVkBackendSemaphoreData data(semaphore);
    return GrBackendSemaphorePriv::MakeGrBackendSemaphore(GrBackendApi::kVulkan, data);
}

VkSemaphore GetVkSemaphore(const GrBackendSemaphore& sem) {
    SkASSERT(sem.backend() == GrBackendApi::kVulkan);
    const GrVkBackendSemaphoreData* data = get_and_cast_data(sem);
    SkASSERT(data);
    return data->semaphore();
}
}  // namespace GrBackendSemaphores
