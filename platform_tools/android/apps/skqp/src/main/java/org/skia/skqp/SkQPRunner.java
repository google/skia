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
import org.junit.runner.Description;
import org.junit.runner.Runner;
import org.junit.runner.notification.Failure;
import org.junit.runner.notification.RunNotifier;

public class SkQPRunner extends Runner {
    private class SkQPException extends Exception {
        public SkQPException(String m) { super(m); }
    }

    private native void nInit(AssetManager assetManager);
    private native float nExecuteGM(int gm, int backend) throws SkQPException;
    private native String[] nExecuteUnitTests(int test);

    private AssetManager mAssetManager;
    private String[] mGMs;
    private String[] mBackends;
    private String[] mUnitTests;

    private static boolean sOnceFlag = false;
    private static final String kSkiaGM = "SkiaGM_";
    private static final String kSkiaUnitTests = "Skia_UnitTests";

    private Description mDescription;

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
        Resources resources = context.getResources();
        mAssetManager = resources.getAssets();
        assert mAssetManager != null;
        this.nInit(mAssetManager);
        assert mGMs != null;
        assert mBackends != null;
        assert mUnitTests != null;
        mDescription = Description.createSuiteDescription(testClass);
        java.lang.annotation.Annotation annots[] = new java.lang.annotation.Annotation[0];
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
        java.lang.annotation.Annotation annots[] = new java.lang.annotation.Annotation[0];
        for (int backend = 0; backend < mBackends.length; backend++) {
            String classname = kSkiaGM + mBackends[backend];
            for (int gm = 0; gm < mGMs.length; gm++) {
                float value = java.lang.Float.MAX_VALUE;
                String error = null;
                try {
                    value = this.nExecuteGM(gm, backend);
                } catch (SkQPException exept) {
                    error = exept.getMessage();
                }
                if (value == 0 && error == null) {
                    continue;
                }
                Description desc = Description.createTestDescription(classname, mGMs[gm], annots);
                String fail;
                if (error != null) {
                    fail = String.format("Exception: %s", error);
                } else {
                    fail = String.format("Failed: %f != 0", value);
                }
                notifier.fireTestFailure(new Failure(desc, new Throwable(fail)));
            }
        }
        for (int unitTest = 0; unitTest < mUnitTests.length; unitTest++) {
            String[] errors = this.nExecuteUnitTests(unitTest);
            if (errors == null || errors.length == 0) {
                continue;
            }
            Description desc = Description.createTestDescription(
                    kSkiaUnitTests, mUnitTests[unitTest], annots);
            for (String error : errors) {
                assert error != null;
                notifier.fireTestFailure(new Failure(desc, new Throwable(error)));
            }
        }
    }
}

