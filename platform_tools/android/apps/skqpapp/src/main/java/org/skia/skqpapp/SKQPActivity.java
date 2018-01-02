package org.skia.skqpapp;

import android.content.Context;
import android.content.res.AssetManager;
import org.skia.skqp.SkQPException;
import org.skia.skqp.SkQPRunner;
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



public class SKQPActivity extends AppCompatActivity {
    private SkQPRunner testRunner = new SkQPRunner();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
       super.onCreate(savedInstanceState);
       setContentView(R.layout.activity_skqp);
       Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
       setSupportActionBar(toolbar);

       Intent launchIntent = getIntent();
       if (launchIntent.getAction().equals("com.google.intent.action.TEST_LOOP")) {
           this.runSuite();
       }

      // End the activity.
      finish();
    }

    private void runSuite() {
        // Note: /sdcard/Android/data/<package-name> is a location an app is allowed to write to.
        //       When running tests on Firebase it expects any result files to have a '/sdcard
        //       prefix or it won't trigger tests from the CLI.

        Context context = getApplicationContext();
        String outputDirPath = "/sdcard/Android/data/" + context.getPackageName();
        testRunner.runTests(context, outputDirPath);
    }
}

