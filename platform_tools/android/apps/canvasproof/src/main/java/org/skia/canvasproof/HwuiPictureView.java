/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.canvasproof;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Picture;
import android.util.Log;
import android.view.View;
import java.io.IOException;
import java.io.InputStream;

public class HwuiPictureView extends View {
    private static final String TAG = "HwuiPictureView";
    private Picture picture;
    private float scale;
    private Picture defaultPicture;

    public boolean fullTime;

    HwuiPictureView(Context context) {
        super(context);
        this.scale = 1.0f;
    }
    public void setScale(float s) {
        this.scale = s;
    }
    public void setPicture(Picture p) {
        this.picture = p;
        this.invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (this.picture != null) {
            canvas.save();
            canvas.scale(scale, scale);
            HwuiPictureView.draw(canvas, this.picture);
            canvas.restore();
            if (fullTime) {
                this.invalidate();
            }
        }
    }

    static private void draw(Canvas canvas, Picture p) {
        if (android.os.Build.VERSION.SDK_INT > 22) {
            try {
                canvas.drawPicture(p);
                return;
            } catch (java.lang.Exception e) {
                Log.e(TAG, "Exception while drawing picture in Hwui");
            }
        }
        if (p.getWidth() > 0 && p.getHeight() > 0) {
            // Fallback to software rendering.
            Bitmap bm = Bitmap.createBitmap(p.getWidth(), p.getHeight(),
                                            Bitmap.Config.ARGB_8888);
            (new Canvas(bm)).drawPicture(p);
            canvas.drawBitmap(bm, 0.0f, 0.0f, null);
            bm.recycle();
        }
    }
}
