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
import android.util.Log;
import androidx.test.InstrumentationRegistry;
import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
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
    private Description mDescription;
    private boolean[] mShouldSkipTest;
    private String mOutputDirectory;
    private SkQP mImpl;
    private static final String TAG = SkQP.LOG_PREFIX;

    private static void Fail(Description desc, RunNotifier notifier, String failure) {
        notifier.fireTestFailure(new Failure(desc, new SkQPFailure(failure)));
    }

    ////////////////////////////////////////////////////////////////////////////

    public SkQPRunner(Class testClass) {
        mImpl = new SkQP();
        Context context = InstrumentationRegistry.getTargetContext();
        String now = (new SimpleDateFormat("yyyy-MM-dd'T'HHmmss")).format(new Date());
        File reportPath = new File(context.getExternalFilesDir(null), "skqp_report_" + now);
        reportPath.mkdirs();
        mOutputDirectory = reportPath.getAbsolutePath();
        Log.i(TAG, String.format("output written to \"%s\"", mOutputDirectory));

        AssetManager assetManager = context.getResources().getAssets();
        mImpl.nInit(assetManager, mOutputDirectory);

        int totalCount = mImpl.mUnitTests.length + mImpl.mGMs.length * mImpl.mBackends.length;
        mTests = new Description[totalCount];
        mShouldSkipTest = new boolean[totalCount]; // = {false, false, ....};
        int index = 0;
        for (int backend = 0; backend < mImpl.mBackends.length; backend++) {
            for (int gm = 0; gm < mImpl.mGMs.length; gm++) {
                mTests[index++] = Description.createTestDescription(SkQPRunner.class,
                    mImpl.mBackends[backend] + "_" + mImpl.mGMs[gm]);
            }
        }
        for (int unitTest = 0; unitTest < mImpl.mUnitTests.length; unitTest++) {
            mTests[index++] = Description.createTestDescription(SkQPRunner.class,
                    "unitTest_" + mImpl.mUnitTests[unitTest]);
        }
        assert(index == totalCount);
        this.updateDescription(null);
    }

    private void updateDescription(Filter filter) {
        mShouldRunTestCount = 0;
        mDescription = Description.createSuiteDescription(SkQP.class);
        assert(mTests.length == mShouldSkipTest.length);
        for (int i = 0; i < mTests.length; ++i) {
            boolean doRunTest = filter != null ? filter.shouldRun(mTests[i]) : true;
            mShouldSkipTest[i] = !doRunTest;
            if (doRunTest) {
                mDescription.addChild(mTests[i]);
                ++mShouldRunTestCount;
            }
        }
    }

    @Override
    public void filter(Filter filter) throws NoTestsRemainException {
        this.updateDescription(filter);
        if (0 == mShouldRunTestCount) {
            throw new NoTestsRemainException();
        }
    }

    @Override
    public Description getDescription() {
        return mDescription;
    }

    @Override
    public void run(RunNotifier notifier) {
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
                    result = "ERROR";
                } else if (value != 0) {
                    SkQPRunner.Fail(desc, notifier, String.format(
                                "Image mismatch: max channel diff = %d", value));
                    Log.w(TAG, String.format("[FAIL] '%s': %d > 0", name, value));
                    result = "FAIL";
                }
                notifier.fireTestFinished(desc);
                Log.i(TAG, String.format("Rendering Test '%s' complete (%d/%d). [%s]",
                                         name, testNumber, mShouldRunTestCount, result));
            }
        }
        for (int unitTest = 0; unitTest < mImpl.mUnitTests.length; unitTest++, testIndex++) {
            Description desc = mTests[testIndex];
            String name = desc.getMethodName();
            if (mShouldSkipTest[testIndex]) {
                continue;
            }
            ++testNumber;
            notifier.fireTestStarted(desc);
            String[] errors = mImpl.nExecuteUnitTest(unitTest);
            String result = "pass";
            if (errors != null && errors.length > 0) {
                Log.w(TAG, String.format("[FAIL] Test '%s' had %d failures.", name, errors.length));
                for (String error : errors) {
                    SkQPRunner.Fail(desc, notifier, error);
                    Log.w(TAG, String.format("[FAIL] '%s': %s", name, error));
                }
                result = "FAIL";
            }
            notifier.fireTestFinished(desc);
            Log.i(TAG, String.format("Test '%s' complete (%d/%d). [%s]",
                                     name, testNumber, mShouldRunTestCount, result));
        }
        mImpl.nMakeReport();
        Log.i(TAG, String.format("output written to \"%s\"", mOutputDirectory));
    }
}
