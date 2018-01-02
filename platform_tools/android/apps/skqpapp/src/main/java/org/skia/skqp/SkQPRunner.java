package org.skia.skqp;

import android.content.Context;
import android.content.res.AssetManager;
import org.skia.skqp.SkQPException;
import java.io.IOException;


import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.regex.Pattern;



public class SkQPRunner {
    private native void nInit(AssetManager assetManager, String dataDir);
    private native float nExecuteGM(int gm, int backend) throws SkQPException;
    private native String[] nExecuteUnitTest(int test);
    private native void nMakeReport();

    private String[] mGMs;
    private String[] mBackends;
    private String[] mUnitTests;

    private static final String kSkiaGM = "SkiaGM_";
    private static final String kSkiaUnitTests = "Skia_UnitTests";
    private static final String LOG_PREFIX = "org.skis.skqp";

    static {
      System.loadLibrary("skqp_app");
    }

    public void runTests(Context context, String outputDirPath) {
        Log.w(LOG_PREFIX, "Output Dir: " + outputDirPath);
        File outputDir = new File(outputDirPath);
        if (outputDir.exists()) {
          try {
              deleteDirectoryContents(outputDir);
          } catch (IOException e) {
              Log.w(LOG_PREFIX, "DeleteDirectoryContents: " + e.getMessage());
          }
        }

        // Note: nInit will initialize the mGMs, mBackends and mUnitTests fields.
        AssetManager assetManager = context.getResources().getAssets();
        this.nInit(assetManager, outputDirPath);

        for (int backend = 0; backend < mBackends.length; backend++) {
          String classname = kSkiaGM + mBackends[backend];
          for (int gm = 0; gm < mGMs.length; gm++) {
              String testName = kSkiaGM + mBackends[backend] + "_" +mGMs[gm];
              float value = java.lang.Float.MAX_VALUE;
              String error = null;
              Log.w(LOG_PREFIX, "Running: " + testName);
              try {
                  value = this.nExecuteGM(gm, backend);
              } catch (SkQPException exept) {
                  error = exept.getMessage();
              }
              if (error != null) {
                // Record error message and carry on.
              } else if (value != 0) {
                // Record failure and carry on.
                  // SkQPRunner.Fail(desc, notifier, String.format(
                  //             "Image mismatch: max channel diff = %f", value));
              } else {
                // Record success for this test.
              }
          }
      }
      for (int unitTest = 0; unitTest < mUnitTests.length; unitTest++) {
          String testName = kSkiaUnitTests + "_" + mUnitTests[unitTest];
          Log.w(LOG_PREFIX, "Running: " + testName);
          String[] errors = this.nExecuteUnitTest(unitTest);
          if (errors != null && errors.length > 0) {
              for (String error : errors) {
                // Record unit test failures.
              }
          } else {
            // Record success.
          }
      }
      nMakeReport();
    }

    private static void deleteDirectoryContents(File f) throws IOException {
      for (File s : f.listFiles()) {
          if (s.isDirectory()) {
              deleteDirectoryContents(s);
          }
          s.delete();
      }
    }
}

