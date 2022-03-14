package org.skia.skqp;

import android.content.Context;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import java.io.File;
import java.io.IOException;

public class SkQPActivity extends AppCompatActivity implements Runnable {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
       super.onCreate(savedInstanceState);
       setContentView(R.layout.activity_skqp);
       Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
       setSupportActionBar(toolbar);

       // Start the tests.
       run();
    }

    // run implements the Runnable interface.
    public void run() {
        // Note: /sdcard/Android/data/<package-name> is a location an app is allowed to write to.
        //       When running tests on Firebase it expects any result files to have a '/sdcard
        //       prefix or it won't trigger tests from the CLI.

        Context context = getApplicationContext();
        String outputDirPath = "/sdcard/Android/data/" + context.getPackageName();
        SkQP impl = new SkQP();

        Log.i(SkQP.LOG_PREFIX, "Output Dir: " + outputDirPath);
        File outputDir = new File(outputDirPath);
        try {
            SkQPActivity.ensureEmptyDirectory(outputDir);
        } catch (IOException e) {
            Log.e(SkQP.LOG_PREFIX, "ensureEmptyDirectory:" + e.getMessage());
        }

        // Note: nInit will initialize the mUnitTests field.
        AssetManager assetManager = context.getResources().getAssets();
        impl.nInit(assetManager, outputDirPath);

        for (int unitTest = 0; unitTest < impl.mUnitTests.length; unitTest++) {
            String testName = SkQP.kSkiaUnitTests + "_" + impl.mUnitTests[unitTest];
            Log.w(SkQP.LOG_PREFIX, "Running: " + testName);
            String[] errors = impl.nExecuteUnitTest(unitTest);
            if (errors != null && errors.length > 0) {
                for (String error : errors) {
                  Log.w(SkQP.LOG_PREFIX, "Error running " + testName + ":" + error);
                }
            } else {
              Log.i(SkQP.LOG_PREFIX, "Test: " + testName + " finished successfully.");
            }
        }
        Log.i(SkQP.LOG_PREFIX, "Finished running all tests.");
        impl.nMakeReport();

        finish();
    }

    private static void ensureEmptyDirectory(File f) throws IOException {
        if (f.exists()) {
            SkQPActivity.delete(f);
        }
        if (!f.mkdirs()) {
            throw new IOException("Unable to create directory:" + f.getAbsolutePath());
        }
    }

    private static void delete(File f) throws IOException {
        if (f.isDirectory()) {
            for (File s : f.listFiles()) {
                SkQPActivity.delete(s);
            }
        }
        if (!f.delete()) {
            throw new IOException("Unable to delete:" + f.getAbsolutePath());
        }
    }
}

