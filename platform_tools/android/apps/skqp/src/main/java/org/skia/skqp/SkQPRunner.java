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
    private Description[] mUnitTestDesc;
    private Description mSuiteDesc;
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

        mUnitTestDesc = new Description[mImpl.mUnitTests.length];
        for (int index = 0; index < mImpl.mUnitTests.length; ++index) {
            mUnitTestDesc[index] = Description.createTestDescription(
                    SkQPRunner.class, "unitTest_" + mImpl.mUnitTests[index]);
        }

        this.applyFilter(null);
    }

    private void applyFilter(Filter filter) {
        mSuiteDesc = Description.createSuiteDescription(SkQP.class);
        addFilteredTestsToSuite(mUnitTestDesc, filter);
    }

    private void addFilteredTestsToSuite(Description[] tests, Filter filter) {
        for (int i = 0; i < tests.length; ++i) {
            if (filter == null || filter.shouldRun(tests[i])) {
                mSuiteDesc.addChild(tests[i]);
            } else {
                tests[i] = Description.EMPTY;
            }
        }
    }

    @Override
    public void filter(Filter filter) throws NoTestsRemainException {
        this.applyFilter(filter);
        if (mSuiteDesc.isEmpty()) {
            throw new NoTestsRemainException();
        }
    }

    @Override
    public Description getDescription() {
        return mSuiteDesc;
    }

    @Override
    public void run(RunNotifier notifier) {
        int testNumber = 0;  // out of number of actually run tests.
        for (int index = 0; index < mUnitTestDesc.length; index++) {
            Description desc = mUnitTestDesc[index];
            if (desc.isEmpty()) {
                // This test was filtered out and can be skipped.
                continue;
            }

            String name = desc.getMethodName();
            ++testNumber;
            notifier.fireTestStarted(desc);
            String[] errors = mImpl.nExecuteUnitTest(index);
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
                                     name, testNumber, mSuiteDesc.testCount(), result));
        }
        mImpl.nMakeReport();
        Log.i(TAG, String.format("output written to \"%s\"", mOutputDirectory));
    }
}
