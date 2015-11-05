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
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.lang.StringBuilder;

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

    private String getFlags() throws IOException {
        InputStream s = getInstrumentation().getTargetContext().getResources().getAssets().open("nanobench_flags.txt");
        BufferedReader r = new BufferedReader(new InputStreamReader(s));
        StringBuilder flags = new StringBuilder();
        String sep = System.getProperty("line.separator");
        String line;
        while ((line = r.readLine()) != null) {
            flags.append(line);
            flags.append(sep);
        }
        s.close();
        return flags.toString();
    }

    public void testVisualBench() throws InterruptedException, IOException {
        String pkg = getInstrumentation().getTargetContext().getPackageName();
        Intent intent = new Intent(getInstrumentation().getTargetContext(),
                                   VisualBenchActivity.class);
        String args = getFlags();
        intent.putExtra("cmdLineFlags", args);
        mActivity = launchActivityWithIntent(pkg, VisualBenchActivity.class, intent);

        assertNotNull("mActivity is null", mActivity);
        Thread.sleep(5000);
        while (!mActivity.isDestroyed()) {
            Log.d("skiatest", "Waiting for subprocess to finish.");
            Thread.sleep(1000);
        }
    }
}
