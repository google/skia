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
import org.junit.runner.manipulation.Filter;
import org.junit.runner.manipulation.Filterable;
import org.junit.runner.manipulation.NoTestsRemainException;
import org.junit.runner.notification.Failure;
import org.junit.runner.notification.RunNotifier;

@RunWith(SkQPRunner.class)
public class SkQPRunner extends Runner implements Filterable {
    private int mShouldRunTestCount;
    private Description[] mTests;
    private boolean[] mShouldSkipTest;
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

    ////////////////////////////////////////////////////////////////////////////

    public SkQPRunner(Class testClass) {
        impl = new SkQP();
        File filesDir = SkQPRunner.GetOutputDir();
        try {
            SkQP.ensureEmtpyDirectory(filesDir);
        } catch (IOException e) {
            Log.w(TAG, "ensureEmtpyDirectory: " + e.getMessage());
        }
        Log.i(TAG, String.format("output written to \"%s\"", filesDir.getAbsolutePath()));

        Resources resources = InstrumentationRegistry.getTargetContext().getResources();
        AssetManager mAssetManager = resources.getAssets();
        impl.nInit(mAssetManager, filesDir.getAbsolutePath(), false);

        mTests = new Description[this.testCount()];
        mShouldSkipTest = new boolean[mTests.length]; // = {false, false, ....};
        int index = 0;
        for (int backend = 0; backend < impl.mBackends.length; backend++) {
            for (int gm = 0; gm < impl.mGMs.length; gm++) {
                mTests[index++] = Description.createTestDescription(SkQPRunner.class,
                    impl.mBackends[backend] + "_" + impl.mGMs[gm]);
            }
        }
        for (int unitTest = 0; unitTest < impl.mUnitTests.length; unitTest++) {
            mTests[index++] = Description.createTestDescription(SkQPRunner.class,
                    "unitTest_" + impl.mUnitTests[unitTest]);
        }
        assert(index == mTests.length);
        mShouldRunTestCount = mTests.length;
    }

    @Override
    public void filter(Filter filter) throws NoTestsRemainException {
        int count = 0;
        for (int i = 0; i < mTests.length; ++i) {
            mShouldSkipTest[i] = !filter.shouldRun(mTests[i]);
            if (!mShouldSkipTest[i]) {
                ++count;
            }
        }
        mShouldRunTestCount = count;
        if (0 == count) {
            throw new NoTestsRemainException();
        }
    }

    @Override
    public Description getDescription() {
        Description d = Description.createSuiteDescription(SkQP.class);
        for (int i = 0; i < mTests.length; ++i) {
            d.addChild(mTests[i]);
        }
        return d;
    }

    @Override
    public int testCount() {
        return impl.mUnitTests.length + impl.mGMs.length * impl.mBackends.length;
    }

    @Override
    public void run(RunNotifier notifier) {
        int testNumber = 1;  // out of number of actually run tests.
        int testIndex = 0;  // out of potential tests.
        for (int backend = 0; backend < impl.mBackends.length; backend++) {
            for (int gm = 0; gm < impl.mGMs.length; gm++, testIndex++) {
                Description desc = mTests[testIndex];
                String name = desc.getMethodName();
                if (mShouldSkipTest[testIndex]) {
                    continue;
                }
                Log.v(TAG, String.format("Rendering Test '%s' started (%d/%d).",
                                         name, testNumber++, mShouldRunTestCount));
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
                    Log.w(TAG, String.format("[ERROR] '%s': %s", name, error));
                } else if (value != 0) {
                    SkQPRunner.Fail(desc, notifier, String.format(
                                "Image mismatch: max channel diff = %f", value));
                    Log.w(TAG, String.format("[FAIL] '%s': %f > 0", name, value));
                } else {
                    Log.i(TAG, String.format("Rendering Test '%s' passed", name));
                }
                notifier.fireTestFinished(desc);
            }
        }
        for (int unitTest = 0; unitTest < impl.mUnitTests.length; unitTest++, testIndex++) {
            Description desc = mTests[testIndex];
            String name = desc.getMethodName();
            if (mShouldSkipTest[testIndex]) {
                continue;
            }

            Log.v(TAG, String.format("Test '%s' started (%d/%d).",
                                     name, testNumber++, mShouldRunTestCount));
            notifier.fireTestStarted(desc);
            String[] errors = impl.nExecuteUnitTest(unitTest);
            if (errors != null && errors.length > 0) {
                Log.w(TAG, String.format("[FAIL] Test '%s' had %d failures.", name, errors.length));
                for (String error : errors) {
                    SkQPRunner.Fail(desc, notifier, error);
                    Log.w(TAG, String.format("[FAIL] '%s': %s", name, error));
                }
            } else {
                Log.i(TAG, String.format("Test '%s' passed.", name));
            }
            notifier.fireTestFinished(desc);
        }
        impl.nMakeReport();
        Log.i(TAG, String.format("output written to \"%s\"", GetOutputDir().getAbsolutePath()));
    }
}
