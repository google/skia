// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

/*
	Generate the tasks.json file.
*/

import (
	"go.skia.org/skia/infra/bots/gen_tasks_logic"
)

// Regenerate the tasks.json file.
func main() {
	gen_tasks_logic.GenTasks(nil)
}
