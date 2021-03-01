// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package gen_tasks_logic

// skpbenchFlags generates flags to skpbench based on the given task properties.
func (b *taskBuilder) skpbenchFlags(doUpload bool) {
	args := []string{
		"skpbench",
	}

	// Test reduceOpsTaskSplitting option on these models
	// See skbug.com/10877#c27
	if b.model("GalaxyS20", "Nexus7", "Nexus5x", "AndroidOne", "NUC7i5BNK") {
		args = append(args, "--reduceOpsTaskSplitting", "true")
	}
}
