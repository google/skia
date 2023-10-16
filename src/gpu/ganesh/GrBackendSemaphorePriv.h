/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSemaphorePriv_DEFINED
#define GrBackendSemaphorePriv_DEFINED

#include "include/core/SkTypes.h"
#include "include/gpu/GrBackendSemaphore.h"

enum class GrBackendApi : unsigned int;

class GrBackendSemaphoreData {
public:
    virtual ~GrBackendSemaphoreData();

#if defined(SK_DEBUG)
    virtual GrBackendApi type() const = 0;
#endif
protected:
    GrBackendSemaphoreData() = default;
    GrBackendSemaphoreData(const GrBackendSemaphoreData&) = default;

    using AnySemaphoreData = GrBackendSemaphore::AnySemaphoreData;

private:
    friend class GrBackendSemaphore;
    virtual void copyTo(AnySemaphoreData&) const = 0;
};

class GrBackendSemaphorePriv final {
public:
    template <typename SemaphoreData>
    static GrBackendSemaphore MakeGrBackendSemaphore(GrBackendApi backend,
                                                     const SemaphoreData& data) {
        return GrBackendSemaphore(backend, data);
    }

    static const GrBackendSemaphoreData* GetBackendData(const GrBackendSemaphore& sem) {
        return sem.fSemaphoreData.get();
    }
};

#endif
