/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTTopoSort_DEFINED
#define GrTTopoSort_DEFINED

#include "include/core/SkRefCnt.h"  // IWYU pragma: keep
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"

#include <cstddef>
#include <cstdint>

#ifdef SK_DEBUG
template <typename T, typename Traits = T>
void GrTTopoSort_CheckAllUnmarked(SkSpan<const sk_sp<T>> graph) {
    for (const auto& node : graph) {
        SkASSERT(!Traits::IsTempMarked(node.get()));
        SkASSERT(!Traits::WasOutput(node.get()));
    }
}

template <typename T, typename Traits = T>
void GrTTopoSort_CleanExit(SkSpan<const sk_sp<T>> graph, uint32_t offset) {
    for (size_t i = 0; i < graph.size(); ++i) {
        SkASSERT(!Traits::IsTempMarked(graph[i].get()));
        SkASSERT(Traits::WasOutput(graph[i].get()));
        SkASSERT(Traits::GetIndex(graph[i].get()) - offset == (uint32_t) i);
    }
}
#endif

// Recursively visit a node and all the other nodes it depends on.
// Return false if there is a loop.
template <typename T, typename Traits = T>
bool GrTTopoSort_Visit(T* node, uint32_t* counter) {
    if (Traits::IsTempMarked(node)) {
        // There is a loop.
        return false;
    }

    // If the node under consideration has been already been output it means it
    // (and all the nodes it depends on) are already in 'result'.
    if (Traits::WasOutput(node)) {
        return true;
    }

    bool succeeded = true;
    // This node hasn't been output yet. Recursively assess all the
    // nodes it depends on outputing them first.
    Traits::SetTempMark(node);
    for (int i = 0; i < Traits::NumDependencies(node); ++i) {
        if (!GrTTopoSort_Visit<T, Traits>(Traits::Dependency(node, i), counter)) {
            succeeded = false;
        }
    }

    Traits::Output(node, *counter); // mark this node as output
    ++(*counter);
    Traits::ResetTempMark(node);

    return succeeded;
}

// Topologically sort the nodes in 'graph'. For this sort, when node 'i' depends
// on node 'j' it means node 'j' must appear in the result before node 'i'. Note that all
// dependencies of a node in the Span must also be in the Span or already have WasOutput() = true.
//
// A false return value means there was a loop and the contents of 'graph' will
// be in some arbitrary state.
//
// Traits requires:
//   static void Output(T* t, uint32_t index) { ... }  // 'index' is 't's position in the result
//   static bool WasOutput(const T* t) { ... }
//   static uint32_t GetIndex() { ... }
//
//   static void SetTempMark(T* t) { ... }        // transiently used during toposort
//   static void ResetTempMark(T* t) { ... }
//   static bool IsTempMarked(const T* t) { ... }
//
//   static int NumDependencies(const T* t) { ... } // 't' will be output after all the other -
//   static T* Dependency(T* t, int index) { ... }  // nodes on which it depends
// We'll look on T for these by default, or you can pass a custom Traits type.
//
// The offset parameter is useful if you are sorting ranges of a larger graph and when Output()
// is called on a T it must know it's position in the full graph array.
//
// TODO: potentially add a version that takes a seed node and just outputs that
// node and all the nodes on which it depends. This could be used to partially
// flush a GrRenderTask DAG.
template <typename T, typename Traits = T>
bool GrTTopoSort(SkSpan<sk_sp<T>> graph, uint32_t offset = 0) {
    uint32_t counter = offset;

#ifdef SK_DEBUG
    GrTTopoSort_CheckAllUnmarked<T, Traits>(graph);
#endif

    bool succeeded = true;

    for (size_t i = 0; i < graph.size(); ++i) {
        if (Traits::WasOutput(graph[i].get())) {
            // This node was depended on by some earlier node and has already
            // been output
            continue;
        }

        // Output this node after all the nodes it depends on have been output.
        if (!GrTTopoSort_Visit<T, Traits>(graph[i].get(), &counter)) {
            succeeded = false;
        }
    }

    SkASSERT(counter - offset == (uint32_t) graph.size());

    // Reorder the array given the output order
    for (uint32_t i = 0; i < (uint32_t) graph.size(); ++i) {
        for (uint32_t correctIndex = Traits::GetIndex(graph[i].get()) - offset;
             correctIndex != i;
             correctIndex = Traits::GetIndex(graph[i].get()) - offset) {
             graph[i].swap(graph[correctIndex]);
        }
    }

#ifdef SK_DEBUG
    GrTTopoSort_CleanExit<T, Traits>(graph, offset);
#endif
    return succeeded;
}

#endif
