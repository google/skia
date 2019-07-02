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
    protected native long nExecuteGM(int gm, int backend) throws SkQPException;
    protected native String[] nExecuteUnitTest(int test);
    protected native void nMakeReport();

    protected String[] mGMs;
    protected String[] mBackends;
    protected String[] mUnitTests;

    protected static final String kSkiaGM = "skqp_";
    protected static final String kSkiaUnitTests = "skqp_unitTest";
    protected static final String LOG_PREFIX = "org.skia.skqp";

    static {
      System.loadLibrary("skqp_app");
    }
}

