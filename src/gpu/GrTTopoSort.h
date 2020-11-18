/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTTopoSort_DEFINED
#define GrTTopoSort_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"

#ifdef SK_DEBUG
template <typename T, typename Traits = T>
void GrTTopoSort_CheckAllUnmarked(const SkTArray<std::unique_ptr<T>>& graph) {
    for (int i = 0; i < graph.count(); ++i) {
        SkASSERT(!Traits::IsTempMarked(graph[i].get()));
        SkASSERT(!Traits::WasOutput(graph[i].get()));
    }
}

template <typename T, typename Traits = T>
void GrTTopoSort_CleanExit(const SkTArray<std::unique_ptr<T>>& graph) {
    for (int i = 0; i < graph.count(); ++i) {
        SkASSERT(!Traits::IsTempMarked(graph[i].get()));
        SkASSERT(Traits::WasOutput(graph[i].get()));
    }
}
#endif

// Recursively visit a node and all the other nodes it depends on.
// Return false if there is a loop.
template <typename T, typename Traits = T>
bool GrTTopoSort_Visit(std::unique_ptr<T> node, SkTArray<std::unique_ptr<T>>* result) {
    bool wasError = false;

    if (Traits::IsTempMarked(node.get())) {
        // There is a loop.
        return false;
    }

    // If the node under consideration has been already been output it means it
    // (and all the nodes it depends on) are already in 'result'.
    if (!Traits::WasOutput(node.get())) {
        // This node hasn't been output yet. Recursively assess all the
        // nodes it depends on outputing them first.
        Traits::SetTempMark(node.get());
        for (int i = 0; i < Traits::NumDependencies(node.get()); ++i) {
            bool succeeded = GrTTopoSort_Visit<T, Traits>(Traits::Dependency(node.get(), i), result);
            if (!succeeded) {
                wasError = true;
            }
        }
        Traits::Output(node, result->count()); // mark this node as output
        Traits::ResetTempMark(node);

        result->push_back(sk_ref_sp(node));
    }

    return wasError;
}

// Topologically sort the nodes in 'graph'. For this sort, when node 'i' depends
// on node 'j' it means node 'j' must appear in the result before node 'i'.
// A false return value means there was a loop and the contents of 'graph' will
// be in some arbitrary state.
//
// Traits requires:
//   static void Output(T* t, int index) { ... }  // 'index' is 't's position in the result
//   static bool WasOutput(const T* t) { ... }
//
//   static void SetTempMark(T* t) { ... }        // transiently used during toposort
//   static void ResetTempMark(T* t) { ... }
//   static bool IsTempMarked(const T* t) { ... }
//
//   static int NumDependencies(const T* t) { ... } // 't' will be output after all the other -
//   static T* Dependency(T* t, int index) { ... }  // nodes on which it depends
// We'll look on T for these by default, or you can pass a custom Traits type.
//
// TODO: potentially add a version that takes a seed node and just outputs that
// node and all the nodes on which it depends. This could be used to partially
// flush a GrRenderTask DAG.
template <typename T, typename Traits = T>
bool GrTTopoSort(SkTArray<std::unique_ptr<T>>* graph) {
    SkTArray<std::unique_ptr<T>> result;

#ifdef SK_DEBUG
    GrTTopoSort_CheckAllUnmarked<T, Traits>(*graph);
#endif

    result.reserve_back(graph->count());

    int currentLocation = 0;

    for (int i = 0; i < graph->count(); ++i) {
        if (Traits::WasOutput((*graph)[i].get())) {
            // This node was depended on by some earlier node and has already
            // been output
            continue;
        }

        // Output this node after all the nodes it depends on have been output.
        if (!GrTTopoSort_Visit<T, Traits>(std::move((*graph)[i]), &result)) {
            return false;
        }
    }

    SkASSERT(graph->count() == result.count());
    graph->swap(result);

#ifdef SK_DEBUG
    GrTTopoSort_CleanExit<T, Traits>(*graph);
#endif
    return true;
}

#endif
