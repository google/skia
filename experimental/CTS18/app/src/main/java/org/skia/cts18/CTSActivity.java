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
            Ctsdriver.load(knowledgeFile);
             String outputDir = getFilesDir().getAbsolutePath();

//            String outputDir = Environment.getExternalStorageDirectory().getAbsolutePath() + "/skia-cts-testoutput";


            Log.i("ctslog", "OutputDir: " + outputDir);
            String output = Ctsdriver.runTests(outputDir);
            Log.i("ctslog", output);
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

