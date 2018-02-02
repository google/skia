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

import org.junit.runner.notification.Failure;
import org.junit.runner.notification.RunNotifier;
import org.junit.runner.Description;

public class SkQP {
    protected native void nInit(AssetManager assetManager, String dataDir, boolean experimentalMode);
    protected native float nExecuteGM(int gm, int backend) throws SkQPException;
    protected native String[] nExecuteUnitTest(int test);
    protected native void nMakeReport();

    protected String[] mGMs;
    protected String[] mBackends;
    protected String[] mUnitTests;

    protected static final String kSkiaGM = "skqp_";
    protected static final String kSkiaUnitTests = "skqp_unitTest";
    protected static final String TAG = "org.skia.skqp";

    static {
      System.loadLibrary("skqp_app");
    }

    protected void runUnitTest(RunNotifier notifier, int unitTest, Description desc) {
        String utName = this.mUnitTests[unitTest];
        Log.v(TAG, String.format("Test %s started", utName));
        if (notifier != null) {
            notifier.fireTestStarted(desc);
        }
        String[] errors = this.nExecuteUnitTest(unitTest);
        if (errors != null && errors.length > 0) {
            Log.w(TAG, String.format("[FAIL] Test %s had %d failures.", utName, errors.length));
            for (String error : errors) {
                SkQP.Fail(desc, notifier, error);
                Log.w(TAG, String.format("[FAIL] %s: %s", utName, error));
            }
        } else {
            Log.i(TAG, String.format("Test %s passed.", utName));
        }
        if (notifier != null) {
            notifier.fireTestFinished(desc);
        }
    }

    protected void runRenderTest(RunNotifier notifier, int backend, int gm, Description desc) {
        String gmName = String.format("%s/%s", this.mBackends[backend], this.mGMs[gm]);
        Log.v(TAG, String.format("Rendering Test %s started.", gmName));
        if (notifier != null) {
            notifier.fireTestStarted(desc);
        }
        float value = java.lang.Float.MAX_VALUE;
        String error = null;
        try {
            value = this.nExecuteGM(gm, backend);
        } catch (SkQPException exept) {
            error = exept.getMessage();
        }
        if (error != null) {
            SkQP.Fail(desc, notifier, String.format("Exception: %s", error));
            Log.w(TAG, String.format("[ERROR] %s: %s", gmName, error));
        } else if (value != 0) {
            SkQP.Fail(desc, notifier, String.format(
                        "Image mismatch: max channel diff = %f", value));
            Log.w(TAG, String.format("[FAIL] %s: %f > 0", gmName, value));
        } else {
            Log.i(TAG, String.format("Rendering Test %s passed", gmName));
        }
        if (notifier != null) {
            notifier.fireTestFinished(desc);
        }
    }

    public void runSingleTest(RunNotifier notifier, Description desc) {
        String classname = desc.getClassName();
        String method = desc.getMethodName();
        if (classname.equals(SkQP.kSkiaUnitTests)) {
            for (int unitTest = 0; unitTest < this.mUnitTests.length; unitTest++) {
                if (this.mUnitTests[unitTest].equals(method)) {
                    this.runUnitTest(notifier, unitTest, desc);
                    break;
                }
            }
        } else {
            for (int backend = 0; backend < this.mBackends.length; backend++) {
                String backendClassname = SkQP.kSkiaGM + this.mBackends[backend];
                if (backendClassname.equals(classname)) {
                    for (int gm = 0; gm < this.mGMs.length; gm++) {
                        if (this.mBackends[backend].equals(method)) {
                            this.runRenderTest(notifier, backend, gm, desc);
                            break;
                        }
                    }
                    break;
                }
            }
        }
        this.nMakeReport();
    }

    public void run(RunNotifier notifier) {
        int numberOfTests = this.mUnitTests.length + this.mGMs.length * this.mBackends.length;
        int testNumber = 1;
        for (int backend = 0; backend < this.mBackends.length; backend++) {
            String classname = SkQP.kSkiaGM + this.mBackends[backend];
            for (int gm = 0; gm < this.mGMs.length; gm++) {
                Log.v(TAG, String.format("Test %d of %d.", testNumber, numberOfTests));
                testNumber++;
                Description desc =
                        Description.createTestDescription(classname, this.mGMs[gm]);
                this.runRenderTest(notifier, backend, gm, desc);
            }
        }
        for (int unitTest = 0; unitTest < this.mUnitTests.length; unitTest++) {
            String utName = this.mUnitTests[unitTest];
            Log.v(TAG, String.format("Test %d of %d.", testNumber, numberOfTests));
            testNumber++;
            Description desc = Description.createTestDescription(SkQP.kSkiaUnitTests, utName);
            this.runSingleTest(notifier, desc);
        }
        this.nMakeReport();
    }

    protected void runTests(Context context, String outputDirPath) {
        Log.i(TAG, "Output Dir: " + outputDirPath);
        File outputDir = new File(outputDirPath);
        try {
            ensureEmtpyDirectory(outputDir);
        } catch (IOException e) {
            Log.e(TAG, "ensureEmtpyDirectory:" + e.getMessage());
        }

        // Note: nInit will initialize the mGMs, mBackends and mUnitTests fields.
        AssetManager assetManager = context.getResources().getAssets();
        this.nInit(assetManager, outputDirPath, true);
        this.run(null);
        Log.i(TAG, String.format("output written to \"%s\"", outputDirPath));
    }

    protected static void ensureEmtpyDirectory(File f) throws IOException {
      if (f.exists()) {
        delete(f);
      }
      if (!f.mkdirs()) {
        throw new IOException("Unable to create directory:" + f.getAbsolutePath());
      }
    }

    protected static void delete(File f) throws IOException {
      if (f.isDirectory()) {
        for (File s : f.listFiles()) {
          delete(s);
        }
      }
      if (!f.delete()) {
        throw new IOException("Unable to delete:" + f.getAbsolutePath());
      }
    }
    private static void Fail(Description desc, RunNotifier notifier, String failure) {
        if (notifier != null) {
            notifier.fireTestFailure(new Failure(desc, new Throwable(failure)));
        }
    }

}

