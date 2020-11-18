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
void GrTTopoSort_CheckAllUnmarked(const SkTArray<sk_sp<T>>& graph) {
    for (int i = 0; i < graph.count(); ++i) {
        SkASSERT(!Traits::IsTempMarked(graph[i].get()));
        SkASSERT(!Traits::WasOutput(graph[i].get()));
    }
}

template <typename T, typename Traits = T>
void GrTTopoSort_CleanExit(const SkTArray<sk_sp<T>>& graph) {
    for (int i = 0; i < graph.count(); ++i) {
        SkASSERT(!Traits::IsTempMarked(graph[i].get()));
        SkASSERT(Traits::WasOutput(graph[i].get()));
        SkASSERT(Traits::GetIndex(graph[i].get()) == (uint32_t) i);
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

    bool succeeded = true;

    // If the node under consideration has been already been output it means it
    // (and all the nodes it depends on) are already in 'result'.
    if (!Traits::WasOutput(node)) {
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
    }

    return succeeded;
}

// Topologically sort the nodes in 'graph'. For this sort, when node 'i' depends
// on node 'j' it means node 'j' must appear in the result before node 'i'.
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
// TODO: potentially add a version that takes a seed node and just outputs that
// node and all the nodes on which it depends. This could be used to partially
// flush a GrRenderTask DAG.
template <typename T, typename Traits = T>
bool GrTTopoSort(SkTArray<sk_sp<T>>* graph) {
    uint32_t counter = 0;

#ifdef SK_DEBUG
    GrTTopoSort_CheckAllUnmarked<T, Traits>(*graph);
#endif

    bool succeeded = true;

    for (int i = 0; i < graph->count(); ++i) {
        if (Traits::WasOutput((*graph)[i].get())) {
            // This node was depended on by some earlier node and has already
            // been output
            continue;
        }

        // Output this node after all the nodes it depends on have been output.
        if (!GrTTopoSort_Visit<T, Traits>((*graph)[i].get(), &counter)) {
            succeeded = false;
        }
    }

    SkASSERT(counter == (uint32_t) graph->count());

    // Reorder the array given the output order
    for (uint32_t i = 0; i < (uint32_t) graph->count(); ++i) {
        for (uint32_t correctIndex = Traits::GetIndex((*graph)[i].get());
             correctIndex != i;
             correctIndex = Traits::GetIndex((*graph)[i].get())) {
            (*graph)[i].swap((*graph)[correctIndex]);
        }
    }

#ifdef SK_DEBUG
    GrTTopoSort_CleanExit<T, Traits>(*graph);
#endif
    return succeeded;
}

#endif
