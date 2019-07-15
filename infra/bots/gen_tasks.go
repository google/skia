// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

/*
	Generate the tasks.json file.
*/

import (
	"path/filepath"

	"go.skia.org/skia/infra/bots/gen_tasks_logic"
)

func main() {
	infrabots := filepath.Join(gen_tasks_logic.CheckoutRoot(), "infra", "bots")
	var jobs []string
	gen_tasks_logic.LoadJson(filepath.Join(infrabots, "jobs.json"), &jobs)
	var cfg gen_tasks_logic.Config
	gen_tasks_logic.LoadJson(filepath.Join(infrabots, "cfg.json"), &cfg)
	gen_tasks_logic.GenTasks(jobs, &cfg)
}
