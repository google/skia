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
<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
import java.text.SimpleDateFormat;
import java.util.Date;
=======
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
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
<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
    private String mOutputDirectory;
    private SkQP mImpl;
=======
    private SkQP impl;
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
    private static final String TAG = SkQP.LOG_PREFIX;

    private static void Fail(Description desc, RunNotifier notifier, String failure) {
        notifier.fireTestFailure(new Failure(desc, new SkQPFailure(failure)));
    }

<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
=======
    private static File GetOutputDir() {
        Context c = InstrumentationRegistry.getTargetContext();
        // File f = c.getFilesDir();
        File f = c.getExternalFilesDir(null);
        return new File(f, "output");
    }

>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
    ////////////////////////////////////////////////////////////////////////////

    public SkQPRunner(Class testClass) {
<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
        mImpl = new SkQP();
        Context context = InstrumentationRegistry.getTargetContext();
        String now = (new SimpleDateFormat("yyyy-MM-dd'T'HHmmss")).format(new Date());
        File reportPath = new File(context.getExternalFilesDir(null), "skqp_report_" + now);
        reportPath.mkdirs();
        mOutputDirectory = reportPath.getAbsolutePath();
        Log.i(TAG, String.format("output written to \"%s\"", mOutputDirectory));
=======
        impl = new SkQP();
        File filesDir = SkQPRunner.GetOutputDir();
        try {
            SkQP.ensureEmtpyDirectory(filesDir);
        } catch (IOException e) {
            Log.w(TAG, "ensureEmtpyDirectory: " + e.getMessage());
        }
        Log.i(TAG, String.format("output written to \"%s\"", filesDir.getAbsolutePath()));
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)

        AssetManager assetManager = context.getResources().getAssets();
        mImpl.nInit(assetManager, mOutputDirectory);

        mTests = new Description[this.testCount()];
        mShouldSkipTest = new boolean[mTests.length]; // = {false, false, ....};
        int index = 0;
<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
        for (int backend = 0; backend < mImpl.mBackends.length; backend++) {
            for (int gm = 0; gm < mImpl.mGMs.length; gm++) {
                mTests[index++] = Description.createTestDescription(SkQPRunner.class,
                    mImpl.mBackends[backend] + "_" + mImpl.mGMs[gm]);
=======
        for (int backend = 0; backend < impl.mBackends.length; backend++) {
            for (int gm = 0; gm < impl.mGMs.length; gm++) {
                mTests[index++] = Description.createTestDescription(SkQPRunner.class,
                    impl.mBackends[backend] + "/" + impl.mGMs[gm]);
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
            }
        }
<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
        for (int unitTest = 0; unitTest < mImpl.mUnitTests.length; unitTest++) {
            mTests[index++] = Description.createTestDescription(SkQPRunner.class,
                    "unitTest_" + mImpl.mUnitTests[unitTest]);
=======
        for (int unitTest = 0; unitTest < impl.mUnitTests.length; unitTest++) {
            mTests[index++] = Description.createTestDescription(SkQPRunner.class,
                    "unitTest/" + impl.mUnitTests[unitTest]);
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
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
        return mImpl.mUnitTests.length + mImpl.mGMs.length * mImpl.mBackends.length;
    }

    @Override
    public void run(RunNotifier notifier) {
<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
        int testNumber = 0;  // out of number of actually run tests.
        int testIndex = 0;  // out of potential tests.
        for (int backend = 0; backend < mImpl.mBackends.length; backend++) {
            for (int gm = 0; gm < mImpl.mGMs.length; gm++, testIndex++) {
                Description desc = mTests[testIndex];
                String name = desc.getMethodName();
                if (mShouldSkipTest[testIndex]) {
                    continue;
                }
                ++testNumber;
=======
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
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
                notifier.fireTestStarted(desc);
                long value = java.lang.Long.MAX_VALUE;
                String error = null;
                try {
                    value = mImpl.nExecuteGM(gm, backend);
                } catch (SkQPException exept) {
                    error = exept.getMessage();
                }
                String result = "pass";
                if (error != null) {
                    SkQPRunner.Fail(desc, notifier, String.format("Exception: %s", error));
                    Log.w(TAG, String.format("[ERROR] '%s': %s", name, error));
<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
                    result = "ERROR";
=======
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
                } else if (value != 0) {
                    SkQPRunner.Fail(desc, notifier, String.format(
<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
                                "Image mismatch: max channel diff = %d", value));
                    Log.w(TAG, String.format("[FAIL] '%s': %d > 0", name, value));
                    result = "FAIL";
=======
                                "Image mismatch: max channel diff = %f", value));
                    Log.w(TAG, String.format("[FAIL] '%s': %f > 0", name, value));
                } else {
                    Log.i(TAG, String.format("Rendering Test '%s' passed", name));
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
                }
                notifier.fireTestFinished(desc);
                Log.i(TAG, String.format("Rendering Test '%s' complete (%d/%d). [%s]",
                                         name, testNumber, mShouldRunTestCount, result));
            }
        }
<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
        for (int unitTest = 0; unitTest < mImpl.mUnitTests.length; unitTest++, testIndex++) {
            Description desc = mTests[testIndex];
            String name = desc.getMethodName();
            if (mShouldSkipTest[testIndex]) {
                continue;
            }
            ++testNumber;
=======
        for (int unitTest = 0; unitTest < impl.mUnitTests.length; unitTest++, testIndex++) {
            Description desc = mTests[testIndex];
            String name = desc.getMethodName();
            if (mShouldSkipTest[testIndex]) {
                continue;
            }

            Log.v(TAG, String.format("Test '%s' started (%d/%d).",
                                     name, testNumber++, mShouldRunTestCount));
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
            notifier.fireTestStarted(desc);
            String[] errors = mImpl.nExecuteUnitTest(unitTest);
            String result = "pass";
            if (errors != null && errors.length > 0) {
                Log.w(TAG, String.format("[FAIL] Test '%s' had %d failures.", name, errors.length));
                for (String error : errors) {
                    SkQPRunner.Fail(desc, notifier, error);
                    Log.w(TAG, String.format("[FAIL] '%s': %s", name, error));
                }
<<<<<<< HEAD   (ac7f23 SkQP: refatctor C++ bits.)
                result = "FAIL";
=======
            } else {
                Log.i(TAG, String.format("Test '%s' passed.", name));
>>>>>>> BRANCH (3e3428 SkQP: Remove tests that use too much RAM)
            }
            notifier.fireTestFinished(desc);
            Log.i(TAG, String.format("Test '%s' complete (%d/%d). [%s]",
                                     name, testNumber, mShouldRunTestCount, result));
        }
        mImpl.nMakeReport();
        Log.i(TAG, String.format("output written to \"%s\"", mOutputDirectory));
    }
}
