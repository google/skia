package org.skia.cts18;

import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.View;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

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
            String outputDir = getFilesDir().getAbsolutePath();
//            Ctsdriver.load(knowledgeFile);

            Ctsdriver.load(knowledgeFile, outputDir);

//            String outputDir = Environment.getExternalStorageDirectory().getAbsolutePath() + "/skia-cts-testoutput";


            Log.i("ctslog", "OutputDir: " + outputDir);
            String[] testNames = Ctsdriver.testNames().split("|");

            for(String testName : testNames) {
                String output = testName + ":";

                ImageStub retImg = new ImageStub();
                output += GMRunner.runGM(testName, retImg);

                output += Ctsdriver.recordTestResult(testName, retImg.asGoImage());
                output += "OK";
                Log.i("ctslog", output);
            }
        }catch (Exception ex) {
            // Log error.
            Log.e("ctslog", "exception:", ex);
            return;
        }

        this.finish();
    }

    // copy the knowledge file to disk so the golang code can open it.
    private String copyKnowledgeFile() {
        String knowledgeFile =getCacheDir()+"/knowlege.zip";
        File f = new File(knowledgeFile);
        if (!f.exists()) try {

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

