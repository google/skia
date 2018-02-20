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
import org.junit.runner.Description;
import org.junit.runner.RunWith;
import org.junit.runner.Runner;
import org.junit.runner.notification.Failure;
import org.junit.runner.notification.RunNotifier;

@RunWith(SkQPRunner.class)
public class SkQPRunner extends Runner {
    private Description mDescription;
    private SkQP impl;
    private static final String TAG = SkQP.LOG_PREFIX;

    private static void Fail(Description desc, RunNotifier notifier, String failure) {
        notifier.fireTestFailure(new Failure(desc, new Throwable(failure)));
    }

    private static File GetOutputDir() {
        Context c = InstrumentationRegistry.getTargetContext();
        // File f = c.getFilesDir();
        File f = c.getExternalFilesDir(null);
        return new File(f, "output");
    }

    private Description gmDesc(int backend, int gm) {
        return Description.createTestDescription(
                SkQP.kSkiaGM + impl.mBackends[backend], impl.mGMs[gm]);
    }
    private Description unitDesc(int test) {
        return Description.createTestDescription(SkQP.kSkiaUnitTests, impl.mUnitTests[test]);
    }

    ////////////////////////////////////////////////////////////////////////////

    public SkQPRunner(Class testClass) {
        impl = new SkQP();
        File filesDir = SkQPRunner.GetOutputDir();
        try {
            SkQP.ensureEmtpyDirectory(filesDir);
        } catch (IOException e) {
            Log.w(TAG, "ensureEmtpyDirectory: " + e.getMessage());
        }
        Log.i(TAG, "output path = " + filesDir.getAbsolutePath());

        Resources resources = InstrumentationRegistry.getTargetContext().getResources();
        AssetManager mAssetManager = resources.getAssets();
        impl.nInit(mAssetManager, filesDir.getAbsolutePath(), false);

        mDescription = Description.createSuiteDescription(testClass);
        for (int backend = 0; backend < impl.mBackends.length; backend++) {
            for (int gm = 0; gm < impl.mGMs.length; gm++) {
                mDescription.addChild(this.gmDesc(backend, gm));
            }
        }
        for (int unitTest = 0; unitTest < impl.mUnitTests.length; unitTest++) {
            mDescription.addChild(this.unitDesc(unitTest));
        }
    }

    @Override
    public Description getDescription() { return mDescription; }

    @Override
    public int testCount() {
        return impl.mUnitTests.length + impl.mGMs.length * impl.mBackends.length;
    }

    @Override
    public void run(RunNotifier notifier) {
        int numberOfTests = this.testCount();
        int testNumber = 1;
        for (int backend = 0; backend < impl.mBackends.length; backend++) {
            String classname = SkQP.kSkiaGM + impl.mBackends[backend];
            for (int gm = 0; gm < impl.mGMs.length; gm++) {
                String gmName = String.format("%s/%s", impl.mBackends[backend], impl.mGMs[gm]);
                if (!SkQPAndroidRunner.filter(gmName)) {
                    continue;
                }
                Log.v(TAG, String.format("Rendering Test %s started (%d/%d).",
                                         gmName, testNumber, numberOfTests));
                testNumber++;
                Description desc = this.gmDesc(backend, gm);
                notifier.fireTestStarted(desc);
                float value = java.lang.Float.MAX_VALUE;
                String error = null;
                try {
                    value = impl.nExecuteGM(gm, backend);
                } catch (SkQPException exept) {
                    error = exept.getMessage();
                }
                if (error != null) {
                    SkQPRunner.Fail(desc, notifier, String.format("Exception: %s", error));
                    Log.w(TAG, String.format("[ERROR] %s: %s", gmName, error));
                } else if (value != 0) {
                    SkQPRunner.Fail(desc, notifier, String.format(
                                "Image mismatch: max channel diff = %f", value));
                    Log.w(TAG, String.format("[FAIL] %s: %f > 0", gmName, value));
                } else {
                    Log.i(TAG, String.format("Rendering Test %s passed", gmName));
                }
                notifier.fireTestFinished(desc);
            }
        }
        for (int unitTest = 0; unitTest < impl.mUnitTests.length; unitTest++) {
            String utName = impl.mUnitTests[unitTest];
            if (!SkQPAndroidRunner.filter(String.format("unitTest/%s", utName))) {
                continue;
            }
            Log.v(TAG, String.format("Test %s started (%d/%d).",
                                     utName, testNumber, numberOfTests));
            testNumber++;
            Description desc = this.unitDesc(unitTest);
            notifier.fireTestStarted(desc);
            String[] errors = impl.nExecuteUnitTest(unitTest);
            if (errors != null && errors.length > 0) {
                Log.w(TAG, String.format("[FAIL] Test %s had %d failures.", utName, errors.length));
                for (String error : errors) {
                    SkQPRunner.Fail(desc, notifier, error);
                    Log.w(TAG, String.format("[FAIL] %s: %s", utName, error));
                }
            } else {
                Log.i(TAG, String.format("Test %s passed.", utName));
            }
            notifier.fireTestFinished(desc);
        }
        impl.nMakeReport();
        Log.i(TAG, String.format("output written to \"%s\"", GetOutputDir().getAbsolutePath()));
    }
}
