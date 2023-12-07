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
import android.graphics.RuntimeShader;
import android.util.Log;
import androidx.test.InstrumentationRegistry;
import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.regex.Pattern;
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
    private Description[] mSkSLErrorTestDesc;
    private Description mSuiteDesc;
    private String mOutputDirectory;
    private SkQP mImpl;
    private static final String TAG = SkQP.LOG_PREFIX;

    private interface TestExecutor {
        public int numTests();
        public String name(int index);
        public Description desc(int index);
        public boolean run(RunNotifier notifier, int index);
    }

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
        for (int index = 0; index < mUnitTestDesc.length; ++index) {
            mUnitTestDesc[index] = Description.createTestDescription(
                    SkQPRunner.class, "UnitTest_" + mImpl.mUnitTests[index]);
        }

        mSkSLErrorTestDesc = new Description[mImpl.mSkSLErrorTestName.length];
        for (int index = 0; index < mSkSLErrorTestDesc.length; ++index) {
            mSkSLErrorTestDesc[index] = Description.createTestDescription(
                    SkQPRunner.class, "SkSLErrorTest_" + mImpl.mSkSLErrorTestName[index]);
        }

        this.applyFilter(null);
    }

    private void applyFilter(Filter filter) {
        mSuiteDesc = Description.createSuiteDescription(SkQP.class);
        addFilteredTestsToSuite(mUnitTestDesc, filter);
        addFilteredTestsToSuite(mSkSLErrorTestDesc, filter);
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
        int testNumber = 0;
        testNumber = runTests(notifier, new SkSLErrorTestExecutor(), testNumber);
        testNumber = runTests(notifier, new UnitTestExecutor(), testNumber);

        mImpl.nMakeReport();
        Log.i(TAG, String.format("output written to \"%s\"", mOutputDirectory));
    }

    private int runTests(RunNotifier notifier, TestExecutor executor, int testNumber) {
        for (int index = 0; index < executor.numTests(); index++) {
            Description desc = executor.desc(index);
            if (desc.isEmpty()) {
                // This test was filtered out and can be skipped.
                continue;
            }

            ++testNumber;
            notifier.fireTestStarted(desc);

            String result = executor.run(notifier, index) ? "pass" : "FAIL";

            notifier.fireTestFinished(desc);
            Log.i(TAG, String.format("Test '%s' complete (%d/%d). [%s]", executor.name(index),
                                     testNumber, mSuiteDesc.testCount(), result));
        }
        return testNumber;
    }

    class UnitTestExecutor implements TestExecutor {
        public int numTests() {
            return mUnitTestDesc.length;
        }
        public String name(int index) {
            return desc(index).getMethodName();
        }
        public Description desc(int index) {
            return mUnitTestDesc[index];
        }
        public boolean run(RunNotifier notifier, int index) {
            String[] errors = mImpl.nExecuteUnitTest(index);
            if (errors != null && errors.length > 0) {
                Log.w(TAG, String.format("[FAIL] Test '%s' had %d failures.",
                                         name(index), errors.length));
                for (String error : errors) {
                    SkQPRunner.Fail(desc(index), notifier, error);
                    Log.w(TAG, String.format("[FAIL] '%s': %s", name(index), error));
                }
                return false;
            }

            return true;
        }
    }

    class SkSLErrorTestExecutor implements TestExecutor {
        public int numTests() {
            return mSkSLErrorTestDesc.length;
        }
        public String name(int index) {
            return mImpl.mSkSLErrorTestName[index];
        }
        public Description desc(int index) {
            return mSkSLErrorTestDesc[index];
        }
        public boolean run(RunNotifier notifier, int index) {
            String shaderText = mImpl.mSkSLErrorTestShader[index];
            try {
                new RuntimeShader(shaderText);
                // Because this is an error test, we expected an exception to be thrown.
                // If we reach this point, no exception occurred; report this as an error.
                SkQPRunner.Fail(desc(index), notifier, "Shader did not generate any errors.");
                Log.w(TAG, String.format("[FAIL] '%s': Shader did not generate any errors",
                                         name(index)));
                return false;
            }
            catch (Exception ex) { } // Assume any exception is an expected SkSL error.
            return true;
        }
    }
}
