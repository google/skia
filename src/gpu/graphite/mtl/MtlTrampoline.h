/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlTrampoline_DEFINED
#define skgpu_graphite_MtlTrampoline_DEFINED

#include "include/core/SkRefCnt.h"

#include <memory>

namespace skgpu {
class SingleOwner;
}

namespace skgpu::graphite {
struct ContextOptions;
class GlobalCache;
struct MtlBackendContext;
class QueueManager;
class ResourceProvider;
class SharedContext;

/*
 * This class is used to hold functions which trampoline from the Graphite cpp code
 * to the Mtl Objective-C files.
 */
class MtlTrampoline {
public:
    static sk_sp<SharedContext> MakeSharedContext(const MtlBackendContext&, const ContextOptions&);
    static std::unique_ptr<QueueManager> MakeQueueManager(const SharedContext*);
    static std::unique_ptr<ResourceProvider> MakeResourceProvider(const SharedContext*,
                                                                  sk_sp<GlobalCache>,
                                                                  SingleOwner*);

};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlTrampoline_DEFINED

