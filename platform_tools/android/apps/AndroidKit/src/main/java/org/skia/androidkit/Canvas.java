package org.skia.androidkit;

import android.graphics.Bitmap;
import android.graphics.Paint;
import android.util.Log;

import java.lang.reflect.Method;

public class Canvas {
    private long mNativeCanvasWrapper;
    private Bitmap mBitmap;

    private static final String TAG = "ANDROIDKIT";

    static {
        Log.d(TAG, "loading lib");
        System.loadLibrary("androidkit");
    }
    public Canvas(Bitmap bitmap) {
        if (!bitmap.isMutable()) {
            throw new IllegalStateException("Immutable bitmap passed to Canvas constructor");
        }
        // --------- ugly class reflection to get the @hide method -----------
        Object bitmapProxy = null;
        try {
            Class c = Class.forName("android.graphics.Bitmap");
            Method m = c.getMethod("getNativeInstance");
            bitmapProxy = m.invoke(bitmap);
        } catch (Exception e) {
            e.printStackTrace();
            throw new RuntimeException(e);
        }
        mNativeCanvasWrapper = nInitRaster(Long.parseLong(bitmapProxy.toString()));
        // --------------------------------------------------------------------
        mBitmap = bitmap;
    }

    public void drawRect(float left, float right, float top, float bottom, Paint paint) {
        //nDrawRect(mNativeCanvasWrapper, left, right, top, bottom, paint.getNativeInstance());
    }

    private static native long nInitRaster(long bitmapHandle);
    private static native long nDrawRect(long canvasProxy, float left, float right, float top, float bottom, long paintProxy);

}
