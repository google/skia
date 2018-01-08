package org.skia.skqp;

import android.content.Context;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;

public class SkQPActivity extends AppCompatActivity implements Runnable {
    private SkQP testRunner = new SkQP();

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
        testRunner.runTests(context, outputDirPath);
        finish();
    }
}

