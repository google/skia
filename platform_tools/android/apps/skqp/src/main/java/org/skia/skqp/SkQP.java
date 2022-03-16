/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skqp;

import android.content.res.AssetManager;

/**
  This class does the heavy lifting for SkQP and provides the JNI interface
  used by both SkQPActivity (firebase interface) and SkQPRunner (JUnit interface).
 */
public class SkQP {
    protected native void nInit(AssetManager assetManager, String dataDir);
    protected native String[] nExecuteUnitTest(int test);
    protected native void nMakeReport();

    protected String[] mUnitTests;

    // These arrays are intended to be a matching set.
    // mSkSLErrorTestName[n] holds a name; mSkSLErrorTestShader[n] has the associated shader text.
    protected String[] mSkSLErrorTestName;
    protected String[] mSkSLErrorTestShader;

    protected static final String kSkiaGM = "skqp_";
    protected static final String kSkiaUnitTests = "skqp_unitTest";
    protected static final String LOG_PREFIX = "org.skia.skqp";

    static {
      System.loadLibrary("skqp_jni");
    }
}

