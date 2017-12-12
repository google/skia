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

    private native void init();

    private native String getGMName(int index);
    private native String getBackendName(int index);
    private native String getUnitTestName(int index);

    private native float executeGM(int gm, int backend) throws SkQPException;
    private native String[] executeUnitTests(int test);

    public AssetManager mAssetManager;
    public int mGMCount;
    public int mBackendCount;
    public int mUnitTestCount;

    private static boolean sOnceFlag = false;
    private static final String kSkiaGM = "SkiaGM_";
    private static final String kSkiaUnitTests = "Skia_UnitTests";

    private Description mDescription;

    public SkQPRunner(Class testClass) {
        synchronized (SkQPRunner.class) {
            if (sOnceFlag) {
                throw new IllegalStateException("Error multiple SkQPs defined");
            }
            sOnceFlag = true;
        }
        System.loadLibrary("skqp_app");

        // Context context = InstrumentationRegistry.getContext();
        Context context = InstrumentationRegistry.getTargetContext();
        Resources resources = context.getResources();
        mAssetManager = resources.getAssets();
        this.init();

        mDescription = Description.createSuiteDescription(testClass);
        java.lang.annotation.Annotation annots[] = new java.lang.annotation.Annotation[0];
        for (int backend = 0; backend < mBackendCount; backend++) {
            String classname = kSkiaGM + this.getBackendName(backend);
            for (int gm = 0; gm < mGMCount; gm++) {
                String gmName = this.getGMName(gm);
                mDescription.addChild(Description.createTestDescription(classname, gmName, annots));
            }
        }
        for (int unitTest = 0; unitTest < mUnitTestCount; unitTest++) {
            String name = this.getUnitTestName(unitTest);
            mDescription.addChild(Description.createTestDescription(kSkiaUnitTests, name, annots));
        }
    }

    @Override
    public Description getDescription() { return mDescription; }

    @Override
    public int testCount() { return mUnitTestCount + mGMCount * mBackendCount; }

    @Override
    public void run(RunNotifier notifier) {
        java.lang.annotation.Annotation annots[] = new java.lang.annotation.Annotation[0];
        for (int backend = 0; backend < mBackendCount; backend++) {
            String classname = kSkiaGM + this.getBackendName(backend);
            for (int gm = 0; gm < mGMCount; gm++) {
                float value = java.lang.Float.MAX_VALUE;
                String error = null;
                try {
                    value = this.executeGM(gm, backend);
                } catch (SkQPException exept) {
                    error = exept.getMessage();
                }
                if (value == 0 && error == null) {
                    continue;
                }
                String gmName = this.getGMName(gm);
                Description desc = Description.createTestDescription(classname, gmName, annots);
                String fail;
                if (error != null) {
                    fail = String.format("Exception: %s", error);
                } else {
                    fail = String.format("Failed: %f != 0", value);
                }
                notifier.fireTestFailure(new Failure(desc, new Throwable(fail)));
            }
        }
        for (int unitTest = 0; unitTest < mUnitTestCount; unitTest++) {
            String[] errors = this.executeUnitTests(unitTest);
            if (errors == null || errors.length == 0) {
                continue;
            }
            String name = this.getUnitTestName(unitTest);
            assert name != null;
            Description desc = Description.createTestDescription(kSkiaUnitTests, name, annots);
            for (String error : errors) {
                assert error != null;
                notifier.fireTestFailure(new Failure(desc, new Throwable(error)));
            }
        }
    }
}

