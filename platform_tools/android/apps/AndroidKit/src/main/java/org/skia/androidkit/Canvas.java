package org.skia.androidkit;

import android.graphics.Bitmap;
import android.graphics.Paint;
import android.util.Log;

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

        mBitmap = bitmap;
        mNativeCanvasWrapper = nCreateRaster(bitmap);
    }

    @Override
    protected void finalize() throws Throwable
    {
        nFinalize(mNativeCanvasWrapper, mBitmap);
    }

    public void drawRect(float left, float right, float top, float bottom, Paint paint) {
        nDrawRect(mNativeCanvasWrapper, left, right, top, bottom, paint);
    }

    private static native long nCreateRaster(Bitmap bitmap);
    private static native void nFinalize(long canvasWrapper, Bitmap bitmap);

    private static native void nDrawRect(long canvasWrapper,
                                         float left, float right, float top, float bottom, Paint p);

}
