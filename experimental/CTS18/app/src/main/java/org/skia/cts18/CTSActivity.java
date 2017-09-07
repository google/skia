package org.skia.cts18;

import android.Manifest;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.View;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.regex.Pattern;

import ctsdriver.Ctsdriver;
import ctsdriver.Image;

import org.skia.cts18.GMRunner.ImageStub;

public class CTSActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_cts);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        Intent launchIntent = getIntent();
        if (launchIntent.getAction().equals("com.google.intent.action.TEST_LOOP")) {
            this.runCTSuite();
        }
    }

    private void runCTSuite() {
        // Get the path to the copy of the knowledge file.
        String knowledgeFile = copyKnowledgeFile();

        try {
            // Note: /sdcard/Android/data/<package-name> is a location an app is allowed to write to.
            //       When running tests on Firebase it expects any result files to have a '/sdcard
            //       prefix or it won't trigger tests from the CLI.
            String outputDir = "/sdcard/Android/data/" + getApplicationContext().getPackageName();

            // Load the knowledge base and set the output directory.
            Ctsdriver.load(knowledgeFile, outputDir);
            String[] availableTestNames = GMRunner.init();

            Log.i("ctslog", "OutputDir: " + outputDir);

            // Get the test names of the tests we wish to run.
            String concatNames = Ctsdriver.testNames();
            String[] testNames = concatNames.split(Pattern.quote("|"));

            // Run the tests for each test.
            for(String testName : testNames) {
                String output = testName + ":";

                ImageStub retImg = new ImageStub();
                String err = GMRunner.runGM(testName, retImg);
                output += err;

                // Record the test.
                Ctsdriver.recordTestResult(testName, retImg.asGoImage(), err);
                Log.i("ctslog", output);
            }
        }catch (Exception ex) {
            // Log error.
            Log.e("ctslog", "exception:", ex);
        } finally {
            String errMsg = Ctsdriver.finished();
            if (!errMsg.equals("")) {
                Log.e("ctslog", "error in finish:" + errMsg);
            }
            this.finish();
        }
    }

    // copy the knowledge file to disk so the golang code can open it.
    private String copyKnowledgeFile() {
        String knowledgeFile = getCacheDir()+"/knowlege.zip";
        File f = new File(knowledgeFile);
        try {
            InputStream is = getAssets().open("knowledge.zip");
            int size = is.available();
            byte[] buffer = new byte[size];
            is.read(buffer);
            is.close();

            FileOutputStream fos = new FileOutputStream(f);
            fos.write(buffer);
            fos.close();
        } catch (Exception e) { throw new RuntimeException(e); }
        return knowledgeFile;
    }
}

