/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.jnibindings;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import org.skia.SkiaJNI;
import org.skia.*;

public class MainActivity extends AppCompatActivity {

  // Used to load the 'native-lib' library on application startup.
  static {
    System.loadLibrary("skiajni");
    SkiaJNI.initialize();
  }

  // Storage Permissions
  private static final int REQUEST_EXTERNAL_STORAGE = 1;
  private static String[] PERMISSIONS_STORAGE = {
      Manifest.permission.READ_EXTERNAL_STORAGE,
      Manifest.permission.WRITE_EXTERNAL_STORAGE
  };


  /**
   * Checks if the app has permission to write to device storage
   *
   * If the app does not has permission then the user will be prompted to grant permissions
   *
   * @param activity
   */
  public static void verifyStoragePermissions(Activity activity) {
    // Check if we have write permission
    int permission = ActivityCompat.checkSelfPermission(activity, Manifest.permission.WRITE_EXTERNAL_STORAGE);

    if (permission != PackageManager.PERMISSION_GRANTED) {
      // We don't have permission so prompt the user
      ActivityCompat.requestPermissions(
          activity,
          PERMISSIONS_STORAGE,
          REQUEST_EXTERNAL_STORAGE
      );
    }
  }

  @Override
  public void onRequestPermissionsResult(int requestCode,
      String[] permissions, int[] grantResults) {
  }


  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);

    final Button button = findViewById(R.id.runTest);
    button.setOnClickListener(new View.OnClickListener() {
      public void onClick(View v) {
        onRunTestPressed();
      }
    });

    final Button generateButton = findViewById(R.id.generate);
    generateButton.setOnClickListener(new View.OnClickListener() {
      public void onClick(View v) {
        onGeneratePressed();
      }
    });

    verifyStoragePermissions(this);
  }

  public void onRunTestPressed() {
      // runs the test, which renders in a PNG file. Result can be downloaded from the phone with
      // "adb pull /sdcard/skia-java-example.png"
      runTest();
  }

  public void onGeneratePressed() {
    // generate JNI bindings in /sdcard/jni -> you can pull them with "adb pull /sdcard/jni"

    File folder = new File(Environment.getExternalStorageDirectory() +
        File.separator + "jni");
    boolean success = true;
    if (!folder.exists()) {
      success = folder.mkdirs();
    }
    if (!success) {
      return;
    }

    SkiaJNI.generateJNI(folder.getPath());
  }

  public SkSurface makeSurface(int w, int h) {
    SkImageInfo info = new SkImageInfo(w, h, SkColorType.RGBA_8888_SK_COLORTYPE,
        SkAlphaType.PREMUL_SK_ALPHATYPE, null);
    //SkSurfaceProps props = new SkSurfaceProps(SkPixelGeometry.UNKNOWN_SK_PIXELGEOMETRY);
    return SkSurface.newRaster(info, null);
  }

  public void draw(SkCanvas canvas) {
    SkPaint fill = new SkPaint();
    fill.setColor(SkColor.argb(0xFF, 0x00, 0x00, 0xFF));
    canvas.drawPaint(fill);

    fill.setColor(SkColor.argb(0xFF, 0x00, 0xFF, 0xFF));
    SkRect rect = new SkRect(100.0f, 100.0f, 540.0f, 380.0f);
    canvas.drawRect(rect,fill);

    SkPaint stroke = new SkPaint();
    stroke.setColor(SkColor.argb(0xFF, 0xFF, 0x00, 0x0));
    stroke.setAntiAlias(true);
    stroke.setStroke(true);
    stroke.setStrokeWidth(5.0f);

    SkPath path = new SkPath();
    path.moveTo(50.0f, 50.0f);
    path.lineTo(590.0f, 50.0f);
    path.cubicTo( -490.0f, 50.0f, 1130.0f, 430.0f, 50.0f, 430.0f);
    path.lineTo(590.0f, 430.0f);
    canvas.drawPath(path, stroke);

    fill.setColor(SkColor.argb(0x80, 0x00, 0xFF, 0x00));
    SkRect rect2 = new SkRect(120.0f, 120.0f, 520.0f, 360.0f);
    canvas.drawOval(rect2, fill);
  }

  public void emit_png(String path, SkSurface surface) {
    SkImage image = surface.newImageSnapshot();
    SkData data = image.encode();
    FileOutputStream f = null;
    try {
      f = new FileOutputStream(new File(path));
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    }
    byte [] imgData = data.getData();
    try {
      f.write(imgData);
      f.flush();
      f.close();
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  public void runTest() {
    SkSurface surface = makeSurface(640, 480);
    SkCanvas canvas = surface.getCanvas();
    //SkMatrix m = new SkMatrix(new float[] {1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,8.0f,9.0f});
    //canvas.concat(m);
    draw(canvas);
    File sdDir = Environment.getExternalStorageDirectory();
    emit_png(sdDir.getPath() + "/skia-java-example.png", surface);
  }

}
