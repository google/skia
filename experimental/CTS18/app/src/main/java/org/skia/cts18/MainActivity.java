package org.skia.cts18;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void startTests(View view) {
        Intent intent = new Intent("com.google.intent.action.TEST_LOOP");
        intent.setDataAndType(null, "application/javascript");

//        Intent intent =
//                new Intent(CTSActivity.class)
//                        .setPackage(packageName)
//                        .putExtra("scenario", scenario)
//                        .setDataAndType(fileUri, "application/javascript")
//                        .setFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);

        startActivity(intent);
    }
}
