/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package com.skia;

import android.app.Instrumentation;
import android.content.Intent;
import android.test.ActivityUnitTestCase;
import android.util.Log;

public class VisualBenchTestActivity extends ActivityUnitTestCase<VisualBenchActivity> {
    private VisualBenchActivity mActivity;
    private Instrumentation mInstrumentation;

    public VisualBenchTestActivity() {
        super(VisualBenchActivity.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
    }

    public void testVisualBench() throws InterruptedException {
        String pkg = getInstrumentation().getTargetContext().getPackageName();
        Intent intent = new Intent(getInstrumentation().getTargetContext(),
                                   VisualBenchActivity.class);
        intent.putExtra("cmdLineFlags", "");
        mActivity = launchActivityWithIntent(pkg, VisualBenchActivity.class, intent);

        assertNotNull("mActivity is null", mActivity);
        Thread.sleep(5000);
        while (!mActivity.isDestroyed()) {
            Log.d("skiatest", "Waiting for subprocess to finish.");
            Thread.sleep(1000);
        }
    }
}
