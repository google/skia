// Copyright 2012 Google Inc. All Rights Reserved.

package com.skia;

import android.app.IntentService;
import android.content.Intent;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;

/**
 * @author borenet@google.com (Eric Boren)
 *
 */
public class SkiaIntentService extends IntentService {
  public SkiaIntentService() {
      super("SkiaIntentService");
  }

  @Override
  public IBinder onBind(Intent arg0) {
      return null;
  }
  @Override
  public void onCreate() {
      super.onCreate();
  }

  @Override
  public void onDestroy() {
      super.onDestroy();
  }

  @Override
  public void onHandleIntent(Intent intent) {

      // Extract command-line arguments
      Bundle bundle = intent.getExtras();

      // Number of times to repeat the SkiaReturnCode in the log.
      int returnRepeats = bundle.getInt("returnRepeats", 1);

      // We require at least the program name to be specified.
      if (!bundle.containsKey("args")) {
          Log.e("skia",
                "No command line arguments supplied.  Unable to continue.");
          SkiaReturn(-1, returnRepeats);
          return;
      }

      String cmd = bundle.getString("args").trim();
      String[] args = cmd.split("\\s+");
      Log.d("skia", "Executing Command: " + cmd);

      // Load the requested library
      String lib = args[0];
      try {
          System.loadLibrary("skia_android");
          System.loadLibrary(lib);
      } catch (UnsatisfiedLinkError e) {
          Log.e("skia", "Library " + lib +
                    " could not be linked!  Unable to continue.");
          SkiaReturn(-1, returnRepeats);
          throw e;
      }

      // JNI call to run the program
      int retval = run(args);
      SkiaReturn(retval, returnRepeats);
  }

  /**
   * Print out the exit code of the native program.  Skia's buildbots watch the
   * logcat output for this line.  The buildbots occasionally have to restart
   * a dead adb process, which causes them to miss some log output (Bug:
   * https://code.google.com/p/skia/issues/detail?id=809).  If this
   * "SKIA_RETURN_CODE" line is missed while adb is being restarted, then the
   * test may never finish.  Therefore, we print the line as many times as the
   * caller specifies, waiting one second in between.
   */
  private void SkiaReturn(int code, int repeats) {
      Log.d("skia", "SKIA_RETURN_CODE " + code);
      for (int i = 1; i < repeats; ++i) {
          try {
              Thread.sleep(1000);
          } catch (InterruptedException e) {
              return;
          }
          Log.d("skia", "SKIA_RETURN_CODE " + code);
      }
  }

  native int run(String[] args);
}
