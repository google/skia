/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrRenderTaskCluster.h"

#include "src/core/SkTHash.h"
#include "src/gpu/ganesh/GrRenderTask.h"

using namespace skia_private;

// Uncomment to get lots of logging.
#define CLUSTER_DEBUGF(...) //SkDebugf(__VA_ARGS__)

static GrSurfaceProxy* first_target(GrRenderTask* task) { return task->target(0); }

#ifdef SK_DEBUG
[[maybe_unused]] static SkString describe_task(GrRenderTask* t) {
    if (GrSurfaceProxy* target = first_target(t)) {
        return SkStringPrintf("%s(%u)", target->getDebugName().c_str(), t->uniqueID());
    } else {
        return SkStringPrintf("%u", t->uniqueID());
    }
}

[[maybe_unused]] static SkString describe_tasks(SkSpan<const sk_sp<GrRenderTask>> collection) {
    SkString s;
    for (const sk_sp<GrRenderTask>& t : collection) {
        s.appendf("%s ", describe_task(t.get()).c_str());
    }
    return s;
}

[[maybe_unused]] static SkString describe_tasks(const SkTInternalLList<GrRenderTask>& collection) {
    SkString s;
    for (GrRenderTask* t : collection) {
        s.appendf("%s ", describe_task(t).c_str());
    }
    return s;
}

static void validate(SkSpan<const sk_sp<GrRenderTask>> input,
                     const SkTInternalLList<GrRenderTask>& llist) {
    // Check that we didn't break dependencies.
    THashSet<GrRenderTask*> seen;
    for (GrRenderTask* t : llist) {
        seen.add(t);
        for (GrRenderTask* dep : t->dependencies()) {
            SkASSERTF(seen.contains(dep),
                      "%s came before dependency %s",
                      describe_task(t).c_str(),
                      describe_task(dep).c_str());
        }
    }
    // Check that llist has the same entries as the input.
    for (const auto& orig : input) {
        seen.remove(orig.get());
    }
    SkASSERT(seen.empty());
}

#endif  // SK_DEBUG

// Returns whether `dependee` is a formal dependent or if it uses a surface `depender` targets.
static bool depends_on(GrRenderTask* depender, GrRenderTask* dependee) {
    // Check if depender writes to something dependee reads.
    // TODO: Establish real DAG dependencies for this during recording? E.g. when a task adds a
    // target, search backward for the last task that uses the target and add a dep.
    for (int i = 0; i < depender->numTargets(); i++) {
        if (dependee->isUsed(depender->target(i))) {
            CLUSTER_DEBUGF("Cluster: Bail, %s can't write before %s reads from %s.\n",
                           describe_task(depender).c_str(),
                           describe_task(dependee).c_str(),
                           depender->target(i)->getDebugName().c_str());
            return true;
        }
    }
    // Check for a formal dependency.
    if (depender->dependsOn(dependee)) {
        CLUSTER_DEBUGF("Cluster: Bail, %s depends on %s.\n",
                       describe_task(depender).c_str(),
                       describe_task(dependee).c_str());
        return true;
    }
    return false;
}

// Returns whether reordering occurred.
static bool task_cluster_visit(GrRenderTask* task, SkTInternalLList<GrRenderTask>* llist,
                               THashMap<GrSurfaceProxy*, GrRenderTask*>* lastTaskMap) {
    CLUSTER_DEBUGF("Cluster: ***Step***\nLooking at %s\n",
                   describe_task(task).c_str());
    if (task->numTargets() != 1) {
        CLUSTER_DEBUGF("Cluster: %d targets. Emitting barriers.\n", task->numTargets());
        // Tasks with 0 or multiple targets are treated as full barriers
        // for all their targets.
        for (int j = 0; j < task->numTargets(); j++) {
            if (lastTaskMap->find(task->target(0))) {
                lastTaskMap->remove(task->target(0));
            }
        }
        return false;
    }

    GrSurfaceProxy* target = first_target(task);
    GrRenderTask* clusterTail = (lastTaskMap->find(target) ? *lastTaskMap->find(target) : nullptr);
    lastTaskMap->set(target, task);

    if (!clusterTail) {
        CLUSTER_DEBUGF("Cluster: Bail, no cluster to extend.\n");
        return false;
    }

    CLUSTER_DEBUGF("Cluster: clusterTail is %s.\n", describe_task(clusterTail).c_str());

    if (clusterTail == llist->tail()) {
        CLUSTER_DEBUGF("Cluster: Bail, cluster is already tail.\n");
        return false;
    }
    GrRenderTask* movedHead = clusterTail->fNext;

    // Now, let's refer to the "cluster" as the chain of tasks with the same target, that we're
    // hoping to extend by reordering. Let "moved tasks" be the ones we want to move to extend the
    // cluster.
    GrRenderTask* clusterHead = clusterTail;
    while (clusterHead->fPrev
           && 1 == clusterHead->fPrev->numTargets()
           && target == first_target(clusterHead->fPrev)) {
        clusterHead = clusterHead->fPrev;
    }

    // We can't reorder if any moved task depends on anything in the cluster.
    // Time complexity here is high, but making a hash set is worse in profiling.
    for (GrRenderTask* moved = movedHead; moved; moved = moved->fNext) {
        for (GrRenderTask* passed = clusterHead; passed != movedHead; passed = passed->fNext) {
            if (depends_on(moved, passed)) {
                return false;
            }
        }
    }

    // Grab the moved tasks and pull them before clusterHead.
    for (GrRenderTask* moved = movedHead; moved;) {
        CLUSTER_DEBUGF("Cluster: Reorder %s behind %s.\n",
                       describe_task(moved).c_str(),
                       describe_task(clusterHead).c_str());
        // Be careful to save fNext before each move.
        GrRenderTask* nextMoved = moved->fNext;
        llist->remove(moved);
        llist->addBefore(moved, clusterHead);
        moved = nextMoved;
    }
    return true;
}

bool GrClusterRenderTasks(SkSpan<const sk_sp<GrRenderTask>> input,
                          SkTInternalLList<GrRenderTask>* llist) {
    SkASSERT(llist->isEmpty());

    if (input.size() < 3) {
        for (const auto& t : input) {
            llist->addToTail(t.get());
        }
        return false;
    }

    CLUSTER_DEBUGF("Cluster: Original order is %s\n", describe_tasks(input).c_str());

    THashMap<GrSurfaceProxy*, GrRenderTask*> lastTaskMap;
    bool didReorder = false;
    for (const auto& t : input) {
        didReorder |= task_cluster_visit(t.get(), llist, &lastTaskMap);
        llist->addToTail(t.get());
        CLUSTER_DEBUGF("Cluster: Output order is now: %s\n", describe_tasks(*llist).c_str());
    }

#ifdef SK_DEBUG
    if (didReorder) {
        validate(input, *llist);
    }
#endif

    return didReorder;
}

#undef CLUSTER_DEBUGF
