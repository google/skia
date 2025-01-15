/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ScratchResourceManager_DEFINED
#define skgpu_graphite_ScratchResourceManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkTHash.h"

#include <string_view>

namespace skgpu::graphite {

class Resource;
class ResourceProvider;
class Texture;
class TextureInfo;
class TextureProxy;

// NOTE: This is temporary while atlas management requires flushing an entire Recorder. That
// can break a scratch Device into multiple DrawTasks and the proxy read count needs to count
// all reads regardless of which DrawTask is referenced. Once scratch devices only produce a
// single DrawTask, DrawTask can hold the pending read count directly.
class ProxyReadCountMap {
public:
    ProxyReadCountMap() = default;

    void increment(const TextureProxy* proxy) {
        int* count = fCounts.find(proxy);
        if (!count) {
            count = fCounts.set(proxy, 0);
        }
        (*count)++;
    }

    bool decrement(const TextureProxy* proxy) {
        int* count = fCounts.find(proxy);
        SkASSERT(count && *count > 0);
        (*count)--;
        return *count == 0;
    }

    int get(const TextureProxy* proxy) const {
        const int* count = fCounts.find(proxy);
        return count ? *count : 0;
    }

    SkDEBUGCODE(bool hasPendingReads() const;)

private:
    skia_private::THashMap<const TextureProxy*, int> fCounts;
};

/**
 * ScratchResourceManager helps coordinate the reuse of resources *within* a Recording that would
 * not otherwise be returned from the ResourceProvider/Cache because the Recorder is holds usage
 * refs on the resources and they are typically not Shareable.
 *
 * A ScratchResourceManager maintains a pool of resources that have been handed out for some use
 * case and then been explicitly returned by the original holder. It is up to the callers to
 * return resources in an optimal manner (for best reuse) and not use them after they've been
 * returned for a later task's use. To help callers manage when they can return resources,
 * the manager maintains a stack that corresponds with the depth-first traversal of the tasks
 * during prepareResources() and provides hooks to register listeners that are invoked when tasks
 * read or sample resources.
 *
 * Once all uninstantiated resources are assigned and prepareResources() succeeds, the
 * ScratchResourceManager can be discarded. The reuse within a Recording's task graph is fixed at
 * that point and remains valid even if the recording is replayed.
 */
class ScratchResourceManager {
public:
    ScratchResourceManager(ResourceProvider* resourceProvider,
                           std::unique_ptr<ProxyReadCountMap>);
    ~ScratchResourceManager();

    // Get a scratch texture with the given size and texture info. The returned texture will
    // not be reusable until the caller invokes `returnResource()`. At that point, subsequent
    // compatible calls to getScratchTexture() may return the texture. If there is no compatible
    // available texture to be reused, the ResourceProvider will be used to find or create one.
    //
    // It is the caller's responsibility to determine when it's acceptable to return a resource.
    // That said, it's not mandatory that the scratch resources be returned. In that case, they just
    // stop being available for reuse for later tasks in a Recording.
    sk_sp<Texture> getScratchTexture(SkISize, const TextureInfo&, std::string_view label);

    // TODO: Eventually update ScratchBuffer and DrawBufferManager to leverage the
    // ScratchResourceManager. There are a few open issues to address first:
    //  - ScratchBuffer uses RAII to return the resource; ScratchResourceManager could adopt this
    //    for buffers but that may only make sense if textures could also operate that way.
    //    Alternatively, ScratchBuffer remains an RAII abstraction on top of ScratchResourceManager.
    //  - ScratchResourceManager is currently only available in snap(), but DrawBufferManager needs
    //    to be available at all times because a DrawPass could be created whenever. b/335644795
    //    considers moving all DrawPass creation into snap() so that would avoid this issue.
    //    Alternatively, ScratchResourceManager could have the same lifetime as the buffer manager.

    // Mark the resource as available for reuse. Must have been previously returned by this manager.
    // If the caller does not ensure that all of its uses of the resource are prepared before
    // tasks that are processed after this call, then undefined results can occur.
    void returnTexture(sk_sp<Texture>);

    // Graphite accumulates tasks into a graph (implicit dependencies defined by the order they are
    // added to the root task list, or explicitly when appending child tasks). The depth-first
    // traversal of this graph helps impose constraints on the read/write windows of resources. To
    // help Tasks with this tracking, ScratchResourceManager maintains a stack of lists of "pending
    // uses".
    //
    // Each recursion in the depth-first traversal of the task graph pushes the stack. Going up
    // pops the stack. A "pending use" allows a task that modifies a resource to register a
    // listener that is triggered when either its scope is popped off or a consuming task that
    // reads that resource notifies the ScratchResourceManager (e.g. a RenderPassTask or CopyTask
    // that sample a scratch texture). Internally, the listeners can decrement a pending read count
    // or otherwise determine when to call returnResource() without having to be coupled directly to
    // the consuming tasks.
    //
    // When a task calls notifyResourcesConsumed(), all "pending use" listeners in the current
    // scope are invoked and removed from the list. This means that tasks must be externally
    // organized such that only the tasks that prepare the scratch resources for that consuming task
    // are at the same depth. Intermingling writes to multiple scratch textures before they are
    // sampled by separate renderpasses would mean that all the scratch textures could be returned
    // for reuse at the first renderpass. Instead, a TaskList can be used to group the scratch
    // writes with the renderpass that samples it to introduce a scope in the stack. Alternatively,
    // if the caller constructs a single list directly to avoid this issue, the extra stack
    // manipulation can be avoided.
    class PendingUseListener {
    public:
        virtual ~PendingUseListener() {}

        virtual void onUseCompleted(ScratchResourceManager*) = 0;
    };

    // Push a new scope onto the stack, preventing previously added pending listeners from being
    // invoked when a task consumes resources.
    void pushScope();

    // Pop the current scope off the stack. This does not invoke any pending listeners that were
    // not consumed by a task within the ending scope. This can happen if an offscreen layer is
    // flushed in a Recording snap() before it's actually been drawn to its target. That final draw
    // can then happen in a subsequent Recording even. By not invoking the pending listener, it will
    // not return the scratch resource, correctly keeping it in use across multiple Recordings.
    // TODO: Eventually, the above scenario should not happen, but that requires atlasing to not
    // force a flush of every Device. Once that is the case, popScope() can ideally assert that
    // there are no more pending listeners to invoke (otherwise it means the tasks were linked
    // incorrectly).
    void popScope();

    // Invoked by tasks that sample from or read from resources. All pending listeners that were
    // marked in the current scope will be invoked.
    void notifyResourcesConsumed();

    // Register a listener that will be invoked on the next call to notifyResourcesConsumed() or
    // popScope() within the current scope. Registering the same listener multiple times will invoke
    // it multiple times.
    //
    // The ScratchResourceManager does not take ownership of these listeners; they are assumed to
    // live for as long as the prepareResources() phase of snapping a Recording.
    void markResourceInUse(PendingUseListener* listener);

    // Temporary access to the proxy read counts stored in the ScratchResourceManager
    int pendingReadCount(const TextureProxy* proxy) const {
        return fProxyReadCounts->get(proxy);
    }

    // Returns true if the read count reached zero; must only be called if it was > 0 previously.
    bool removePendingRead(const TextureProxy* proxy) {
        return fProxyReadCounts->decrement(proxy);
    }

private:
    ResourceProvider* fResourceProvider;

    // ScratchResourceManager holds a pointer to each un-returned scratch resource that it's
    // fetched from the ResourceProvider that should be considered unavailable when making
    // additional resource requests from the cache with compatible keys. They are bare pointers
    // because the resources are kept alive by the proxies and tasks that queried the
    // ScratchResourceManager.
    // NOTE: This is the same type as ResourceCache::ScratchResourceSet but cannot be forward
    // declared because it's both a template and an inner type.
    skia_private::THashSet<const Resource*> fUnavailable;

    // This single list is organized into a stack of sublists by using null pointers to mark the
    // start of a new scope.
    skia_private::TArray<PendingUseListener*> fListenerStack;

    std::unique_ptr<ProxyReadCountMap> fProxyReadCounts;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ResourceReuseManager_DEFINED
