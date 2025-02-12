/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skqp;

import android.content.res.AssetManager;
import android.util.Log;

import java.lang.UnsatisfiedLinkError;

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

    // Must include each variant of libskqp_jni listed in Android.bp, with the "lib" prefix omitted.
    private static final String[] SKQP_JNI_NAME_VARIANTS = {
        "skqp_jni",
        "skqp_jni_alltests",
    };

    static {
        // Iteratively attempt to load the variant of libskqp_jni shared library that was included
        // in this particular build of SkQP.
        String loadedLibrary = null;
        for (String libraryName : SKQP_JNI_NAME_VARIANTS) {
            try {
                System.loadLibrary(libraryName);
                // Exit on first successfully loaded library.
                loadedLibrary = libraryName;
                break;
            } catch (UnsatisfiedLinkError err) {
                continue;
            }
        }

        if (loadedLibrary != null) {
            Log.d(LOG_PREFIX, "Loaded: " + loadedLibrary + " JNI shared library");
        }
        else {
            var linkError = new UnsatisfiedLinkError("Failed to load any variants of libskqp_jni");
            Log.e(LOG_PREFIX, null, linkError);
            throw linkError;
        }
    }
}

