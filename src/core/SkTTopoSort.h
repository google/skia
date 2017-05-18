/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTTopoSort_DEFINED
#define SkTTopoSort_DEFINED

#include "SkTDArray.h"

#ifdef SK_DEBUG
template <typename T, typename Traits = T>
void SkTTopoSort_CheckAllUnmarked(const SkTDArray<T*>& graph) {
    for (int i = 0; i < graph.count(); ++i) {
        SkASSERT(!Traits::IsTempMarked(graph[i]));
        SkASSERT(!Traits::WasOutput(graph[i]));
    }
}

template <typename T, typename Traits = T>
void SkTTopoSort_CleanExit(const SkTDArray<T*>& graph) {
    for (int i = 0; i < graph.count(); ++i) {
        SkASSERT(!Traits::IsTempMarked(graph[i]));
        SkASSERT(Traits::WasOutput(graph[i]));
    }
}
#endif

// Recursively visit a node and all the other nodes it depends on.
// Return false if there is a loop.
template <typename T, typename Traits = T>
bool SkTTopoSort_Visit(T* node, SkTDArray<T*>* result) {
    if (Traits::IsTempMarked(node)) {
        // There is a loop.
        return false;
    }

    // If the node under consideration has been already been output it means it
    // (and all the nodes it depends on) are already in 'result'.
    if (!Traits::WasOutput(node)) {
        // This node hasn't been output yet. Recursively assess all the
        // nodes it depends on outputing them first.
        Traits::SetTempMark(node);
        for (int i = 0; i < Traits::NumDependencies(node); ++i) {
            if (!SkTTopoSort_Visit<T, Traits>(Traits::Dependency(node, i), result)) {
                return false;
            }
        }
        Traits::Output(node, result->count()); // mark this node as output
        Traits::ResetTempMark(node);

        *result->append() = node;
    }

    return true;
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
// flush a GrOpList DAG.
template <typename T, typename Traits = T>
bool SkTTopoSort(SkTDArray<T*>* graph) {
    SkTDArray<T*> result;

#ifdef SK_DEBUG
    SkTTopoSort_CheckAllUnmarked<T, Traits>(*graph);
#endif

    result.setReserve(graph->count());

    for (int i = 0; i < graph->count(); ++i) {
        if (Traits::WasOutput((*graph)[i])) {
            // This node was depended on by some earlier node and has already
            // been output
            continue;
        }

        // Output this node after all the nodes it depends on have been output.
        if (!SkTTopoSort_Visit<T, Traits>((*graph)[i], &result)) {
            return false;
        }
    }

    SkASSERT(graph->count() == result.count());
    graph->swap(result);

#ifdef SK_DEBUG
    SkTTopoSort_CleanExit<T, Traits>(*graph);
#endif
    return true;
}

#endif
