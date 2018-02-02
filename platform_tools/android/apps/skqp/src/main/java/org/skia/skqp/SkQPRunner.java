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
import org.junit.runner.notification.RunNotifier;

@RunWith(SkQPRunner.class)
public class SkQPRunner extends Runner {
    private Description mDescription;
    private SkQP impl;

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
            Log.w(SkQP.TAG, "ensureEmtpyDirectory: " + e.getMessage());
        }
        Log.i(SkQP.TAG, "output path = " + filesDir.getAbsolutePath());

        Resources resources = InstrumentationRegistry.getTargetContext().getResources();
        AssetManager mAssetManager = resources.getAssets();
        impl.nInit(mAssetManager, filesDir.getAbsolutePath(), false);

        mDescription = Description.createSuiteDescription(testClass);
        for (int backend = 0; backend < impl.mBackends.length; backend++) {
            String classname = SkQP.kSkiaGM + impl.mBackends[backend];
            for (int gm = 0; gm < impl.mGMs.length; gm++) {
                mDescription.addChild(
                        Description.createTestDescription(classname, impl.mGMs[gm]));
            }
        }
        for (int unitTest = 0; unitTest < impl.mUnitTests.length; unitTest++) {
            mDescription.addChild(Description.createTestDescription(SkQP.kSkiaUnitTests,
                        impl.mUnitTests[unitTest]));
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
        impl.run(notifier);
        Log.i(SkQP.TAG, String.format("output written to \"%s\"", GetOutputDir().getAbsolutePath()));
    }
}
