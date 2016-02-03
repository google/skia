/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package com.skia;

import org.libsdl.app.SDLActivity;

import android.content.Intent;

public class VisualBenchActivity extends SDLActivity {
  protected String[] getArguments() {
    String flags = this.getIntent().getStringExtra("cmdLineFlags");
    if (flags != null && !flags.isEmpty()) {
	return flags.split("\\s+");
    }
    return new String[0];
  }
}
