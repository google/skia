/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


package org.skia;

public class SkiaJNI {

  static {
      System.loadLibrary("skiajni");
  }

  public static void initialize() {
      // The does nothing, but refencing the function runs static initializer, which loads the
      // library.
  }

  // generate full JNI for Skia C API: both Java and C++ code.
  public static native void generateJNI(String folder);
}
