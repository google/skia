/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTClusterer_DEFINED
#define GrTClusterer_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkTHash.h"
#include "src/core/SkScopeExit.h"
#include "src/core/SkSpan.h"
#include "src/core/SkTInternalLList.h"

#ifdef SK_DEBUG
template <typename T, typename Traits>
SkString GrTCluster_DebugStr(T* t) {
    if (Traits::NumTargets(t) != 1) {
        return SkStringPrintf("%d", Traits::GetID(t));
    } else {
        return SkStringPrintf("%d(%d)", Traits::GetID(t), Traits::GetTarget(t, 0));
    }
}
template <typename T, typename Traits>
SkString GrTCluster_DebugStr(SkSpan<const sk_sp<T>> collection) {
    SkString s;
    for (const sk_sp<T>& t_sp : collection) {
        s.appendf("%s ", GrTCluster_DebugStr<T, Traits>(t_sp.get()).c_str());
    }
    return s;
}
template <typename T, typename Traits>
SkString GrTCluster_DebugStr(const SkTInternalLList<T>& collection) {
    SkString s;
    for (T* t : collection) {
        s.appendf("%s ", GrTCluster_DebugStr<T, Traits>(t).c_str());
    }
    return s;
}
#endif

// Uncomment to get lots of logging.
#define CLUSTER_DEBUGF(...) //SkDebugf(__VA_ARGS__)

// Returns whether reordering occurred.
template <typename T, typename Traits>
bool GrTCluster_Visit(T* task, SkTInternalLList<T>* llist,
                      SkTHashMap<uint32_t, T*>* lastTaskMap) {
    SkScopeExit exit([&]{
        llist->addToTail(task);
        CLUSTER_DEBUGF("Cluster: Output order is now: %s\n",
                       GrTCluster_DebugStr<T, Traits>(*llist).c_str());
    });
    CLUSTER_DEBUGF("Cluster: ***Step***\nLooking at %s\n",
                   GrTCluster_DebugStr<T, Traits>(task).c_str());
    if (Traits::NumTargets(task) != 1) {
        CLUSTER_DEBUGF("Cluster: %d targets. Emitting barriers.\n", Traits::NumTargets(task));
        // Tasks with 0 or multiple targets are treated as full barriers
        // for all their targets.
        for (int j = 0; j < Traits::NumTargets(task); j++) {
            lastTaskMap->remove(Traits::GetTarget(task, j));
        }
        return false;
    }

    uint32_t target = Traits::GetTarget(task, 0);
    T* lastTask = (lastTaskMap->find(target) ? *lastTaskMap->find(target) : nullptr);
    lastTaskMap->set(target, task);

    if (!lastTask) {
        CLUSTER_DEBUGF("Cluster: Bail: No lastTask.\n", SkToInt(target));
        return false;
    }

    CLUSTER_DEBUGF("Cluster: lastTask is %s.\n", GrTCluster_DebugStr<T, Traits>(lastTask).c_str());

    if (lastTask == llist->tail()) {
        CLUSTER_DEBUGF("Cluster: Bail: lastTask already adjacent.\n");
        return false;
    }

    // Find the "new next" aka the first task in the chain for this surface, that we're reordering
    // other surface tasks behind.
    // For instance, we may currently have [B1 A1 A2 C1] and be looking at A3. We want to try
    // moving C1 out before A1, so A1 is the "new next."
    T* newNext = lastTask;
    while (newNext->fPrev
           && 1 == Traits::NumTargets(newNext->fPrev)
           && target == Traits::GetTarget(newNext->fPrev, 0)) {
        newNext = newNext->fPrev;
    }

    // Collect all deps for the cluster.
    SkTHashSet<T*> blockingDeps;
    for (T* passed = newNext; passed->fPrev != lastTask; passed = passed->fNext) {
        blockingDeps.add(passed);
    }

    // Can't reorder any tasks behind these deps.
    for (T* moved = lastTask->fNext; moved; moved = moved->fNext) {
        for (int j = 0; j < Traits::NumDependencies(moved); j++) {
            if (blockingDeps.contains(Traits::Dependency(moved, j))) {
                CLUSTER_DEBUGF("Cluster: Bail, %s depends on %s.\n",
                               GrTCluster_DebugStr<T, Traits>(moved).c_str(),
                               GrTCluster_DebugStr<T, Traits>(Traits::Dependency(moved, j)).c_str());
                return false;
            }
        }
    }

    // Grab the tasks between last and tail, and pull them before newNext.
    for (T* moved = lastTask->fNext; moved;) {
        CLUSTER_DEBUGF("Cluster: Reorder %s behind %s.\n",
                       GrTCluster_DebugStr<T, Traits>(moved).c_str(),
                       GrTCluster_DebugStr<T, Traits>(newNext).c_str());
        T* nextMoved = moved->fNext;
        llist->remove(moved);
        llist->addBefore(moved, newNext);
        moved = nextMoved;
    }
    return true;
}

#ifdef SK_DEBUG
template<typename T, typename Traits>
void GrTCluster_Validate(SkSpan<const sk_sp<T>> input, const SkTInternalLList<T>& llist) {
    // Check that we didn't break dependencies.
    SkTHashSet<T*> seen;
    for (T* t = llist.head(); t; t = t->fNext) {
        seen.add(t);
        for (int i = 0; i < Traits::NumDependencies(t); i++) {
            T* dep = Traits::Dependency(t, i);
            SkASSERTF(seen.contains(dep),
                      "%s came before dependency %s",
                      GrTCluster_DebugStr<T, Traits>(t).c_str(),
                      GrTCluster_DebugStr<T, Traits>(dep).c_str());
        }
    }
    // Check that llist has the same entries as the input.
    for (const auto& orig : input) {
        seen.remove(orig.get());
    }
    SkASSERT(seen.empty());
}
#endif

// Take a topologically-sorted DAG and cluster the entries together while maintaining the
// dependency order.
//
// If no clustering is possible, returns false.
// Otherwise, returns true and populates the provided llist as such:
//   - Contains the same set of entries as `input`.
//   - Obeys the dependency rules in `input`.
//   - Places entries with the same Target (see Traits) adjacent to each other.
//   - Entries with multiple targets act as reordering barriers for all their targets.
//
// T must declare a public SkTInternalLList interface.
//
// Traits requires:
//
//   static int NumTargets(const T* t) { ... }      //
//   static uint32_t GetTarget(const T* t, int i)   //
//   static int NumDependencies(const T* t) { ... } //
//   static T* Dependency(T* t, int index) { ... }  //
//   static uint32_t GetID(T* t) { ... }            //
template <typename T, typename Traits = T>
bool GrTCluster(SkSpan<const sk_sp<T>> input, SkTInternalLList<T>* llist) {
    SkASSERT(llist->isEmpty());

    if (input.count() <= 2) {
        return false;
    }

    CLUSTER_DEBUGF("Cluster: Original order is %s\n",
                   GrTCluster_DebugStr<T, Traits>(input).c_str());

    SkTHashMap<uint32_t, T*> lastTaskMap;
    bool didReorder = false;
    for (const auto& t : input) {
        didReorder |= GrTCluster_Visit<T, Traits>(t.get(), llist, &lastTaskMap);
    }

#ifdef SK_DEBUG
    if (didReorder) {
        GrTCluster_Validate<T, Traits>(input, *llist);
    }
#endif

    return didReorder;
}

#undef CLUSTER_DEBUGF

#endif
