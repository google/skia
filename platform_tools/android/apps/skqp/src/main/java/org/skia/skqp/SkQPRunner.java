/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skqp;

import android.content.Context;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.support.test.InstrumentationRegistry;
import android.util.Log;
import java.io.File;
import java.io.IOException;
import java.lang.annotation.Annotation;
import org.junit.runner.Description;
import org.junit.runner.Runner;
import org.junit.runner.notification.Failure;
import org.junit.runner.notification.RunNotifier;

public class SkQPRunner extends Runner {
    private native void nInit(AssetManager assetManager, String dataDir);
    private native float nExecuteGM(int gm, int backend) throws SkQPException;
    private native String[] nExecuteUnitTest(int test);
    private native void nMakeReport();

    private AssetManager mAssetManager;
    private String[] mGMs;
    private String[] mBackends;
    private String[] mUnitTests;

    private static boolean sOnceFlag = false;
    private static final String kSkiaGM = "SkiaGM_";
    private static final String kSkiaUnitTests = "Skia_UnitTests";

    private Description mDescription;

    private static void DeleteDirectoryContents(File f) throws IOException {
        for (File s : f.listFiles()) {
            if (s.isDirectory()) {
                SkQPRunner.DeleteDirectoryContents(s);
            }
            s.delete();
        }
    }

    private static void Fail(Description desc, RunNotifier notifier, String failure) {
        notifier.fireTestFailure(new Failure(desc, new Throwable(failure)));
    }

    ////////////////////////////////////////////////////////////////////////////

    public SkQPRunner(Class testClass) {
        synchronized (SkQPRunner.class) {
            if (sOnceFlag) {
                throw new IllegalStateException("Error multiple SkQPs defined");
            }
            sOnceFlag = true;
        }
        System.loadLibrary("skqp_app");

        Context context = InstrumentationRegistry.getTargetContext();
        File filesDir = context.getFilesDir();
        try {
            SkQPRunner.DeleteDirectoryContents(filesDir);
        } catch (IOException e) {
            Log.w("org.skis.skqp", "DeleteDirectoryContents: " + e.getMessage());
        }

        Resources resources = context.getResources();
        mAssetManager = resources.getAssets();
        this.nInit(mAssetManager, filesDir.getAbsolutePath());

        mDescription = Description.createSuiteDescription(testClass);
        Annotation annots[] = new Annotation[0];
        for (int backend = 0; backend < mBackends.length; backend++) {
            String classname = kSkiaGM + mBackends[backend];
            for (int gm = 0; gm < mGMs.length; gm++) {
                mDescription.addChild(Description.createTestDescription(classname, mGMs[gm], annots));
            }
        }
        for (int unitTest = 0; unitTest < mUnitTests.length; unitTest++) {
            mDescription.addChild(Description.createTestDescription(kSkiaUnitTests,
                        mUnitTests[unitTest], annots));
        }
    }

    @Override
    public Description getDescription() { return mDescription; }

    @Override
    public int testCount() { return mUnitTests.length + mGMs.length * mBackends.length; }

    @Override
    public void run(RunNotifier notifier) {
        Annotation annots[] = new Annotation[0];
        for (int backend = 0; backend < mBackends.length; backend++) {
            String classname = kSkiaGM + mBackends[backend];
            for (int gm = 0; gm < mGMs.length; gm++) {
                Description desc = Description.createTestDescription(classname, mGMs[gm], annots);
                notifier.fireTestStarted(desc);
                float value = java.lang.Float.MAX_VALUE;
                String error = null;
                try {
                    value = this.nExecuteGM(gm, backend);
                } catch (SkQPException exept) {
                    error = exept.getMessage();
                }
                if (error != null) {
                    SkQPRunner.Fail(desc, notifier, String.format("Exception: %s", error));
                } else if (value != 0) {
                    SkQPRunner.Fail(desc, notifier, String.format(
                                "Image mismatch: max channel diff = %f", value));
                }
                notifier.fireTestFinished(desc);
            }
        }
        for (int unitTest = 0; unitTest < mUnitTests.length; unitTest++) {
            Description desc = Description.createTestDescription(
                    kSkiaUnitTests, mUnitTests[unitTest], annots);
            notifier.fireTestStarted(desc);
            String[] errors = this.nExecuteUnitTest(unitTest);
            if (errors != null && errors.length > 0) {
                for (String error : errors) {
                    SkQPRunner.Fail(desc, notifier, error);
                }
            }
            notifier.fireTestFinished(desc);
        }
        this.nMakeReport();
    }
}

