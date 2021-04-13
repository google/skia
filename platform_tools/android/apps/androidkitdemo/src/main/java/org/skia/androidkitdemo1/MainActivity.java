/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkitdemo1;

import android.app.Activity;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;
import org.skia.androidkit.Canvas;
import org.skia.androidkit.Color;
import org.skia.androidkit.Paint;
import org.skia.androidkit.Surface;

public class MainActivity extends Activity {
    static {
        System.loadLibrary("androidkit");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Bitmap.Config conf = Bitmap.Config.ARGB_8888;
        Bitmap bmp = Bitmap.createBitmap(200, 200, conf);
        Surface surface = new Surface(bmp);
        Paint p = new Paint();
        p.setColor(new Color(0, 1, 0, 1));
        surface.getCanvas().drawRect(0, 0, 100, 100, p);
        ImageView image = findViewById(R.id.image);
        image.setImageBitmap(bmp);
    }
}
