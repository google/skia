/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTaskCluster_DEFINED
#define GrRenderTaskCluster_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkSpan.h"
#include "src/core/SkTInternalLList.h"

class GrRenderTask;

// Take a topologically-sorted DAG and cluster the tasks together while maintaining the
// dependencies.
//
// If no clustering is possible, returns false.
// Otherwise, returns true and populates the provided llist as such:
//   - Contains the same set of tasks as `input`.
//   - Obeys the dependency rules in `input`.
//   - Places tasks with the same target adjacent to each other.
//   - Tasks with multiple targets act as reordering barriers for all their targets.
bool GrClusterRenderTasks(SkSpan<const sk_sp<GrRenderTask>> input,
                          SkTInternalLList<GrRenderTask>* llist);

#endif
