/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skqp;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;
import java.io.File;
import java.io.IOException;

public class SkQP {
    protected native void nInit(AssetManager assetManager, String dataDir);
    protected native float nExecuteGM(int gm, int backend) throws SkQPException;
    protected native String[] nExecuteUnitTest(int test);
    protected native void nMakeReport();

    protected String[] mGMs;
    protected String[] mBackends;
    protected String[] mUnitTests;

    protected static final String kSkiaGM = "SkiaGM_";
    protected static final String kSkiaUnitTests = "Skia_UnitTests";
    protected static final String LOG_PREFIX = "org.skia.skqp";

    static {
      System.loadLibrary("skqp_app");
    }

    protected void runTests(Context context, String outputDirPath) {
        Log.w(LOG_PREFIX, "Output Dir: " + outputDirPath);
        File outputDir = new File(outputDirPath);
        try {
            ensureEmtpyDirectory(outputDir);
        } catch (IOException e) {
            Log.e(LOG_PREFIX, "Error creating output directory:" + e.getMessage());
        }

        // Note: nInit will initialize the mGMs, mBackends and mUnitTests fields.
        AssetManager assetManager = context.getResources().getAssets();
        this.nInit(assetManager, outputDirPath);

        for (int backend = 0; backend < mBackends.length; backend++) {
          String classname = kSkiaGM + mBackends[backend];
          for (int gm = 0; gm < mGMs.length; gm++) {
              String testName = kSkiaGM + mBackends[backend] + "_" +mGMs[gm];
              float value = java.lang.Float.MAX_VALUE;
              String error = null;
              Log.w(LOG_PREFIX, "Running: " + testName);
              try {
                  value = this.nExecuteGM(gm, backend);
              } catch (SkQPException exept) {
                  error = exept.getMessage();
              }
              if (error != null) {
                // Record error message and carry on.
              } else if (value != 0) {
                // Record failure and carry on.
                  // SkQPRunner.Fail(desc, notifier, String.format(
                  //             "Image mismatch: max channel diff = %f", value));
              } else {
                // Record success for this test.
              }
          }
        }
        for (int unitTest = 0; unitTest < mUnitTests.length; unitTest++) {
            String testName = kSkiaUnitTests + "_" + mUnitTests[unitTest];
            Log.w(LOG_PREFIX, "Running: " + testName);
            String[] errors = this.nExecuteUnitTest(unitTest);
            if (errors != null && errors.length > 0) {
                for (String error : errors) {
                  // Record unit test failures.
                }
            } else {
              // Record success.
            }
        }
        Log.w(LOG_PREFIX, "Finished running all tests.");
        nMakeReport();
    }

    protected static void ensureEmtpyDirectory(File f) throws IOException {
      if (f.exists()) {
        if (f.isDirectory()) {
          deleteDirectory(f);
        } else if (!f.delete()) {
            throw new IOException("Unable to delete file:" + f.getAbsolutePath());
        }
      }
      if (!f.mkdirs()) {
        throw new IOException("Unable to create directory:" + f.getAbsolutePath());
      }
    }

    protected static void deleteDirectory(File dir) throws IOException {
      for (File s : dir.listFiles()) {
          if (s.isDirectory()) {
            deleteDirectory(s);
          } else {
            // Note: We are not using java.nio.Files.delete because we want this
            // to work before API v26.
            if (!s.delete()) {
              throw new IOException("Unable to delete file:" + s.getAbsolutePath());
            }
          }
      }
      if (!dir.delete()) {
        throw new IOException("Unable to delete directory:" + dir.getAbsolutePath());
      }
    }
}

