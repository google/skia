// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package gen_tasks_logic

// skpbenchFlags generates flags to skpbench based on the given task properties.
func (b *taskBuilder) skpbenchFlags() {
	if b.model(REDUCE_OPS_TASK_SPLITTING_MODELS...) {
		b.recipeProp("reduce_ops_task_splitting", "true")
	}
}
