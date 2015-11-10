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
    // intent get intent extras if triggered from the command line
    Intent intent = this.getIntent();
    String flags = intent.getStringExtra("cmdLineFlags");
    
    return flags.split("\\s+");
  }
}
